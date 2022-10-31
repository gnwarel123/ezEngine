#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraph.h>
#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraphQt.moc.h>
#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptNodeRegistry.h>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(EditorPluginVisualScript, Factories)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    EZ_DEFAULT_NEW(ezVisualScriptNodeRegistry);
    const ezRTTI* pBaseType = ezVisualScriptNodeRegistry::GetSingleton()->GetNodeBaseType();

    ezQtNodeScene::GetPinFactory().RegisterCreator(ezGetStaticRTTI<ezVisualScriptPin>(), [](const ezRTTI* pRtti)->ezQtPin* { return new ezQtVisualScriptPin(); });
    /*ezQtNodeScene::GetConnectionFactory().RegisterCreator(ezGetStaticRTTI<ezVisualScriptConnection>(), [](const ezRTTI* pRtti)->ezQtConnection* { return new ezQtVisualScriptConnection(); });    */
    ezQtNodeScene::GetNodeFactory().RegisterCreator(pBaseType, [](const ezRTTI* pRtti)->ezQtNode* { return new ezQtVisualScriptNode(); });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    const ezRTTI* pBaseType = ezVisualScriptNodeRegistry::GetSingleton()->GetNodeBaseType();

    ezQtNodeScene::GetPinFactory().UnregisterCreator(ezGetStaticRTTI<ezVisualScriptPin>());
    //ezQtNodeScene::GetConnectionFactory().UnregisterCreator(ezGetStaticRTTI<ezVisualScriptConnection>());
    ezQtNodeScene::GetNodeFactory().UnregisterCreator(pBaseType);

    ezVisualScriptNodeRegistry* pDummy = ezVisualScriptNodeRegistry::GetSingleton();
    EZ_DEFAULT_DELETE(pDummy);
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptPin::ezQtVisualScriptPin() = default;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptConnection::ezQtVisualScriptConnection() = default;

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptNode::ezQtVisualScriptNode() = default;

void ezQtVisualScriptNode::UpdateState()
{
  ezStringBuilder sTitle;

  if (auto pTitleAttribute = GetObject()->GetType()->GetAttributeByType<ezTitleAttribute>())
  {
    sTitle = pTitleAttribute->GetTitle();

    ezHybridArray<ezAbstractProperty*, 32> properties;
    GetObject()->GetType()->GetAllProperties(properties);    

    ezStringBuilder temp;
    for (const auto& pin : GetInputPins())
    {
      if (pin->HasAnyConnections())
      {
        temp.Set("{", pin->GetPin()->GetName(), "}");
        sTitle.ReplaceAll(temp, pin->GetPin()->GetName());
      }
    }

    ezVariant val;
    ezStringBuilder sVal;
    for (const auto& prop : properties)
    {
      val = GetObject()->GetTypeAccessor().GetValue(prop->GetPropertyName());

      if (prop->GetSpecificType()->IsDerivedFrom<ezEnumBase>() || prop->GetSpecificType()->IsDerivedFrom<ezBitflagsBase>())
      {
        ezReflectionUtils::EnumerationToString(prop->GetSpecificType(), val.ConvertTo<ezInt64>(), sVal);
        sVal = ezTranslate(sVal);
      }
      else if (val.CanConvertTo<ezString>())
      {
        sVal = val.ConvertTo<ezString>();
      }

      temp.Set("{", prop->GetPropertyName(), "}");
      sTitle.ReplaceAll(temp, sVal);
    }
  }
  else
  {
    sTitle = GetObject()->GetTypeAccessor().GetType()->GetTypeName();
    if (sTitle.StartsWith_NoCase("VisualScriptNode_"))
      sTitle.Shrink(17, 0);
  }

  m_pLabel->setPlainText(sTitle.GetData());
}

//////////////////////////////////////////////////////////////////////////

ezQtVisualScriptNodeScene::ezQtVisualScriptNodeScene(QObject* parent /*= nullptr*/)
  : ezQtNodeScene(parent)
{
}

ezQtVisualScriptNodeScene::~ezQtVisualScriptNodeScene() = default;
