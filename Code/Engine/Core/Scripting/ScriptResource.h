#pragma once

#include <Core/CoreDLL.h>
#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/ArrayMap.h>

class ezWorld;

using ezScriptResourceHandle = ezTypedResourceHandle<class ezScriptResource>;

struct ezScriptFunctionTable_Base
{
  EZ_DECLARE_POD_TYPE();
};

struct ezScriptFunctionTable_Component : public ezScriptFunctionTable_Base
{
  ezAbstractFunctionProperty* m_pInitializeFunction = nullptr;
  ezAbstractFunctionProperty* m_pDeinitializeFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnActivatedFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnDeactivatedFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnSimulationStartedFunction = nullptr;
  ezAbstractFunctionProperty* m_pUpdateFunction = nullptr;
};

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

  const ezScriptFunctionTable_Base* GetFunctionTable() const { return m_pFunctionTable.Borrow(); }

  //virtual bool InstantiateWhenSimulationStarted() const { return false; }
  virtual ezUniquePtr<ezScriptInstance> Instantiate(ezReflectedClass& owner, ezWorld& world) const = 0;

protected:
  void CreateScriptType(const char* szName, const ezRTTI* pParentType);
  void DeleteScriptType();

  ezHybridArray<ezAbstractProperty*, 8> m_Functions;
  ezHybridArray<ezAbstractMessageHandler*, 8> m_MessageHandlers;
  ezUniquePtr<ezScriptRTTI> m_pType;
  ezUniquePtr<ezScriptFunctionTable_Base> m_pFunctionTable = nullptr;
};
