//! @style.h
//! @brief Substance Link Module style
//! @author Ymmanuel Flores - Adobe
//! @date 20193009
//! @copyright Adobe. All rights reserved.

#pragma once

#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#if WITH_EDITOR
#include "ClassIconFinder.h"
#include "EditorStyleSet.h"
#endif

namespace SubstanceEditorModule
{
class FSubstanceLinkStyle
{
public:
	//! @brief Initialize the substance link module style
	static void Initialize();

	//! @brief Shuts down the substance link module style
	static void Shutdown();

	//! @brief Returns the substance link module style's name
	static FName GetStyleSetName();

	//! @brief Static pointer to the substance link module style
	static TSharedPtr<class ISlateStyle> Get();

private:
	//! @brief Internal pointer to the substance link module style
	static TSharedPtr<class FSlateStyleSet> m_style_set;

};

}
