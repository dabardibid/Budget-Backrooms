// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceEditorModule.h

#pragma once
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"
#include "ISubstanceEditor.h"

namespace SubstanceEditorModule
{
extern const FName SubstanceEditorAppIdentifier;
}

class ISubstanceEditorModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
	/** Creates a new Substance Graph Editor */
	virtual TSharedRef<ISubstanceEditor> CreateSubstanceEditor(const TSharedPtr< IToolkitHost >& InitToolkitHost, USubstanceGraphInstance* Font) = 0;
};
