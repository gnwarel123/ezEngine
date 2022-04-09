#pragma once

#include <Core/CoreDLL.h>
#include <Core/ResourceManager/Resource.h>

using ezScriptResourceHandle = ezTypedResourceHandle<class ezScriptResource>;

class EZ_CORE_DLL ezScriptResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptResource, ezResource);

public:
  ezScriptResource();

  

private:
};
