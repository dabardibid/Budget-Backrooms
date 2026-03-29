// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceOptionsWindow.h

#pragma once
#include "Styling/SlateTypes.h"
#include "AssetRegistryModule.h"
#include "SubstanceImportOptionsUi.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "PropertyCustomizationHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "SubstanceOption"

namespace Substance
{
static FString InstancePath;
static FString MaterialPath;
}

class SSubstanceOptionWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubstanceOptionWindow)
		: _ImportUI(NULL)
		, _WidgetWindow()
	{}

	SLATE_ARGUMENT(USubstanceImportOptionsUi*, ImportUI)
	SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	FReply OnImport()
	{
		bCanImport = true;

		//Make sure the names are not the same (UE4-372)
		if (InstanceNameWidget.Pin()->GetText().ToString() == MaterialNameWidget.Pin()->GetText().ToString())
		{
			if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The Graph Instance and Material names can not be the same.")))
			{
				return FReply::Handled();
			}
		}

		ImportUI->InstanceName = InstanceNameWidget.Pin()->GetText().ToString();
		ImportUI->MaterialName = MaterialNameWidget.Pin()->GetText().ToString();

		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}

		if (ImportUI->uMaterialParentType == ESubstanceMaterialParentType::Custom)
			ImportUI->ParentMaterial = SelectedCustomMaterial;
		else if (ImportUI->uMaterialParentType == ESubstanceMaterialParentType::Default)
			ImportUI->ParentMaterial = SelectedDefaultMaterial;
		else
			ImportUI->ParentMaterial = nullptr;

		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bCanImport = false;
		if (WidgetWindow.IsValid())
		{
			WidgetWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	bool ShouldImport()
	{
		return bCanImport;
	}

	SSubstanceOptionWindow()
		: ImportUI(NULL)
		, bCanImport(false)
	{}

	void InstancePathSelected(const FString& InPathName)
	{
		ImportUI->InstanceDestinationPath = InPathName;
	}

	void MaterialPathSelected(const FString& InPathName)
	{
		ImportUI->MaterialDestinationPath = InPathName;
	}

private:
	bool bIsReportingError;

	USubstanceImportOptionsUi* ImportUI;
	bool bCanImport;
	bool bReimport;

	FString ErrorMessage;
	SVerticalBox::FSlot* CustomBox;

	TWeakPtr<SWindow> WidgetWindow;
	TSharedPtr<SButton> ImportButton;

	TSharedPtr<SWidget> InstancePathPicker;
	TSharedPtr<SWidget> MaterialPathPicker;

	/** Instance Name textbox widget */
	TWeakPtr<SEditableTextBox> InstanceNameWidget;

	/** Material Name textbox widget */
	TWeakPtr<SEditableTextBox> MaterialNameWidget;

	UMaterial* ConfigDefaultMaterial;

	TSharedPtr<SWidgetSwitcher> MaterialTypeSwitcher;

	TSharedPtr<SWidget> MaterialSelectionWidget;
	UMaterial* SelectedCustomMaterial = nullptr;

	TSharedPtr<SWidget> DefaultMaterialTemplates;
	UMaterial* SelectedDefaultMaterial = nullptr;

	TSharedPtr<STextBlock> SelectedMaterialText;

	TArray<UMaterial*> SubstanceIncludedMaterials;


	void OnInstanceNameChanged(const FText& InNewName);

	void OnMaterialNameChanged(const FText& InNewName);

	void OnCreateInstanceChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);
		if (ImportUI->bForceCreateInstance)
		{
			return;
		}

		ImportUI->bCreateInstance = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	ECheckBoxState GetCreateInstance() const
	{
		if (ImportUI->bForceCreateInstance)
		{
			return ECheckBoxState::Checked;
		}

		return ImportUI->bCreateInstance ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState GetOverrideInstancePath() const
	{
		check(ImportUI);
		return ImportUI->bOverrideInstancePath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnOverrideInstancePathChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);
		ImportUI->bOverrideInstancePath = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	void OnInstancePathCommitted(const FText& NewValue, ETextCommit::Type CommitInfo)
	{
		check(ImportUI);
		ImportUI->InstanceDestinationPath = NewValue.ToString();
	}

	bool ShowInstancePathPicker() const
	{
		check(ImportUI);
		return ImportUI->bOverrideInstancePath;
	}

	bool CreateInstance() const
	{
		return ImportUI->bCreateInstance ? true : false;
	}

	bool CreateMaterial() const
	{
		if (ImportUI->bCreateInstance)
		{
			return ImportUI->bCreateMaterial ? true : false;
		}

		return false;
	}

	void OnUseCustomMaterialChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);

		if (InNewValue == ECheckBoxState::Checked)
		{
			ImportUI->uMaterialParentType = ESubstanceMaterialParentType::Custom;

			MaterialTypeSwitcher->SetActiveWidgetIndex(static_cast<int8>(ImportUI->uMaterialParentType));
			
			ImportUI->ParentMaterial = SelectedCustomMaterial;
		}
	}

	void OnUseDefaultMaterialChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);

		if (InNewValue == ECheckBoxState::Checked)
		{
			ImportUI->uMaterialParentType = ESubstanceMaterialParentType::Default;

			MaterialTypeSwitcher->SetActiveWidgetIndex(static_cast<int8>(ImportUI->uMaterialParentType));
			
			ImportUI->ParentMaterial = SelectedDefaultMaterial;
		}
	}

	void OnGenerateMaterialChanged(ECheckBoxState InNewValue)
	{
		check(ImportUI);

		if (InNewValue == ECheckBoxState::Checked)
		{
			ImportUI->uMaterialParentType = ESubstanceMaterialParentType::Generated;

			MaterialTypeSwitcher->SetActiveWidgetIndex(static_cast<int8>(ImportUI->uMaterialParentType));

			ImportUI->ParentMaterial = nullptr;
		}
	}

	ECheckBoxState GetCreateMaterial() const
	{
		return ImportUI->bCreateMaterial ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState GetUseDefaultMaterial() const
	{
		return ImportUI->uMaterialParentType == ESubstanceMaterialParentType::Default ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState GetUseCustomMaterial() const
	{
		return ImportUI->uMaterialParentType == ESubstanceMaterialParentType::Custom ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState GetGenerateMaterial() const
	{
		return ImportUI->uMaterialParentType == ESubstanceMaterialParentType::Generated ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}


	void OnCreateMaterialChanged(ECheckBoxState InNewValue) const
	{
		check(ImportUI);
		ImportUI->bCreateMaterial = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	ECheckBoxState GetOverrideMaterialPath() const
	{
		check(ImportUI);
		return ImportUI->bOverrideMaterialPath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnOverrideMaterialPathChanged(ECheckBoxState InNewValue) const
	{
		check(ImportUI);
		ImportUI->bOverrideMaterialPath = InNewValue == ECheckBoxState::Checked ? true : false;
	}

	void OnMaterialPathCommitted(const FText& NewValue, ETextCommit::Type CommitInfo) const
	{
		check(ImportUI);
		ImportUI->MaterialDestinationPath = NewValue.ToString();
	}

	bool ShowMaterialPathPicker() const
	{
		check(ImportUI);
		return ImportUI->bOverrideMaterialPath;
	}

	void RefreshImportButtonState()
	{
		//Make sure that each of the names are valid
		bool InstanceNameValid = !InstanceNameWidget.Pin()->GetText().ToString().IsEmpty();
		bool MaterialNameValid = !MaterialNameWidget.Pin()->GetText().ToString().IsEmpty();
		ImportButton->SetEnabled(MaterialNameValid && InstanceNameValid);
	}

	void OnGetClassesForAssetPicker(TArray<const UClass*>& OutClasses)
	{
		OutClasses.AddUnique(UMaterial::StaticClass());
	}


	void OnCustomMaterialSelected(const FAssetData& AssetData)
	{
		SelectedCustomMaterial = (UMaterial*)AssetData.GetAsset();
	}

	bool OnFilterCustomMaterial(const FAssetData &)
	{
		return false;
	}

	TSharedRef<SWidget> CreateCustomMaterialSelectionWidget()
	{
		TArray<const UClass*> AllowedClasses;
		AllowedClasses.Add(UMaterial::StaticClass());
		GetDefault<USubstanceSettings>()->DefaultTemplateMaterial.LoadSynchronous();
		UMaterial* SelectedMaterial = (UMaterial*)GetDefault<USubstanceSettings>()->DefaultTemplateMaterial.Get();

		SelectedCustomMaterial = SelectedMaterial;

		return PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
			FAssetData((UObject*)SelectedMaterial),
			false,
			AllowedClasses,
			PropertyCustomizationHelpers::GetNewAssetFactoriesForClasses(AllowedClasses),
			FOnShouldFilterAsset::CreateSP(this, &SSubstanceOptionWindow::OnFilterCustomMaterial),
			FOnAssetSelected::CreateSP(this, &SSubstanceOptionWindow::OnCustomMaterialSelected),
			nullptr);
	}

	TSharedRef<SWidget> CreateDefaultMaterialCombo()
	{
		GetDefault<USubstanceSettings>()->DefaultTemplateMaterial.LoadSynchronous();
		UMaterial* SelectedMaterial = (UMaterial*)GetDefault<USubstanceSettings>()->DefaultTemplateMaterial.Get();

		if (SubstanceIncludedMaterials.Contains(SelectedMaterial))
			SelectedDefaultMaterial = SelectedMaterial;
		else if (SubstanceIncludedMaterials.Num() > 0)
			SelectedDefaultMaterial = SubstanceIncludedMaterials[0];

		SelectedMaterialText = SNew(STextBlock)
			.Text(FText::FromString(SelectedDefaultMaterial->GetName()));

		return SNew(SComboBox<UMaterial*>)
			.OptionsSource(&SubstanceIncludedMaterials)
			.InitiallySelectedItem(SelectedDefaultMaterial)
			.IsEnabled(this, &SSubstanceOptionWindow::CreateMaterial)
			.OnGenerateWidget(this, &SSubstanceOptionWindow::GenerateComboWidget)
			.OnSelectionChanged(this, &SSubstanceOptionWindow::OnDefaultMaterialSelected)
			.Content()
			[
				SelectedMaterialText.ToSharedRef()
			];
	}

	void OnDefaultMaterialSelected(UMaterial* mat, ESelectInfo::Type infotype)
	{
		SelectedDefaultMaterial = mat;
		SelectedMaterialText->SetText(FText::FromString(SelectedDefaultMaterial->GetName()));
	}

	TSharedRef<SWidget> GenerateComboWidget(UMaterial* Item)
	{
		return  SNew(STextBlock)
			.Text(FText::FromString(Item->GetName()))
			.ColorAndOpacity(FColor::White);
	}

};

#undef LOCTEXT_NAMESPACE

