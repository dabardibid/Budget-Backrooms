// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SSubstanceEditorPanel.h

#pragma once
#include "SEditorViewport.h"
#include "AssetThumbnail.h"

#include "SubstanceGraphInstance.h"
#include "SubstanceCoreHelpers.h"

#include "Templates/SharedPointer.h"
#include "Widgets/Input/SComboBox.h"

//Forward declarations
namespace SubstanceAir
{
struct InputDescBase;
class InputInstanceBase;
class InputInstanceImage;
}

class SSubstanceEditorPanel : public SCompoundWidget
{
public:
	/** Slate Arguments */
	SLATE_BEGIN_ARGS(SSubstanceEditorPanel) {}
	SLATE_ARGUMENT(TWeakPtr<ISubstanceEditor>, SubstanceEditor)
	SLATE_END_ARGS()

	/** Destructor */
	~SSubstanceEditorPanel();

	/** SCompoundWidget interface */
	void Construct(const FArguments& InArgs);

	/** Get the substance editor effected by this panel */
	TWeakPtr<ISubstanceEditor> GetSubstanceEditor() const;

	/** Event triggered on Undo */
	void OnUndo();

	/** Event triggered on Redo */
	void OnRedo();

	/** Called when we reset all inputs to reset thumbnails */
	void ResetThumbnailInputs(SubstanceAir::GraphInstance* Graph);

	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;

	void FinalizeSubstanceChanges();

private:
	template<typename T> struct FInputValue
	{
		T NewValue;
		SubstanceAir::InputInstanceBase* Input;
		int32 Index;
	};

	USubstanceGraphInstance* Graph;
	TAttribute<FLinearColor> Color;

	TMap<SubstanceAir::InputInstanceImage*, TSharedPtr<FAssetThumbnail>> ThumbnailInputs;

	/** Pointer back to the Substance editor tool that owns us */
	TWeakPtr<ISubstanceEditor> SubstanceEditorPtr;

	TSharedPtr<SExpandableArea> DescArea;
	TSharedPtr<SExpandableArea> OutputsArea;
	TSharedPtr<SExpandableArea> InputsArea;
	TSharedPtr<SExpandableArea> ImageInputsArea;

	/** Widgets used for the output size combo boxes */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> WidgetSizeX;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> WidgetSizeY;

	typedef TArray<TSharedPtr<FString>> SharedFStringArray;
	TArray<TSharedPtr<SharedFStringArray>> ComboBoxLabels;

	TAttribute<bool> RatioLocked;
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;

	uint32 OutputSizePow2Min;
	uint32 OutputSizePow2Max;

	FString LastCachedValuesPreset;
	void CachePresetValues();
	bool HasPresetChanged();

	void ConstructDescription();
	void ConstructOutputs();
	void ConstructInputs();

	void ConstructWidget();

	bool GetInputsEnabled() const
	{
		return !Graph->bIsFrozen;
	}

	void OnFreezeGraphValueChanged(ECheckBoxState InNewState);
	ECheckBoxState GetFreezeGraphValue() const;

	TSharedRef<SWidget> GetInputWidget(SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> GetImageInputWidget(SubstanceAir::InputInstanceBase* Input);
	FString GetImageInputPath(SubstanceAir::InputInstanceBase* Input);

	template< typename T > void SetValue(T NewValue, SubstanceAir::InputInstanceBase* Input, int32 Index);
	template< typename T > void SetValues(uint32 NumInputValues, FInputValue<T>* Inputs);
	template< typename T > TOptional< T > GetInputValue(SubstanceAir::InputInstanceBase* Input, int32 Index) const;
	FText GetInputValue(SubstanceAir::InputInstanceBase* Input) const;

	void OnToggleValueChanged(ECheckBoxState InNewState, SubstanceAir::InputInstanceBase* Input);
	ECheckBoxState GetToggleValue(SubstanceAir::InputInstanceBase* Input) const;

	void OnToggleOutput(ECheckBoxState InNewState, SubstanceAir::OutputInstance* Input);
	ECheckBoxState GetOutputState(SubstanceAir::OutputInstance*  Input) const;
	void OnBrowseTexture(SubstanceAir::OutputInstance* Texture);

	FReply PickColor(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, SubstanceAir::InputInstanceBase* Input);
	FReply RandomizeSeed(SubstanceAir::InputInstanceBase* Input);

	FLinearColor GetColor(SubstanceAir::InputInstanceBase* Input) const;
	void UpdateColor(FLinearColor NewColor, SubstanceAir::InputInstanceBase* Input);
	void CancelColor(FLinearColor OldColor, SubstanceAir::InputInstanceBase* Input);
	void CloseColorPicker(const TSharedRef<SWindow>&);
	void UpdateString(const FText& NewValue, SubstanceAir::InputInstanceBase* Input);

	void OnComboboxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> MakeInputComboWidget(TSharedPtr<FString> InItem);

	bool OnAssetDraggedOver(const UObject* InObject) const;
	void OnAssetDropped(UObject* InObject, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail);
	void OnSetImageInput(const UObject* InObject, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail);

	FReply OnResetImageInput(SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail);
	FReply OnResetInput(SubstanceAir::InputInstanceBase* Input);
	void OnBrowseImageInput(SubstanceAir::InputInstanceImage* ImageInput);

	void OnUseSelectedImageInput(SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail);
	void OnGetClassesForAssetPicker(TArray<const UClass*>& OutClasses);
	void OnAssetSelected(const FAssetData& AssetData, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail);

	TSharedRef<SWidget> MakeInputSizeComboWidget(TSharedPtr<FString> InItem);
	void OnSizeComboboxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, SubstanceAir::InputInstanceBase* Input, int idx);
	FText GetOutputSizeValue(SubstanceAir::InputInstanceBase* Input, int32 Idx) const;
	ECheckBoxState GetLockRatioValue() const;
	void OnLockRatioValueChanged(ECheckBoxState InNewState);

	void BeginSliderMovement(SubstanceAir::InputInstanceBase* Input);

	TSharedRef<SHorizontalBox> GetBaseInputWidget(SubstanceAir::InputInstanceBase* Input, FString InputLabel = TEXT(""));

	template< typename T > TSharedRef<SHorizontalBox> GetInputWidgetSlider(SubstanceAir::InputInstanceBase* Input);
	template< typename T > TSharedRef<SNumericEntryBox<T>> GetInputWidgetSlider_internal(SubstanceAir::InputInstanceBase* Input, const int32 SliderIndex);
	TSharedRef<SHorizontalBox> GetInputWidgetString(SubstanceAir::InputInstanceBase* Input);

	TSharedRef<SWidget> GetInputWidgetAngle(SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> GetInputWidgetCombobox(SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> GetInputWidgetTogglebutton(SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> GetInputWidgetSizePow2(SubstanceAir::InputInstanceBase* Input);
	TSharedRef<SWidget> GetInputWidgetRandomSeed(SubstanceAir::InputInstanceBase* Input);
};

template< typename T > void GetMinMaxValues(const SubstanceAir::InputDescBase* Desc, const int32 i, T& Min, T& Max, bool& Clamped);

template< > TSharedRef< SNumericEntryBox< float >> SSubstanceEditorPanel::GetInputWidgetSlider_internal<float>(SubstanceAir::InputInstanceBase* Input, const int32 SliderIndex);

template< > TSharedRef< SNumericEntryBox< int32 >> SSubstanceEditorPanel::GetInputWidgetSlider_internal<int32>(SubstanceAir::InputInstanceBase* Input, const int32 SliderIndex);
