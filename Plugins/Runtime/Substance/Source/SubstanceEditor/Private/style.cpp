//! @style.cpp
//! @brief Substance Link Module style
//! @author Ymmanuel Flores - Adobe
//! @date 20193009
//! @copyright Adobe. All rights reserved.

#include "style.h"

namespace SubstanceEditorModule
{
TSharedPtr<FSlateStyleSet> FSubstanceLinkStyle::m_style_set = nullptr;

void FSubstanceLinkStyle::Initialize()
{
	// Only register once
	if (m_style_set.IsValid())
	{
		return;
	}

	// Create style set
	m_style_set = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	m_style_set->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	m_style_set->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	const FVector2D Icon24x24(24.0f, 24.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Substance"))->GetContentDir();
	FString path = (ContentDir / "LinkUI/icon_Heart") + ".png";
	m_style_set->Set("SubstanceAssetsButtonIcon", new FSlateImageBrush(path, Icon40x40));
	m_style_set->Set("SubstanceAssetsButtonIcon.Small", new FSlateImageBrush(path, Icon24x24));
	m_style_set->Set("SubstanceAssetsButtonIcon.Medium", new FSlateImageBrush(path, Icon40x40));
	m_style_set->Set("SubstanceAssetsButtonIcon.Large", new FSlateImageBrush(path, Icon64x64));
	m_style_set->Set("SubstanceAssetsButtonIcon.XLarge", new FSlateImageBrush(path, Icon128x128));

	FString path2 = (ContentDir / "LinkUI/icon_Heart_Community") + ".png";
	m_style_set->Set("SubstanceCommunityAssetsButtonIcon", new FSlateImageBrush(path2, Icon40x40));
	m_style_set->Set("SubstanceCommunityAssetsButtonIcon.Small", new FSlateImageBrush(path2, Icon24x24));
	m_style_set->Set("SubstanceCommunityAssetsButtonIcon.Medium", new FSlateImageBrush(path2, Icon40x40));
	m_style_set->Set("SubstanceCommunityAssetsButtonIcon.Large", new FSlateImageBrush(path2, Icon64x64));
	m_style_set->Set("SubstanceCommunityAssetsButtonIcon.XLarge", new FSlateImageBrush(path2, Icon128x128));


#if WITH_EDITOR
	FSlateStyleRegistry::RegisterSlateStyle(*m_style_set.Get());
#endif
}

void FSubstanceLinkStyle::Shutdown()
{
	if (m_style_set.IsValid())
	{
#if WITH_EDITOR
		FSlateStyleRegistry::UnRegisterSlateStyle(*m_style_set.Get());
#endif
		ensure(m_style_set.IsUnique());
		m_style_set.Reset();
	}
}

FName FSubstanceLinkStyle::GetStyleSetName()
{
	static FName SubstanceStyleName(TEXT("SubstanceLinkStyle"));
	return SubstanceStyleName;
}

TSharedPtr<class ISlateStyle> FSubstanceLinkStyle::Get()
{
	return m_style_set;
}

}