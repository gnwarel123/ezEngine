#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/Reflection.h>

class EZ_CORE_DLL ezScriptExtensionRegistry
{
public:
  static ezScriptExtensionRegistry* GetInstance();

  template <typename T>
  EZ_ALWAYS_INLINE bool RegisterScriptExtension()
  {
    return RegisterScriptExtension(ezGetStaticRTTI<T>());
  }

private:
  EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, ScriptExtensionRegistry);

  ezScriptExtensionRegistry();
  bool RegisterScriptExtension(const ezRTTI* pRtti);

  static void PluginEventHandler(const ezPluginEvent& EventData);
  void ClearUnloadedTypes();
};

#define EZ_REGISTER_SCRIPT_EXTENSION(T) \
  bool EZ_CONCAT(_scriptExt, EZ_SOURCE_LINE) = ezScriptExtensionRegistry::GetInstance()->RegisterScriptExtension<T>();
