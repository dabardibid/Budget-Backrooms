// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceImportOptionsUI.cpp

#include "SubstanceImportOptionsUi.h"
#include "SubstanceEditorPrivatePCH.h"

USubstanceImportOptionsUi::USubstanceImportOptionsUi(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	bOverrideFullName = false;
	bCreateInstance = true;
	bCreateMaterial = true;
}