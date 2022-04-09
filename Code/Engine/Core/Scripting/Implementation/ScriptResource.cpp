#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptResource.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScriptResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezScriptResource::ezScriptResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}


