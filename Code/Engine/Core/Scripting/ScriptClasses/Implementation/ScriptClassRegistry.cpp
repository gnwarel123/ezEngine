#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScriptExtension, 1, ezRTTIDefaultAllocator<ezScriptExtension>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("TypeName", m_sTypeName),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


ezScriptClassRegistry::ezScriptClassRegistry() = default;

// static
ezScriptClassRegistry* ezScriptClassRegistry::GetInstance()
{
  static ezScriptClassRegistry* pInstance = new ezScriptClassRegistry();
  return pInstance;
}

const ezScriptClassRegistry::BaseClass* ezScriptClassRegistry::GetBaseClass(ezStringView sClassName) const
{
  ezTempHashedString sClassNameHashed(sClassName);
  return m_BaseClasses.GetValue(sClassNameHashed);
}

bool ezScriptClassRegistry::RegisterScriptBaseClass(const ezRTTI* pType, FunctionTableCreator functionTableCreator)
{
  ezHashedString sClassNameHashed;
  sClassNameHashed.Assign(pType->GetParentType()->GetTypeName());

  BaseClass baseClass;
  baseClass.m_pParentType = pType;
  baseClass.m_FunctionTableCreator = functionTableCreator;

  return m_BaseClasses.Insert(sClassNameHashed, baseClass);
}
