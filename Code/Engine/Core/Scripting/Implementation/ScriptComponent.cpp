#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptComponent.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezScriptComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("UpdateInterval", GetUpdateInterval, SetUpdateInterval)->AddAttributes(new ezClampValueAttribute(ezTime::Zero(), ezVariant())),
    EZ_ACCESSOR_PROPERTY("Script", GetScriptFile, SetScriptFile)->AddAttributes(new ezAssetBrowserAttribute("Script")),
    EZ_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new ezExposedParametersAttribute("Script")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Scripting"),
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

  s << m_hScript;
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

  s >> m_hScript;
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

void ezScriptComponent::SetScript(const ezScriptResourceHandle& hScript)
{
  m_hScript = hScript;
}

void ezScriptComponent::SetScriptFile(const char* szFile)
{
  ezScriptResourceHandle hScript;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hScript = ezResourceManager::LoadResource<ezScriptResource>(szFile);
  }

  SetScript(hScript);
}

const char* ezScriptComponent::GetScriptFile() const
{
  return m_hScript.IsValid() ? m_hScript.GetResourceID().GetData() : "";
}

void ezScriptComponent::SetUpdateInterval(ezTime interval)
{
  m_UpdateInterval = interval;
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
    [this](const ezUInt32& it) -> const char* { return m_Parameters.GetKey(it).GetString().GetData(); });
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
  ClearCaches();

  ezResourceLock<ezScriptResource> pScript(m_hScript, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pScript.GetAcquireResult() == ezResourceAcquireResult::MissingFallback)
  {
    ezLog::Error("Failed to load script '{}'", GetScriptFile());
    return;
  }

  const ezRTTI* pScriptType = pScript->GetScriptType();
  if (pScriptType->IsDerivedFrom(ezGetStaticRTTI<ezComponent>()) == false)
  {
    ezLog::Error("Script type '{}' is not a component", pScriptType->GetTypeName());
    return;
  }

  for (auto pFunction : pScriptType->GetFunctions())
  {
    // only void function with 0 arguments
    if (pFunction->GetReturnType() != nullptr || pFunction->GetArgumentCount() > 0)
      continue;

    ezTempHashedString sFunctionName(pFunction->GetPropertyName());

    if (sFunctionName == ezTempHashedString("Initialize"))
    {
      m_pInitializeFunction = pFunction;
    }
  }

  m_pMessageDispatchType = pScriptType;
  
  m_pContext = pScript->CreateScriptContext();
  m_pContext->ApplyParameters(m_Parameters);  
}

void ezScriptComponent::ClearCaches()
{
  m_pContext = nullptr;
  m_pInitializeFunction = nullptr;

  m_pMessageDispatchType = GetDynamicRTTI();
}
