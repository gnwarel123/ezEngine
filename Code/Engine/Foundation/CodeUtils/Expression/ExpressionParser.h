#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>

class EZ_FOUNDATION_DLL ezExpressionParser
{
public:
  ezExpressionParser();
  ~ezExpressionParser();

  struct Stream
  {
    Stream(ezStringView sName, ezProcessingStream::DataType dataType)
      : m_DataType(dataType)
    {
      m_sName.Assign(sName);
    }

    ezHashedString m_sName;
    ezProcessingStream::DataType m_DataType;
  };

  struct Options
  {
    bool m_bTreatUnknownVariablesAsInputs = false;
  };

  ezResult Parse(ezStringView code, ezArrayPtr<Stream> inputs, ezArrayPtr<Stream> outputs, const Options& options, ezExpressionAST& out_ast);

private:
  static constexpr int s_iLowestPrecedence = 20;

  void RegisterKnownTypes();
  void RegisterBuiltinFunctions();
  void SetupInAndOutputs(ezArrayPtr<Stream> inputs, ezArrayPtr<Stream> outputs);

  ezResult ParseStatement();
  ezResult ParseType(ezStringView sTypeName, ezEnum<ezExpressionAST::DataType>& out_type);
  ezResult ParseVariableDefinition(ezEnum<ezExpressionAST::DataType> type);
  ezResult ParseAssignment();

  ezExpressionAST::Node* ParseFactor();
  ezExpressionAST::Node* ParseExpression(int iPrecedence = s_iLowestPrecedence);
  ezExpressionAST::Node* ParseUnaryExpression();
  ezExpressionAST::Node* ParseFunctionCall(ezStringView sFunctionName);

  bool AcceptStatementTerminator();
  bool AcceptBinaryOperator(ezExpressionAST::NodeType::Enum& out_binaryOp, int& out_iOperatorPrecedence);
  ezExpressionAST::Node* GetVariable(ezStringView sVarName);

  ezResult Expect(const char* szToken, const ezToken** pExpectedToken = nullptr);
  ezResult Expect(ezTokenType::Enum Type, const ezToken** pExpectedToken = nullptr);

  void ReportError(const ezToken* pToken, const ezFormatString& message);

  /// \brief Checks whether all outputs have been written
  ezResult CheckOutputs();

  Options m_Options;

  ezTokenParseUtils::TokenStream m_TokenStream;
  ezUInt32 m_uiCurrentToken = 0;
  ezExpressionAST* m_pAST = nullptr;

  ezHashTable<ezHashedString, ezEnum<ezExpressionAST::DataType>> m_KnownTypes;

  struct KnownVariable
  {
    EZ_DECLARE_POD_TYPE();

    ezExpressionAST::Node* m_pNode = nullptr;
    ezEnum<ezExpressionAST::DataType> m_Type;
  };

  ezHashTable<ezHashedString, KnownVariable> m_KnownVariables;
  ezHashTable<ezHashedString, ezEnum<ezExpressionAST::NodeType>> m_BuiltinFunctions;
};

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionParser_inl.h>
