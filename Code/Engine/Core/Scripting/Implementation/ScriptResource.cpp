#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptResource.h>
#include <Core/World/Component.h>

ezScriptRTTI::ezScriptRTTI(const char* szName, const ezRTTI* pParentType, ezArrayPtr<ezAbstractProperty*> functions, ezArrayPtr<ezAbstractMessageHandler*> messageHandlers)
  : ezRTTI(nullptr, pParentType, 0, 1, ezVariantType::Invalid, ezTypeFlags::Class, nullptr, ezArrayPtr<ezAbstractProperty*>(), functions, ezArrayPtr<ezPropertyAttribute*>(), messageHandlers, ezArrayPtr<ezMessageSenderInfo>(), nullptr)
{
  m_sTypeNameStorage = szName;
  m_szTypeName = m_sTypeNameStorage.GetData();

  RegisterType();

  SetupParentHierarchy();
  GatherDynamicMessageHandlers();
}

ezScriptRTTI::~ezScriptRTTI()
{
  UnregisterType();
  m_szTypeName = nullptr;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScriptResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezScriptResource);
// clang-format on

ezScriptResource::ezScriptResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}

void ezScriptResource::CreateScriptType(const char* szName, const ezRTTI* pParentType)
{
  m_pType = EZ_DEFAULT_NEW(ezScriptRTTI, szName, pParentType, m_Functions, m_MessageHandlers);

  if (pParentType->IsDerivedFrom<ezComponent>())
  {
    auto functionTable = EZ_DEFAULT_NEW(ezScriptFunctionTable_Component);

    for (auto pFunction : m_pType->GetFunctions())
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

    m_pFunctionTable = functionTable;
  }
  else
  {
    EZ_REPORT_FAILURE("Unknown parent type: '{}'", pParentType->GetTypeName());
  }
}

void ezScriptResource::DeleteScriptType()
{
  m_pType = nullptr;
  m_pFunctionTable = nullptr;

  m_Functions.Clear();
  m_MessageHandlers.Clear();
}
