#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptBaseClass_Component.h>
#include <Core/Scripting/ScriptComponent.h>
#include <Core/Scripting/ScriptWorldModule.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezScriptComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("UpdateInterval", GetUpdateInterval, SetUpdateInterval)->AddAttributes(new ezClampValueAttribute(ezTime::Zero(), ezVariant())),
    EZ_ACCESSOR_PROPERTY("ScriptClass", GetScriptClassFile, SetScriptClassFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_ScriptClass")),
    EZ_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new ezExposedParametersAttribute("ScriptClass")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Scripting"),
    new ezInDevelopmentAttribute(ezInDevelopmentAttribute::Phase::Alpha),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezScriptComponent::ezScriptComponent() = default;
ezScriptComponent::~ezScriptComponent() = default;

void ezScriptComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);
  auto& s = stream.GetStream();

  s << m_hScriptClass;
  s << m_UpdateInterval;

  ezUInt16 uiNumParams = static_cast<ezUInt16>(m_Parameters.GetCount());
  s << uiNumParams;

  for (ezUInt32 p = 0; p < uiNumParams; ++p)
  {
    s << m_Parameters.GetKey(p);
    s << m_Parameters.GetValue(p);
  }
}

void ezScriptComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  // const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = stream.GetStream();

  s >> m_hScriptClass;
  s >> m_UpdateInterval;

  ezUInt16 uiNumParams = 0;
  s >> uiNumParams;
  m_Parameters.Reserve(uiNumParams);

  ezHashedString key;
  ezVariant value;
  for (ezUInt32 p = 0; p < uiNumParams; ++p)
  {
    s >> key;
    s >> value;

    m_Parameters.Insert(key, value);
  }
}

void ezScriptComponent::Initialize()
{
  SUPER::Initialize();

  if (m_hScriptClass.IsValid())
  {
    InstantiateScript();
  }

  CallScriptFunction(m_pFunctionTable->m_pInitializeFunction);
}

void ezScriptComponent::Deinitialize()
{
  SUPER::Deinitialize();

  CallScriptFunction(m_pFunctionTable->m_pDeinitializeFunction);
}

void ezScriptComponent::OnActivated()
{
  SUPER::OnActivated();

  CallScriptFunction(m_pFunctionTable->m_pOnActivatedFunction);
}

void ezScriptComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  CallScriptFunction(m_pFunctionTable->m_pOnDeactivatedFunction);
}

void ezScriptComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  CallScriptFunction(m_pFunctionTable->m_pOnSimulationStartedFunction);
}

void ezScriptComponent::BroadcastEventMsg(ezEventMessage& msg)
{
  const ezRTTI* pType = msg.GetDynamicRTTI();
  for (auto& sender : m_EventSenders)
  {
    if (sender.m_pMsgType == pType)
    {
      sender.m_Sender.SendEventMessage(msg, this, GetOwner());
      return;
    }
  }

  auto& sender = m_EventSenders.ExpandAndGetRef();
  sender.m_pMsgType = pType;
  sender.m_Sender.SendEventMessage(msg, this, GetOwner());
}

void ezScriptComponent::SetScriptClass(const ezScriptClassResourceHandle& hScript)
{
  if (m_hScriptClass == hScript)
    return;

  m_hScriptClass = hScript;

  if (IsActiveAndInitialized() && m_hScriptClass.IsValid())
  {
    InstantiateScript();
  }
}

void ezScriptComponent::SetScriptClassFile(const char* szFile)
{
  ezScriptClassResourceHandle hScript;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hScript = ezResourceManager::LoadResource<ezScriptClassResource>(szFile);
  }

  SetScriptClass(hScript);
}

const char* ezScriptComponent::GetScriptClassFile() const
{
  return m_hScriptClass.IsValid() ? m_hScriptClass.GetResourceID().GetData() : "";
}

void ezScriptComponent::SetUpdateInterval(ezTime interval)
{
  m_UpdateInterval = interval;

  if (IsActiveAndInitialized())
  {
    UpdateScheduling();
  }
}

ezTime ezScriptComponent::GetUpdateInterval() const
{
  return m_UpdateInterval;
}

const ezRangeView<const char*, ezUInt32> ezScriptComponent::GetParameters() const
{
  return ezRangeView<const char*, ezUInt32>([]() -> ezUInt32
    { return 0; },
    [this]() -> ezUInt32
    { return m_Parameters.GetCount(); },
    [](ezUInt32& it)
    { ++it; },
    [this](const ezUInt32& it) -> const char*
    { return m_Parameters.GetKey(it).GetString().GetData(); });
}

void ezScriptComponent::SetParameter(const char* szKey, const ezVariant& value)
{
  ezHashedString hs;
  hs.Assign(szKey);

  auto it = m_Parameters.Find(hs);
  if (it != ezInvalidIndex && m_Parameters.GetValue(it) == value)
    return;

  m_Parameters[hs] = value;
}

void ezScriptComponent::RemoveParameter(const char* szKey)
{
  if (m_Parameters.RemoveAndCopy(ezTempHashedString(szKey)))
  {
  }
}

bool ezScriptComponent::GetParameter(const char* szKey, ezVariant& out_value) const
{
  ezUInt32 it = m_Parameters.Find(szKey);

  if (it == ezInvalidIndex)
    return false;

  out_value = m_Parameters.GetValue(it);
  return true;
}

void ezScriptComponent::InstantiateScript()
{
  ClearInstance();

  ezResourceLock<ezScriptClassResource> pScript(m_hScriptClass, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pScript.GetAcquireResult() != ezResourceAcquireResult::Final)
  {
    ezLog::Error("Failed to load script '{}'", GetScriptClassFile());
    return;
  }

  const ezRTTI* pScriptType = pScript->GetType();
  if (pScriptType->IsDerivedFrom(ezGetStaticRTTI<ezComponent>()) == false)
  {
    ezLog::Error("Script type '{}' is not a component", pScriptType->GetTypeName());
    return;
  }

  m_pFunctionTable = pScript->GetFunctionTable<ezScriptFunctionTable_Component>();
  m_pMessageDispatchType = pScriptType;

  m_pInstance = pScript->Instantiate(*this, *GetWorld());
  if (m_pInstance != nullptr)
  {
    m_pInstance->ApplyParameters(m_Parameters);
  }

  UpdateScheduling();
}

void ezScriptComponent::ClearInstance()
{
  if (m_pFunctionTable != nullptr && m_pFunctionTable->m_pUpdateFunction != nullptr && m_pInstance != nullptr)
  {
    auto pModule = GetWorld()->GetOrCreateModule<ezScriptWorldModule>();
    pModule->RemoveUpdateFunctionToSchedule(m_pFunctionTable->m_pUpdateFunction, m_pInstance.Borrow());
  }

  m_pInstance = nullptr;
  m_pFunctionTable = nullptr;

  m_pMessageDispatchType = GetDynamicRTTI();
}

void ezScriptComponent::CallScriptFunction(ezAbstractFunctionProperty* pFunction)
{
  if (m_pInstance != nullptr && pFunction != nullptr)
  {
    ezVariant returnValue;
    pFunction->Execute(m_pInstance.Borrow(), ezArrayPtr<ezVariant>(), returnValue);
  }
}

void ezScriptComponent::UpdateScheduling()
{
  if (m_pFunctionTable != nullptr && m_pFunctionTable->m_pUpdateFunction != nullptr && m_pInstance != nullptr)
  {
    auto pModule = GetWorld()->GetOrCreateModule<ezScriptWorldModule>();
    pModule->AddUpdateFunctionToSchedule(m_pFunctionTable->m_pUpdateFunction, m_pInstance.Borrow(), m_UpdateInterval);
  }
}
