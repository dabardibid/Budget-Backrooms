/** @file errors.h
	@brief Substance error codes definitions. Returned by all Substance functions.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved. */

#ifndef _SUBSTANCE_ERRORS_H
#define _SUBSTANCE_ERRORS_H


/** @brief Substance error codes enumeration */
typedef enum
{
	Substance_Error_None                 = 0, /**< 0x00: No errors */
	Substance_Error_IndexOutOfBounds,         /**< 0x01: In/output index overflow */
	Substance_Error_InvalidData,              /**< 0x02: Substance data not recognized */
	Substance_Error_PlatformMismatch,         /**< 0x03: Substance targeted engine mismatch */
	Substance_Error_VersionNotSupported,      /**< 0x04: Substance data version invalid */
	Substance_Error_BadEndian,                /**< 0x05: Substance data bad endianness */
	Substance_Error_BadDataAlignment,         /**< 0x06: Substance data not correctly aligned */
	Substance_Error_InvalidContext,           /**< 0x07: Context is not valid */
	Substance_Error_NotEnoughMemory,          /**< 0x08: Not enough system memory */
	Substance_Error_InvalidHandle,            /**< 0x09: Invalid handle used */
	Substance_Error_ProcessStillRunning,      /**< 0x0A: Renderer unexpectedly running*/
	Substance_Error_NoProcessRunning,         /**< 0x0B: Rend. expected to be running*/
	Substance_Error_TypeMismatch,             /**< 0x0C: Types do not match */
	Substance_Error_InvalidDevice,            /**< 0x0D: Platform spec. device invalid*/
	Substance_Error_NYI,                      /**< 0x0E: Not yet implemented */
	Substance_Error_CannotCreateMore,         /**< 0x0F: Cannot create more items */
	Substance_Error_InvalidParams,            /**< 0x10: Invalid parameters */
	Substance_Error_InternalError,            /**< 0x11: Internal error */
	Substance_Error_NotComputed,              /**< 0x12: Output(s) not yet comp. */
	Substance_Error_PlatformDeviceInitFailed, /**< 0x13: Cannot init the platform device */
	Substance_Error_EmptyRenderList,          /**< 0x14: The render list is empty */
	Substance_Error_BadThreadUsed,            /**< 0x15: API function thread misuse */
	Substance_Error_EmptyOutputsList,         /**< 0x16: The Outputs list is empty */
	Substance_Error_OutputIdNotFound,         /**< 0x17: Cannot find the Output Id */
	Substance_Error_ReadOnlyContext,          /**< 0x18: The context is Read-Only */
	Substance_Error_InputNoImage,             /**< 0x19: The input is not an image */
	Substance_Error_InvalidInput,             /**< 0x1A: The input is not valid */
	Substance_Error_DeviceLost,               /**< 0x1B: Hardware device is lost */
	Substance_Error_InvalidCacheMapping,      /**< 0x1C: Invalid cache mapping data */
	Substance_Error_NoCacheMappingDiff,       /**< 0x1D: No previous cache mapping diff */
	Substance_Error_APIVersionMismatch,       /**< 0x1E: Engine DDL older than .h */
	Substance_Error_APIPlatformMismatch,      /**< 0x1F: Engine platform and .h mismatch */
	Substance_Error_BadEvaluationBackend,     /**< 0x20: Evaluation backend mismatch */
	Substance_Error_BadEvaluationPlatform,    /**< 0x21: Evaluation OS or x86/x64 mismatch */
	Substance_Error_FontParsingError,         /**< 0x22: Font decoding error */
	Substance_Error_InvalidSRGBFormat,        /**< 0x23: sRGB flag used with incompatible format */

	/* etc. */

	Substance_Error_Internal_Count

} SubstanceErrorCodes;

#endif /* ifndef _SUBSTANCE_ERRORS_H */
