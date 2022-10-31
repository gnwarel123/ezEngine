#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClassResource.h>
#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>

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
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScriptClassResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezScriptClassResource);
// clang-format on

ezScriptClassResource::ezScriptClassResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}

ezScriptClassResource::~ezScriptClassResource() = default;

void ezScriptClassResource::CreateScriptType(const char* szName, const ezRTTI* pParentType)
{
  auto pBaseClass = ezScriptClassRegistry::GetInstance()->GetBaseClass(pParentType->GetTypeName());
  if (pBaseClass == nullptr)
  {
    ezLog::Error("Script base class '{}' not found.", pParentType->GetTypeName());
    return;
  }

  m_pType = EZ_DEFAULT_NEW(ezScriptRTTI, szName, pParentType, m_Functions, m_MessageHandlers);
  m_pFunctionTable = pBaseClass->m_FunctionTableCreator(*m_pType);
}

void ezScriptClassResource::DeleteScriptType()
{
  m_pType = nullptr;
  m_pFunctionTable = nullptr;

  m_Functions.Clear();
  m_MessageHandlers.Clear();
}
