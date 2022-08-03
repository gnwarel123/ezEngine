#pragma once

#include <Core/Utils/IntervalScheduler.h>
#include <Core/World/WorldModule.h>

class ezScriptWorldModule : public ezWorldModule
{
  EZ_DECLARE_WORLD_MODULE();
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptWorldModule, ezWorldModule);

public:
  ezScriptWorldModule(ezWorld* pWorld);

  virtual void Initialize() override;

  void AddUpdateFunctionToSchedule(const ezAbstractFunctionProperty* pFunction, void* pInstance, ezTime updateInterval);
  void RemoveUpdateFunctionToSchedule(const ezAbstractFunctionProperty* pFunction, void* pInstance);

  struct FunctionContext
  {
    const ezAbstractFunctionProperty* m_pFunction = nullptr;
    void* m_pInstance = nullptr;

    bool operator==(const FunctionContext& other) const
    {
      return m_pFunction == other.m_pFunction && m_pInstance == other.m_pInstance;
    }
  };

private:
  void CallUpdateFunctions(const ezWorldModule::UpdateContext& context);

  ezIntervalScheduler<FunctionContext> m_Scheduler;
};
