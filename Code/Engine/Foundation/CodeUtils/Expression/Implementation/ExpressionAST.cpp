#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Utilities/DGMLWriter.h>

// static
bool ezExpressionAST::NodeType::IsUnary(Enum nodeType)
{
  return nodeType > FirstUnary && nodeType < LastUnary;
}

// static
bool ezExpressionAST::NodeType::IsBinary(Enum nodeType)
{
  return nodeType > FirstBinary && nodeType < LastBinary;
}

// static
bool ezExpressionAST::NodeType::IsTernary(Enum nodeType)
{
  return nodeType > FirstTernary && nodeType < LastTernary;
}

// static
bool ezExpressionAST::NodeType::IsConstant(Enum nodeType)
{
  return nodeType == Constant;
}

// static
bool ezExpressionAST::NodeType::IsInput(Enum nodeType)
{
  return nodeType == Input;
}

// static
bool ezExpressionAST::NodeType::IsOutput(Enum nodeType)
{
  return nodeType == Output;
}

// static
bool ezExpressionAST::NodeType::IsFunctionCall(Enum nodeType)
{
  return nodeType == FunctionCall;
}

// static
bool ezExpressionAST::NodeType::IsConstructorCall(Enum nodeType)
{
  return nodeType == ConstructorCall;
}

namespace
{
  static const char* s_szNodeTypeNames[] = {
    "Invalid",

    // Unary
    "",
    "Negate",
    "Absolute",
    "Saturate",
    "Sqrt",
    "Sin",
    "Cos",
    "Tan",
    "ASin",
    "ACos",
    "ATan",
    "TypeConversion",
    "",

    // Binary
    "",
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "Min",
    "Max",
    "",

    // Ternary
    "",
    "Clamp",
    "Select",
    "",

    "Constant",
    "Input",
    "Output",

    "FunctionCall",
    "ConstructorCall",
  };

  static_assert(EZ_ARRAY_SIZE(s_szNodeTypeNames) == ezExpressionAST::NodeType::Count);

  static ezUInt8 s_uiNodeTypeValidDataTypes[] = {
    0, // Invalid,

    // Unary
    0,                                           // FirstUnary,
    ezExpression::RegisterType::FloatAndIntBits, // Negate,
    ezExpression::RegisterType::FloatAndIntBits, // Absolute,
    ezExpression::RegisterType::FloatAndIntBits, // Saturate,
    ezExpression::RegisterType::FloatBit,        // Sqrt,
    ezExpression::RegisterType::FloatBit,        // Sin,
    ezExpression::RegisterType::FloatBit,        // Cos,
    ezExpression::RegisterType::FloatBit,        // Tan,
    ezExpression::RegisterType::FloatBit,        // ASin,
    ezExpression::RegisterType::FloatBit,        // ACos,
    ezExpression::RegisterType::FloatBit,        // ATan,
    ezExpression::RegisterType::AnyBits,         // TypeConversion,
    0,                                           // LastUnary,

    // Binary
    0,                                           // FirstBinary,
    ezExpression::RegisterType::FloatAndIntBits, // Add,
    ezExpression::RegisterType::FloatAndIntBits, // Subtract,
    ezExpression::RegisterType::FloatAndIntBits, // Multiply,
    ezExpression::RegisterType::FloatAndIntBits, // Divide,
    ezExpression::RegisterType::FloatAndIntBits, // Min,
    ezExpression::RegisterType::FloatAndIntBits, // Max,
    0,                                           // LastBinary,

    // Ternary
    0,                                           // FirstTernary,
    ezExpression::RegisterType::FloatAndIntBits, // Clamp,
    ezExpression::RegisterType::AnyBits,         // Select,
    0,                                           // LastTernary,

    ezExpression::RegisterType::AnyBits, // Constant,
    ezExpression::RegisterType::AnyBits, // Input,
    ezExpression::RegisterType::AnyBits, // Output,

    ezExpression::RegisterType::AnyBits, // FunctionCall,
    ezExpression::RegisterType::AnyBits, // ConstructorCall,
  };

  static_assert(EZ_ARRAY_SIZE(s_uiNodeTypeValidDataTypes) == ezExpressionAST::NodeType::Count);
} // namespace

// static
const char* ezExpressionAST::NodeType::GetName(Enum nodeType)
{
  EZ_ASSERT_DEBUG(nodeType >= 0 && nodeType < EZ_ARRAY_SIZE(s_szNodeTypeNames), "Out of bounds access");
  return s_szNodeTypeNames[nodeType];
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  static ezVariantType::Enum s_DataTypeVariantTypes[] = {
    ezVariantType::Float,   // Float,
    ezVariantType::Vector2, // Float2,
    ezVariantType::Vector3, // Float3,
    ezVariantType::Vector4, // Float4,

    ezVariantType::Int32,    // Int,
    ezVariantType::Vector2I, // Int2,
    ezVariantType::Vector3I, // Int3,
    ezVariantType::Vector4I, // Int4,

    ezVariantType::Bool,    // Bool,
    ezVariantType::Invalid, // Bool2,
    ezVariantType::Invalid, // Bool3,
    ezVariantType::Invalid, // Bool4,

    ezVariantType::Invalid, // Unknown,
  };
  static_assert(EZ_ARRAY_SIZE(s_DataTypeVariantTypes) == (size_t)ezExpressionAST::DataType::Count);

  static ezExpressionAST::DataType::Enum s_DataTypeFromStreamType[] = {
    ezExpressionAST::DataType::Float,  // Half,
    ezExpressionAST::DataType::Float2, // Half2,
    ezExpressionAST::DataType::Float3, // Half3,
    ezExpressionAST::DataType::Float4, // Half4,

    ezExpressionAST::DataType::Float,  // Float,
    ezExpressionAST::DataType::Float2, // Float2,
    ezExpressionAST::DataType::Float3, // Float3,
    ezExpressionAST::DataType::Float4, // Float4,

    ezExpressionAST::DataType::Int,  // Byte,
    ezExpressionAST::DataType::Int2, // Byte2,
    ezExpressionAST::DataType::Int3, // Byte3,
    ezExpressionAST::DataType::Int4, // Byte4,

    ezExpressionAST::DataType::Int,  // Short,
    ezExpressionAST::DataType::Int2, // Short2,
    ezExpressionAST::DataType::Int3, // Short3,
    ezExpressionAST::DataType::Int4, // Short4,

    ezExpressionAST::DataType::Int,  // Int,
    ezExpressionAST::DataType::Int2, // Int2,
    ezExpressionAST::DataType::Int3, // Int3,
    ezExpressionAST::DataType::Int4, // Int4,
  };
  static_assert(EZ_ARRAY_SIZE(s_DataTypeFromStreamType) == (size_t)ezProcessingStream::DataType::Count);

  static_assert(ezExpressionAST::DataType::Float >> 2 == ezExpression::RegisterType::Float);
  static_assert(ezExpressionAST::DataType::Int >> 2 == ezExpression::RegisterType::Int);
  static_assert(ezExpressionAST::DataType::Bool >> 2 == ezExpression::RegisterType::Bool);
  static_assert(ezExpressionAST::DataType::Unknown >> 2 == ezExpression::RegisterType::Unknown);

  static const char* s_szDataTypeNames[] = {
    "Float",  // Float,
    "Float2", // Float2,
    "Float3", // Float3,
    "Float4", // Float4,

    "Int",  // Int,
    "Int2", // Int2,
    "Int3", // Int3,
    "Int4", // Int4,

    "Bool",  // Bool,
    "Bool2", // Bool2,
    "Bool3", // Bool3,
    "Bool4", // Bool4,

    "Unknown", // Unknown,
  };

  static_assert(EZ_ARRAY_SIZE(s_szDataTypeNames) == ezExpressionAST::DataType::Count);
} // namespace


// static
ezVariantType::Enum ezExpressionAST::DataType::GetVariantType(Enum dataType)
{
  EZ_ASSERT_DEBUG(dataType >= 0 && dataType < EZ_ARRAY_SIZE(s_DataTypeVariantTypes), "Out of bounds access");
  return s_DataTypeVariantTypes[dataType];
}

// static
ezExpressionAST::DataType::Enum ezExpressionAST::DataType::FromStreamType(ezProcessingStream::DataType dataType)
{
  EZ_ASSERT_DEBUG(static_cast<ezUInt32>(dataType) >= 0 && static_cast<ezUInt32>(dataType) < EZ_ARRAY_SIZE(s_DataTypeFromStreamType), "Out of bounds access");
  return s_DataTypeFromStreamType[static_cast<ezUInt32>(dataType)];
}

// static
ezExpressionAST::DataType::Enum ezExpressionAST::DataType::ClampToSupportedDataTypes(Enum dataType, ezUInt32 uiSupportedDataTypes)
{
  auto lowerType = static_cast<ezExpression::RegisterType::Enum>(ezMath::FirstBitLow(uiSupportedDataTypes));
  auto upperType = static_cast<ezExpression::RegisterType::Enum>(ezMath::FirstBitHigh(uiSupportedDataTypes));
  if (dataType == Unknown && lowerType != upperType)
    return Unknown;

  return ezMath::Clamp(dataType, FromRegisterType(lowerType), FromRegisterType(upperType));
}

// static
const char* ezExpressionAST::DataType::GetName(Enum dataType)
{
  EZ_ASSERT_DEBUG(dataType >= 0 && dataType < EZ_ARRAY_SIZE(s_szDataTypeNames), "Out of bounds access");
  return s_szDataTypeNames[dataType];
}

//////////////////////////////////////////////////////////////////////////

ezExpressionAST::ezExpressionAST()
  : m_Allocator("Expression AST", ezFoundation::GetAlignedAllocator())
{
}

ezExpressionAST::~ezExpressionAST() {}

ezExpressionAST::UnaryOperator* ezExpressionAST::CreateUnaryOperator(NodeType::Enum type, Node* pOperand, DataType::Enum dataType /*= DataType::Unknown*/)
{
  EZ_ASSERT_DEBUG(NodeType::IsUnary(type), "Type '{}' is not an unary operator", NodeType::GetName(type));

  auto pUnaryOperator = EZ_NEW(&m_Allocator, UnaryOperator);
  pUnaryOperator->m_Type = type;
  pUnaryOperator->m_DataType = DataType::ClampToSupportedDataTypes(dataType, s_uiNodeTypeValidDataTypes[type]);
  pUnaryOperator->m_uiSupportedDataTypes = s_uiNodeTypeValidDataTypes[type];
  pUnaryOperator->m_pOperand = pOperand;

  return pUnaryOperator;
}

ezExpressionAST::BinaryOperator* ezExpressionAST::CreateBinaryOperator(NodeType::Enum type, Node* pLeftOperand, Node* pRightOperand, DataType::Enum dataType /*= DataType::Unknown*/)
{
  EZ_ASSERT_DEBUG(NodeType::IsBinary(type), "Type '{}' is not a binary operator", NodeType::GetName(type));

  auto pBinaryOperator = EZ_NEW(&m_Allocator, BinaryOperator);
  pBinaryOperator->m_Type = type;
  pBinaryOperator->m_DataType = DataType::ClampToSupportedDataTypes(dataType, s_uiNodeTypeValidDataTypes[type]);
  pBinaryOperator->m_uiSupportedDataTypes = s_uiNodeTypeValidDataTypes[type];
  pBinaryOperator->m_pLeftOperand = pLeftOperand;
  pBinaryOperator->m_pRightOperand = pRightOperand;

  return pBinaryOperator;
}

ezExpressionAST::TernaryOperator* ezExpressionAST::CreateTernaryOperator(NodeType::Enum type, Node* pFirstOperand, Node* pSecondOperand, Node* pThirdOperand, DataType::Enum dataType /*= DataType::Unknown*/)
{
  EZ_ASSERT_DEBUG(NodeType::IsTernary(type), "Type '{}' is not a ternary operator", NodeType::GetName(type));

  auto pTernaryOperator = EZ_NEW(&m_Allocator, TernaryOperator);
  pTernaryOperator->m_Type = type;
  pTernaryOperator->m_DataType = DataType::ClampToSupportedDataTypes(dataType, s_uiNodeTypeValidDataTypes[type]);
  pTernaryOperator->m_uiSupportedDataTypes = s_uiNodeTypeValidDataTypes[type];
  pTernaryOperator->m_pFirstOperand = pFirstOperand;
  pTernaryOperator->m_pSecondOperand = pSecondOperand;
  pTernaryOperator->m_pThirdOperand = pThirdOperand;

  return pTernaryOperator;
}

ezExpressionAST::Constant* ezExpressionAST::CreateConstant(const ezVariant& value, DataType::Enum dataType /*= DataType::Float*/)
{
  ezVariantType::Enum variantType = DataType::GetVariantType(dataType);
  EZ_ASSERT_DEV(variantType != ezVariantType::Invalid, "Invalid constant type '{}'", DataType::GetName(dataType));

  auto pConstant = EZ_NEW(&m_Allocator, Constant);
  pConstant->m_Type = NodeType::Constant;
  pConstant->m_DataType = dataType;
  pConstant->m_Value = value.ConvertTo(DataType::GetVariantType(dataType));

  EZ_ASSERT_DEV(pConstant->m_Value.IsValid(), "Invalid constant value or conversion to target data type failed");

  return pConstant;
}

ezExpressionAST::Input* ezExpressionAST::CreateInput(const ezExpression::StreamDesc& desc)
{
  auto pInput = EZ_NEW(&m_Allocator, Input);
  pInput->m_Type = NodeType::Input;
  pInput->m_DataType = DataType::FromStreamType(desc.m_DataType);
  pInput->m_Desc = desc;

  return pInput;
}

ezExpressionAST::Output* ezExpressionAST::CreateOutput(const ezExpression::StreamDesc& desc, Node* pExpression)
{
  auto pOutput = EZ_NEW(&m_Allocator, Output);
  pOutput->m_Type = NodeType::Output;
  pOutput->m_DataType = DataType::FromStreamType(desc.m_DataType);
  pOutput->m_Desc = desc;
  pOutput->m_pExpression = pExpression;

  return pOutput;
}

ezExpressionAST::FunctionCall* ezExpressionAST::CreateFunctionCall(const ezExpression::FunctionDesc& desc)
{
  auto pFunctionCall = EZ_NEW(&m_Allocator, FunctionCall);
  pFunctionCall->m_Type = NodeType::FunctionCall;
  pFunctionCall->m_DataType = DataType::FromRegisterType(desc.m_OutputType);
  pFunctionCall->m_Desc = desc;

  return pFunctionCall;
}

ezExpressionAST::ConstructorCall* ezExpressionAST::CreateConstructorCall(DataType::Enum dataType)
{
  EZ_ASSERT_DEV(dataType < DataType::Unknown, "Invalid data type for constructor");

  auto pConstructorCall = EZ_NEW(&m_Allocator, ConstructorCall);
  pConstructorCall->m_Type = NodeType::ConstructorCall;
  pConstructorCall->m_DataType = dataType;

  return pConstructorCall;
}

// static
ezArrayPtr<ezExpressionAST::Node*> ezExpressionAST::GetChildren(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto& pChild = static_cast<UnaryOperator*>(pNode)->m_pOperand;
    return ezMakeArrayPtr(&pChild, 1);
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto& pChildren = static_cast<BinaryOperator*>(pNode)->m_pLeftOperand;
    return ezMakeArrayPtr(&pChildren, 2);
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto& pChildren = static_cast<TernaryOperator*>(pNode)->m_pFirstOperand;
    return ezMakeArrayPtr(&pChildren, 3);
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto& pChild = static_cast<Output*>(pNode)->m_pExpression;
    return ezMakeArrayPtr(&pChild, 1);
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto& args = static_cast<FunctionCall*>(pNode)->m_Arguments;
    return args;
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    auto& args = static_cast<ConstructorCall*>(pNode)->m_Arguments;
    return args;
  }

  EZ_ASSERT_DEV(NodeType::IsInput(nodeType) || NodeType::IsConstant(nodeType), "Unknown node type");
  return ezArrayPtr<Node*>();
}

// static
ezArrayPtr<const ezExpressionAST::Node*> ezExpressionAST::GetChildren(const Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto& pChild = static_cast<const UnaryOperator*>(pNode)->m_pOperand;
    return ezMakeArrayPtr((const Node**)&pChild, 1);
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto& pChildren = static_cast<const BinaryOperator*>(pNode)->m_pLeftOperand;
    return ezMakeArrayPtr((const Node**)&pChildren, 2);
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto& pChildren = static_cast<const TernaryOperator*>(pNode)->m_pFirstOperand;
    return ezMakeArrayPtr((const Node**)&pChildren, 3);
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto& pChild = static_cast<const Output*>(pNode)->m_pExpression;
    return ezMakeArrayPtr((const Node**)&pChild, 1);
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto& args = static_cast<const FunctionCall*>(pNode)->m_Arguments;
    return ezArrayPtr<const Node*>((const Node**)args.GetData(), args.GetCount());
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    auto& args = static_cast<const ConstructorCall*>(pNode)->m_Arguments;
    return ezArrayPtr<const Node*>((const Node**)args.GetData(), args.GetCount());
  }

  EZ_ASSERT_DEV(NodeType::IsInput(nodeType) || NodeType::IsConstant(nodeType), "Unknown node type");
  return ezArrayPtr<const Node*>();
}

ezExpressionAST::DataType::Enum ezExpressionAST::GetExpectedChildDataType(Node* pNode, ezUInt32 uiChildIndex)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum dataType = pNode->m_DataType;
  EZ_ASSERT_DEV(dataType != DataType::Unknown, "Data type must not be unknown");
  if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsOutput(nodeType))
  {
    if (nodeType == NodeType::TypeConversion)
    {
      return DataType::Unknown;
    }

    return dataType;
  }
  else if (NodeType::IsTernary(nodeType))
  {
    if (nodeType == NodeType::Select && uiChildIndex == 0)
    {
      return DataType::FromRegisterType(ezExpression::RegisterType::Bool, DataType::GetElementCount(dataType));
    }

    return dataType;
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto& desc = static_cast<const FunctionCall*>(pNode)->m_Desc;
    return DataType::FromRegisterType(desc.m_InputTypes[uiChildIndex]);
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    return DataType::FromRegisterType(DataType::GetRegisterType(dataType));
  }

  EZ_ASSERT_NOT_IMPLEMENTED;
  return DataType::Unknown;
}

namespace
{
  struct NodeInfo
  {
    EZ_DECLARE_POD_TYPE();

    const ezExpressionAST::Node* m_pNode;
    ezUInt32 m_uiParentGraphNode;
  };
} // namespace

void ezExpressionAST::PrintGraph(ezDGMLGraph& graph) const
{
  ezHybridArray<NodeInfo, 64> nodeStack;

  ezStringBuilder sTmp;
  for (auto pOutputNode : m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      continue;

    sTmp = NodeType::GetName(pOutputNode->m_Type);
    sTmp.Append("(", DataType::GetName(pOutputNode->m_DataType), ")");
    sTmp.Append(": ", pOutputNode->m_Desc.m_sName);

    ezDGMLGraph::NodeDesc nd;
    nd.m_Color = ezColorScheme::LightUI(ezColorScheme::Blue);
    ezUInt32 uiGraphNode = graph.AddNode(sTmp, &nd);

    nodeStack.PushBack({pOutputNode->m_pExpression, uiGraphNode});
  }

  ezHashTable<const Node*, ezUInt32> nodeCache;

  while (!nodeStack.IsEmpty())
  {
    NodeInfo currentNodeInfo = nodeStack.PeekBack();
    nodeStack.PopBack();

    ezUInt32 uiGraphNode = 0;
    if (currentNodeInfo.m_pNode != nullptr)
    {
      if (!nodeCache.TryGetValue(currentNodeInfo.m_pNode, uiGraphNode))
      {
        NodeType::Enum nodeType = currentNodeInfo.m_pNode->m_Type;
        sTmp = NodeType::GetName(nodeType);
        sTmp.Append("(", DataType::GetName(currentNodeInfo.m_pNode->m_DataType), ")");
        ezColor color = ezColor::White;

        if (NodeType::IsConstant(nodeType))
        {
          sTmp.AppendFormat(": {0}", static_cast<const Constant*>(currentNodeInfo.m_pNode)->m_Value.ConvertTo<ezString>());
        }
        else if (NodeType::IsInput(nodeType))
        {
          auto pInputNode = static_cast<const Input*>(currentNodeInfo.m_pNode);
          sTmp.Append(": ", pInputNode->m_Desc.m_sName);
          color = ezColorScheme::LightUI(ezColorScheme::Green);
        }
        else if (NodeType::IsFunctionCall(nodeType))
        {
          sTmp.Append(": ", static_cast<const FunctionCall*>(currentNodeInfo.m_pNode)->m_Desc.m_sName);
          color = ezColorScheme::LightUI(ezColorScheme::Yellow);
        }

        ezDGMLGraph::NodeDesc nd;
        nd.m_Color = color;
        uiGraphNode = graph.AddNode(sTmp, &nd);
        nodeCache.Insert(currentNodeInfo.m_pNode, uiGraphNode);

        // push children
        auto children = GetChildren(currentNodeInfo.m_pNode);
        for (auto pChild : children)
        {
          nodeStack.PushBack({pChild, uiGraphNode});
        }
      }
    }
    else
    {
      ezDGMLGraph::NodeDesc nd;
      nd.m_Color = ezColor::OrangeRed;
      uiGraphNode = graph.AddNode("Invalid", &nd);
    }

    graph.AddConnection(uiGraphNode, currentNodeInfo.m_uiParentGraphNode);
  }
}
