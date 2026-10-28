#ifndef KX_PLATFORM_H
#define KX_PLATFORM_H

/*
 * kx_platform.h
 *
 * Platform detection, visibility/calling-convention macros, handle
 * definition macro, and C/C++ compatibility for the public Kromax ABI.
 *
 * This header compiles cleanly as both C11 and C++20.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Platform detection
 * --------------------------------------------------------------------- */
#if defined(_WIN32) || defined(_WIN64)
#  define KX_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  define KX_PLATFORM_LINUX 1
#elif defined(__APPLE__)
#  define KX_PLATFORM_MACOS 1
#else
#  define KX_PLATFORM_UNKNOWN 1
#endif

/* -----------------------------------------------------------------------
 * Symbol visibility and calling convention
 * --------------------------------------------------------------------- */
#if defined(KX_PLATFORM_WINDOWS)
#  ifdef KX_BUILD_DLL
#    define KX_API __declspec(dllexport)
#  else
#    define KX_API __declspec(dllimport)
#  endif
#  define KX_CALL __stdcall
#else
#  define KX_API __attribute__((visibility("default")))
#  define KX_CALL
#endif

/* -----------------------------------------------------------------------
 * Opaque handle macro
 *
 * Usage: KX_DEFINE_HANDLE(KxInstance)
 * Expands to: typedef struct KxInstance_T* KxInstance;
 * --------------------------------------------------------------------- */
#define KX_DEFINE_HANDLE(type) typedef struct type##_T* type;

/* -----------------------------------------------------------------------
 * C++ feature detection (internal implementation uses these)
 * --------------------------------------------------------------------- */
#if defined(__cplusplus)
#  if __cplusplus >= 202002L
#    define KX_CPP20_OR_GREATER 1
#  elif __cplusplus >= 201703L
#    define KX_CPP17_OR_GREATER 1
#  elif __cplusplus >= 201103L
#    define KX_CPP11_OR_GREATER 1
#  endif
#endif

/* -----------------------------------------------------------------------
 * Standard integer types
 * --------------------------------------------------------------------- */
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_PLATFORM_H */