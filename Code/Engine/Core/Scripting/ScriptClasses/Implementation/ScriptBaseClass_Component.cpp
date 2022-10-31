#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptBaseClass_Component.h>
#include <Foundation/Strings/HashedString.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezScriptBaseClass_Component, ezComponent, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsActive", IsActive),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

static ezUniquePtr<ezScriptFunctionTable_Base> BuildComponentFunctionTable(const ezRTTI& type)
{
  auto functionTable = EZ_DEFAULT_NEW(ezScriptFunctionTable_Component);

  for (auto pFunction : type.GetFunctions())
  {
    // only void function with 0 arguments
    if (pFunction->GetReturnType() != nullptr || pFunction->GetArgumentCount() > 0)
      continue;

    ezTempHashedString sFunctionName(pFunction->GetPropertyName());

    if (sFunctionName == ezTempHashedString("Initialize"))
    {
      functionTable->m_pInitializeFunction = pFunction;
    }
    else if (sFunctionName == ezTempHashedString("Deinitialize"))
    {
      functionTable->m_pDeinitializeFunction = pFunction;
    }
    else if (sFunctionName == ezTempHashedString("OnActivated"))
    {
      functionTable->m_pOnActivatedFunction = pFunction;
    }
    else if (sFunctionName == ezTempHashedString("OnDeactivated"))
    {
      functionTable->m_pOnDeactivatedFunction = pFunction;
    }
    else if (sFunctionName == ezTempHashedString("OnSimulationStarted"))
    {
      functionTable->m_pOnSimulationStartedFunction = pFunction;
    }
    else if (sFunctionName == ezTempHashedString("Update") || sFunctionName == ezTempHashedString("Tick"))
    {
      functionTable->m_pUpdateFunction = pFunction;
    }
  }

  return functionTable;
}

EZ_REGISTER_SCRIPT_BASE_CLASS(ezScriptBaseClass_Component, &BuildComponentFunctionTable);
