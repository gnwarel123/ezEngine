#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraph.h>
#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraphQt.moc.h>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(EditorPluginVisualScript, Factories)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezQtNodeScene::GetPinFactory().RegisterCreator(ezGetStaticRTTI<ezVisualScriptPin>(), [](const ezRTTI* pRtti)->ezQtPin* { return new ezQtVisualScriptPin(); });
    /*ezQtNodeScene::GetConnectionFactory().RegisterCreator(ezGetStaticRTTI<ezVisualScriptConnection>(), [](const ezRTTI* pRtti)->ezQtConnection* { return new ezQtVisualScriptConnection(); });    
    ezQtNodeScene::GetNodeFactory().RegisterCreator(ezGetStaticRTTI<ezVisualScriptNodeBase>(), [](const ezRTTI* pRtti)->ezQtNode* { return new ezQtVisualScriptNode(); });*/
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezQtNodeScene::GetPinFactory().UnregisterCreator(ezGetStaticRTTI<ezVisualScriptPin>());
    /*ezQtNodeScene::GetConnectionFactory().UnregisterCreator(ezGetStaticRTTI<ezVisualScriptConnection>());
    ezQtNodeScene::GetNodeFactory().UnregisterCreator(ezGetStaticRTTI<ezVisualScriptNodeBase>());*/
  }

EZ_END_SUBSYSTEM_DECLARATION;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptPin::ezQtVisualScriptPin() = default;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptConnection::ezQtVisualScriptConnection() = default;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptNode::ezQtVisualScriptNode() = default;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptNodeScene::ezQtVisualScriptNodeScene(QObject* parent /*= nullptr*/)
  : ezQtNodeScene(parent)
{
}

ezQtVisualScriptNodeScene::~ezQtVisualScriptNodeScene() = default;
