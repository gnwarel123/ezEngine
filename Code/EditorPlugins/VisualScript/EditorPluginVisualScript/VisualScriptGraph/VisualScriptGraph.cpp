#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraph.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVisualScriptPin, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezVisualScriptPin::ezVisualScriptPin(Type type, const char* szName, const ezColorGammaUB& color, const ezDocumentObject* pObject)
  : ezPin(type, szName, color, pObject)
{
}

//////////////////////////////////////////////////////////////////////////

ezVisualScriptNodeManager::ezVisualScriptNodeManager() = default;
ezVisualScriptNodeManager::~ezVisualScriptNodeManager() = default;

bool ezVisualScriptNodeManager::InternalIsNode(const ezDocumentObject* pObject) const
{
  /*if (pObject != nullptr)
  {
    auto pType = pObject->GetTypeAccessor().GetType();
    return pType->IsDerivedFrom<ezVisualScriptNodeBase>();
  }*/
  return false;
}

ezStatus ezVisualScriptNodeManager::InternalCanConnect(const ezPin& source, const ezPin& target, CanConnectResult& out_Result) const
{
  out_Result = CanConnectResult::ConnectNtoN;
  return ezStatus(EZ_SUCCESS);
}

void ezVisualScriptNodeManager::InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& node)
{
  if (IsNode(pObject) == false)
    return;
}

void ezVisualScriptNodeManager::GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& Types) const
{
  //Types.PushBack(ezGetStaticRTTI<ezVisualScriptNode>());
  //Types.PushBack(ezGetStaticRTTI<ezVisualScriptNodeAny>());
}
