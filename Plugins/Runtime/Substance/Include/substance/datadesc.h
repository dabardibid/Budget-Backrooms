/** @file datadesc.h
	@brief Substance cooked data description structure
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceDataDesc structure. Filled using the substanceHandleGetDesc
	function declared in handle.h.
	Contains information about the data to render: number of outputs, inputs,
	etc.
*/

#ifndef _SUBSTANCE_DATADESC_H
#define _SUBSTANCE_DATADESC_H


/** @brief Substance cooked data description structure definition

	Filled using the substanceHandleGetDesc function declared in handle.h. */
typedef struct SubstanceDataDesc_
{
	/** @brief Number of outputs (texture results) */
	unsigned int outputsCount;
	
	/** @brief Number of inputs (render parameters) */
	unsigned int inputsCount;
	
	/** @brief Format version the data was cooked for */
	unsigned short formatVersion;	
	
	/** @brief Identifier of the platform the data was cooked for */
	unsigned char platformId;

} SubstanceDataDesc;

#endif /* ifndef _SUBSTANCE_DATADESC_H */
