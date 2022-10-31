#include <VisualScriptPlugin/VisualScriptPluginPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <VisualScriptPlugin/Resources/VisualScriptClassResource.h>
#include <Core/Assets/AssetFileHeader.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVisualScriptClassResource, 1, ezRTTIDefaultAllocator<ezVisualScriptClassResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezVisualScriptClassResource);

EZ_BEGIN_SUBSYSTEM_DECLARATION(TypeScript, Resource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ResourceManager" 
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP 
  {
    ezResourceManager::RegisterResourceOverrideType(ezGetStaticRTTI<ezVisualScriptClassResource>(), [](const ezStringBuilder& sResourceID) -> bool  {
        return sResourceID.HasExtension(".ezVisualScriptClassBin");
      });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::UnregisterResourceOverrideType(ezGetStaticRTTI<ezVisualScriptClassResource>());
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

ezVisualScriptClassResource::ezVisualScriptClassResource() = default;
ezVisualScriptClassResource::~ezVisualScriptClassResource() = default;

ezResourceLoadDesc ezVisualScriptClassResource::UnloadData(Unload WhatToUnload)
{
  DeleteScriptType();

  ezResourceLoadDesc ld;
  ld.m_State = ezResourceState::Unloaded;
  ld.m_uiQualityLevelsDiscardable = 0;
  ld.m_uiQualityLevelsLoadable = 0;

  return ld;
}

ezResourceLoadDesc ezVisualScriptClassResource::UpdateContent(ezStreamReader* pStream)
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

  //const ezRTTI* pParentType = ezGetStaticRTTI<ezComponent>();
  //CreateScriptType(sTypeName, pParentType);

  ld.m_State = ezResourceState::Loaded;

  return ld;
}

void ezVisualScriptClassResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = (ezUInt32)sizeof(ezVisualScriptClassResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

ezUniquePtr<ezScriptInstance> ezVisualScriptClassResource::Instantiate(ezReflectedClass& owner, ezWorld& world) const
{
  return nullptr;
}
