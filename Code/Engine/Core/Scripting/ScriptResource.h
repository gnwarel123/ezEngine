#pragma once

#include <Core/CoreDLL.h>
#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/ArrayMap.h>

using ezScriptResourceHandle = ezTypedResourceHandle<class ezScriptResource>;

class EZ_CORE_DLL ezScriptInstance
{
public:
  virtual ~ezScriptInstance() {}
  virtual void ApplyParameters(const ezArrayMap<ezHashedString, ezVariant>& parameters) = 0;
  virtual const ezRTTI* GetType() const = 0;
};

class EZ_CORE_DLL ezScriptResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezScriptResource);

public:
  ezScriptResource();

  virtual bool InstantiateWhenSimulationStarted() const { return false; }
  virtual ezUniquePtr<ezScriptInstance> Instantiate(const ezReflectedClass* pContext) const = 0;
};
