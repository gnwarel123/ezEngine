#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/UniquePtr.h>

struct ezScriptFunctionTable_Base
{
  EZ_DECLARE_POD_TYPE();
};

class EZ_CORE_DLL ezScriptClassRegistry
{
public:
  static ezScriptClassRegistry* GetInstance();

  using FunctionTableCreator = ezUniquePtr<ezScriptFunctionTable_Base> (*)(const ezRTTI& type);

  struct BaseClass
  {
    const ezRTTI* m_pParentType = nullptr;
    FunctionTableCreator m_FunctionTableCreator = nullptr;
  };

  const BaseClass* GetBaseClass(ezStringView sClassName) const;



  bool RegisterScriptBaseClass(const ezRTTI* pType, FunctionTableCreator functionTableCreator);

private:
  EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, ScriptExtensionRegistry);

  ezScriptClassRegistry();
  bool RegisterScriptExtensionClass(const ezRTTI* pRtti);

  static void PluginEventHandler(const ezPluginEvent& EventData);
  void ClearUnloadedTypes();

  ezHashTable<ezHashedString, BaseClass> m_BaseClasses;
};

#define EZ_REGISTER_SCRIPT_BASE_CLASS(T, func) \
  bool EZ_CONCAT(_scriptBaseClass, EZ_SOURCE_LINE) = ezScriptClassRegistry::GetInstance()->RegisterScriptBaseClass(ezGetStaticRTTI<T>(), func)
