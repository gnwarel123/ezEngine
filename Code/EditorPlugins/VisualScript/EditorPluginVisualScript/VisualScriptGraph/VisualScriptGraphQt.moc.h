#pragma once

#include <GuiFoundation/NodeEditor/Connection.h>
#include <GuiFoundation/NodeEditor/Node.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <GuiFoundation/NodeEditor/Pin.h>

class ezQtVisualScriptPin : public ezQtPin
{
public:
  ezQtVisualScriptPin();
};

class ezQtVisualScriptConnection : public ezQtConnection
{
public:
  ezQtVisualScriptConnection();
};

class ezQtVisualScriptNode : public ezQtNode
{
public:
  ezQtVisualScriptNode();

  virtual void UpdateState() override;
};

class ezQtVisualScriptNodeScene : public ezQtNodeScene
{
  Q_OBJECT

public:
  ezQtVisualScriptNodeScene(QObject* parent = nullptr);
  ~ezQtVisualScriptNodeScene();
};
