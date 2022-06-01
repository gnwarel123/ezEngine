#include <TypeScriptPlugin/TypeScriptPluginPCH.h>

#include <Core/World/Component.h>
#include <Foundation/Configuration/Startup.h>
#include <TypeScriptPlugin/Resources/TypeScriptResource.h>

void ezTypeScriptInstance::ApplyParameters(const ezArrayMap<ezHashedString, ezVariant>& parameters)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTypeScriptResource, 1, ezRTTIDefaultAllocator<ezTypeScriptResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezTypeScriptResource);

EZ_BEGIN_SUBSYSTEM_DECLARATION(TypeScript, Resource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ResourceManager" 
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP 
  {
    ezResourceManager::RegisterResourceOverrideType(ezGetStaticRTTI<ezTypeScriptResource>(), [](const ezStringBuilder& sResourceID) -> bool  {
        return sResourceID.HasExtension(".ezTypeScriptRes");
      });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::UnregisterResourceOverrideType(ezGetStaticRTTI<ezTypeScriptResource>());
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

ezTypeScriptResource::ezTypeScriptResource() = default;
ezTypeScriptResource::~ezTypeScriptResource() = default;

ezResourceLoadDesc ezTypeScriptResource::UnloadData(Unload WhatToUnload)
{
  m_pType = nullptr;
  m_Functions.Clear();
  m_MessageHandlers.Clear();

  ezResourceLoadDesc ld;
  ld.m_State = ezResourceState::Unloaded;
  ld.m_uiQualityLevelsDiscardable = 0;
  ld.m_uiQualityLevelsLoadable = 0;

  return ld;
}

ezResourceLoadDesc ezTypeScriptResource::UpdateContent(ezStreamReader* pStream)
{
  ezResourceLoadDesc ld;
  ld.m_uiQualityLevelsDiscardable = 0;
  ld.m_uiQualityLevelsLoadable = 0;

  if (pStream == nullptr)
  {
    ld.m_State = ezResourceState::LoadedResourceMissing;
    return ld;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    ezString sAbsFilePath;
    (*pStream) >> sAbsFilePath;
  }

  // skip the asset file header at the start of the file
  ezAssetFileHeader AssetHash;
  AssetHash.Read(*pStream).IgnoreResult();

  const char* name = "TypescriptTest";
  const ezRTTI* pParentType = ezGetStaticRTTI<ezComponent>();

  CreateScriptType(name, pParentType);  

  ld.m_State = ezResourceState::Loaded;

  return ld;
}

void ezTypeScriptResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = (ezUInt32)sizeof(ezTypeScriptResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

ezUniquePtr<ezScriptInstance> ezTypeScriptResource::Instantiate(const ezReflectedClass* pContext) const
{
  return EZ_DEFAULT_NEW(ezTypeScriptInstance);
}
