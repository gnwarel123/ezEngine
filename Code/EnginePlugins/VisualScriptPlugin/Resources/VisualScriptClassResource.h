#pragma once

#include <Core/Scripting/ScriptClassResource.h>
#include <VisualScriptPlugin/VisualScriptPluginDLL.h>

#if 0
class EZ_VISUALSCRIPTPLUGIN_DLL ezVisualScriptInstance : public ezScriptInstance
{
public:
  ezVisualScriptInstance(ezReflectedClass& owner);

  virtual void ApplyParameters(const ezArrayMap<ezHashedString, ezVariant>& parameters) override;
};
#endif

class EZ_VISUALSCRIPTPLUGIN_DLL ezVisualScriptClassResource : public ezScriptClassResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVisualScriptClassResource, ezScriptClassResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezVisualScriptClassResource);

public:
  ezVisualScriptClassResource();
  ~ezVisualScriptClassResource();

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  virtual ezUniquePtr<ezScriptInstance> Instantiate(ezReflectedClass& owner, ezWorld& world) const override;
};
