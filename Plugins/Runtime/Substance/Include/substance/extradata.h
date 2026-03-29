/** @file extradata.h
	@brief Substance extra data enumeration definition. 
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance headers
	@date 20100414
	@copyright Allegorithmic. All rights reserved. 
	
	Used by Substance Cooker and Linker to embed and extract extra data into 
	assemblies. */

#ifndef _SUBSTANCE_EXTRADATA_H
#define _SUBSTANCE_EXTRADATA_H


/** Basic type defines */
#include <stddef.h>


/** @brief Substance extra data UID enumeration */
typedef enum
{
	Substance_ExtraData_INVALID    = 0x0,       /**< Unknown, do not use */
	
	/* etc */
	
	Substance_ExtraData_User       = 0x8000,    /**< User defined extra data */
	
	Substance_ExtraData_FORCEDWORD = 0xFFFFFFFF /**< Force DWORD enumeration */
	
} SubstanceExtraDataEnum;


#endif /* ifndef _SUBSTANCE_EXTRADATA_H */
