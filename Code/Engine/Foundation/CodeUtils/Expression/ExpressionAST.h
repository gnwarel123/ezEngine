#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Memory/StackAllocator.h>

class ezDGMLGraph;

class EZ_FOUNDATION_DLL ezExpressionAST
{
public:
  struct NodeType
  {
    using StorageType = ezUInt32;

    enum Enum
    {
      Invalid,
      Default = Invalid,

      // Unary
      FirstUnary,
      Negate,
      Absolute,
      Saturate,
      Sqrt,
      Sin,
      Cos,
      Tan,
      ASin,
      ACos,
      ATan,
      TypeConversion,
      LastUnary,

      // Binary
      FirstBinary,
      Add,
      Subtract,
      Multiply,
      Divide,
      Min,
      Max,
      LastBinary,

      // Ternary
      FirstTernary,
      Clamp,
      Select,
      LastTernary,

      // Constant
      Constant,

      // Input
      Input,

      // Output
      Output,

      FunctionCall,

      Count
    };

    static bool IsUnary(Enum nodeType);
    static bool IsBinary(Enum nodeType);
    static bool IsTernary(Enum nodeType);
    static bool IsConstant(Enum nodeType);
    static bool IsInput(Enum nodeType);
    static bool IsOutput(Enum nodeType);

    static const char* GetName(Enum nodeType);
  };

  struct DataType
  {
    using StorageType = ezUInt32;

    enum Enum
    {
      Float,
      Float2,
      Float3,
      Float4,

      Int,
      Int2,
      Int3,
      Int4,

      Bool,
      Bool2,
      Bool3,
      Bool4,

      Unknown,
      Default = Unknown,

      Count
    };

    static ezVariantType::Enum GetVariantType(Enum dataType);

    static Enum FromStreamType(ezProcessingStream::DataType dataType);

    static ezExpression::RegisterType::Enum GetRegisterType(Enum dataType);
    static Enum FromRegisterType(ezExpression::RegisterType::Enum registerType);

    static const char* GetName(Enum dataType);
  };

  struct Node
  {
    ezEnum<NodeType> m_Type;
    ezEnum<DataType> m_DataType;
  };

  struct UnaryOperator : public Node
  {
    Node* m_pOperand = nullptr;
  };

  struct BinaryOperator : public Node
  {
    Node* m_pLeftOperand = nullptr;
    Node* m_pRightOperand = nullptr;
  };

  struct TernaryOperator : public Node
  {
    Node* m_pFirstOperand = nullptr;
    Node* m_pSecondOperand = nullptr;
    Node* m_pThirdOperand = nullptr;
  };

  struct Constant : public Node
  {
    ezVariant m_Value;
  };

  struct Input : public Node
  {
    ezExpression::StreamDesc m_Desc;
  };

  struct Output : public Node
  {
    ezExpression::StreamDesc m_Desc;
    Node* m_pExpression = nullptr;
  };

  struct FunctionCall : public Node
  {
    ezExpression::FunctionDesc m_Desc;
    ezHybridArray<Node*, 8> m_Arguments;
  };

public:
  ezExpressionAST();
  ~ezExpressionAST();

  UnaryOperator* CreateUnaryOperator(NodeType::Enum type, Node* pOperand, DataType::Enum dataType = DataType::Unknown);
  BinaryOperator* CreateBinaryOperator(NodeType::Enum type, Node* pLeftOperand, Node* pRightOperand, DataType::Enum dataType = DataType::Unknown);
  TernaryOperator* CreateTernaryOperator(NodeType::Enum type, Node* pFirstOperand, Node* pSecondOperand, Node* pThirdOperand, DataType::Enum dataType = DataType::Unknown);
  Constant* CreateConstant(const ezVariant& value, DataType::Enum dataType = DataType::Float);
  Input* CreateInput(const ezExpression::StreamDesc& desc);
  Output* CreateOutput(const ezExpression::StreamDesc& desc, Node* pExpression);
  FunctionCall* CreateFunctionCall(const ezExpression::FunctionDesc& desc);

  static ezArrayPtr<Node*> GetChildren(Node* pNode);
  static ezArrayPtr<const Node*> GetChildren(const Node* pNode);

  void PrintGraph(ezDGMLGraph& graph) const;

  ezHybridArray<Output*, 8> m_OutputNodes;

  // Transforms
  Node* TypeDeductionAndConversion(Node* pNode);
  Node* ReplaceUnsupportedInstructions(Node* pNode);
  Node* FoldConstants(Node* pNode);

private:
  ezStackAllocator<> m_Allocator;
};
