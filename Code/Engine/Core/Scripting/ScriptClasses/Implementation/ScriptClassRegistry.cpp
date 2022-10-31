#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>

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
