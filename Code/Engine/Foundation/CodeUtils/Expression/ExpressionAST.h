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
      BitwiseNot,
      LogicalNot,
      TypeConversion,
      LastUnary,

      // Binary
      FirstBinary,
      Add,
      Subtract,
      Multiply,
      Divide,
      Modulo,
      Min,
      Max,
      BitshiftLeft,
      BitshiftRight,
      BitwiseAnd,
      BitwiseXor,
      BitwiseOr,
      Equal,
      NotEqual,
      Less,
      LessEqual,
      Greater,
      GreaterEqual,
      LogicalAnd,
      LogicalOr,
      LastBinary,

      // Ternary
      FirstTernary,
      Clamp,
      Select,
      LastTernary,

      Constant,
      Input,
      Output,

      FunctionCall,
      ConstructorCall,

      Count
    };

    static bool IsUnary(Enum nodeType);
    static bool IsBinary(Enum nodeType);
    static bool IsTernary(Enum nodeType);
    static bool IsConstant(Enum nodeType);
    static bool IsInput(Enum nodeType);
    static bool IsOutput(Enum nodeType);
    static bool IsFunctionCall(Enum nodeType);
    static bool IsConstructorCall(Enum nodeType);

    static const char* GetName(Enum nodeType);
  };

  struct DataType
  {
    using StorageType = ezUInt8;

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

    EZ_ALWAYS_INLINE static ezExpression::RegisterType::Enum GetRegisterType(Enum dataType)
    {
      return static_cast<ezExpression::RegisterType::Enum>(dataType >> 2);
    }

    EZ_ALWAYS_INLINE static Enum FromRegisterType(ezExpression::RegisterType::Enum registerType, ezUInt32 uiElementCount = 1)
    {
      return static_cast<ezExpressionAST::DataType::Enum>((registerType << 2) + uiElementCount - 1);
    }

    EZ_ALWAYS_INLINE static ezUInt32 GetElementCount(Enum dataType) { return (dataType & 0x3) + 1; }

    EZ_ALWAYS_INLINE static bool IsSupported(Enum dataType, ezUInt32 uiSupportedDataTypes)
    {
      ezUInt32 dataTypeBit = EZ_BIT(DataType::GetRegisterType(dataType));
      return (uiSupportedDataTypes & dataTypeBit) != 0;
    }

    static Enum ClampToSupportedDataTypes(Enum dataType, ezUInt32 uiSupportedDataTypes);

    static const char* GetName(Enum dataType);
  };

  struct Node
  {
    ezEnum<NodeType> m_Type;
    ezEnum<DataType> m_DataType;
    ezUInt8 m_uiSupportedDataTypes = 0;
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
    ezSmallArray<Node*, 8> m_Arguments;
  };

  struct ConstructorCall : public Node
  {
    ezSmallArray<Node*, 4> m_Arguments;
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
  ConstructorCall* CreateConstructorCall(DataType::Enum dataType);

  static ezArrayPtr<Node*> GetChildren(Node* pNode);
  static ezArrayPtr<const Node*> GetChildren(const Node* pNode);

  static DataType::Enum GetExpectedChildDataType(Node* pNode, ezUInt32 uiChildIndex);

  void PrintGraph(ezDGMLGraph& graph) const;

  ezHybridArray<Output*, 8> m_OutputNodes;

  // Transforms
  Node* TypeDeductionAndConversion(Node* pNode);
  Node* ReplaceUnsupportedInstructions(Node* pNode);
  Node* FoldConstants(Node* pNode);
  Node* Validate(Node* pNode);

private:
  ezStackAllocator<> m_Allocator;
};
