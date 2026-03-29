/** @file version.h
	@brief Substance current version description structure and accessor function
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080610
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceVersion structure. Filled using the
	substanceGetCurrentVersion function.
*/

#ifndef _SUBSTANCE_VERSION_H
#define _SUBSTANCE_VERSION_H


/** Base definitions */
#include "defines.h"

/** Engines IDs list */
#include "engineid.h"


/** @brief Version of the header of the Substance Engine API (Major|Minor)
    Required by the substanceContextInit macro */
#define SUBSTANCE_API_VERSION 0x00010009u


/** @brief Substance current version description

	Filled using the substanceGetCurrentVersionImpl function declared just
	below. */
typedef struct SubstanceVersion_
{
	/** @brief Current engine implementation platform ID (see ./engineid.h) */
	SubstanceEngineIDEnum platformImplEnum;

	/** @brief Current engine implementation platform name
		Correspond to platformImplEnum. */
	const char* platformImplName ;

	/** @brief Current engine version: major */
	unsigned int versionMajor;

	/** @brief Current engine version: minor */
	unsigned int versionMinor;

	/** @brief Current engine version: patch */
	unsigned int versionPatch;

	/** @brief Current engine stage (Release, alpha, beta, RC, etc.) */
	const char* currentStage;

	/** @brief Substance engine build date (Mmm dd yyyy) */
	const char* buildDate;

	/** @brief Substance engine sources revision */
	unsigned int sourcesRevision;

	/** @brief Current engine API (headers) platform ID
	           API headers platform (SUBSTANCE_API_PLATFORM) must match the
	           returned platformApiEnum.
	    @note platformApiEnum is independent from platformImplEnum (e.g. an
	          OpenGL implementation engine can have a BLEND platform API) */
	SubstanceEngineIDEnum platformApiEnum;

	/** @brief Current engine API version */
	unsigned int versionApi;

	/** @brief Version of binary data format read by current engine
	           No backward or forward compatibility: linker data version must match.
	           This version number is engine platform specific.
	    @note Available since API version 1.8 */
	unsigned int versionData;

} SubstanceVersion;


/** @brief Get the current substance engine version
    @param[out] version Pointer to the version struct to fill.

	This macro is used to call substanceGetCurrentVersionImpl with correct
	arguments (Caller API version). */
#define substanceGetCurrentVersion(version) \
	substanceGetCurrentVersionImpl(version,SUBSTANCE_API_VERSION)

/** @brief Get the current substance engine version
    @param[out] version Pointer to the version struct to fill.
    @param apiVersion The value of this parameter should be
           SUBSTANCE_API_VERSION.

	A helper macro 'substanceGetCurrentVersion' is provided. It calls this
	function with the apiVersion argument correctly filled. */
SUBSTANCE_EXPORT void substanceGetCurrentVersionImpl(
	SubstanceVersion* version,
	unsigned int apiVersion);


#endif /* ifndef _SUBSTANCE_VERSION_H */
