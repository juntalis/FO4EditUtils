/**
 * @file pch.h
 * 
 * TODO: Description
 */
#ifndef _PCH_H_
#define _PCH_H_
#pragma once

/** Macro used to combine two things. */
#define XPASTE2(A,B) A##B
#define XPASTE(A,B)  XPASTE2(A,B)

/** Stringifying macros */
#define XWIDE(X) XPASTE2(L,X)
#define XSTR2(X) #X
#define XSTRA(X) XSTR2(X)
#define XSTRW(X) XWIDE(XSTRA(X))

/** Windows Version */
#if !defined(WINVER)
#	define WINVER 0x0501
#elif (WINVER < 0x0501)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

#if !defined(_WIN32_WINNT)
#	define _WIN32_WINNT 0x0501
#elif (_WIN32_WINNT < 0x0501)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

#if !defined(NTDDI_VERSION)
#	define NTDDI_VERSION 0x05010000
#elif (NTDDI_VERSION < 0x05010000)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

/** Speed up build process with minimal headers. */
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif

/** Compiler/Architecture Detection */
#if defined(__clang__) || defined(__MINGW32__) || defined(__GNUC__)
//	Clang supports enough of the GCC stuff that we its targeted
#	define BUILD_GNUC
#	if defined(__i386__)
#		define BUILD_X86
#	elif defined(__x86_64__)
#		define BUILD_X64
#	endif
#elif defined(_MSC_VER)
#	define BUILD_MSVC
#	define _CRT_NON_CONFORMING_SWPRINTFS 1
#	if (defined(_M_IA64) || defined(_M_AMD64) || defined(_WIN64))
#		define BUILD_X64
#	elif defined(_M_IX86)
#		define BUILD_X86
#	endif
//	Insecure function usage warnings
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
//	Depreciation warnings
#	pragma warning(disable:4996)
//	Reduce the headers further
#	define VC_EXTRALEAN
//	SDK Version Definitions
#	include <SDKDDKVer.h>
#endif

/** If we can't detect the architecture, error out */
#if !defined(BUILD_X86) && !defined(BUILD_X64)
#	error Could not detect platform architecture.
#endif

/** System Includes */
#include <windows.h>
#include <tchar.h>
#include <memory.h>
#ifdef __cplusplus
#	include <cstdlib>
#	include <cstring>
#	include <cstdarg>
#	include <cassert>
#else
#	include <stdlib.h>
#	include <string.h>
#	include <stdarg.h>
#	include <assert.h>
#endif

/** Utility Macros */
#define ARRAYCOUNT(ARRAY) \
	(sizeof(ARRAY) / sizeof(*(ARRAY)))

#define IS_VALID_HANDLE(HANDLE) \
	(((HANDLE) != NULL) && ((HANDLE) != INVALID_HANDLE_VALUE))

#define IS_INVALID_HANDLE(HANDLE) \
	(((HANDLE) == NULL) || ((HANDLE) == INVALID_HANDLE_VALUE))

// Wchar Utilities
#define T_EMPTY _T("")
#define TSIZE(COUNT) ((COUNT) * sizeof(TCHAR))
#define ZTSIZE(COUNT) TSIZE((COUNT) + 1)
#define TLEN(TSTRING)  (sizeof(TSTRING) / sizeof(TCHAR))
#define ZTLEN(TSTRING)  (TLEN(TSTRING) - 1)

/** Normalize the code style */
#ifndef CONST
#	define CONST const
#endif

#ifndef STATIC
#	define STATIC static
#endif

#ifndef VOID
#	define VOID void
#endif

#ifndef CDECL
#	define CDECL __cdecl
#endif

#ifndef xfree
#	define xfree(PTR) if((PTR) != NULL) free((void*)(PTR))
#endif

#ifdef BUILD_GNUC
#	ifndef INLINE
#		define INLINE __inline
#	endif
#	define NOINLINE      __attribute__((noinline))
#	define NORETURN void __attribute__((__noreturn))
#else
#	ifndef INLINE
#		define INLINE __forceinline
#	endif
#	define NOINLINE      __declspec(noinline)
#	define NORETURN void __declspec(noreturn)
#endif

#if defined(_CONSOLE) || defined(CONSOLE)
#	define BUILD_CUI
#elif defined(_WINDOWS) || defined(WINDOWS)
#	define BUILD_GUI
#else
#	define WINDOWS 1
#	define _WINDOWS 1
#	define BUILD_GUI
#endif

#ifdef BUILD_CUI
#	define XINFO(FMTSTR,...) \
		_tprintf( _T("INFO: ") FMTSTR _T("\n"), __VA_ARGS__ )
#else
#	define XINFO(FMTSTR,...) 
#endif

#endif /* _PCH_H_ */
