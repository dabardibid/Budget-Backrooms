/** @file enums.h
	@brief Substance engine handle related enumerations
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20140523
	@copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_ENUMS_H
#define _SUBSTANCE_ENUMS_H


/** @brief Render list push options (flags)

	Used by the substanceHandlePush[...] functions.

	Allows to set the 'Hint only' flag */
typedef enum
{
	/* Hint only flag (the action to push is only a hint that helps Substance to
		anticipate which texture is likely to be required soon). */
	Substance_PushOpt_NotAHint       = 0x0,     /**< Not a hint (default) */
	Substance_PushOpt_HintOnly       = 0x800,   /**< Hint only flag */

} SubstancePushOption;


/** @brief Selection list interpretation flag

	Used by the substanceHandlePushOutputs
		and substanceHandleGetXXXXXOutput functions.

	Allows to select how the values in the selection list should be interpreted:
	  - textures are selected by output indices
	  - textures are selected by identifiers */
typedef enum
{
	/* Selection list interpretation (mutually exclusive flags) */
	Substance_OutOpt_OutIndex   = 0x0, /**< Index output selection (default) */
	Substance_OutOpt_OutputId   = 0x2, /**< Output unique identifiers output selection */
	Substance_OutOpt_TextureId  = 0x2, /**< Deprecated, use Substance_OutOpt_OutputId instead */

	/* Selection list allocation flags. */
	Substance_OutOpt_CopyNeeded = 0x0000, /**< Volatile Selec. list, copy needed (default)*/
	Substance_OutOpt_DoNotCopy  = 0x1000, /**< Persistent Selection list */

	Substance_OutOpt_Numerical  = 0x10,   /**< Internal use only */


} SubstanceOutputSelectionOption;


/** @brief Synchronization options. */
typedef enum
{
	/* Synchronization mode (mutually exclusive flags) */
	Substance_Sync_Synchronous  = 0x0,  /**< Synchronous computation */
	Substance_Sync_Asynchronous = 0x1,  /**< Asynchronous computation */

} SubstanceSyncOption;


/** @brief Start options.

	Used by substanceHandleStart 'flags' parameter. */
typedef enum
{
	/* Ending option (mutually exclusive flags) */
	Substance_Ending_Stop    = 0x0, /**< Process stopped on end */
	Substance_Ending_Waiting = 0x2, /**< Process waiting, explicit stop needed*/

} SubstanceStartOption;


/** @brief Substante state flags.

	Used by substanceHandleGetState 'state' output parameter. */
typedef enum
{
	Substance_State_Started        = 0x1,  /**< Process currently running (Started) */
	Substance_State_Asynchronous   = 0x2,  /**< Asynchronous process running */
	Substance_State_EndingWaiting  = 0x4,  /**< Process started with Substance_Ending_Waiting option */
	Substance_State_NoMoreRender   = 0x8,  /**< The render list is empty */
	Substance_State_NoMoreRsc      = 0x10, /**< Process idle: no hardware resources available */
	Substance_State_PendingCommand = 0x20, /**< User command (API calls) pending */

} SubstanceStateFlag;


#endif /* ifndef _SUBSTANCE_ENUMS_H */
