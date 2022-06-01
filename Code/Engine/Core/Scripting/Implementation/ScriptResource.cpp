#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptResource.h>

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
}
