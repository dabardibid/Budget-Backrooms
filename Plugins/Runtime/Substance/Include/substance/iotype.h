/** @file iotype.h
	@brief Substance input/output type enumeration
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20190107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceIOType enumeration. Both used by input and output
	description structures.
*/

#ifndef _SUBSTANCE_IOTYPE_H
#define _SUBSTANCE_IOTYPE_H


/** @brief Substance type enumeration

	Mutually exclusive values. 4 bits format */
typedef enum
{
	Substance_IOType_Float    = 0x0, /**< Float (scalar) type */
	Substance_IOType_Float2   = 0x1, /**< 2D Float (vector) type */
	Substance_IOType_Float3   = 0x2, /**< 3D Float (vector) type */
	Substance_IOType_Float4   = 0x3, /**< 4D Float (vector) type (e.g. color) */

	Substance_IOType_Integer  = 0x4, /**< Integer type (int 32bits, enum or bool) */
	Substance_IOType_Integer2 = 0x8, /**< 2D Integer (vector) type */
	Substance_IOType_Integer3 = 0x9, /**< 3D Integer (vector) type */
	Substance_IOType_Integer4 = 0xA, /**< 4D Integer (vector) type */

	Substance_IOType_Image    = 0x5, /**< Compositing I/O: bitmap/texture data */

	Substance_IOType_String   = 0x6, /**< NULL terminated UNICODE 32b (UCS4) string (input only) */
	Substance_IOType_Font     = 0x7  /**< SubstanceInputFont (input only) */

} SubstanceIOType;


#endif /* ifndef _SUBSTANCE_IOTYPE_H */
