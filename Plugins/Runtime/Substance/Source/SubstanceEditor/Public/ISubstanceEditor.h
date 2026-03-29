// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: ISubstanceEditor.h

#pragma once
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "SubstanceCoreClasses.h"

class ISubstanceEditor : public FAssetEditorToolkit
{
public:
	/** Returns the graph asset being edited */
	virtual USubstanceGraphInstance* GetGraph() const = 0;
};
