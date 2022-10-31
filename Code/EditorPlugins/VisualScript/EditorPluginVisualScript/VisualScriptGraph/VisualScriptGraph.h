#pragma once

#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>

class ezVisualScriptPin : public ezPin
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVisualScriptPin, ezPin);

public:
  ezVisualScriptPin(Type type, const char* szName, const ezColorGammaUB& color, const ezDocumentObject* pObject);
};

class ezVisualScriptNodeManager : public ezDocumentNodeManager
{
public:
  ezVisualScriptNodeManager();
  ~ezVisualScriptNodeManager();

private:
  virtual bool InternalIsNode(const ezDocumentObject* pObject) const override;
  virtual ezStatus InternalCanConnect(const ezPin& source, const ezPin& target, CanConnectResult& out_Result) const override;

  virtual void InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& node) override;

  virtual void GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& Types) const override;
};
