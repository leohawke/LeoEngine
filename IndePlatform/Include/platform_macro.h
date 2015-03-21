#ifndef IndePlatform_platform_macro_h
#define IndePlatform_platform_maco_h

#ifdef XENON
#undef ARCH_XENON
#define ARCH_XENON
#endif

#ifdef _M_IX86
#define ARCH_X86
#elif defined(_M_IX64)
#define ARCH_AMD64
#elif defined(_M_ARM)
#define ARCH_ARM
#else
#error "unsupprot arch"
#endif


#ifndef ARCH_ARM
#define LM_SSE_INTRINSICS
#else
#define LM_ARM_NEON_INTRINSICS
#endif

#ifdef PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#define STRICT
#endif

#endif