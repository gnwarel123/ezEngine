#pragma once

#include <Core/Scripting/ScriptResource.h>
#include <TypeScriptPlugin/TypeScriptPluginDLL.h>

class ezComponent;
class ezTypeScriptBinding;

class EZ_TYPESCRIPTPLUGIN_DLL ezTypeScriptInstance : public ezScriptInstance
{
public:
  ezTypeScriptInstance(ezComponent& owner, ezTypeScriptBinding& binding);

  virtual void ApplyParameters(const ezArrayMap<ezHashedString, ezVariant>& parameters) override;

  ezTypeScriptBinding& GetBinding() { return m_Binding; }

  ezComponent& GetComponent() { return m_Component; }

private:
  ezTypeScriptBinding& m_Binding;
  ezComponent& m_Component;
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

  virtual ezUniquePtr<ezScriptInstance> Instantiate(ezReflectedClass& owner, ezWorld& world) const override;

private:
  ezUuid m_Guid;
};
