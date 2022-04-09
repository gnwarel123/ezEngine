#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptExtensions/ScriptExtensionRegistry.h>
#include <Core/Scripting/ScriptExtensions/ScriptExtension_Component.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezScriptExtension_Component, ezComponent, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsActive", IsActive),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

EZ_REGISTER_SCRIPT_EXTENSION(ezScriptExtension_Component);
