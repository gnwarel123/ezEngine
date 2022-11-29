#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Containers/DynamicArray.h>

class ezExpressionByteCode;

class EZ_FOUNDATION_DLL ezExpressionVM
{
public:
  ezExpressionVM();
  ~ezExpressionVM();

  void RegisterFunction(const ezExpressionFunction& func);

  ezResult Execute(const ezExpressionByteCode& byteCode, ezArrayPtr<const ezProcessingStream> inputs, ezArrayPtr<ezProcessingStream> outputs, ezUInt32 uiNumInstances, const ezExpression::GlobalData& globalData = ezExpression::GlobalData());

private:
  void RegisterDefaultFunctions();

  ezResult MapStreams(ezArrayPtr<const ezExpression::StreamDesc> streamDescs, ezArrayPtr<const ezProcessingStream> streams, const char* szStreamType, ezUInt32 uiNumInstances, ezDynamicArray<ezUInt32>& out_Mapping);
  ezResult MapFunctions(ezArrayPtr<const ezExpression::FunctionDesc> functionDescs, const ezExpression::GlobalData& globalData);

  ezResult ExecuteUnaryOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances);
  ezResult ExecuteBinaryOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances);
  ezResult ExecuteBinaryWithConstantOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezUInt32 uiNumSimd4Instances);
  ezResult ExecuteSpecialOp(const ezExpressionByteCode::StorageType*& pByteCode, ezExpressionByteCode::OpCode::Enum opCode, ezArrayPtr<const ezProcessingStream> inputs, ezArrayPtr<ezProcessingStream> outputs, const ezExpression::GlobalData& globalData, ezUInt32 uiNumInstances, ezUInt32 uiNumSimd4Instances);

  ezDynamicArray<ezExpression::Register, ezAlignedAllocatorWrapper> m_Registers;

  ezDynamicArray<ezUInt32> m_InputMapping;
  ezDynamicArray<ezUInt32> m_OutputMapping;
  ezDynamicArray<ezUInt32> m_FunctionMapping;

  ezDynamicArray<ezExpressionFunction> m_Functions;
  ezHashTable<ezHashedString, ezUInt32> m_FunctionNamesToIndex;
};
