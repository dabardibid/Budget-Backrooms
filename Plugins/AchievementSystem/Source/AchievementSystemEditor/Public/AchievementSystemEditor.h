// Copyright 2021 Sam Carey. All Rights Reserved.
#pragma once

#include "AchievementPinFactory.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(AchievementSystemEditor, All, All);

class FAchievementSystemEditor : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	TSharedPtr<FAchievementPinFactory> BlueprintGraphPanelPinFactory;
};