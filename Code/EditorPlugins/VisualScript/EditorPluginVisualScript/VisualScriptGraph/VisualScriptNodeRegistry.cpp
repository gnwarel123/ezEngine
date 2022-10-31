#include <EditorPluginVisualScript/EditorPluginVisualScriptPCH.h>

#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptNodeRegistry.h>
#include <Foundation/Profiling/Profiling.h>
#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>

constexpr const char* szPluginName = "EditorPluginVisualScript";

const ezRTTI* FindTopMostBaseClass(const ezRTTI* pRtti)
{
  const ezRTTI* pReflectedClass = ezGetStaticRTTI<ezReflectedClass>();
  while (pRtti->GetParentType() != nullptr && pRtti->GetParentType() != pReflectedClass)
  {
    pRtti = pRtti->GetParentType();
  }
  return pRtti;
}

ezStringView StripTypeName(ezStringView sTypeName)
{
  if (sTypeName.StartsWith("ez"))
  {
    sTypeName.Shrink(2, 0);
  }
  return sTypeName;
}

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_SINGLETON(ezVisualScriptNodeRegistry);

ezVisualScriptNodeRegistry::ezVisualScriptNodeRegistry()
  : m_SingletonRegistrar(this)
{
  ezPhantomRttiManager::s_Events.AddEventHandler(ezMakeDelegate(&ezVisualScriptNodeRegistry::PhantomTypeRegistryEventHandler, this));

  UpdateNodeTypes();
}

ezVisualScriptNodeRegistry::~ezVisualScriptNodeRegistry()
{
  ezPhantomRttiManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezVisualScriptNodeRegistry::PhantomTypeRegistryEventHandler, this));
}

void ezVisualScriptNodeRegistry::PhantomTypeRegistryEventHandler(const ezPhantomRttiManagerEvent& e)
{
  if ((e.m_Type == ezPhantomRttiManagerEvent::Type::TypeAdded && m_KnownTypes.Contains(e.m_pChangedType) == false) ||
    e.m_Type == ezPhantomRttiManagerEvent::Type::TypeChanged)
  {
    UpdateNodeType(e.m_pChangedType);
  }
}

void ezVisualScriptNodeRegistry::UpdateNodeTypes()
{
  EZ_PROFILE_SCOPE("Update VS Node Types");

  // Base Node Type
  if (m_pBaseType == nullptr)
  {
    ezReflectedTypeDescriptor desc;
    desc.m_sTypeName = "ezVisualScriptNodeBase";
    desc.m_sPluginName = szPluginName;
    desc.m_sParentTypeName = ezGetStaticRTTI<ezReflectedClass>()->GetTypeName();
    desc.m_Flags = ezTypeFlags::Phantom | ezTypeFlags::Abstract | ezTypeFlags::Class;

    m_pBaseType = ezPhantomRttiManager::RegisterType(desc);
  }

  for (const ezRTTI* pRtti = ezRTTI::GetFirstInstance(); pRtti != nullptr; pRtti = pRtti->GetNextInstance())
  {
    UpdateNodeType(pRtti);
  }
}

void ezVisualScriptNodeRegistry::UpdateNodeType(const ezRTTI* pRtti)
{
  if (pRtti->GetTypeFlags().IsAnySet(ezTypeFlags::Abstract) || pRtti->GetAttributeByType<ezHiddenAttribute>() != nullptr)
    return;  

  // expose reflected functions and properties to visual scripts
  {
    for (const ezAbstractFunctionProperty* pFuncProp : pRtti->GetFunctions())
    {
      CreateFunctionCallNodeType(pRtti, pFuncProp);
    }
  }
}

void ezVisualScriptNodeRegistry::CreateFunctionCallNodeType(const ezRTTI* pRtti, const ezAbstractFunctionProperty* pFunction)
{
  const ezScriptableFunctionAttribute* pScriptableFunctionAttribute = pFunction->GetAttributeByType<ezScriptableFunctionAttribute>();
  if (pScriptableFunctionAttribute == nullptr)
    return;

  ezReflectedTypeDescriptor desc;
  FillDesc(desc, pRtti);

  ezStringBuilder temp;
  temp.Set(desc.m_sTypeName, "::", pFunction->GetPropertyName());
  desc.m_sTypeName = temp;  
  
  m_KnownTypes.Insert(ezPhantomRttiManager::RegisterType(desc));
}

void ezVisualScriptNodeRegistry::FillDesc(ezReflectedTypeDescriptor& desc, const ezRTTI* pRtti)
{
  ezStringBuilder sTypeName = pRtti->GetTypeName();
  if (auto pScriptExtension = pRtti->GetAttributeByType<ezScriptExtension>())
  {
    sTypeName = pScriptExtension->m_sTypeName;
  }
  sTypeName = StripTypeName(sTypeName);

  ezStringBuilder sCategory;
  const ezRTTI* pBaseClass = FindTopMostBaseClass(pRtti);
  if (pBaseClass != pRtti)
  {
    sCategory.Append(StripTypeName(pBaseClass->GetTypeName()), "/", sTypeName);
  }
  else
  {
    sCategory = sTypeName;
  }

  sTypeName.Prepend("VisualScriptNode_");

  desc.m_sTypeName = sTypeName;
  desc.m_sPluginName = szPluginName;
  desc.m_sParentTypeName = m_pBaseType->GetTypeName();
  desc.m_Flags = ezTypeFlags::Phantom | ezTypeFlags::Class;

  auto pAttr = EZ_DEFAULT_NEW(ezCategoryAttribute, sCategory);
  desc.m_Attributes.PushBack(pAttr);
}
