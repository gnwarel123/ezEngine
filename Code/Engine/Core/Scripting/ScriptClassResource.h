#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/ArrayMap.h>

class ezWorld;
struct ezScriptFunctionTable_Base;
using ezScriptClassResourceHandle = ezTypedResourceHandle<class ezScriptClassResource>;

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

class EZ_CORE_DLL ezScriptClassResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptClassResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezScriptClassResource);

public:
  ezScriptClassResource();
  ~ezScriptClassResource();

  const ezRTTI* GetType() const { return m_pType.Borrow(); }

  template<typename T>
  const T* GetFunctionTable() const { return static_cast<const T*>(m_pFunctionTable.Borrow()); }

  virtual ezUniquePtr<ezScriptInstance> Instantiate(ezReflectedClass& owner, ezWorld& world) const = 0;

protected:
  void CreateScriptType(const char* szName, const ezRTTI* pParentType);
  void DeleteScriptType();

  ezHybridArray<ezAbstractProperty*, 8> m_Functions;
  ezHybridArray<ezAbstractMessageHandler*, 8> m_MessageHandlers;
  ezUniquePtr<ezScriptRTTI> m_pType;
  ezUniquePtr<ezScriptFunctionTable_Base> m_pFunctionTable = nullptr;
};
