#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Logging/Log.h>

ezExpressionAST::Node* ezExpressionAST::TypeDeductionAndConversion(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum returnType = pNode->m_ReturnType;
  if (returnType == DataType::Unknown)
  {
    ResolveOverloads(pNode);

    if (returnType == DataType::Unknown)
    {
      ezLog::Error("No matching overload found for '{}'", NodeType::GetName(nodeType));
      return nullptr;
    }
  }

  auto children = GetChildren(pNode);
  for (ezUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    if (pChildNode == nullptr)
    {
      return nullptr;
    }

    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);
    
    if (expectedChildDataType != DataType::Unknown && pChildNode->m_ReturnType != expectedChildDataType)
    {
      pChildNode = CreateUnaryOperator(NodeType::TypeConversion, pChildNode, expectedChildDataType);
    }
  }

  return pNode;
}

ezExpressionAST::Node* ezExpressionAST::ReplaceUnsupportedInstructions(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  DataType::Enum returnType = pNode->m_ReturnType;
  if (nodeType == NodeType::Negate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      const double fValue = pConstantNode->m_Value.ConvertTo<double>();
      return CreateConstant(-fValue, returnType);
    }
    else
    {
      auto pZero = CreateConstant(0, returnType);
      auto pValue = pUnaryNode->m_pOperand;
      return CreateBinaryOperator(NodeType::Subtract, pZero, pValue);
    }
  }
  else if (nodeType == NodeType::Saturate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      const double fValue = pConstantNode->m_Value.ConvertTo<double>();
      return CreateConstant(ezMath::Saturate(fValue), returnType);
    }
    else
    {
      auto pZero = CreateConstant(0, returnType);
      auto pOne = CreateConstant(1, returnType);
      auto pValue = static_cast<const UnaryOperator*>(pNode)->m_pOperand;
      return CreateBinaryOperator(NodeType::Max, pZero, CreateBinaryOperator(NodeType::Min, pOne, pValue));
    }
  }
  else if (nodeType == NodeType::Clamp)
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    auto pValue = pTernaryNode->m_pFirstOperand;
    auto pMinValue = pTernaryNode->m_pSecondOperand;
    auto pMaxValue = pTernaryNode->m_pThirdOperand;
    return CreateBinaryOperator(NodeType::Max, pMinValue, CreateBinaryOperator(NodeType::Min, pMaxValue, pValue));
  }
  else if (nodeType == NodeType::ConstructorCall)
  {
    auto pConstructorCallNode = static_cast<const ConstructorCall*>(pNode);
    if (pConstructorCallNode->m_Arguments.GetCount() > 1)
    {
      ezLog::Error("Constructor of type '{}' has too many arguments", DataType::GetName(returnType));
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
  DataType::Enum returnType = pNode->m_ReturnType;
  if (NodeType::IsUnary(nodeType))
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      if (nodeType == NodeType::TypeConversion)
      {
        return CreateConstant(pConstantNode->m_Value, returnType);
      }
      else if (nodeType >= NodeType::Negate && nodeType <= NodeType::Sqrt)
      {
        const double fValue = pConstantNode->m_Value.ConvertTo<double>();

        switch (nodeType)
        {
          case NodeType::Negate:
            return CreateConstant(-fValue, returnType);
          case NodeType::Absolute:
            return CreateConstant(ezMath::Abs(fValue), returnType);
          case NodeType::Saturate:
            return CreateConstant(ezMath::Saturate(fValue), returnType);
          case NodeType::Sqrt:
            return CreateConstant(ezMath::Sqrt(fValue), returnType);
        }
      }
      else
      {
        if (returnType != DataType::Float)
        {
          ezLog::Error("Unsupported data type '{}' for trig function '{}'", DataType::GetName(returnType), NodeType::GetName(nodeType));
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
            return CreateConstant(fLeftValue + fRightValue, returnType);
          case NodeType::Subtract:
            return CreateConstant(fLeftValue - fRightValue, returnType);
          case NodeType::Multiply:
            return CreateConstant(fLeftValue * fRightValue, returnType);
          case NodeType::Min:
            return CreateConstant(ezMath::Min(fLeftValue, fRightValue), returnType);
          case NodeType::Max:
            return CreateConstant(ezMath::Max(fLeftValue, fRightValue), returnType);

          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else
      {
        if (returnType == DataType::Float)
        {
          const float fLeftValue = pLeftConstant->m_Value.Get<float>();
          const float fRightValue = pRightConstant->m_Value.Get<float>();
          return CreateConstant(fLeftValue / fRightValue, returnType);
        }
        else if (returnType == DataType::Int)
        {
          const int iLeftValue = pLeftConstant->m_Value.Get<int>();
          const int iRightValue = pRightConstant->m_Value.Get<int>();
          return CreateConstant(iLeftValue / iRightValue, returnType);
        }
        else
        {
          ezLog::Error("Unsupported data type '{}' for divide operation", DataType::GetName(returnType));
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
            return CreateBinaryOperator(ezExpressionAST::NodeType::Add, pConstantNode, pOperand);
          case NodeType::Subtract:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Add, CreateConstant(-fValue, returnType), pOperand);
          case NodeType::Multiply:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Multiply, pConstantNode, pOperand);
          case NodeType::Min:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Min, pConstantNode, pOperand);
          case NodeType::Max:
            return CreateBinaryOperator(ezExpressionAST::NodeType::Max, pConstantNode, pOperand);

          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (returnType == DataType::Float) // Divide optimization only works for float
      {
        const float fValue = pConstantNode->m_Value.Get<float>();
        return CreateBinaryOperator(ezExpressionAST::NodeType::Multiply, CreateConstant(1.0f / fValue, returnType), pOperand);
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

  if (pNode->m_ReturnType == DataType::Unknown)
  {
    ezLog::Error("Unresolved return type on '{}'", NodeType::GetName(nodeType));
    return nullptr;
  }

  if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsTernary(nodeType))
  {
    if (pNode->m_uiOverloadIndex == 0xFF)
    {
      ezLog::Error("Unresolved overload on '{}'", NodeType::GetName(nodeType));
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
    if (pNode->m_uiOverloadIndex == 0xFF)
    {
      ezLog::Error("Unresolved function overload on");
      return nullptr;
    }

    auto pFunctionCall = static_cast<FunctionCall*>(pNode);
    auto pDesc = pFunctionCall->m_Descs[pNode->m_uiOverloadIndex];
    if (pFunctionCall->m_Arguments.GetCount() < pDesc->m_uiNumRequiredInputs)
    {
      ezLog::Error("Not enough arguments for function '{}'", pDesc->m_sName);
      return nullptr;
    }
  }

  auto children = GetChildren(pNode);
  for (ezUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);

    if (expectedChildDataType != DataType::Unknown && pChildNode->m_ReturnType != expectedChildDataType)
    {
      ezLog::Error("Invalid data type for argument {} on '{}'. Expected {} got {}", i, NodeType::GetName(nodeType), DataType::GetName(expectedChildDataType), DataType::GetName(pChildNode->m_ReturnType));
      return nullptr;
    }
  }

  return pNode;
}
