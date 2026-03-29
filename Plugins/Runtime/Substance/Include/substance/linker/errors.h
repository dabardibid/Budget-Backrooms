/** @file errors.h
	@brief Substance linker error codes definitions.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20100406
	@copyright Allegorithmic. All rights reserved.

	Returned by all Substance linker functions. */

#ifndef _SUBSTANCE_LINKER_ERRORS_H
#define _SUBSTANCE_LINKER_ERRORS_H


/** @brief Substance linker error codes enumeration */
typedef enum
{
	Substance_Linker_Error_None               =  0, /**< No errors */
	Substance_Linker_Error_InvalidContext     =  1, /**< Context is not valid */
	Substance_Linker_Error_InvalidHandle      =  2, /**< Invalid handle used */
	Substance_Linker_Error_InvalidParams      =  3, /**< Invalid parameters */
	Substance_Linker_Error_InternalError      =  4, /**< Internal error */
	Substance_Linker_Error_NYI                =  5, /**< Not yet implemented */
	Substance_Linker_Error_APIVersionMismatch =  6, /**< Linker DLL older than .h */
	Substance_Linker_Error_UnsupportedEngine  =  7, /**< Target engine not supported */
	Substance_Linker_Error_OpenAssembly       =  8, /**< Cannot open the assembly */
	Substance_Linker_Error_InvalidAssembly    =  9, /**< Invalid assembly */
	Substance_Linker_Error_NoAssemblyToLink   = 10, /**< The assemblies list is empty */
	Substance_Linker_Error_LinkFailed         = 11, /**< Linking process failed */
	Substance_Linker_Error_InvalidUID         = 12, /**< UID not found */
	Substance_Linker_Error_HaveDependencies   = 13, /**< Dependencies, but callback not set */
	Substance_Linker_Error_MissingDependency  = 14, /**< Dependency not provided */
	Substance_Linker_Error_InvalidArchive     = 15, /**< Invalid archive (SBSAR) */
	Substance_Linker_Error_EmptyArchive       = 16, /**< No SBSASM in archive */
	Substance_Linker_Error_SameAssembly       = 17, /**< Input an output in same assembly */
	Substance_Linker_Error_CyclicDependency   = 18, /**< Cyclic dependency */
	Substance_Linker_Error_DisabledInput      = 19, /**< Input already connected or fused */
	Substance_Linker_Error_TypeMismatch       = 20, /**< Input types does not match */
	Substance_Linker_Error_InvalidType        = 21, /**< Input type not handled */
	Substance_Linker_Error_NoLink             = 22, /**< No link process launched */
	Substance_Linker_Error_NoPushWithUID      = 23, /**< No assemblies pushed w/ user UID */
	Substance_Linker_Error_InvalidMapping     = 24, /**< Invalid prev. cache mapping data */
	Substance_Linker_Error_NeedOutputUID      = 25, /**< Create output need valid outputUID */
	Substance_Linker_Error_NoSBSARSupport     = 26, /**< No support for SBSAR files, only SBSASM are allowed */
	/* etc. */

	Substance_Linker_Error_Internal_Count

} SubstanceLinkerErrorCodes;

#endif /* ifndef _SUBSTANCE_LINKER_ERRORS_H */
