#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptResource.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScriptResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezScriptResource);
// clang-format on

ezScriptResource::ezScriptResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}
