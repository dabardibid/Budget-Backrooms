/** @file options.h
	@brief Substance linker options definitions. 
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20100408
	@copyright Allegorithmic. All rights reserved. 
	
	Returned by all Substance linker functions. */

#ifndef _SUBSTANCE_LINKER_OPTIONS_H
#define _SUBSTANCE_LINKER_OPTIONS_H


/** Basic type defines */
#include <stddef.h>


/** @brief Substance linker options enumeration
	Used by substanceLinkerContextSetOption */
typedef enum
{
	Substance_Linker_Option_INVALID      = 0,  /**< Unknown option, do not use */
	
	/** @brief Linking optimizations level: unsigned integer option
		Combination of SubstanceLinkerOptionOpt flags. Default is 
		Substance_Linker_Opt_Full. */
	Substance_Linker_Option_Optimizations,
	
	/** @brief Maximum size of compositing nodes: unsigned integer option
		Maximum width and height of all compositing nodes and outputs
		generated during link.
		The default value is dependent to the targeted engine. */
	Substance_Linker_Option_SizeLimit,
	
	/** @brief Global tweaks handling: unsigned integer option
		Combination of SubstanceLinkerOptionGlobal flags. Default is 
		Substance_Linker_Global_FuseAll. */
	Substance_Linker_Option_GlobalTweaks,
	
	/** @brief Target architecture/OS: unsigned integer option
		SubstanceLinkerOptionPlatform flags combination. Default is 
		Substance_Linker_Platform_Current: the linker generates binary results
		specific to current OS, architecture (x86/x64) and CPU SIMD support.
		Use this option to target an other OS/Architecture/CPU (cross linking).
		@note Not used if current targeted engine is not compatible
			with these platforms (useful only on Desktop platforms). */
	Substance_Linker_Option_TargetPlatform,

	/* etc. */
	
	Substance_Linker_Option_Internal_Count
	
} SubstanceLinkerOptionEnum;


/** @brief Linking optimizations flags 
	Used by Substance_Linker_Option_Optimizations option. */
typedef enum
{
	/** @brief No optimizations, fastest linking process */
	Substance_Linker_Opt_None                  = 0x0,   
	
	/** @brief Combination of all available optimizations
		Best performance and memory footprint, slowest linking process */
	Substance_Linker_Opt_Full                  = 0xFF,
	
	/** @brief Merge data redundancies (reduce binary size)
		Only useful when several assemblies are linked together.
		Included in Substance_Linker_Opt_Full. */
	Substance_Linker_Opt_MergeData             = 0x01,
	
	/** @brief Merge graph redundancies inter assemblies (best performance)
		Only useful when several assemblies are linked together.
		Included in Substance_Linker_Opt_Full. */
	Substance_Linker_Opt_MergeGraph            = 0x02
	
} SubstanceLinkerOptionOpt;


/** @brief Linking global tweaks handling flags
	Used by Substance_Linker_Option_GlobalTweaks option. */
typedef enum
{
	/** @brief 'Time' and 'Normal Format' tweaks are not implicitly fused 
		It cause UID collisions if assemblies w/ 'Time' and/or 'Normal Format' 
		defined are pushed twice or more. */
	Substance_Linker_Global_FuseNone           = 0x0,
	
	/** @brief 'Time' and 'Normal Format' global tweaks are implicitly fused */
	Substance_Linker_Global_FuseAll            = 0xFF,   
	
	/** @brief 'Time' ($time) global tweak is implicitly fused */
	Substance_Linker_Global_FuseTime           = 0x01,
	
	/** @brief 'Normal Format' ($normalformat) tweak is implicitly fused */
	Substance_Linker_Global_FuseNormalFormat   = 0x02
	
} SubstanceLinkerOptionGlobal;


/** @brief Linking target platform enum
	Used by Substance_Linker_Platform_Current option.
	Substance_Linker_Platform_Current must be used alone.
	Substance_Linker_Platform_Bytecode must be used alone.
	Substance_Linker_Platform_Supports_SSE4_1 can be combined w/ OS/Architecture
	enum. */
typedef enum
{
	/** @brief Use current platform
		Target platform is the Architecture/OS of the linker binaries.
		SSE4.1 support is retrieve from current CPU */
	Substance_Linker_Platform_Current              = 0x10000,
	
	/** @brief Force UNIX Intel x86 target platform */
	Substance_Linker_Platform_UNIX_x86             =     0x0,
	
	/** @brief Force UNIX Intel x64 target platform */
	Substance_Linker_Platform_UNIX_x64             =     0x1,

	/** @brief Force Windows Intel x86 target platform */
	Substance_Linker_Platform_Windows_x86          =     0x2,
	
	/** @brief Force Windows Intel x64 target platform */
	Substance_Linker_Platform_Windows_x64          =     0x3,
	
	/** @brief Platform w/ SSE4.1 support flag */
	Substance_Linker_Platform_Supports_SSE4_1      =   0x100,

	/** @brief Force Bytecode generation for engine w\ native evaluation. */
	Substance_Linker_Platform_Bytecode             =  0x1000
	
} SubstanceLinkerOptionPlatform;


/** @brief Union used to set option (substanceLinkerContextSetOption) */
typedef union 
{
	int integer;                /**< For integer type options */
	unsigned int uinteger;      /**< For unsigned integer type options */
	float fp;                   /**< For floating point type options */
	const wchar_t* ustring;     /**< For UNICODE string type options */

} SubstanceLinkerOptionValue;

#endif /* ifndef _SUBSTANCE_LINKER_OPTIONS_H */
