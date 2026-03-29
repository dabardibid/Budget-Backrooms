/** @file outputshuffle.h
	@brief Substance linker outputs shuffling structures
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20101117
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceLinkerChannel and SubstanceLinkerShuffle structures.
	The SubstanceLinkerShuffle structure is used as member of the
	SubstanceLinkerOutputCreate structure (outputcreate.h).<BR>
*/


#ifndef _SUBSTANCE_LINKER_OUTPUTSHUFFLE_H
#define _SUBSTANCE_LINKER_OUTPUTSHUFFLE_H



/** @brief Output shuffling: one channel description.

	Used by the SubstanceLinkerShuffle structure. */
typedef struct
{
	/** @brief UID of the Output to use as source (or 0 to set channel to 0)
		@warning The assembly that contains this output UID must be already 
			pushed into the handle (if not 0) */
	unsigned int outputUID;
	
	/** @brief Channel index of the output to use, discarded if grayscale output
		0: Red
		1: Green
		2: Blue
		3: Alpha channel */
	unsigned int channelIndex;
	
	/** @brief Channel value remapping: new minimum level (default: 0.0) 
		@note Only used if useLevels member of SubstanceLinkerShuffle parent 
			structure is not null. */
	float levelMin;
	
	/** @brief Channel value remapping: new maximum level (default: 1.0) 
		@note Only used if useLevels member of SubstanceLinkerShuffle parent 
			structure is not null. */
	float levelMax;

} SubstanceLinkerChannel;


/** @brief Output shuffling: R,G,B,A channels descriptions
 
	Used by SubstanceLinkerOutputCreate structure (outputcreate.h). */
typedef struct
{
	/** @brief R(0),G(1),B(2),A(3) channels description. */
	SubstanceLinkerChannel channels[4];
	
	/** @brief Enable(!=0) or Disable (0) channel values remapping 
		see levelMin and levelMax members of SubstanceLinkerChannel structure */
	unsigned int useLevels;

} SubstanceLinkerShuffle;



#endif /* ifndef _SUBSTANCE_LINKER_OUTPUTSHUFFLE_H */
