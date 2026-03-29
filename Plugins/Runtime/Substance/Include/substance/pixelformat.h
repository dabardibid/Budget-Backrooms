/** @file pixelformat.h
	@brief Substance pixel formats definitions. Use by texture outputs
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstancePixelFormat and SubstanceChannelsOrder enumerations.
	
*/

#ifndef _SUBSTANCE_PIXELFORMAT_H
#define _SUBSTANCE_PIXELFORMAT_H


/** @brief Substance pixel formats enumeration

	A pixel format is stored on 8 bits.
	Starting from MSB:
	 - 1 unused bit
	 - 1-bit precision (FP flag)
	 - 1-bit misc. (sRGB)
	 - 1-bit precision (8b/16b)
	 - 2-bit channels (L/RGB/RGBx/RGBA)
	 - 2-bit format (RAW/BC/others) */
typedef enum
{
	/* Format (2 bits) */
	Substance_PF_RAW   = 0x0,                    /**< Non-compressed flag */
	Substance_PF_BC    = 0x1,                    /**< DXT compression flag */
	Substance_PF_PVRTC = 0x3,                    /**< PVRTC compression flag */
	Substance_PF_ETC   = 0x3,                    /**< ETC compression flag */
	Substance_PF_Misc  = 0x2,                    /**< Other compression flag */
	Substance_PF_MASK_RAWFormat = 0x3,
	/* etc. */
	
	/* RAW format */

	/* -> RAW format channels (2 bits) */
	Substance_PF_RGBA = Substance_PF_RAW | 0x0,  /**< RGBA 4x8bits format */
	Substance_PF_RGBx = Substance_PF_RAW | 0x4,  /**< RGBx (3+1:pad)x8bits format */
	Substance_PF_RGB  = Substance_PF_RAW | 0x8,  /**< RGB 3x8bits format */
	Substance_PF_L    = Substance_PF_RAW | 0xC,  /**< Luminance 8bits format */
	Substance_PF_MASK_RAWChannels = 0xC,
	
	/* -> RAW format precision flags (2bits: combined to channels) */
	Substance_PF_16b  = 0x10, /**< 16bits flag */
	Substance_PF_FP   = 0x40, /**< Floating point */

	Substance_PF_8I   = Substance_PF_RAW,                   /**< 8bits  integer */
	Substance_PF_16I  = Substance_PF_16b,                   /**< 16bits integer */
	Substance_PF_16F  = Substance_PF_16b | Substance_PF_FP, /**< 16bits FP */
	Substance_PF_32F  = Substance_PF_FP,                    /**< 32bits FP */
	Substance_PF_MASK_RAWPrecision = 0x50,
	Substance_PF_MASK = Substance_PF_MASK_RAWPrecision | Substance_PF_MASK_RAWChannels | Substance_PF_MASK_RAWFormat,

	/* BC/DXT formats */
	Substance_PF_BC1 = Substance_PF_BC | 0x0,  /**< BC1/DXT1 format */
	Substance_PF_BC2 = Substance_PF_BC | 0x8,  /**< BC2/DXT2/DXT3 format */
	Substance_PF_BC3 = Substance_PF_BC | 0x10, /**< BC3/DXT4/DXT5 format */
	Substance_PF_BC4 = Substance_PF_BC | 0x18, /**< BC4/RGTC1/ATI1 format */
	Substance_PF_BC5 = Substance_PF_BC | 0x14, /**< BC5/RGTC2/ATI2 format */
	
	/* PVRTC/ETC formats (3bits) */
	Substance_PF_PVRTC2 = Substance_PF_PVRTC | 0x0,  /**< PVRTC 2bpp */
	Substance_PF_PVRTC4 = Substance_PF_PVRTC | 0x4,  /**< PVRTC 4bpp */
	Substance_PF_ETC1   = Substance_PF_ETC   | 0x8,  /**< ETC1 RGB8 */
	
	/* Miscellaneous compressed formats (3bits) */
	Substance_PF_JPEG = Substance_PF_Misc | 0x0, /**< JPEG format (input only) */
	
	/* Other flags: can be combined to any format */
	Substance_PF_sRGB = 0x20,                 /**< sRGB (gamma trans.) flag */

	/* Deprecated flags (syntax changed) */
	Substance_PF_DXT  = Substance_PF_BC,
	Substance_PF_DXT1 = Substance_PF_BC1,
	Substance_PF_DXT2 = Substance_PF_DXT | 0x4,
	Substance_PF_DXT3 = Substance_PF_BC2,
	Substance_PF_DXT4 = Substance_PF_DXT | 0xC,
	Substance_PF_DXT5 = Substance_PF_BC3,
	Substance_PF_DXTn = Substance_PF_BC5,

	/* etc. */

} SubstancePixelFormat;


/** @brief Substance channels order in memory enumeration

	Useful on generic platform that put results in system mem (BLEND).
	8 bits format.
	| 2bits: channel A index | 2bits: channel B index |
	  2bits: channel G index | 2bits: channel R index |

	The mask and rshift enum values can be used to extract the index of the
	channel in the pixel. An example to extract the index of the blue pixel:
	index = (channelOrder & Substance_ChanOrder_MASK_B)
				>> Substance_ChanOrder_RSHIFT_B;
*/
typedef enum
{
	Substance_ChanOrder_NC       = 0,     /**< Not applicable */
	Substance_ChanOrder_RGBA     = 0xE4,
	Substance_ChanOrder_BGRA     = 0xC6,
	Substance_ChanOrder_ABGR     = 0x1B,
	Substance_ChanOrder_ARGB     = 0x39,

	Substance_ChanOrder_MASK_R   = 0x03,
	Substance_ChanOrder_MASK_G   = 0x0C,
	Substance_ChanOrder_MASK_B   = 0x30,
	Substance_ChanOrder_MASK_A   = 0xC0,

	Substance_ChanOrder_RSHIFT_R = 0x00,
	Substance_ChanOrder_RSHIFT_G = 0x02,
	Substance_ChanOrder_RSHIFT_B = 0x04,
	Substance_ChanOrder_RSHIFT_A = 0x06
} SubstanceChannelsOrder;


#endif /* ifndef _SUBSTANCE_PIXELFORMAT_H */
