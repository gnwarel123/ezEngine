#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Types/UniquePtr.h>
#include <Foundation/Utilities/DGMLWriter.h>

namespace
{
  void DumpAST(const ezExpressionAST& ast, ezStringView sOutputName)
  {
    ezDGMLGraph dgmlGraph;
    ast.PrintGraph(dgmlGraph);

    ezStringBuilder sFileName;
    sFileName.Format(":output/Expression/{}_AST.dgml", sOutputName);

    ezDGMLGraphWriter dgmlGraphWriter;
    if (dgmlGraphWriter.WriteGraphToFile(sFileName, dgmlGraph).Succeeded())
    {
      ezLog::Info("AST was dumped to: {}", sFileName);
    }
    else
    {
      ezLog::Error("Failed to dump AST to: {}", sFileName);
    }
  }

  void DumpDisassembly(const ezExpressionByteCode& byteCode, ezStringView sOutputName, ezUInt32 uiCounter)
  {
    ezStringBuilder sDisassembly;
    byteCode.Disassemble(sDisassembly);

    ezStringBuilder sFileName;
    sFileName.Format(":output/Expression/{}_{}_ByteCode.txt", ezArgU(uiCounter, 2, true), sOutputName);

    ezFileWriter fileWriter;
    if (fileWriter.Open(sFileName).Succeeded())
    {
      fileWriter.WriteBytes(sDisassembly.GetData(), sDisassembly.GetElementCount()).IgnoreResult();

      ezLog::Error("Disassembly was dumped to: {}", sFileName);
    }
    else
    {
      ezLog::Error("Failed to dump Disassembly to: {}", sFileName);
    }
  }

  static ezUInt32 s_uiNumByteCodeComparisons = 0;

  bool CompareByteCode(const ezExpressionByteCode& testCode, const ezExpressionByteCode& referenceCode)
  {
    ezUInt32 uiCounter = s_uiNumByteCodeComparisons;
    ++s_uiNumByteCodeComparisons;

    if (testCode != referenceCode)
    {
      DumpDisassembly(referenceCode, "Reference", uiCounter);
      DumpDisassembly(testCode, "Test", uiCounter);
      return false;
    }

    return true;
  }

  static ezHashedString s_sA = ezMakeHashedString("a");
  static ezHashedString s_sB = ezMakeHashedString("b");
  static ezHashedString s_sC = ezMakeHashedString("c");
  static ezHashedString s_sD = ezMakeHashedString("d");
  static ezHashedString s_sOutput = ezMakeHashedString("output");

  static ezUniquePtr<ezExpressionParser> s_pParser;
  static ezUniquePtr<ezExpressionCompiler> s_pCompiler;
  static ezUniquePtr<ezExpressionVM> s_pVM;

  template <typename T>
  struct StreamDataTypeDeduction
  {
  };

  template <>
  struct StreamDataTypeDeduction<float>
  {
    static constexpr ezProcessingStream::DataType Type = ezProcessingStream::DataType::Float;
  };

  template <>
  struct StreamDataTypeDeduction<int>
  {
    static constexpr ezProcessingStream::DataType Type = ezProcessingStream::DataType::Int;
  };

  template <typename T>
  void Compile(ezStringView code, ezExpressionByteCode& out_ByteCode, bool dumpASTs = false)
  {
    ezExpressionParser::Stream inputs[] = {
      ezExpressionParser::Stream(s_sA, StreamDataTypeDeduction<T>::Type),
      ezExpressionParser::Stream(s_sB, StreamDataTypeDeduction<T>::Type),
      ezExpressionParser::Stream(s_sC, StreamDataTypeDeduction<T>::Type),
      ezExpressionParser::Stream(s_sD, StreamDataTypeDeduction<T>::Type),
    };

    ezExpressionParser::Stream outputs[] = {
      ezExpressionParser::Stream(s_sOutput, StreamDataTypeDeduction<T>::Type),
    };

    ezExpressionAST ast;
    EZ_TEST_BOOL(s_pParser->Parse(code, inputs, outputs, {}, ast).Succeeded());

    if (dumpASTs)
    {
      DumpAST(ast, "ParserTest");
    }

    EZ_TEST_BOOL(s_pCompiler->Compile(ast, out_ByteCode).Succeeded());

    if (dumpASTs)
    {
      DumpAST(ast, "ParserTest_Opt");
    }
  }

  template <typename T>
  T Execute(const ezExpressionByteCode& byteCode, T a = 0, T b = 0, T c = 0, T d = 0)
  {
    ezProcessingStream inputs[] = {
      ezProcessingStream(s_sA, ezMakeArrayPtr(&a, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      ezProcessingStream(s_sB, ezMakeArrayPtr(&b, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      ezProcessingStream(s_sC, ezMakeArrayPtr(&c, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      ezProcessingStream(s_sD, ezMakeArrayPtr(&d, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
    };

    T output = ezMath::NaN<float>();
    ezProcessingStream outputs[] = {
      ezProcessingStream(s_sOutput, ezMakeArrayPtr(&output, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
    };

    EZ_TEST_BOOL(s_pVM->Execute(byteCode, inputs, outputs, 1).Succeeded());

    return output;
  };

  template <typename T>
  bool CompareCode(ezStringView testCode, ezString referenceCode, ezExpressionByteCode& out_testByteCode)
  {
    Compile<T>(testCode, out_testByteCode);

    ezExpressionByteCode referenceByteCode;
    Compile<T>(referenceCode, referenceByteCode);

    return CompareByteCode(out_testByteCode, referenceByteCode);
  }

} // namespace

EZ_CREATE_SIMPLE_TEST(CodeUtils, Expression)
{
  s_uiNumByteCodeComparisons = 0;

  ezStringBuilder outputPath = ezTestFramework::GetInstance()->GetAbsOutputPath();
  EZ_TEST_BOOL(ezFileSystem::AddDataDirectory(outputPath.GetData(), "test", "output", ezFileSystem::AllowWrites) == EZ_SUCCESS);

  s_pParser = EZ_DEFAULT_NEW(ezExpressionParser);
  s_pCompiler = EZ_DEFAULT_NEW(ezExpressionCompiler);
  s_pVM = EZ_DEFAULT_NEW(ezExpressionVM);
  EZ_SCOPE_EXIT(s_pParser = nullptr; s_pCompiler = nullptr; s_pVM = nullptr;);

  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Local variables")
  {
    ezExpressionByteCode referenceByteCode;
    {
      ezStringView code = "output = (a + b) * 2";
      Compile<float>(code, referenceByteCode);
    }

    ezExpressionByteCode testByteCode;

    ezStringView code = "var e = a + b; output = e * 2";
    Compile<float>(code, testByteCode);
    EZ_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; e = e * 2; output = e";
    Compile<float>(code, testByteCode);
    EZ_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; e *= 2; output = e";
    Compile<float>(code, testByteCode);
    EZ_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; var f = e; e = 2; output = f * e";
    Compile<float>(code, testByteCode);
    EZ_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    const float a = 2;
    const float b = 3;
    EZ_TEST_FLOAT(Execute(testByteCode, a, b), 10.0f, ezMath::DefaultEpsilon<float>());
  }

  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Constant folding")
  {
    ezStringView testCode = "var x = abs(-7) + saturate(2) + 2\n"
                            "var v = (sqrt(25) - 4) * 5\n"
                            "var m = min(300, 1000) / max(1, 3);"
                            "var r = m - x * 5 - v - clamp(13, 1, 3);\n"
                            "output = r";

    ezStringView referenceCode = "output = 42";

    ezExpressionByteCode testByteCode;
    EZ_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));

    EZ_TEST_FLOAT(Execute<float>(testByteCode), 42.0f, ezMath::DefaultEpsilon<float>());
  }

  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Constant instructions")
  {
    // There are special instructions in the vm which take the constant as the first operand in place and
    // don't require an extra mov for the constant.
    // This test checks whether the compiler transforms operations with constants as second operands to the preferred form.

    ezStringView testCode = "output = (a + 2) + (b - 1) + (c * 2) + (d / 10) + min(c, 1) + max(d, 2)";

    ezStringView referenceCode = "output = (2 + a) + (-1 + b) + (2 * c) + (0.1 * d) + min(1, c) + max(2, d)";

    ezExpressionByteCode testByteCode;
    EZ_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));

    const float a = 1;
    const float b = 2;
    const float c = 3;
    const float d = 40;
    EZ_TEST_FLOAT(Execute(testByteCode, a, b, c, d), 55.0f, ezMath::DefaultEpsilon<float>());
  }

  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Integer and float conversions")
  {
    ezStringView testCode = "var x = 7; var y = 0.1\n"
                            "var e = a * x + b * y\n"
                            "output = e";

    ezStringView referenceCode = "output = (a * 7) + (b * 0.1)";

    ezExpressionByteCode testByteCode;
    EZ_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));

    EZ_TEST_INT(Execute<int>(testByteCode), 42.0f);
  }

#if 0
  EZ_TEST_BLOCK(ezTestBlock::Enabled, "Scalarization")
  {
    // The VM does only support scalar data types.
    // This test checks whether the compiler transforms everything correctly to scalar operation.

    ezExpressionByteCode referenceByteCode;
    {
      ezStringView code = "output = a * vec3(1, 2, 3) + b";
      Compile<ezVec3>(code, referenceByteCode);
    }

    ezExpressionByteCode testByteCode;

    ezStringView code = "output = a * vec3(1, 2, 3) + b";
    Compile<ezVec3>(code, testByteCode);
    EZ_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));
  }
#endif
}
