#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptExtensions/ScriptExtensionRegistry.h>

ezScriptExtensionRegistry::ezScriptExtensionRegistry() = default;

// static
ezScriptExtensionRegistry* ezScriptExtensionRegistry::GetInstance()
{
  static ezScriptExtensionRegistry* pInstance = new ezScriptExtensionRegistry();
  return pInstance;
}

bool ezScriptExtensionRegistry::RegisterScriptExtension(const ezRTTI* pRtti)
{
  return true;
}
