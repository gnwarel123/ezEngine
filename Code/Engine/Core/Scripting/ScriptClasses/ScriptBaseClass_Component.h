#pragma once

#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>
#include <Core/World/Component.h>

class EZ_CORE_DLL ezScriptBaseClass_Component : public ezComponent
{
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_CORE_DLL, ezScriptBaseClass_Component);

struct ezScriptFunctionTable_Component : public ezScriptFunctionTable_Base
{
  ezAbstractFunctionProperty* m_pInitializeFunction = nullptr;
  ezAbstractFunctionProperty* m_pDeinitializeFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnActivatedFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnDeactivatedFunction = nullptr;
  ezAbstractFunctionProperty* m_pOnSimulationStartedFunction = nullptr;
  ezAbstractFunctionProperty* m_pUpdateFunction = nullptr;
};
