#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Reflection/Reflection.h>

class EZ_CORE_DLL ezScriptExtensionClass_Log
{
public:
  static void Info(const char* szText);
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_CORE_DLL, ezScriptExtensionClass_Log);
