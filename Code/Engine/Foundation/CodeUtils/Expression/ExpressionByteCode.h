#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Containers/DynamicArray.h>

class ezStreamWriter;
class ezStreamReader;

class EZ_FOUNDATION_DLL ezExpressionByteCode
{
public:
  struct OpCode
  {
    enum Enum
    {
      Nop,

      // Unary
      FirstUnary,

      AbsF_R,
      AbsI_R,
      SqrtF_R,

      SinF_R,
      CosF_R,
      TanF_R,

      ASinF_R,
      ACosF_R,
      ATanF_R,

      IToF_R,
      FToI_R,

      LastUnary,

      // Binary
      FirstBinary,

      AddF_RR,
      AddI_RR,

      SubF_RR,
      SubI_RR,

      MulF_RR,
      MulI_RR,

      DivF_RR,
      DivI_RR,

      MinF_RR,
      MinI_RR,

      MaxF_RR,
      MaxI_RR,

      LastBinary,

      FirstBinaryWithConstant,

      AddF_CR,
      AddI_CR,

      SubF_CR,
      SubI_CR,

      MulF_CR,
      MulI_CR,

      DivF_CR,
      DivI_CR,

      MinF_CR,
      MinI_CR,

      MaxF_CR,
      MaxI_CR,

      LastBinaryWithConstant,

      MovX_R,
      MovX_C,
      LoadF,
      LoadI,
      StoreF,
      StoreI,

      Call,

      Count
    };
  };

  using StorageType = ezUInt32;

  ezExpressionByteCode();
  ~ezExpressionByteCode();

  bool operator==(const ezExpressionByteCode& other) const;
  bool operator!=(const ezExpressionByteCode& other) const { return !(*this == other); }

  void Clear();
  bool IsEmpty() const { return m_ByteCode.IsEmpty(); }

  const StorageType* GetByteCode() const;
  const StorageType* GetByteCodeEnd() const;

  ezUInt32 GetNumInstructions() const;
  ezUInt32 GetNumTempRegisters() const;
  ezArrayPtr<const ezExpression::StreamDesc> GetInputs() const;
  ezArrayPtr<const ezExpression::StreamDesc> GetOutputs() const;
  ezArrayPtr<const ezExpression::FunctionDesc> GetFunctions() const;

  static OpCode::Enum GetOpCode(const StorageType*& pByteCode);
  static ezUInt32 GetRegisterIndex(const StorageType*& pByteCode);
  static ezExpression::Register GetConstant(const StorageType*& pByteCode);
  static ezUInt32 GetFunctionIndex(const StorageType*& pByteCode);
  static ezUInt32 GetFunctionArgCount(const StorageType*& pByteCode);

  void Disassemble(ezStringBuilder& out_sDisassembly) const;
  static const char* GetOpCodeName(OpCode::Enum opCode);

  void Save(ezStreamWriter& stream) const;
  ezResult Load(ezStreamReader& stream);

private:
  friend class ezExpressionCompiler;

  ezDynamicArray<StorageType> m_ByteCode;
  ezDynamicArray<ezExpression::StreamDesc> m_Inputs;
  ezDynamicArray<ezExpression::StreamDesc> m_Outputs;
  ezDynamicArray<ezExpression::FunctionDesc> m_Functions;

  ezUInt32 m_uiNumInstructions = 0;
  ezUInt32 m_uiNumTempRegisters = 0;
};

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionByteCode_inl.h>
