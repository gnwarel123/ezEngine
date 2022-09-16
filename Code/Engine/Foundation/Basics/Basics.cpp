#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/CommonAllocators.h>

#include <Foundation/TraceProvider.h>

#if EZ_ENABLED(EZ_USE_GUARDED_ALLOCATIONS)
typedef ezGuardedAllocator DefaultHeapType;
typedef ezGuardedAllocator DefaultAlignedHeapType;
typedef ezGuardedAllocator DefaultStaticHeapType;
#else
typedef ezHeapAllocator DefaultHeapType;
typedef ezAlignedHeapAllocator DefaultAlignedHeapType;
typedef ezHeapAllocator DefaultStaticHeapType;
#endif

// 798c51ab-4d4f-4cea-9466-38eee628b6de
TRACELOGGING_DEFINE_PROVIDER(FoundationProvider, "ezEngine.Foundation", (0x798c51ab, 0x4d4f, 0x4cea, 0x94, 0x66, 0x38, 0xee, 0xe6,0x28, 0xb6, 0xde));

struct TraceProviderRegisterHelper
{
  TraceProviderRegisterHelper()
  {
    TraceLoggingRegister(FoundationProvider);
  }

  ~TraceProviderRegisterHelper()
  {
    TraceLoggingUnregister(FoundationProvider);
  }
};

TraceProviderRegisterHelper g_FoundationProviderRegister;

enum
{
  HEAP_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultHeapType),
  ALIGNED_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultAlignedHeapType)
};

alignas(EZ_ALIGNMENT_MINIMUM) static ezUInt8 s_DefaultAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];
alignas(EZ_ALIGNMENT_MINIMUM) static ezUInt8 s_StaticAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];

alignas(EZ_ALIGNMENT_MINIMUM) static ezUInt8 s_AlignedAllocatorBuffer[ALIGNED_ALLOCATOR_BUFFER_SIZE];

bool ezFoundation::s_bIsInitialized = false;
ezAllocatorBase* ezFoundation::s_pDefaultAllocator = nullptr;
ezAllocatorBase* ezFoundation::s_pAlignedAllocator = nullptr;

void ezFoundation::Initialize()
{
  if (s_bIsInitialized)
    return;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  ezMemoryUtils::ReserveLower4GBAddressSpace();
#endif

  if (s_pDefaultAllocator == nullptr)
  {
    s_pDefaultAllocator = new (s_DefaultAllocatorBuffer) DefaultHeapType("DefaultHeap");
  }

  if (s_pAlignedAllocator == nullptr)
  {
    s_pAlignedAllocator = new (s_AlignedAllocatorBuffer) DefaultAlignedHeapType("AlignedHeap");
  }

  s_bIsInitialized = true;
}

#if defined(EZ_CUSTOM_STATIC_ALLOCATOR_FUNC)
extern ezAllocatorBase* EZ_CUSTOM_STATIC_ALLOCATOR_FUNC();
#endif

ezAllocatorBase* ezFoundation::GetStaticAllocator()
{
  static ezAllocatorBase* pStaticAllocator = nullptr;

  if (pStaticAllocator == nullptr)
  {
#if defined(EZ_CUSTOM_STATIC_ALLOCATOR_FUNC)

#  if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)

#    if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
    typedef ezAllocatorBase* (*GetStaticAllocatorFunc)();

    HMODULE hThisModule = GetModuleHandle(nullptr);
    GetStaticAllocatorFunc func = (GetStaticAllocatorFunc)GetProcAddress(hThisModule, EZ_CUSTOM_STATIC_ALLOCATOR_FUNC);
    if (func != nullptr)
    {
      pStaticAllocator = (*func)();
      return pStaticAllocator;
    }
#    else
#      error "Customizing static allocator not implemented"
#    endif

#  else
    return EZ_CUSTOM_STATIC_ALLOCATOR_FUNC();
#  endif

#endif

    pStaticAllocator = new (s_StaticAllocatorBuffer) DefaultStaticHeapType(EZ_STATIC_ALLOCATOR_NAME);
  }

  return pStaticAllocator;
}



EZ_STATICLINK_FILE(Foundation, Foundation_Basics_Basics);
