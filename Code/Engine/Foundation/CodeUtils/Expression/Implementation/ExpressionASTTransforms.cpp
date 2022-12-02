#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Logging/Log.h>

namespace
{
  ezExpressionAST::DataType::Enum BiggerDataType(ezExpressionAST::DataType::Enum a, ezExpressionAST::DataType::Enum b)
  {
    ezExpression::RegisterType::Enum ra = ezExpressionAST::DataType::GetRegisterType(a);
    ezExpression::RegisterType::Enum rb = ezExpressionAST::DataType::GetRegisterType(b);

    const ezUInt32 ea = ezExpressionAST::DataType::GetElementCount(a);
    const ezUInt32 eb = ezExpressionAST::DataType::GetElementCount(b);

    return ezExpressionAST::DataType::FromRegisterType(ezMath::Min(ra, rb), ezMath::Max(ea, eb));
  }
} // namespace

ezExpressionAST::Node* ezExpressionAST::TypeDeductionAndConversion(Node* pNode)
{
  auto children = GetChildren(pNode);

  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum dataType = pNode->m_DataType;
  if (dataType == DataType::Unknown)
  {
    for (auto pChildNode : children)
    {
      if (pChildNode == nullptr)
      {
        return nullptr;
      }

      dataType = BiggerDataType(dataType, pChildNode->m_DataType);
    }

    if (dataType == DataType::Unknown)
    {
      ezLog::Error("Failed to deduce type for '{}' node", NodeType::GetName(nodeType));
      return nullptr;
    }

    pNode->m_DataType = DataType::ClampToSupportedDataTypes(dataType, pNode->m_uiSupportedDataTypes);
  }

  for (ezUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    if (pChildNode == nullptr)
    {
      return nullptr;
    }

    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);
    
    if (expectedChildDataType != DataType::Unknown && pChildNode->m_DataType != expectedChildDataType)
    {
      pChildNode = CreateUnaryOperator(NodeType::TypeConversion, pChildNode, expectedChildDataType);
    }
  }

  return pNode;
}

ezExpressionAST::Node* ezExpressionAST::ReplaceUnsupportedInstructions(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum dataType = pNode->m_DataType;
  if (nodeType == NodeType::Negate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      const double fValue = pConstantNode->m_Value.ConvertTo<double>();
      return CreateConstant(-fValue, dataType);
    }
    else
    {
      auto pZero = CreateConstant(0, dataType);
      auto pValue = pUnaryNode->m_pOperand;
      return CreateBinaryOperator(NodeType::Subtract, pZero, pValue, dataType);
    }
  }
  else if (nodeType == NodeType::Saturate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      const double fValue = pConstantNode->m_Value.ConvertTo<double>();
      return CreateConstant(ezMath::Saturate(fValue), dataType);
    }
    else
    {
      auto pZero = CreateConstant(0, dataType);
      auto pOne = CreateConstant(1, dataType);
      auto pValue = static_cast<const UnaryOperator*>(pNode)->m_pOperand;
      return CreateBinaryOperator(NodeType::Max, pZero, CreateBinaryOperator(NodeType::Min, pOne, pValue, dataType), dataType);
    }
  }
  else if (nodeType == NodeType::Clamp)
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    auto pValue = pTernaryNode->m_pFirstOperand;
    auto pMinValue = pTernaryNode->m_pSecondOperand;
    auto pMaxValue = pTernaryNode->m_pThirdOperand;
    return CreateBinaryOperator(NodeType::Max, pMinValue, CreateBinaryOperator(NodeType::Min, pMaxValue, pValue, dataType), dataType);
  }
  else if (nodeType == NodeType::ConstructorCall)
  {
    auto pConstructorCallNode = static_cast<const ConstructorCall*>(pNode);
    if (pConstructorCallNode->m_Arguments.GetCount() > 1)
    {
      ezLog::Error("Constructor of type '{}' has too many arguments", DataType::GetName(dataType));
      return nullptr;
    }

    return pConstructorCallNode->m_Arguments[0];
  }

  return pNode;
}

//////////////////////////////////////////////////////////////////////////

ezExpressionAST::Node* ezExpressionAST::FoldConstants(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum dataType = pNode->m_DataType;
  if (NodeType::IsUnary(nodeType))
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      if (nodeType == NodeType::TypeConversion)
      {
        return CreateConstant(pConstantNode->m_Value, dataType);
      }
      else if (nodeType >= NodeType::Negate && nodeType <= NodeType::Sqrt)
      {
        const double fValue = pConstantNode->m_Value.ConvertTo<double>();

        switch (nodeType)
        {
          case NodeType::Negate:
            return CreateConstant(-fValue, dataType);
          case NodeType::Absolute:
            return CreateConstant(ezMath::Abs(fValue), dataType);
          case NodeType::Saturate:
            return CreateConstant(ezMath::Saturate(fValue), dataType);
          case NodeType::Sqrt:
            return CreateConstant(ezMath::Sqrt(fValue), dataType);
        }
      }
      else
      {
        if (dataType != DataType::Float)
        {
          ezLog::Error("Unsupported data type '{}' for trig function '{}'", DataType::GetName(dataType), NodeType::GetName(nodeType));
          return nullptr;
        }

        const float fValue = pConstantNode->m_Value.Get<float>();

        switch (nodeType)
        {
          case NodeType::Sin:
            return CreateConstant(ezMath::Sin(ezAngle::Radian(fValue)));
          case NodeType::Cos:
            return CreateConstant(ezMath::Cos(ezAngle::Radian(fValue)));
          case NodeType::Tan:
            return CreateConstant(ezMath::Tan(ezAngle::Radian(fValue)));
          case NodeType::ASin:
            return CreateConstant(ezMath::ASin(fValue).GetRadian());
          case NodeType::ACos:
            return CreateConstant(ezMath::ACos(fValue).GetRadian());
          case NodeType::ATan:
            return CreateConstant(ezMath::ATan(fValue).GetRadian());

          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
    }
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    const bool bLeftIsConstant = NodeType::IsConstant(pBinaryNode->m_pLeftOperand->m_Type);
    const bool bRightIsConstant = NodeType::IsConstant(pBinaryNode->m_pRightOperand->m_Type);
    if (bLeftIsConstant && bRightIsConstant)
    {
      auto pLeftConstant = static_cast<const Constant*>(pBinaryNode->m_pLeftOperand);
      auto pRightConstant = static_cast<const Constant*>(pBinaryNode->m_pRightOperand);
      if (nodeType != NodeType::Divide)
      {
        const double fLeftValue = pLeftConstant->m_Value.ConvertTo<double>();
        const double fRightValue = pRightConstant->m_Value.ConvertTo<double>();

        switch (nodeType)
        {
          case NodeType::Add:
            return CreateConstant(fLeftValue + fRightValue, dataType);
          case NodeType::Subtract:
            return CreateConstant(fLeftValue - fRightValue, dataType);
          case NodeType::Multiply:
            return CreateConstant(fLeftValue * fRightValue, dataType);
          case NodeType::Min:
            return CreateConstant(ezMath::Min(fLeftValue, fRightValue), dataType);
          case NodeType::Max:
            return CreateConstant(ezMath::Max(fLeftValue, fRightValue), dataType);

          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else
      {
        if (dataType == DataType::Float)
        {
          const float fLeftValue = pLeftConstant->m_Value.Get<float>();
          const float fRightValue = pRightConstant->m_Value.Get<float>();
          return CreateConstant(fLeftValue / fRightValue, dataType);
        }
        else if (dataType == DataType::Int)
        {
          const int iLeftValue = pLeftConstant->m_Value.Get<int>();
          const int iRightValue = pRightConstant->m_Value.Get<int>();
          return CreateConstant(iLeftValue / iRightValue, dataType);
        }
        else
        {
          ezLog::Error("Unsupported data type '{}' for divide operation", DataType::GetName(dataType));
          return nullptr;
        }
      }
    }
    else if (bRightIsConstant)
    {
      auto pOperand = pBinaryNode->m_pLeftOperand;
      auto pConstantNode = static_cast<Constant*>(pBinaryNode->m_pRightOperand);
      if (nodeType != NodeType::Divide)
      {
        const double fValue = pConstantNode->m_Value.ConvertTo<double>();

        switch (nodeType)
        {
          case NodeType::Add:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Add, pConstantNode, pOperand, dataType);
          case NodeType::Subtract:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Add, CreateConstant(-fValue, dataType), pOperand, dataType);
          case NodeType::Multiply:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Multiply, pConstantNode, pOperand, dataType);
          case NodeType::Min:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Min, pConstantNode, pOperand, dataType);
          case NodeType::Max:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Max, pConstantNode, pOperand, dataType);

          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (dataType == DataType::Float) // Divide optimization only works for float
      {
        const float fValue = pConstantNode->m_Value.Get<float>();
        return CreateBinaryOperator(ezExpressionAST::NodeType::Multiply, CreateConstant(1.0f / fValue), pOperand, dataType);
      }
    }
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    if (nodeType == NodeType::Select)
    {
      if (NodeType::IsConstant(pTernaryNode->m_pFirstOperand->m_Type))
      {
        auto pConstantNode = static_cast<Constant*>(pTernaryNode->m_pFirstOperand);
        const bool bValue = pConstantNode->m_Value.Get<bool>();
        return bValue ? pTernaryNode->m_pSecondOperand : pTernaryNode->m_pThirdOperand;
      }
    }

    EZ_ASSERT_NOT_IMPLEMENTED;
    return pNode;
  }

  return pNode;
}

ezExpressionAST::Node* ezExpressionAST::Validate(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum dataType = pNode->m_DataType;

  if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsTernary(nodeType))
  {
    if (DataType::IsSupported(dataType, pNode->m_uiSupportedDataTypes) == false)
    {
      ezLog::Error("Unsupported data type {} on '{}'", DataType::GetName(dataType), NodeType::GetName(nodeType));
      return nullptr;
    }
  }
  else if (NodeType::IsConstant(nodeType))
  {
    auto pConstantNode = static_cast<Constant*>(pNode);
    if (pConstantNode->m_Value.IsValid() == false)
    {
      ezLog::Error("Invalid constant value");
      return nullptr;
    }
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto pFunctionCall = static_cast<FunctionCall*>(pNode);
    if (pFunctionCall->m_Arguments.GetCount() < pFunctionCall->m_Desc.m_uiNumRequiredInputs)
    {
      ezLog::Error("Not enough arguments for function '{}'", pFunctionCall->m_Desc.m_sName);
      return nullptr;
    }
  }

  auto children = GetChildren(pNode);
  for (ezUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);

    if (expectedChildDataType != DataType::Unknown && pChildNode->m_DataType != expectedChildDataType)
    {
      ezLog::Error("Invalid data type for argument {} on '{}'. Expected {} got {}", i, NodeType::GetName(nodeType), DataType::GetName(expectedChildDataType), DataType::GetName(pChildNode->m_DataType));
      return nullptr;
    }
  }

  return pNode;
}
