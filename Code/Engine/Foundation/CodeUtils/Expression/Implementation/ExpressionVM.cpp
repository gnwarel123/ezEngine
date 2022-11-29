#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/SimdMath/SimdMath.h>
#include <Foundation/Math/Float16.h>

// #define DEBUG_VM

namespace
{
  template <typename TargetType, typename SourceType>
  EZ_ALWAYS_INLINE TargetType ReadInputData(const ezUInt8*& pData, ezUInt32 uiStride)
  {
    SourceType value = *reinterpret_cast<const SourceType*>(pData);
    pData += uiStride;
    return value;
  }

  template <typename RegisterType, typename ValueType, typename InputType>
  void LoadInput(RegisterType* r, RegisterType* re, const ezProcessingStream& input, ezUInt32 uiNumRemainderInstances)
  {
    const ezUInt8* pInputData = input.GetData<ezUInt8>();
    const ezUInt32 uiByteStride = input.GetElementStride();
    
    if (uiByteStride == sizeof(ValueType) && std::is_same<ValueType, InputType>::value)
    {
      while (r != re)
      {
        r->Load<4>(reinterpret_cast<const ValueType*>(pInputData));

        ++r;
        pInputData += sizeof(ValueType) * 4;
      }
    }
    else
    {
      ValueType x[4] = {};
      while (r != re)
      {
        x[0] = ReadInputData<ValueType, InputType>(pInputData, uiByteStride);
        x[1] = ReadInputData<ValueType, InputType>(pInputData, uiByteStride);
        x[2] = ReadInputData<ValueType, InputType>(pInputData, uiByteStride);
        x[3] = ReadInputData<ValueType, InputType>(pInputData, uiByteStride);

        r->Load<4>(x);

        ++r;
      }
    }

    if (uiNumRemainderInstances > 0)
    {
      ValueType x[3];
      x[0] = uiNumRemainderInstances == 1 ? ReadInputData<ValueType, InputType>(pInputData, uiByteStride) : 0;
      x[1] = uiNumRemainderInstances == 2 ? ReadInputData<ValueType, InputType>(pInputData, uiByteStride) : 0;
      x[2] = uiNumRemainderInstances == 3 ? ReadInputData<ValueType, InputType>(pInputData, uiByteStride) : 0;

      r->Set(x[0],x[1], x[2], 0);
    }
  }
}

#if 0

  VM_INLINE void StoreOutputData(ezUInt8* pData, float fData)
  {
    *reinterpret_cast<float*>(pData) = fData;
  }

  void VMStoreOutput(const ezExpressionByteCode::StorageType*& pByteCode, ezSimdVec4f* pRegisters, ezUInt32 uiNumRegisters,
    ezArrayPtr<ezProcessingStream> outputs, ezArrayPtr<ezUInt32> outputMapping)
  {
    ezUInt32 uiOutputIndex = ezExpressionByteCode::GetRegisterIndex(pByteCode, 1);
    uiOutputIndex = outputMapping[uiOutputIndex];
    auto& output = outputs[uiOutputIndex];
    ezUInt32 uiByteStride = output.GetElementStride();
    ezUInt8* pOutputData = output.GetWritableData<ezUInt8>();
    ezUInt8* pOutputDataEnd = pOutputData + output.GetDataSize() - uiByteStride;

    ezSimdVec4f* r = pRegisters + ezExpressionByteCode::GetRegisterIndex(pByteCode, uiNumRegisters);
    ezSimdVec4f* re = r + uiNumRegisters;

    while (r != re)
    {
      float data[4];
      r->Store<4>(data);

      StoreOutputData(pOutputData, data[0]);
      pOutputData += pOutputData < pOutputDataEnd ? uiByteStride : 0;
      StoreOutputData(pOutputData, data[1]);
      pOutputData += pOutputData < pOutputDataEnd ? uiByteStride : 0;
      StoreOutputData(pOutputData, data[2]);
      pOutputData += pOutputData < pOutputDataEnd ? uiByteStride : 0;
      StoreOutputData(pOutputData, data[3]);
      pOutputData += pOutputData < pOutputDataEnd ? uiByteStride : 0;

      ++r;
    }
  }

  void VMCall(const ezExpressionByteCode::StorageType*& pByteCode, ezSimdVec4f* pRegisters, ezUInt32 uiNumRegisters,
    const ezExpression::GlobalData& globalData, ezExpressionFunction& func)
  {
    ezSimdVec4f* r = pRegisters + ezExpressionByteCode::GetRegisterIndex(pByteCode, uiNumRegisters);
    ezUInt32 uiNumArgs = ezExpressionByteCode::GetFunctionArgCount(pByteCode);

    ezHybridArray<ezArrayPtr<const ezSimdVec4f>, 32> inputs;
    inputs.Reserve(uiNumArgs);
    for (ezUInt32 uiArgIndex = 0; uiArgIndex < uiNumArgs; ++uiArgIndex)
    {
      ezSimdVec4f* x = pRegisters + ezExpressionByteCode::GetRegisterIndex(pByteCode, uiNumRegisters);
      inputs.PushBack(ezMakeArrayPtr(x, uiNumRegisters));
    }

    ezExpression::Output output = ezMakeArrayPtr(r, uiNumRegisters);

    func(inputs, output, globalData);
  }
} // namespace
#endif

//////////////////////////////////////////////////////////////////////////

ezExpressionVM::ezExpressionVM() = default;
ezExpressionVM::~ezExpressionVM() = default;

void ezExpressionVM::RegisterFunction(const ezExpressionFunction& func)
{
  ezUInt32 uiFunctionIndex = m_Functions.GetCount();
  m_FunctionNamesToIndex.Insert(func.m_Desc.m_sName, uiFunctionIndex);

  m_Functions.PushBack(func);
}

ezResult ezExpressionVM::Execute(const ezExpressionByteCode& byteCode, ezArrayPtr<const ezProcessingStream> inputs,
  ezArrayPtr<ezProcessingStream> outputs, ezUInt32 uiNumInstances, const ezExpression::GlobalData& globalData)
{
  EZ_SUCCEED_OR_RETURN(MapStreams(byteCode.GetInputs(), inputs, "Input", uiNumInstances, m_InputMapping));
  EZ_SUCCEED_OR_RETURN(MapStreams(byteCode.GetOutputs(), outputs, "Output", uiNumInstances, m_OutputMapping));
  EZ_SUCCEED_OR_RETURN(MapFunctions(byteCode.GetFunctions(), globalData));

  //const ezUInt32 uiNumSimd16Instances = uiNumInstances / 16;
  const ezUInt32 uiNumSimd4Instances = (uiNumInstances + 3) / 4;

  const ezUInt32 uiTotalNumRegisters = byteCode.GetNumTempRegisters() * ((uiNumInstances + 3) / 4);
  m_Registers.SetCountUninitialized(uiTotalNumRegisters);

  // Execute bytecode
  const ezExpressionByteCode::StorageType* pByteCode = byteCode.GetByteCode();
  const ezExpressionByteCode::StorageType* pByteCodeEnd = byteCode.GetByteCodeEnd();

#ifdef DEBUG_VM
  ezUInt32 uiInstructionIndex = 0;
#endif

  while (pByteCode < pByteCodeEnd)
  {
    ezExpressionByteCode::OpCode::Enum opCode = ezExpressionByteCode::GetOpCode(pByteCode);

#ifdef DEBUG_VM
    ezLog::Info("{}: {}", uiInstructionIndex, ezExpressionByteCode::GetOpCodeName(opCode));
    uiInstructionIndex++;
#endif

    if (opCode > ezExpressionByteCode::OpCode::FirstUnary && opCode < ezExpressionByteCode::OpCode::LastUnary)
    {
      EZ_SUCCEED_OR_RETURN(ExecuteUnaryOp(pByteCode, opCode, uiNumSimd4Instances));
    }
    else if (opCode > ezExpressionByteCode::OpCode::FirstBinary && opCode < ezExpressionByteCode::OpCode::LastBinary)
    {
      EZ_SUCCEED_OR_RETURN(ExecuteBinaryOp(pByteCode, opCode, uiNumSimd4Instances));
    }
    else if (opCode > ezExpressionByteCode::OpCode::FirstBinaryWithConstant && opCode < ezExpressionByteCode::OpCode::LastBinaryWithConstant)
    {
      EZ_SUCCEED_OR_RETURN(ExecuteBinaryWithConstantOp(pByteCode, opCode, uiNumSimd4Instances));
    }
    else
    {
      EZ_SUCCEED_OR_RETURN(ExecuteSpecialOp(pByteCode, opCode, inputs, outputs, globalData, uiNumInstances, uiNumSimd4Instances));
    }

#if 0
    switch (opCode)
    {
        // unary

      case ezExpressionByteCode::OpCode::Sqrt_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return x.GetSqrt(); });
        break;

      case ezExpressionByteCode::OpCode::Sin_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::Sin(x); });
        break;

      case ezExpressionByteCode::OpCode::Cos_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::Cos(x); });
        break;

      case ezExpressionByteCode::OpCode::Tan_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::Tan(x); });
        break;

      case ezExpressionByteCode::OpCode::ASin_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::ASin(x); });
        break;

      case ezExpressionByteCode::OpCode::ACos_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::ACos(x); });
        break;

      case ezExpressionByteCode::OpCode::ATan_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return ezSimdMath::ATan(x); });
        break;

      case ezExpressionByteCode::OpCode::Mov_R:
        VMOperation1(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return x; });
        break;

      case ezExpressionByteCode::OpCode::Mov_C:
        VMOperation1_C(pByteCode, pRegisters, uiNumRegisters, [](const ezSimdVec4f& x)
          { return x; });
        break;

      case ezExpressionByteCode::OpCode::Load:
        VMLoadInput(pByteCode, pRegisters, uiNumRegisters, inputs, m_InputMapping);
        break;

      case ezExpressionByteCode::OpCode::Store:
        VMStoreOutput(pByteCode, pRegisters, uiNumRegisters, outputs, m_OutputMapping);
        break;     
     

        // call
      case ezExpressionByteCode::OpCode::Call:
      {
        ezUInt32 uiFunctionIndex = ezExpressionByteCode::GetFunctionIndex(pByteCode);
        uiFunctionIndex = m_FunctionMapping[uiFunctionIndex];
        auto& func = m_Functions[uiFunctionIndex].m_Func;

        VMCall(pByteCode, pRegisters, uiNumRegisters, globalData, func);
      }
      break;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return EZ_FAILURE;
    }
#endif
  }


  return EZ_SUCCESS;
}

void ezExpressionVM::RegisterDefaultFunctions()
{
  RegisterFunction(ezDefaultExpressionFunctions::s_RandomFunc);
  RegisterFunction(ezDefaultExpressionFunctions::s_PerlinNoiseFunc);
}

ezResult ezExpressionVM::MapStreams(ezArrayPtr<const ezExpression::StreamDesc> streamDescs, ezArrayPtr<const ezProcessingStream> streams, const char* szStreamType, ezUInt32 uiNumInstances, ezDynamicArray<ezUInt32>& out_Mapping)
{
  out_Mapping.Clear();
  out_Mapping.Reserve(streamDescs.GetCount());

  for (auto& streamDesc : streamDescs)
  {
    bool bFound = false;

    for (ezUInt32 i = 0; i < streams.GetCount(); ++i)
    {
      auto& stream = streams[i];
      if (stream.GetName() == streamDesc.m_sName)
      {
        // verify stream data type
        auto expectedDataType = ezExpressionAST::DataType::FromStreamType(streamDesc.m_DataType);
        auto actualDataType = ezExpressionAST::DataType::FromStreamType(stream.GetDataType());

        if (actualDataType != expectedDataType)
        {
          ezLog::Error("{} stream '{}' expects data of type '{}' or a compatible type. Given type '{}' is not compatible.", szStreamType, streamDesc.m_sName, ezExpressionAST::DataType::GetName(expectedDataType), ezProcessingStream::GetDataTypeName(stream.GetDataType()));
          return EZ_FAILURE;
        }

        // verify stream size
        ezUInt32 uiElementSize = stream.GetElementSize();
        ezUInt32 uiExpectedSize = stream.GetElementStride() * (uiNumInstances - 1) + uiElementSize;

        if (stream.GetDataSize() < uiExpectedSize)
        {
          ezLog::Error("{} stream '{}' data size must be {} bytes or more. Only {} bytes given", szStreamType, streamDesc.m_sName, uiExpectedSize, stream.GetDataSize());
          return EZ_FAILURE;
        }

        out_Mapping.PushBack(i);
        bFound = true;
        break;
      }
    }

    if (!bFound)
    {
      ezLog::Error("Bytecode expects an {} stream '{}'", szStreamType, streamDesc.m_sName);
      return EZ_FAILURE;
    }
  }

  return EZ_SUCCESS;
}

ezResult ezExpressionVM::MapFunctions(ezArrayPtr<const ezExpression::FunctionDesc> functionDescs, const ezExpression::GlobalData& globalData)
{
  m_FunctionMapping.Clear();
  m_FunctionMapping.Reserve(functionDescs.GetCount());

  for (auto& functionDesc : functionDescs)
  {
    ezUInt32 uiFunctionIndex = 0;
    if (!m_FunctionNamesToIndex.TryGetValue(functionDesc.m_sName, uiFunctionIndex))
    {
      ezLog::Error("Bytecode expects a function called '{0}' but it was not registered for this VM", functionDesc.m_sName);
      return EZ_FAILURE;
    }

    auto& registeredFunction = m_Functions[uiFunctionIndex];

    // verify signature
    if (functionDesc.m_InputTypes != registeredFunction.m_Desc.m_InputTypes || functionDesc.m_OutputType != registeredFunction.m_Desc.m_OutputType)
    {
      ezLog::Error("Signature for registered function '{}' does not match the expected signature from bytecode", functionDesc.m_sName);
      return EZ_FAILURE;
    }

    if (registeredFunction.m_ValidateGlobalDataFunc != nullptr)
    {
      if (registeredFunction.m_ValidateGlobalDataFunc(globalData).Failed())
      {
        ezLog::Error("Global data validation for function '{0}' failed.", functionDesc.m_sName);
        return EZ_FAILURE;
      }
    }

    m_FunctionMapping.PushBack(uiFunctionIndex);
  }

  return EZ_SUCCESS;
}

#define DEFINE_TARGET_REGISTER()                                                                                    \
  ezExpression::Register* r = pRegisters + ezExpressionByteCode::GetRegisterIndex(pByteCode) * uiNumSimd4Instances; \
  ezExpression::Register* re = r + uiNumSimd4Instances;

#define DEFINE_OP_REGISTER(name) \
  const ezExpression::Register* name = pRegisters + ezExpressionByteCode::GetRegisterIndex(pByteCode) * uiNumSimd4Instances;

#define DEFINE_CONSTANT(name) \
  const ezExpression::Register name = ezExpressionByteCode::GetConstant(pByteCode);

#define BEGIN_OP_LOOP() \
  while (r != re)       \
  {

#define END_OP_LOOP_UNARY() \
  ++r;                      \
  ++a;                      \
  }                         \
  break;

#define END_OP_LOOP_UNARY_C() \
  ++r;                        \
  }                           \
  break;

#define END_OP_LOOP_BINARY() \
  ++r;                       \
  ++a;                       \
  ++b;                       \
  }                          \
  break;

#define END_OP_LOOP_BINARY_C() \
  ++r;                         \
  ++b;                         \
  }                            \
  break;

ezResult ezExpressionVM::ExecuteUnaryOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances)
{
  ezExpression::Register* pRegisters = m_Registers.GetData();

  DEFINE_TARGET_REGISTER();
  DEFINE_OP_REGISTER(a);

  switch (opCode)
  {
    case ezExpressionByteCode::OpCode::AbsF_R:
      BEGIN_OP_LOOP();
      r->f = a->f.Abs();
      END_OP_LOOP_UNARY();

    case ezExpressionByteCode::OpCode::AbsI_R:
      BEGIN_OP_LOOP();
      r->i = a->i.Abs();
      END_OP_LOOP_UNARY();

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezExpressionVM::ExecuteBinaryOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances)
{
  ezExpression::Register* pRegisters = m_Registers.GetData();

  DEFINE_TARGET_REGISTER();
  DEFINE_OP_REGISTER(a);
  DEFINE_OP_REGISTER(b);

  switch (opCode)
  {
    case ezExpressionByteCode::OpCode::AddF_RR:
      BEGIN_OP_LOOP();
      r->f = a->f + b->f;
      END_OP_LOOP_BINARY();

    case ezExpressionByteCode::OpCode::AddI_RR:
      BEGIN_OP_LOOP();
      r->i = a->i + b->i;
      END_OP_LOOP_BINARY();

    case ezExpressionByteCode::OpCode::SubI_RR:
      BEGIN_OP_LOOP();
      r->i = a->i - b->i;
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MulF_RR:
      BEGIN_OP_LOOP();
      r->f = a->f.CompMul(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MulI_RR:
      BEGIN_OP_LOOP();
      r->i = a->i.CompMul(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::DivF_RR:
      BEGIN_OP_LOOP();
      r->f = a->f.CompDiv(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::DivI_RR:
      BEGIN_OP_LOOP();
      EZ_ASSERT_NOT_IMPLEMENTED;
      //r->i = a->i.CompDiv(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MinF_RR:
      BEGIN_OP_LOOP();
      r->f = a->f.CompMin(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MinI_RR:
      BEGIN_OP_LOOP();
      r->i = a->i.CompMin(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MaxF_RR:
      BEGIN_OP_LOOP();
      r->f = a->f.CompMax(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MaxI_RR:
      BEGIN_OP_LOOP();
      r->i = a->i.CompMax(b->i);
      END_OP_LOOP_BINARY_C();

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}


ezResult ezExpressionVM::ExecuteBinaryWithConstantOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances)
{
  ezExpression::Register* pRegisters = m_Registers.GetData();

  DEFINE_TARGET_REGISTER();
  DEFINE_CONSTANT(a);
  DEFINE_OP_REGISTER(b);

  switch (opCode)
  {
    case ezExpressionByteCode::OpCode::AddF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f + b->f;
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::AddI_CR:
      BEGIN_OP_LOOP();
      r->i = a.i + b->i;
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::SubF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f - b->f;
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::SubI_CR:
      BEGIN_OP_LOOP();
      r->i = a.i - b->i;
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MulF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f.CompMul(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MulI_CR:
      BEGIN_OP_LOOP();
      r->i = a.i.CompMul(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::DivF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f.CompDiv(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::DivI_CR:
      BEGIN_OP_LOOP();
      EZ_ASSERT_NOT_IMPLEMENTED;
      //r->i = a.i.CompDiv(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MinF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f.CompMin(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MinI_CR:
      BEGIN_OP_LOOP();
      r->i = a.i.CompMin(b->i);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MaxF_CR:
      BEGIN_OP_LOOP();
      r->f = a.f.CompMax(b->f);
      END_OP_LOOP_BINARY_C();

    case ezExpressionByteCode::OpCode::MaxI_CR:
      BEGIN_OP_LOOP();
      r->i = a.i.CompMax(b->i);
      END_OP_LOOP_BINARY_C();

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezExpressionVM::ExecuteSpecialOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezArrayPtr<const ezProcessingStream> inputs, ezArrayPtr<ezProcessingStream> outputs, const ezExpression::GlobalData& globalData, ezUInt32 uiNumInstances, ezUInt32 uiNumSimd4Instances)
{
  ezExpression::Register* pRegisters = m_Registers.GetData();
  const ezUInt32 uiNumRemainderInstances = uiNumInstances & 0x3;

  switch (opCode)
  {
    case ezExpressionByteCode::OpCode::MovX_R:
    {
      DEFINE_TARGET_REGISTER();
      DEFINE_OP_REGISTER(a);
      BEGIN_OP_LOOP();
      r->i = a->i;
      END_OP_LOOP_UNARY();
    }

    case ezExpressionByteCode::OpCode::MovX_C:
    {
      DEFINE_TARGET_REGISTER();
      DEFINE_CONSTANT(a);
      BEGIN_OP_LOOP();
      r->i = a.i;
      END_OP_LOOP_UNARY_C();
    }

    case ezExpressionByteCode::OpCode::LoadF:
    {
      DEFINE_TARGET_REGISTER();
      if (uiNumRemainderInstances > 0)
        --re;

      const ezUInt32 uiInputIndex = m_InputMapping[ezExpressionByteCode::GetRegisterIndex(pByteCode)];
      auto& input = inputs[uiInputIndex];
      
      if (input.GetDataType() == ezProcessingStream::DataType::Float)
      {
        LoadInput<ezSimdVec4f, float, float>(reinterpret_cast<ezSimdVec4f*>(r), reinterpret_cast<ezSimdVec4f*>(re), input, uiNumRemainderInstances);
      }
      else
      {
        EZ_ASSERT_DEBUG(input.GetDataType() == ezProcessingStream::DataType::Half, "Unsupported input type '{}' for LoadF instruction", ezProcessingStream::GetDataTypeName(input.GetDataType()));        
        LoadInput<ezSimdVec4f, float, ezFloat16>(reinterpret_cast<ezSimdVec4f*>(r), reinterpret_cast<ezSimdVec4f*>(re), input, uiNumRemainderInstances);
      }

      break;
    }

    case ezExpressionByteCode::OpCode::LoadI:
    {
      DEFINE_TARGET_REGISTER();
      if (uiNumRemainderInstances > 0)
        --re;

      const ezUInt32 uiInputIndex = m_InputMapping[ezExpressionByteCode::GetRegisterIndex(pByteCode)];
      auto& input = inputs[uiInputIndex];

      if (input.GetDataType() == ezProcessingStream::DataType::Int)
      {
        LoadInput<ezSimdVec4i, int, int>(reinterpret_cast<ezSimdVec4i*>(r), reinterpret_cast<ezSimdVec4i*>(re), input, uiNumRemainderInstances);
      }
      else if (input.GetDataType() == ezProcessingStream::DataType::Short)
      {
        LoadInput<ezSimdVec4i, int, ezInt16>(reinterpret_cast<ezSimdVec4i*>(r), reinterpret_cast<ezSimdVec4i*>(re), input, uiNumRemainderInstances);
      }
      else
      {
        EZ_ASSERT_DEBUG(input.GetDataType() == ezProcessingStream::DataType::Byte, "Unsupported input type '{}' for LoadI instruction", ezProcessingStream::GetDataTypeName(input.GetDataType()));
        LoadInput<ezSimdVec4i, int, ezInt8>(reinterpret_cast<ezSimdVec4i*>(r), reinterpret_cast<ezSimdVec4i*>(re), input, uiNumRemainderInstances);
      }

      break;
    }

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

#undef DEFINE_TARGET_REGISTER
#undef DEFINE_OP_REGISTER
#undef DEFINE_CONSTANT
#undef BEGIN_OP_LOOP
#undef END_OP_LOOP_UNARY
#undef END_OP_LOOP_UNARY_C
#undef END_OP_LOOP_BINARY
#undef END_OP_LOOP_BINARY_C
