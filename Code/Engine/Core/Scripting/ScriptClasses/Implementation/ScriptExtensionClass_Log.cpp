#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptClassRegistry.h>
#include <Core/Scripting/ScriptClasses/ScriptExtensionClass_Log.h>


// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezScriptExtensionClass_Log, ezNoBase, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(Info, In, "Text"),
  }
  EZ_END_FUNCTIONS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezScriptExtension("Log"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

// static
void ezScriptExtensionClass_Log::Info(const char* szText)
{
  ezLog::Info(szText);
}
