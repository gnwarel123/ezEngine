#pragma once

#include <EditorFramework/EditorFrameworkDLL.h>
#include <EditorFramework/Gizmos/NonUniformBoxGizmo.h>
#include <EditorFramework/Manipulators/ManipulatorAdapter.h>

struct ezGizmoEvent;

class ezBoxManipulatorAdapter : public ezManipulatorAdapter
{
public:
  ezBoxManipulatorAdapter();
  ~ezBoxManipulatorAdapter();

  virtual void QueryGridSettings(ezGridSettingsMsgToEngine& outGridSettings) override;

protected:
  virtual void Finalize() override;
  virtual void Update() override;
  void GizmoEventHandler(const ezGizmoEvent& e);

  virtual void UpdateGizmoTransform() override;

  ezVec3 m_vPositionOffset;
  ezQuat m_qRotation;
  ezNonUniformBoxGizmo m_Gizmo;

  ezVec3 m_vOldSize;
};
