#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Logging/Log.h>

namespace
{
  static const char* s_szOpCodeNames[] = {
    "Nop",

    // Unary
    "",

    "AbsF_R",
    "AbsI_R",
    "SqrtF_R",

    "SinF_R",
    "CosF_R",
    "TanF_R",

    "ASinF_R",
    "ACosF_R",
    "ATanF_R",

    "",

    // Binary
    "",

    "AddF_RR",
    "AddI_RR",

    "SubF_RR",
    "SubI_RR",

    "MulF_RR",
    "MulI_RR",

    "DivF_RR",
    "DivI_RR",

    "MinF_RR",
    "MinI_RR",

    "MaxF_RR",
    "MaxI_RR",

    "",

    // Binary With Constant
    "",

    "AddF_CR",
    "AddI_CR",

    "SubF_CR",
    "SubI_CR",

    "MulF_CR",
    "MulI_CR",

    "DivF_CR",
    "DivI_CR",

    "MinF_CR",
    "MinI_CR",

    "MaxF_CR",
    "MaxI_CR",

    "",

    "MovX_R",
    "MovX_C",
    "LoadF",
    "LoadI",
    "StoreF",
    "StoreI",

    "Call",
  };

  static_assert(EZ_ARRAY_SIZE(s_szOpCodeNames) == ezExpressionByteCode::OpCode::Count);
  static_assert(ezExpressionByteCode::OpCode::LastBinary - ezExpressionByteCode::OpCode::FirstBinary == ezExpressionByteCode::OpCode::LastBinaryWithConstant - ezExpressionByteCode::OpCode::FirstBinaryWithConstant);

  static bool FirstArgIsConstant(ezExpressionByteCode::OpCode::Enum opCode)
  {
    return opCode > ezExpressionByteCode::OpCode::FirstBinaryWithConstant && opCode < ezExpressionByteCode::OpCode::LastBinaryWithConstant;
  }
} // namespace

ezExpressionByteCode::ezExpressionByteCode() = default;
ezExpressionByteCode::~ezExpressionByteCode() = default;

bool ezExpressionByteCode::operator==(const ezExpressionByteCode& other) const
{
  return m_ByteCode == other.m_ByteCode &&
         m_Inputs == other.m_Inputs &&
         m_Outputs == other.m_Outputs &&
         m_Functions == other.m_Functions;
}

void ezExpressionByteCode::Clear()
{
  m_ByteCode.Clear();
  m_Inputs.Clear();
  m_Outputs.Clear();
  m_Functions.Clear();

  m_uiNumInstructions = 0;
  m_uiNumTempRegisters = 0;
}

void ezExpressionByteCode::Disassemble(ezStringBuilder& out_sDisassembly) const
{
  out_sDisassembly.Append("// Inputs:\n");
  for (ezUInt32 i = 0; i < m_Inputs.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_Inputs[i].m_sName, ezProcessingStream::GetDataTypeName(m_Outputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Outputs:\n");
  for (ezUInt32 i = 0; i < m_Outputs.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_Outputs[i].m_sName, ezProcessingStream::GetDataTypeName(m_Outputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Functions:\n");
  for (ezUInt32 i = 0; i < m_Functions.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {} {}(", i, ezExpression::RegisterType::GetName(m_Functions[i].m_OutputType), m_Functions[i].m_sName);
    const ezUInt32 uiNumArguments = m_Functions[i].m_InputTypes.GetCount();
    for (ezUInt32 j = 0; j < uiNumArguments; ++i)
    {
      out_sDisassembly.Append(ezExpression::RegisterType::GetName(m_Functions[i].m_InputTypes[j]));
      if (j < uiNumArguments-1)
      {
        out_sDisassembly.Append(", ");
      }
    }
    out_sDisassembly.Append(")\n");
  }

  out_sDisassembly.AppendFormat("\n// Temp Registers: {}\n", m_uiNumTempRegisters);
  out_sDisassembly.AppendFormat("// Instructions: {}\n\n", m_uiNumInstructions);


  const StorageType* pByteCode = GetByteCode();
  const StorageType* pByteCodeEnd = GetByteCodeEnd();

  while (pByteCode < pByteCodeEnd)
  {
    OpCode::Enum opCode = GetOpCode(pByteCode);
    const char* szOpCode = s_szOpCodeNames[opCode];

    if (opCode > OpCode::FirstUnary && opCode < OpCode::LastUnary)
    {
      ezUInt32 r = GetRegisterIndex(pByteCode, 1);
      ezUInt32 x = GetRegisterIndex(pByteCode, 1);

      if (FirstArgIsConstant(opCode))
      {
        out_sDisassembly.AppendFormat("{} r{} {}({})\n", szOpCode, r, ezArgU(x, 8, false, 16), ezArgF(*reinterpret_cast<float*>(&x), 6));
      }
      else
      {
        if (opCode == OpCode::LoadF || opCode == OpCode::LoadI)
        {
          out_sDisassembly.AppendFormat("{} r{} i{}({})\n", szOpCode, r, x, m_Inputs[x].m_sName);
        }
        else if (opCode == OpCode::StoreF || opCode == OpCode::StoreI)
        {
          out_sDisassembly.AppendFormat("{} o{}({}) r{}\n", szOpCode, r, m_Outputs[r].m_sName, x);
        }
        else
        {
          out_sDisassembly.AppendFormat("{} r{} r{}\n", szOpCode, r, x);
        }
      }
    }
    else if (opCode > OpCode::FirstBinary && opCode < OpCode::LastBinary)
    {
      ezUInt32 r = GetRegisterIndex(pByteCode, 1);
      ezUInt32 a = GetRegisterIndex(pByteCode, 1);
      ezUInt32 b = GetRegisterIndex(pByteCode, 1);

      if (FirstArgIsConstant(opCode))
      {
        out_sDisassembly.AppendFormat("{} r{} {}({}) r{}\n", szOpCode, r, ezArgU(a, 8, false, 16) , ezArgF(*reinterpret_cast<float*>(&a), 6), b);
      }
      else
      {
        out_sDisassembly.AppendFormat("{} r{} r{} r{}\n", szOpCode, r, a, b);
      }
    }
    else if (opCode == OpCode::Call)
    {
      ezUInt32 uiIndex = GetFunctionIndex(pByteCode);
      const char* szName = m_Functions[uiIndex].m_sName;

      ezStringBuilder sName;
      if (ezStringUtils::IsNullOrEmpty(szName))
      {
        sName.Format("Unknown_{0}", uiIndex);
      }
      else
      {
        sName = szName;
      }

      ezUInt32 r = GetRegisterIndex(pByteCode, 1);

      out_sDisassembly.AppendFormat("{0} {1} r{2}", szOpCode, sName, r);

      ezUInt32 uiNumArgs = GetFunctionArgCount(pByteCode);
      for (ezUInt32 uiArgIndex = 0; uiArgIndex < uiNumArgs; ++uiArgIndex)
      {
        ezUInt32 x = GetRegisterIndex(pByteCode, 1);
        out_sDisassembly.AppendFormat(" r{0}", x);
      }

      out_sDisassembly.Append("\n");
    }
    else
    {
      EZ_ASSERT_NOT_IMPLEMENTED;
    }
  }
}

const char* ezExpressionByteCode::GetOpCodeName(OpCode::Enum opCode)
{
  return s_szOpCodeNames[opCode];
}

static constexpr ezUInt32 s_uiMetaDataVersion = 4;
static constexpr ezUInt32 s_uiCodeVersion = 3;

void ezExpressionByteCode::Save(ezStreamWriter& stream) const
{
  ezChunkStreamWriter chunk(stream);

  chunk.BeginStream(1);

  {
    chunk.BeginChunk("MetaData", s_uiMetaDataVersion);

    chunk << m_uiNumInstructions;
    chunk << m_uiNumTempRegisters;
    chunk.WriteArray(m_Inputs).IgnoreResult();
    chunk.WriteArray(m_Outputs).IgnoreResult();
    chunk.WriteArray(m_Functions).IgnoreResult();

    chunk.EndChunk();
  }

  {
    chunk.BeginChunk("Code", s_uiCodeVersion);

    chunk << m_ByteCode.GetCount();
    chunk.WriteBytes(m_ByteCode.GetData(), m_ByteCode.GetCount() * sizeof(StorageType)).IgnoreResult();

    chunk.EndChunk();
  }

  chunk.EndStream();
}

ezResult ezExpressionByteCode::Load(ezStreamReader& stream)
{
  ezChunkStreamReader chunk(stream);
  chunk.SetEndChunkFileMode(ezChunkStreamReader::EndChunkFileMode::SkipToEnd);

  chunk.BeginStream();

  while (chunk.GetCurrentChunk().m_bValid)
  {
    if (chunk.GetCurrentChunk().m_sChunkName == "MetaData")
    {
      if (chunk.GetCurrentChunk().m_uiChunkVersion >= s_uiMetaDataVersion)
      {
        chunk >> m_uiNumInstructions;
        chunk >> m_uiNumTempRegisters;
        EZ_SUCCEED_OR_RETURN(chunk.ReadArray(m_Inputs));
        EZ_SUCCEED_OR_RETURN(chunk.ReadArray(m_Outputs));
        EZ_SUCCEED_OR_RETURN(chunk.ReadArray(m_Functions));
      }
      else
      {
        ezLog::Error("Invalid MetaData Chunk Version {}. Expected >= {}", chunk.GetCurrentChunk().m_uiChunkVersion, s_uiMetaDataVersion);

        chunk.EndStream();
        return EZ_FAILURE;
      }
    }
    else if (chunk.GetCurrentChunk().m_sChunkName == "Code")
    {
      if (chunk.GetCurrentChunk().m_uiChunkVersion >= s_uiCodeVersion)
      {
        ezUInt32 uiByteCodeCount = 0;
        chunk >> uiByteCodeCount;

        m_ByteCode.SetCountUninitialized(uiByteCodeCount);
        chunk.ReadBytes(m_ByteCode.GetData(), uiByteCodeCount * sizeof(StorageType));
      }
      else
      {
        ezLog::Error("Invalid Code Chunk Version {}. Expected >= {}", chunk.GetCurrentChunk().m_uiChunkVersion, s_uiCodeVersion);

        chunk.EndStream();
        return EZ_FAILURE;
      }
    }

    chunk.NextChunk();
  }

  chunk.EndStream();

  return EZ_SUCCESS;
}
