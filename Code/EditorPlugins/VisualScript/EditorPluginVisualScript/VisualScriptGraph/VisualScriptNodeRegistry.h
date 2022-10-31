#pragma once

#include <Foundation/Configuration/Singleton.h>

class ezVisualScriptNodeRegistry
{
  EZ_DECLARE_SINGLETON(ezVisualScriptNodeRegistry);

public:
  ezVisualScriptNodeRegistry();
  ~ezVisualScriptNodeRegistry();

  const ezRTTI* GetNodeBaseType() const { return m_pBaseType; }

private:
  void PhantomTypeRegistryEventHandler(const ezPhantomRttiManagerEvent& e);
  void UpdateNodeTypes();
  void UpdateNodeType(const ezRTTI* pRtti);

  void CreateFunctionCallNodeType(const ezRTTI* pRtti, const ezAbstractFunctionProperty* pFunction);

  void FillDesc(ezReflectedTypeDescriptor& desc, const ezRTTI* pRtti);

  const ezRTTI* m_pBaseType = nullptr;
  ezHashSet<const ezRTTI*> m_KnownTypes;
};
