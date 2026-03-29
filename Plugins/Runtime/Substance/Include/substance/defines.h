/** @file defines.h
	@brief Base definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20140523
	@copyright Allegorithmic. All rights reserved.

	Defines utilities macros. */

#ifndef _SUBSTANCE_DEFINES_H
#define _SUBSTANCE_DEFINES_H


#ifndef SUBSTANCE_WIN32
	#ifdef __WIN32__
		#define SUBSTANCE_WIN32 1
	#endif
	#ifdef _WIN32
		#define SUBSTANCE_WIN32 1
	#endif
#endif

/* Base types defines */
#include <stddef.h>

/* C/C++ macros */
#ifndef SUBSTANCE_EXTERNC
	#ifdef __cplusplus
		#define SUBSTANCE_EXTERNC extern "C"
	#else
		#define SUBSTANCE_EXTERNC
	#endif
#endif

/* SUBSTANCE_EXPORT macros */
#ifndef SUBSTANCE_EXPORT
	#ifdef SUBSTANCE_DLL
		#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
			#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC __declspec(dllexport)
		#elif defined(__GNUC__)
			#if __GNUC__<4
				#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC
			#else
				#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC __attribute__((visibility("default")))
			#endif
		#else
			#error Unknown compiler
		#endif
	#else
		#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC
	#endif
#endif

/* Callback functions */
#ifndef SUBSTANCE_CALLBACK
	#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
		#define SUBSTANCE_CALLBACK __stdcall
	#else
		#define SUBSTANCE_CALLBACK
	#endif
#endif


#endif /* ifndef _SUBSTANCE_DEFINES_H */
