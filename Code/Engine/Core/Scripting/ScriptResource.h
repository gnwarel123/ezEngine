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
};

class EZ_CORE_DLL ezScriptRTTI : public ezRTTI
{
public:
  ezScriptRTTI(const char* szName, const ezRTTI* pParentType, ezArrayPtr<ezAbstractProperty*> functions, ezArrayPtr<ezAbstractMessageHandler*> messageHandlers);
  ~ezScriptRTTI();

private:
  ezString m_sTypeNameStorage;
};

class EZ_CORE_DLL ezScriptResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezScriptResource);

public:
  ezScriptResource();

  const ezRTTI* GetType() const { return m_pType.Borrow(); }

  virtual bool InstantiateWhenSimulationStarted() const { return false; }
  virtual ezUniquePtr<ezScriptInstance> Instantiate(const ezReflectedClass* pContext) const = 0;

protected:
  void CreateScriptType(const char* szName, const ezRTTI* pParentType);

  ezHybridArray<ezAbstractProperty*, 8> m_Functions;
  ezHybridArray<ezAbstractMessageHandler*, 8> m_MessageHandlers;
  ezUniquePtr<ezScriptRTTI> m_pType;
};
