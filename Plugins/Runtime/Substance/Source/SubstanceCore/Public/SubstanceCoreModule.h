// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreModule.h

#pragma once
#include "ISubstanceCore.h"
#include "Containers/Ticker.h"
#include "SubstanceSettings.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

class FSubstanceCoreModule : public ISubstanceCore
{
	//Substance Core Initialization
	virtual void StartupModule() override;

	//Module cleanup
	virtual void ShutdownModule() override;

	//Returns the max output size based on the set Substance Engine
	virtual uint32 GetMaxOutputTextureSize() const override;

	//Module update
	virtual bool Tick(float DeltaTime);

	//Registers the Substance settings to appear within the project settings (Editor Only)
	void RegisterSettings();

	//Unregister the Substance settings from the project settings (Editor Only)
	void UnregisterSettings();

#if WITH_EDITOR

	//Event callback when the game begins playing within the Unreal Editor
	void OnBeginPIE(const bool bIsSimulating);

	//Event callback for when a game which is playing within the Unreal Editor has ended
	void OnEndPIE(const bool bIsSimulating);

public:
	//Checks if a play in editor session is currently active
	bool isPie() const;

#endif //WITH_EDITOR

private:
	//Tick delegate to register module update
	FTickerDelegate TickDelegate;

	//Callback for when the game world is loaded and initialized
	static void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);

	//Callback for when a never level is added
	static void OnLevelAdded(ULevel* Level, UWorld* World);

	//Stores if game is currently in Play In Editor mode
	bool PIE;

	//Used to write out the Substance settings for debug purposes
	void LogSubstanceSettings();
};
