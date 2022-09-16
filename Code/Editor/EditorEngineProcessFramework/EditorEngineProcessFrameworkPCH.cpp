#include <EditorEngineProcessFramework/EditorEngineProcessFrameworkPCH.h>

#include <EditorEngineProcessFramework/TraceProvider.h>

EZ_STATICLINK_LIBRARY(EditorEngineFramework)
{
  if (bReturn)
    return;
}

// 3a419e7c-beb0-4959-b483-7e81bf82ef97
TRACELOGGING_DEFINE_PROVIDER(EditorEngineProcessFrameworkProvider, "ezEngine.EditorEngineProcessFramework", (0x3a419e7c, 0xbeb0, 0x4959, 0xb4, 0x83, 0x7e, 0x81, 0xbf, 0x82, 0xef, 0x97));

struct TraceProviderRegisterHelper
{
  TraceProviderRegisterHelper()
  {
    TraceLoggingRegister(EditorEngineProcessFrameworkProvider);
  }

  ~TraceProviderRegisterHelper()
  {
    TraceLoggingUnregister(EditorEngineProcessFrameworkProvider);
  }
};

TraceProviderRegisterHelper EditorEngineProcessFrameworkProviderRegister;