#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Logging/Log.h>

namespace
{
  ezExpressionAST::DataType::Enum BiggerDataType(ezExpressionAST::DataType::Enum a, ezExpressionAST::DataType::Enum b)
  {
    if (a == ezExpressionAST::DataType::Unknown)
      return b;

    if (b == ezExpressionAST::DataType::Unknown)
      return a;

    return static_cast<ezUInt32>(a) < static_cast<ezUInt32>(b) ? a : b;
  }
} // namespace

ezExpressionAST::Node* ezExpressionAST::TypeDeductionAndConversion(Node* pNode)
{
  auto children = GetChildren(pNode);

  DataType::Enum dataType = pNode->m_DataType;
  if (dataType == DataType::Unknown)
  {
    for (auto pChildNode : children)
    {
      dataType = BiggerDataType(dataType, pChildNode->m_DataType);
    }

    if (dataType == DataType::Unknown)
    {
      ezLog::Error("Failed to deduce type for '{}' node", NodeType::GetName(pNode->m_Type));
      return nullptr;
    }

    pNode->m_DataType = dataType;
  }

  for (auto& pChildNode : children)
  {
    if (pChildNode->m_DataType != pNode->m_DataType)
    {
      pChildNode = CreateUnaryOperator(NodeType::TypeConversion, pChildNode, pNode->m_DataType);
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
        return CreateBinaryOperator(ezExpressionAST::NodeType::Multiply, CreateConstant(1.0f / fValue), pOperand);
      }
    }
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    const bool bFirstIsConstant = NodeType::IsConstant(pTernaryNode->m_pFirstOperand->m_Type);
    const bool bSecondIsConstant = NodeType::IsConstant(pTernaryNode->m_pSecondOperand->m_Type);
    const bool bThirdIsConstant = NodeType::IsConstant(pTernaryNode->m_pThirdOperand->m_Type);
  }

  return pNode;
}
