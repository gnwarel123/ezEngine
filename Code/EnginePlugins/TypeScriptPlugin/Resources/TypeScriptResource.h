#pragma once

#include <Core/Scripting/ScriptResource.h>
#include <TypeScriptPlugin/TypeScriptPluginDLL.h>

class EZ_TYPESCRIPTPLUGIN_DLL ezTypeScriptInstance : public ezScriptInstance
{
public:
  virtual void ApplyParameters(const ezArrayMap<ezHashedString, ezVariant>& parameters) override;
};

class EZ_TYPESCRIPTPLUGIN_DLL ezTypeScriptResource : public ezScriptResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTypeScriptResource, ezScriptResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezTypeScriptResource);

public:
  ezTypeScriptResource();
  ~ezTypeScriptResource();

  private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  virtual bool InstantiateWhenSimulationStarted() const override { return true; }
  virtual ezUniquePtr<ezScriptInstance> Instantiate(const ezReflectedClass* pContext) const override;
};
