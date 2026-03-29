// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SSubstanceEditorPanel.cpp

#include "SSubstanceEditorPanel.h"
#include "SubstanceEditorPrivatePCH.h"
#include "SubstanceEditorModule.h"
#include "SubstanceCoreModule.h"
#include "SubstanceCoreClasses.h"

#include "PropertyCustomizationHelpers.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "MouseDeltaTracker.h"
#include "MaterialGraph/MaterialGraph.h"
#include "ScopedTransaction.h"
#include "LevelEditor.h"
#include "SAssetDropTarget.h"
#include "Misc/ScopedSlowTask.h"

#include "substance/framework/graph.h"
#include "substance/framework/input.h"
#include "substance/framework/typedefs.h"

#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Input/SCheckBox.h"

#include "Misc/MessageDialog.h"

#include "Engine/Selection.h"
#include "Editor.h"

#include "SubstanceInput.inl"

#define LOC_NAMESPACE TEXT("SubstanceEditor")
#define LOCTEXT_NAMESPACE "SubstanceEditor"

SSubstanceEditorPanel::~SSubstanceEditorPanel()
{
	if (Graph && HasPresetChanged())
	{
		FinalizeSubstanceChanges();
	}
}

TWeakPtr<ISubstanceEditor> SSubstanceEditorPanel::GetSubstanceEditor() const
{
	return SubstanceEditorPtr;
}

void SSubstanceEditorPanel::OnRedo()
{
	OnUndo();
}

void SSubstanceEditorPanel::OnUndo()
{
	//The input instances with the graph and their desc
	for (const auto& ItInInst : Graph->Instance->getInputs())
	{
		if (ItInInst->mDesc.isImage())
		{
			SubstanceAir::InputInstanceImage* ImgInput = (SubstanceAir::InputInstanceImage*)ItInInst;
			if (!Graph->LinkImageInput(ImgInput))
			{
				ImgInput->reset();
			}
		}
	}

	ConstructInputs();
	ConstructWidget();
	Substance::Helpers::RenderAsync(Graph->Instance);
}

void SSubstanceEditorPanel::Construct(const FArguments& InArgs)
{
	SubstanceEditorPtr = InArgs._SubstanceEditor;

	OutputSizePow2Min = 5;
	OutputSizePow2Max = FMath::FloorLog2(FModuleManager::GetModuleChecked<ISubstanceCore>("SubstanceCore").GetMaxOutputTextureSize());

	Graph = SubstanceEditorPtr.Pin()->GetGraph();

	if (!Graph->ParentFactory || !Graph->Instance)
	{
		UE_LOG(LogSubstanceEditor, Error, TEXT("Invalid Substance Graph Instance, cannot edit the instance."));
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
#if ENGINE_MAJOR_VERSION > 4
	ThumbnailPool = UThumbnailManager::Get().GetSharedThumbnailPool();
#else
	ThumbnailPool = LevelEditorModule.GetFirstLevelEditor()->GetThumbnailPool();
#endif
	CachePresetValues();
	ConstructDescription();
	ConstructOutputs();
	ConstructInputs();

	ConstructWidget();
}

void SSubstanceEditorPanel::CachePresetValues()
{
	LastCachedValuesPreset = Substance::Helpers::GetPresetAsXMLString(Graph);
}

bool SSubstanceEditorPanel::HasPresetChanged()
{
	return (LastCachedValuesPreset != Substance::Helpers::GetPresetAsXMLString(Graph));
}

void SSubstanceEditorPanel::ConstructDescription()
{
	DescArea = SNew(SExpandableArea)
	           .AreaTitle(FText::FromString(TEXT("Graph Description:")))
	           .InitiallyCollapsed(false)
	           .BodyContent()
	           [
	               SNew(SBorder)
	               .Content()
	               [
	                   SNew(SVerticalBox)
	                   + SVerticalBox::Slot()
	                   .Padding(0.1f)
	                   .AutoHeight()
	                   [
	                       SNew(SHorizontalBox)
	                       + SHorizontalBox::Slot()
	                       .FillWidth(0.3f)
	                       [
	                           SNew(STextBlock)
	                           .Text(FText::FromString(TEXT("Label:")))
	                       ]
	                       + SHorizontalBox::Slot()
	                       .Padding(0.1f)
	                       [
	                           SNew(STextBlock)
	                           .Text(FText::FromString(Graph->Instance->mDesc.mLabel.c_str()))
	                       ]
	                   ]
	                   + SVerticalBox::Slot()
	                   .Padding(0.1f)
	                   .AutoHeight()
	                   [
	                       SNew(SHorizontalBox)
	                       + SHorizontalBox::Slot()
	                       .FillWidth(0.3f)
	                       [
	                           SNew(STextBlock)
	                           .Text(FText::FromString(TEXT("Description:")))
	                       ]
	                       + SHorizontalBox::Slot()
	                       .Padding(0.1f)
	                       [
	                           SNew(STextBlock)
	                           .AutoWrapText(true)
	                           .Text(Graph->Instance->mDesc.mDescription.length() ? FText::FromString(Graph->Instance->mDesc.mDescription.c_str()) : FText::FromString(TEXT("N/A")))
	                       ]
	                   ]
	                   + SVerticalBox::Slot()
	                   .AutoHeight()
	                   .Padding(0.0f, 3.0f)
	                   [
	                       SNew(SHorizontalBox)
	                       + SHorizontalBox::Slot()
	                       .FillWidth(0.3f)
	                       [
	                           SNew(STextBlock)
	                           .Text(FText::FromString(TEXT("Runtime modifications:")))
	                       ]
	                       + SHorizontalBox::Slot()
	                       .Padding(0.1f)
	                       [
	                           SNew(SCheckBox)
	                           .OnCheckStateChanged(this, &SSubstanceEditorPanel::OnFreezeGraphValueChanged)
	                           .IsChecked(this, &SSubstanceEditorPanel::GetFreezeGraphValue)
	                           .Content()
	                           [
	                               SNew(STextBlock)
	                               .Text(FText::FromString(TEXT("Disable")))
	                           ]
	                       ]
	                   ]
	               ]
	           ];
}

void SSubstanceEditorPanel::ConstructOutputs()
{
	TSharedPtr<SVerticalBox> OutputsBox = SNew(SVerticalBox);

	for (const auto& OutputItr : Graph->Instance->getOutputs())
	{
		FReferencerInformationList Refs;
		UObject* TextureObject = nullptr;
		if (OutputItr->mUserData != 0 && OutputItr->mDesc.isImage())
		{
			TextureObject = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData();
		}
		else if(OutputItr->mDesc.isNumerical())
		{
			continue;
		}

		const SubstanceAir::OutputDesc* OutputDesc = &OutputItr->mDesc;

		OutputsBox->AddSlot()
		[
		    SNew(SHorizontalBox)
		    + SHorizontalBox::Slot()
		    .FillWidth(0.3f)
		    [
		        SNew(SHorizontalBox)
		        + SHorizontalBox::Slot()
		        [
		            SNew(STextBlock)
		            .Text(OutputDesc ? FText::FromString(OutputDesc->mLabel.c_str()) : FText::FromString(TEXT("Invalid output")))
		        ]
		        + SHorizontalBox::Slot()
		        .HAlign(HAlign_Right)
		        [
		            PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP(this, &SSubstanceEditorPanel::OnBrowseTexture, OutputItr))
		        ]
		    ]
		    + SHorizontalBox::Slot()
		    [
		        SNew(SCheckBox)
		        .OnCheckStateChanged(this, &SSubstanceEditorPanel::OnToggleOutput, OutputItr)
		        .IsChecked(this, &SSubstanceEditorPanel::GetOutputState, OutputItr)
		        .IsEnabled(nullptr != OutputDesc)
		        .ToolTipText(FText::FromString(OutputDesc->mIdentifier.c_str()))
		    ]
		];
	}

	OutputsArea = SNew(SExpandableArea)
	              .AreaTitle(FText::FromString(TEXT("Outputs:")))
	              .InitiallyCollapsed(false)
	              .BodyContent()
	              [
	                  SNew(SBorder)
	                  .BorderBackgroundColor(FLinearColor(0.35, 0.35, 0.35))
	                  .ForegroundColor(FEditorStyle::GetColor("TableView.Header", ".Foreground"))
	                  .Content()
	                  [
	                      OutputsBox.ToSharedRef()
	                  ]
	              ];
}

void SSubstanceEditorPanel::ConstructWidget()
{

	TSharedPtr<SVerticalBox> ChildWidget =
	    SNew(SVerticalBox)
	    + SVerticalBox::Slot()
	    .AutoHeight()
	    .Padding(0.0f, 3.0f)
	    [
	        DescArea.ToSharedRef()
	    ]
	    + SVerticalBox::Slot()
	    .AutoHeight()
	    .Padding(0.0f, 3.0f)
	    [
	        OutputsArea.ToSharedRef()
	    ];

	if (InputsArea.IsValid())
	{
		ChildWidget->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
		    InputsArea.ToSharedRef()
		];
	}

	if (ImageInputsArea.IsValid())
	{
		ChildWidget->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
		    ImageInputsArea.ToSharedRef()
		];
	}

	ChildSlot
	[
	    SNew(SScrollBox)
	    + SScrollBox::Slot()
	    .Padding(0.0f)
	    [
	        SNew(SVerticalBox)
	        + SVerticalBox::Slot()
	        .AutoHeight()
	        .Padding(0.0f)
	        [
	            ChildWidget.ToSharedRef()
	        ]
	    ]
	];
}

void ActualizeMaterialGraph(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	for (const auto& OutputItr : Instance->getOutputs())
	{
		if (OutputItr->mUserData == 0)
			continue;

		FReferencerInformationList Refs;
		UObject* TextureObject = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData();

		//Check and see whether we are referenced by any objects that won't be garbage collected.
		bool bIsReferenced = TextureObject ? IsReferenced(TextureObject, GARBAGE_COLLECTION_KEEPFLAGS, EInternalObjectFlags::GarbageCollectionKeepFlags, true, &Refs) : false;

		TArray<UMaterial*> RefMaterials;

		for (const auto& RefItr : Refs.ExternalReferences)
		{
			UMaterial* Material = Cast<UMaterial>(RefItr.Referencer);

			if (Material && Material->MaterialGraph)
			{
				RefMaterials.AddUnique(Material);
			}
		}

		for (const auto& MatItr : RefMaterials)
		{
			MatItr->MaterialGraph->RebuildGraph();
		}
	}
}

void SSubstanceEditorPanel::OnToggleOutput(ECheckBoxState InNewState, SubstanceAir::OutputInstance* Output)
{
	UTexture* Texture = nullptr;
	USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);
	if (Output->mUserData != 0 && OutputData->GetData())
		Texture = Cast<UTexture2D>(OutputData->GetData());

	if (InNewState == ECheckBoxState::Checked)
	{
		Substance::Helpers::EnableTexture(Output, Graph);

		FinalizeSubstanceChanges();

		OutputData = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);

		if (Output->mDesc.mType == Substance_IOType_Image)
		{
			TArray<UObject*> NewTexture;
			NewTexture.Add(OutputData->GetData());

			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(NewTexture);
		}

		// Should rebuild the graph of any material using a substance output
		Substance::Helpers::GenerateMaterialExpressions(Graph->Instance.get(), Graph->ConstantCreatedMaterial, Graph);
	}
	else if (InNewState == ECheckBoxState::Unchecked)
	{

		if (reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData())
		{
			OutputData = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);
			auto Texture2D = Cast<UTexture2D>(OutputData->GetData());
			Substance::Helpers::RegisterForDeletion(Texture2D);
			auto mat = Graph->ConstantCreatedMaterial;
			for (int i = 0; i < mat->TextureParameterValues.Num(); i++)
			{
				if (mat->TextureParameterValues[i].ParameterInfo == OutputData->ParamInfo)
				{
					mat->TextureParameterValues.RemoveAt(i);
					break;
				}
			}
			mat->PostEditChange();
			OutputData->CacheGuid.Invalidate();
			OutputData->SetData(nullptr);
		}
		Output->mEnabled = false;
		ActualizeMaterialGraph(Graph->Instance);
	}
}

ECheckBoxState SSubstanceEditorPanel::GetOutputState(SubstanceAir::OutputInstance* Output) const
{
	ECheckBoxState CurrentState = ECheckBoxState::Unchecked;
	USubstanceOutputData* data = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);
	if (data != 0)
	{
		(data->GetData() != 0) ? CurrentState = ECheckBoxState::Checked : CurrentState = ECheckBoxState::Unchecked;
	}
	return CurrentState;
}

void SSubstanceEditorPanel::OnBrowseTexture(SubstanceAir::OutputInstance* Output)
{
	if (Output->mUserData && reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData())
	{
		TArray<UObject*> Objects;
		Objects.Add(reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData());
		GEditor->SyncBrowserToObjects(Objects);
	}
}

void SSubstanceEditorPanel::OnBrowseImageInput(SubstanceAir::InputInstanceImage* ImageInput)
{
	if (ImageInput->getImage())
	{
		TArray<UObject*> Objects;
		Objects.Add(reinterpret_cast<UTexture2D*>(ImageInput->getImage().get()->mUserData));
		GEditor->SyncBrowserToObjects(Objects);
	}
}

TSharedRef<SHorizontalBox> SSubstanceEditorPanel::GetBaseInputWidget(SubstanceAir::InputInstanceBase* Input, FString InputLabel)
{
	if (0 == InputLabel.Len())
	{
		InputLabel = Input->mDesc.mLabel.c_str();
	}

	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       [
	           SNew(STextBlock)
	           .Text(FText::FromString(InputLabel))
	           .ToolTipText(FText::FromString(Input->mDesc.mIdentifier.c_str()))
	           .Font(FEditorStyle::GetFontStyle("BoldFont"))
	       ]
	       + SHorizontalBox::Slot()
	       .AutoWidth()
	       .HAlign(HAlign_Left)
	       .VAlign(VAlign_Center)
	       [
	           SNew(SButton)
	           .ToolTipText(NSLOCTEXT("PropertyEditor", "ResetToDefaultToolTip", "Reset to Default"))
	           .ButtonStyle(FEditorStyle::Get(), "NoBorder")
	           .OnClicked(this, &SSubstanceEditorPanel::OnResetInput, Input)
	           .Content()
	           [
	               SNew(SImage)
	               .Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
	           ]
	       ];
}

void SSubstanceEditorPanel::ConstructInputs()
{
	TSharedPtr<SScrollBox> InputsBox = TSharedPtr<SScrollBox>(nullptr);
	TSharedPtr<SScrollBox> ImageInputsBox = TSharedPtr<SScrollBox>(nullptr);

	InputsArea = TSharedPtr<SExpandableArea>(nullptr);
	ImageInputsArea = TSharedPtr<SExpandableArea>(nullptr);

	TMap<FString, TSharedPtr<SVerticalBox>> Groups;

	// make a copy of the list of inputs to sort them by index
	std::vector<SubstanceAir::InputInstanceBase*> IdxSortedInputs;

	for (const auto& Sptr : Graph->Instance->getInputs())
	{
		IdxSortedInputs.push_back(Sptr);
	}

	struct FCompareInputIdx
	{
		FORCEINLINE bool operator()(const SubstanceAir::InputInstanceBase* A, const SubstanceAir::InputInstanceBase* B) const
		{
			return atoi(A->mDesc.mUserTag.c_str()) < atoi(B->mDesc.mUserTag.c_str());
		}
	};

	std::sort(IdxSortedInputs.begin(), IdxSortedInputs.begin() + Graph->Instance->getInputs().size(), FCompareInputIdx());

	for (SubstanceAir::InputInstanceBase* ItIn : IdxSortedInputs)
	{
		if (ItIn->mDesc.mIdentifier == "$pixelsize" || ItIn->mDesc.mIdentifier == "$time" || ItIn->mDesc.mIdentifier == "$normalformat")
		{
			continue;
		}

		if (ItIn->mDesc.isNumerical())
		{
			if (false == InputsBox.IsValid())
			{
				InputsBox =
				    SNew(SScrollBox)
				    .IsEnabled(this, &SSubstanceEditorPanel::GetInputsEnabled);

				InputsArea =
				    SNew(SExpandableArea)
				    .AreaTitle(FText::FromString(TEXT("Inputs:")))
				    .InitiallyCollapsed(false)
				    .BodyContent()
				    [
				        SNew(SBorder)
				        .Content()
				        [
				            InputsBox.ToSharedRef()
				        ]
				    ];
			}

			FString GroupName = ItIn->mDesc.mGuiGroup.c_str();

			if (GroupName.Len())
			{
				TSharedPtr<SVerticalBox> GroupBox;

				if (Groups.Contains(GroupName))
				{
					GroupBox = *Groups.Find(GroupName);
				}
				else
				{
					GroupBox = SNew(SVerticalBox);
					Groups.Add(GroupName, GroupBox);
				}

				GroupBox->AddSlot()
				[
				    SNew(SSeparator)
				    .Orientation(EOrientation::Orient_Horizontal)
				];

				GroupBox->AddSlot()
				.AutoHeight()
				.Padding(10.0f, 0.0f)
				[
				    GetInputWidget(ItIn)
				];
			}
			else
			{
				InputsBox->AddSlot()
				[
				    GetInputWidget(ItIn)
				];

				InputsBox->AddSlot()
				[
				    SNew(SSeparator)
				    .Orientation(EOrientation::Orient_Horizontal)
				];
			}
		}
		else if (ItIn->mDesc.isString())
		{
			if (false == InputsBox.IsValid())
			{
				InputsBox =
				    SNew(SScrollBox)
				    .IsEnabled(this, &SSubstanceEditorPanel::GetInputsEnabled);

				InputsArea =
				    SNew(SExpandableArea)
				    .AreaTitle(FText::FromString(TEXT("Inputs:")))
				    .InitiallyCollapsed(false)
				    .BodyContent()
				    [
				        SNew(SBorder)
				        .Content()
				        [
				            InputsBox.ToSharedRef()
				        ]
				    ];
			}

			FString GroupName = ItIn->mDesc.mGuiGroup.c_str();

			if (GroupName.Len())
			{
				TSharedPtr<SVerticalBox> GroupBox;

				if (Groups.Contains(GroupName))
				{
					GroupBox = *Groups.Find(GroupName);
				}
				else
				{
					GroupBox = SNew(SVerticalBox);
					Groups.Add(GroupName, GroupBox);
				}

				GroupBox->AddSlot()
				[
				    SNew(SSeparator)
				    .Orientation(EOrientation::Orient_Horizontal)
				];

				GroupBox->AddSlot()
				.AutoHeight()
				.Padding(10.0f, 0.0f)
				[
				    GetInputWidget(ItIn)
				];
			}
			else
			{
				InputsBox->AddSlot()
				[
				    GetInputWidget(ItIn)
				];

				InputsBox->AddSlot()
				[
				    SNew(SSeparator)
				    .Orientation(EOrientation::Orient_Horizontal)
				];
			}
		}
		else
		{
			if (false == ImageInputsBox.IsValid())
			{
				ImageInputsBox = SNew(SScrollBox);

				ImageInputsArea = SNew(SExpandableArea)
				                  .AreaTitle(FText::FromString(TEXT("Image Inputs:")))
				                  .InitiallyCollapsed(false)
				                  .BodyContent()
				                  [
				                      SNew(SBorder)
				                      .Content()
				                      [
				                          ImageInputsBox.ToSharedRef()
				                      ]
				                  ];
			}

			ImageInputsBox->AddSlot()
			[
			    SNew(SVerticalBox)
			    + SVerticalBox::Slot()
			    [
			        GetImageInputWidget(ItIn)
			    ]
			];
		}
	}

	TMap< FString, TSharedPtr< SVerticalBox >>::TIterator ItGroups(Groups);

	FLinearColor OddColor(.35, .35, .35, 0.0f);
	FLinearColor EvenColor(.4, .4, .4, 0.0f);

	int32 Idx = 0;

	for (; ItGroups; ++ItGroups)
	{
		InputsBox->AddSlot()
		[
		    SNew(SExpandableArea)
		    .AreaTitle(FText::FromString(ItGroups.Key()))
		    .InitiallyCollapsed(false)
		    .BodyContent()
		    [
		        ItGroups.Value().ToSharedRef()
		    ]
		];

		InputsBox->AddSlot()
		[
		    SNew(SSeparator)
		    .Orientation(EOrientation::Orient_Horizontal)
		];
	}
}

void SSubstanceEditorPanel::ResetThumbnailInputs(SubstanceAir::GraphInstance* graphInstance)
{
	for (const auto& InputItr : graphInstance->getInputs())
	{
		if (InputItr->mDesc.isImage())
		{
			SubstanceAir::InputInstanceImage* TypedInput = static_cast<SubstanceAir::InputInstanceImage*>(InputItr);
			ThumbnailInputs[TypedInput]->SetAsset(nullptr);
		}
	}
}

void SSubstanceEditorPanel::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	if (!HasPresetChanged() || !Graph || HasKeyboardFocus())
		return;

	bool bCache = true;

	if (NewWidgetPath.TopLevelWindow != nullptr && NewWidgetPath.GetWindow()->GetTitle().ToString() != Graph->GetName())
	{
		TSharedPtr<SWindow> window = NewWidgetPath.GetWindow()->GetParentWindow();
		while (window)
		{
			if (window->GetTitle().ToString() == Graph->GetName())
			{
				bCache = false;
			}
			window = window->GetParentWindow();
		}
	}
	else
	{
		bCache = false;
	}

	if (bCache)
	{
		FinalizeSubstanceChanges();
	}
}

void SSubstanceEditorPanel::FinalizeSubstanceChanges()
{
	FScopedSlowTask SlowTask(1, LOCTEXT("ProcessSubstanceOutputs", "Generating Substance UTexture2D Data"));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();

	CachePresetValues();
	Graph->PrepareOutputsForSave();
	Substance::Helpers::RenderSync(Graph->Instance, true);
	Graph->SaveAllOutputs();
}


void SSubstanceEditorPanel::OnFreezeGraphValueChanged(ECheckBoxState InNewState)
{
	check(Graph);

	if (Graph)
	{
		Graph->bIsFrozen = ECheckBoxState::Checked == InNewState ? true : false;
		Graph->Modify();
	}
}

ECheckBoxState SSubstanceEditorPanel::GetFreezeGraphValue() const
{
	if (Graph)
	{
		return Graph->bIsFrozen ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	return ECheckBoxState::Unchecked;
}

template< typename T> void SSubstanceEditorPanel::SetValue(T NewValue, SubstanceAir::InputInstanceBase* Input, int32 Index)
{
	//TODO:: (UE4-300) Check to make sure the value is in range and clamp the value!
	FInputValue<T> value = { NewValue, Input, Index };
	SetValues(1, &value);
}

template<typename T> void SSubstanceEditorPanel::SetValues(uint32 NumInputValues, FInputValue<T>* Inputs)
{
	bool renderAsync = false;

	SubstanceAir::InputInstanceNumericalBase* InputInst = (SubstanceAir::InputInstanceNumericalBase*)Inputs[0].Input;
	TArray<T> NewValueSet;
	Substance::GetNumericalInputValue<T>(InputInst, NewValueSet);

	//Early out
	if (NewValueSet.Num() == 0 || NumInputValues == 0)
	{
		UE_LOG(LogSubstanceEditor, Warning, TEXT("Attempted to update an invalid input parameter"));
		return;
	}

	for (uint32 i = 0; i < NumInputValues; i++)
	{
		FInputValue<T>& currentInput = Inputs[i];

		//Make sure the value has changed
		if (NewValueSet[currentInput.Index] == currentInput.NewValue)
		{
			continue;
		}

		NewValueSet[currentInput.Index] = currentInput.NewValue;
		renderAsync = true;
	}

	Substance::Helpers::SetNumericalInputValue(InputInst, NewValueSet);
	Substance::Helpers::UpdateInput(Graph->Instance, InputInst, NewValueSet);

	if (renderAsync)
	{
		Substance::Helpers::RenderAsync(Graph->Instance);
		Graph->MarkPackageDirty();
	}
}

template<typename T> TOptional<T> SSubstanceEditorPanel::GetInputValue(SubstanceAir::InputInstanceBase* Input, int32 Index) const
{
	SubstanceAir::InputInstanceNumerical<T>* TypedInst = (SubstanceAir::InputInstanceNumerical<T>*)Input;

	TArray<T> Values;
	Substance::GetNumericalInputValue<T>((SubstanceAir::InputInstanceNumericalBase*)Input, Values);
	return Values[Index];
}

FText SSubstanceEditorPanel::GetInputValue(SubstanceAir::InputInstanceBase* Input) const
{
	if (Input->mDesc.isString())
	{
		SubstanceAir::InputInstanceString* StringInst = (SubstanceAir::InputInstanceString*)(Input);
		return FText::FromString(StringInst->getString().c_str());
	}

	SubstanceAir::InputInstanceInt* TypedInst = (SubstanceAir::InputInstanceInt*)Input;
	SubstanceAir::InputDescInt* TypedDesc = (SubstanceAir::InputDescInt*)&TypedInst->mDesc;
	int32 CurrentValue = TypedInst->getValue();

	//NOTE:: We cannot index into the enum values as they are not always 0 index based.
	//Instead we much loop through until we find the current value pair.
	auto EnItr = TypedDesc->mEnumValues.begin();
	for (; EnItr != TypedDesc->mEnumValues.end(); ++EnItr)
	{
		if (EnItr->first == CurrentValue)
		{
			return FText::FromString(EnItr->second.c_str());
		}
	}

	return FText();
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidgetCombobox(SubstanceAir::InputInstanceBase* Input)
{
	SubstanceAir::InputDescInt* TypedDesc = (SubstanceAir::InputDescInt*)&Input->mDesc;
	SharedFStringArray* ItemsForWidget = new SharedFStringArray();
	TSharedPtr<FString> CurrentValue;

	//Get all of the labels for the combo box
	for (const auto& mEnumValue : TypedDesc->mEnumValues)
	{
		TSharedPtr<FString> enumValue = MakeShareable(new FString(mEnumValue.second.c_str()));
		ItemsForWidget->Emplace(enumValue);
		if (FString(mEnumValue.second.c_str()) == FString(GetInputValue(Input).ToString()))
			CurrentValue = enumValue;
	}

	//Store the array within the class member so that it persists as long as the editor window is open
	ComboBoxLabels.Emplace(MakeShareable(ItemsForWidget));

	TSharedRef<SComboBox<TSharedPtr<FString>>> ComboBox =
	    SNew(SComboBox<TSharedPtr<FString>>)
	    .OptionsSource(ComboBoxLabels.Last().Get())
	    .OnGenerateWidget(this, &SSubstanceEditorPanel::MakeInputComboWidget)
	    .OnSelectionChanged(this, &SSubstanceEditorPanel::OnComboboxSelectionChanged, Input)
	    .InitiallySelectedItem(CurrentValue)
	    .ContentPadding(0)
	    [
	        SNew(STextBlock)
	        .Text(this, &SSubstanceEditorPanel::GetInputValue, Input)
	    ];

	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       .FillWidth(0.3f)
	       [
	           GetBaseInputWidget(Input)
	       ]
	       + SHorizontalBox::Slot()
	       [
	           ComboBox
	       ];
}

TSharedRef<SWidget> SSubstanceEditorPanel::MakeInputComboWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
	       .Text(FText::FromString(*InItem));
}

bool SSubstanceEditorPanel::OnAssetDraggedOver(const UObject* InObject) const
{
	return (InObject && Cast<UTexture2D>(InObject));
}

void SSubstanceEditorPanel::OnAssetDropped(UObject* InObject, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail)
{
	OnSetImageInput(InObject, Input, Thumbnail);
}

void SSubstanceEditorPanel::OnComboboxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, SubstanceAir::InputInstanceBase* Input/*, TSharedRef<STextBlock> ComboText*/)
{
	SubstanceAir::InputDescInt* TypedDesc = (SubstanceAir::InputDescInt*)&Input->mDesc;

	if (!Item.IsValid() || TypedDesc->mEnumValues.empty())
	{
		return;
	}

	for (const auto& EnItr : TypedDesc->mEnumValues)
	{
		if (EnItr.second.c_str() == *Item)
		{
			FInputValue<int> value = { EnItr.first, Input, 0 };
			SetValues<int>(1, &value);
		}
	}
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidgetTogglebutton(SubstanceAir::InputInstanceBase* Input)
{
	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       .FillWidth(0.3f)
	       [
	           GetBaseInputWidget(Input)
	       ]
	       + SHorizontalBox::Slot()
	       [
	           SNew(SCheckBox)
	           .OnCheckStateChanged(this, &SSubstanceEditorPanel::OnToggleValueChanged, Input)
	           .IsChecked(this, &SSubstanceEditorPanel::GetToggleValue, Input)
	       ];
}

void SSubstanceEditorPanel::OnToggleValueChanged(ECheckBoxState InNewState, SubstanceAir::InputInstanceBase* Input)
{
	FScopedTransaction Transaction(NSLOCTEXT("SubstanceEditor", "SubstanceSetInput", "Substance set input"));
	Graph->Modify();

	FInputValue<int32> value = { 1, Input, 0 };

	if (InNewState == ECheckBoxState::Checked)
	{
		value.NewValue = 1;
		SetValues<int>(1, &value);
	}
	else
	{
		value.NewValue = 0;
		SetValues<int>(1, &value);
	}
}

ECheckBoxState SSubstanceEditorPanel::GetToggleValue(SubstanceAir::InputInstanceBase* Input) const
{
	ECheckBoxState State;
	(GetInputValue<int32>(Input, 0).GetValue() > 0.0f) ? State = ECheckBoxState::Checked : State = ECheckBoxState::Unchecked;
	return State;
}

void SSubstanceEditorPanel::UpdateColor(FLinearColor NewColor, SubstanceAir::InputInstanceBase* Input)
{
	if (Substance_IOType_Float == Input->mDesc.mType)
	{
		FInputValue<float> value = { NewColor.Desaturate(1.0f).R, Input, 0 };
		SetValues<float>(1, &value);
		return;
	}

	FInputValue<float> values[] =
	{
		{ NewColor.R, Input, 0 },
		{ NewColor.G, Input, 1 },
		{ NewColor.B, Input, 2 },
		{ NewColor.A, Input, 3 },
	};

	if (Substance_IOType_Float4 == Input->mDesc.mType)
	{
		SetValues<float>(4, values);
	}
	else
	{
		SetValues<float>(3, values);
	}
}

void SSubstanceEditorPanel::CancelColor(FLinearColor OldColor, SubstanceAir::InputInstanceBase* Input)
{
	UpdateColor(OldColor, Input);
}

void SSubstanceEditorPanel::CloseColorPicker(const TSharedRef<SWindow>&)
{
	if (Graph && HasPresetChanged())
	{
		FinalizeSubstanceChanges();
	}
}

void SSubstanceEditorPanel::UpdateString(const FText& NewValue, SubstanceAir::InputInstanceBase* InputInst)
{
	Substance::Helpers::SetStringInputValue(InputInst, NewValue.ToString());
	Substance::Helpers::UpdateInput(Graph->Instance, InputInst, NewValue.ToString());
	Substance::Helpers::RenderAsync(Graph->Instance);
	Graph->MarkPackageDirty();
}

FReply SSubstanceEditorPanel::PickColor(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, SubstanceAir::InputInstanceBase* Input)
{
	FColorPickerArgs PickerArgs;

	FLinearColor InputColor = GetColor(Input);

	if (Input->mDesc.mType == Substance_IOType_Float4)
	{
		PickerArgs.bUseAlpha = true;
	}
	else
	{
		PickerArgs.bUseAlpha = false;
	}

	PickerArgs.ParentWidget = this->AsShared();
	PickerArgs.bOnlyRefreshOnOk = false;
	PickerArgs.bOnlyRefreshOnMouseUp = false;
	PickerArgs.InitialColorOverride = InputColor;
	PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SSubstanceEditorPanel::UpdateColor, Input);
	PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SSubstanceEditorPanel::CancelColor, Input);
	PickerArgs.OnColorPickerWindowClosed = FOnWindowClosed::CreateSP(this, &SSubstanceEditorPanel::CloseColorPicker);
	OpenColorPicker(PickerArgs);

	return FReply::Handled();
}

FText SSubstanceEditorPanel::GetOutputSizeValue(SubstanceAir::InputInstanceBase* Input, int32 Idx) const
{
	SubstanceAir::InputInstanceInt2* TypedInst = (SubstanceAir::InputInstanceInt2*)Input;

	int32 SizeValue = FMath::RoundToInt(FMath::Pow(2.0f, (float)TypedInst->getValue()[Idx]));

	return FText::FromString(FString::Printf(TEXT("%i"), SizeValue));
}

/** Creates the combo box for the output texture resolutions */
TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidgetSizePow2(SubstanceAir::InputInstanceBase* Input)
{
	SharedFStringArray* ItemsForWidget = new SharedFStringArray();

	FString CurX = GetOutputSizeValue(Input, 0).ToString();
	FString CurY = GetOutputSizeValue(Input, 1).ToString();

	TSharedPtr< FString > CurrentX;
	TSharedPtr< FString > CurrentY;

	uint32 CurrentPow2 = OutputSizePow2Min;
	while (CurrentPow2 <= OutputSizePow2Max)
	{
		int32 SizeValue = FMath::Pow(2.0f, (float)CurrentPow2);
		CurrentPow2++;
		ItemsForWidget->Add(MakeShareable(new FString(FString::Printf(TEXT("%i"), SizeValue))));

		if (CurX == FString(FString::Printf(TEXT("%i"), SizeValue)))
		{
			CurrentX = ItemsForWidget->Last();
		}
		if (CurY == FString(FString::Printf(TEXT("%i"), SizeValue)))
		{
			CurrentY = ItemsForWidget->Last();
		}
	}

	ComboBoxLabels.Emplace(MakeShareable(ItemsForWidget));

	WidgetSizeX =
	    SNew(SComboBox<TSharedPtr<FString>>)
	    .InitiallySelectedItem(CurrentX)
	    .OptionsSource(ComboBoxLabels.Last().Get())
	    .OnGenerateWidget(this, &SSubstanceEditorPanel::MakeInputSizeComboWidget)
	    .OnSelectionChanged(this, &SSubstanceEditorPanel::OnSizeComboboxSelectionChanged, Input, 0)
	    .ContentPadding(0)
	    .Content()
	    [
	        SNew(STextBlock)
	        .Text(this, &SSubstanceEditorPanel::GetOutputSizeValue, Input, 0)
	    ];

	WidgetSizeY =
	    SNew(SComboBox<TSharedPtr<FString>>)
	    .InitiallySelectedItem(CurrentY)
	    .OptionsSource(ComboBoxLabels.Last().Get())
	    .OnGenerateWidget(this, &SSubstanceEditorPanel::MakeInputSizeComboWidget)
	    .OnSelectionChanged(this, &SSubstanceEditorPanel::OnSizeComboboxSelectionChanged, Input, 1)
	    .ContentPadding(0)
	    .Content()
	    [
	        SNew(STextBlock)
	        .Text(this, &SSubstanceEditorPanel::GetOutputSizeValue, Input, 1)
	    ];

	RatioLocked.Set(true);

	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       .FillWidth(0.3f)
	       [
	           GetBaseInputWidget(Input, TEXT("Output Size"))
	       ]
	       + SHorizontalBox::Slot()
	       [
	           SNew(SHorizontalBox)
	           + SHorizontalBox::Slot()
	           [
	               WidgetSizeX.ToSharedRef()
	           ]
	           + SHorizontalBox::Slot()
	           [
	               WidgetSizeY.ToSharedRef()
	           ]
	           + SHorizontalBox::Slot()
	           [
	               SNew(SCheckBox)
	               .OnCheckStateChanged(this, &SSubstanceEditorPanel::OnLockRatioValueChanged)
	               .IsChecked(this, &SSubstanceEditorPanel::GetLockRatioValue)
	               .Content()
	               [
	                   SNew(STextBlock)
	                   .Text(FText::FromString(TEXT("Lock Ratio")))
	               ]
	           ]
	       ];
}

void SSubstanceEditorPanel::OnLockRatioValueChanged(ECheckBoxState InNewState)
{
	RatioLocked.Set(InNewState == ECheckBoxState::Checked ? true : false);
}

ECheckBoxState SSubstanceEditorPanel::GetLockRatioValue() const
{
	return RatioLocked.Get() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

TSharedRef<SWidget> SSubstanceEditorPanel::MakeInputSizeComboWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock).Text(FText::FromString(*InItem));
}

void SSubstanceEditorPanel::OnSizeComboboxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, SubstanceAir::InputInstanceBase* Input, int Idx)
{
	if (!Item.IsValid())
	{
		return;
	}

	SubstanceAir::InputInstanceInt2* TypedInst = (SubstanceAir::InputInstanceInt2*)Input;

	float PrevValue = (float)TypedInst->getValue()[Idx];

	int32 SizeValue = FCString::Atoi(**Item);
	int32 NewValue = (int32)FMath::FloorLog2(SizeValue);

	FInputValue<int32> values[2];

	values[0].NewValue = NewValue;
	values[0].Input = Input;
	values[0].Index = Idx;

	FScopedTransaction Transaction(TEXT("SubstanceEd"), NSLOCTEXT("SubstanceEditor", "SubstanceSetOutputSize", "Substance change output size"), Graph);
	Graph->Modify();

	if (RatioLocked.Get())
	{
		const float DeltaValue = NewValue - PrevValue;
		const int OtherIdx = Idx == 0 ? 1 : 0;

		values[1].NewValue = NewValue;
		values[1].Input = Input;
		values[1].Index = OtherIdx;

		//Set selection of each input
		if (WidgetSizeY.IsValid() && Idx == 0 && Item.Get()->Compare(*WidgetSizeY.Get()->GetSelectedItem().Get()) != 0)
		{
			WidgetSizeY.Get()->SetSelectedItem(Item);
			SetValues<int>(2, values);
		}
		else if (WidgetSizeX.IsValid() && Idx == 1 && Item.Get()->Compare(*WidgetSizeX.Get()->GetSelectedItem().Get()) != 0)
		{
			WidgetSizeX.Get()->SetSelectedItem(Item);
			SetValues<int>(2, values);
		}
	}
	else
	{
		SetValues<int>(1, values);
	}

	FSlateApplication::Get().SetAllUserFocus(WidgetSizeX.ToSharedRef());
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidgetRandomSeed(SubstanceAir::InputInstanceBase* Input)
{
	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       .FillWidth(0.3f)
	       [
	           GetBaseInputWidget(Input, TEXT("Random Seed"))
	       ]
	       + SHorizontalBox::Slot()
	       [
	           SNew(SHorizontalBox)
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           [
	               SNew(SButton)
	               .Text(FText::FromString(TEXT("Randomize Seed")))
	               .OnClicked(this, &SSubstanceEditorPanel::RandomizeSeed, Input)
	           ]
	           + SHorizontalBox::Slot()
	           [
	               GetInputWidgetSlider_internal<int32>(Input, 0)
	           ]
	       ];
}

FReply SSubstanceEditorPanel::RandomizeSeed(SubstanceAir::InputInstanceBase* Input)
{
	const int32 rand = FMath::Rand();
	FScopedTransaction Transaction(NSLOCTEXT("SubstanceEditor", "SubstanceSetInput", "Substance set input"));
	Graph->Modify();

	FInputValue<int32> value = { rand, Input, 0 };
	SetValues<int>(1, &value);

	return FReply::Handled();
}

void SSubstanceEditorPanel::OnGetClassesForAssetPicker(TArray<const UClass*>& OutClasses)
{
	OutClasses.AddUnique(UTexture2D::StaticClass());

	// disable substance output as input feature for now
	//OutClasses.AddUnique(USubstanceTexture2D::StaticClass());
}

void SSubstanceEditorPanel::OnAssetSelected(const FAssetData& AssetData, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail)
{
	OnSetImageInput(AssetData.GetAsset(), Input, Thumbnail);
}

void SSubstanceEditorPanel::OnUseSelectedImageInput(SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail)
{
	// Load selected assets
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	USelection* Selection = GEditor->GetSelectedObjects();
	if (Selection)
	{
		UTexture2D* ImageInput = Selection->GetTop<UTexture2D>();
		if (ImageInput)
		{
			OnSetImageInput(ImageInput, Input, Thumbnail);
		}
	}
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetImageInputWidget(SubstanceAir::InputInstanceBase* Input)
{
	const int ThumbSize = 64;

	SubstanceAir::InputDescBase* ImgInputDesc = (SubstanceAir::InputDescBase*)&Input->mDesc;
	SubstanceAir::InputInstanceImage* ImageInput = (SubstanceAir::InputInstanceImage*)Input;

	TSharedPtr<FAssetThumbnail> Thumbnail;
	if (ImageInput->getImage() && ImageInput->getImage()->mUserData != 0)
	{
		UObject* inputImageWrapper = reinterpret_cast<UTexture2D*>(ImageInput->getImage()->mUserData);
		Thumbnail = MakeShareable(new FAssetThumbnail(inputImageWrapper, ThumbSize, ThumbSize, ThumbnailPool));
		ThumbnailInputs.Add(ImageInput, Thumbnail);
	}
	else
	{
		Thumbnail = MakeShareable(new FAssetThumbnail(nullptr, ThumbSize, ThumbSize, ThumbnailPool));
		ThumbnailInputs.Add(ImageInput, Thumbnail);
	}



	return SNew(SHorizontalBox)
	       + SHorizontalBox::Slot()
	       .FillWidth(0.3f)
	       [
	           SNew(SHorizontalBox)
	           + SHorizontalBox::Slot()
	           [
	               SNew(STextBlock)
	               .Text(FText::FromString(Input->mDesc.mLabel.c_str()))
	               .Font(FEditorStyle::GetFontStyle("BoldFont"))
	           ]
	           + SHorizontalBox::Slot()
	           [
	               SNew(STextBlock)
	               .Text(FText::FromString(ImgInputDesc->mGuiDescription.c_str()))
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           [
	               SNew(SSpacer)
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           .VAlign(VAlign_Center)
	           [
	               SNew(SVerticalBox)
	               + SVerticalBox::Slot()
	               .AutoHeight()
	               .HAlign(HAlign_Center)
	               [
	                   SNew(SAssetDropTarget)
	                   .OnIsAssetAcceptableForDrop(this, &SSubstanceEditorPanel::OnAssetDraggedOver)
	                   .OnAssetDropped(this, &SSubstanceEditorPanel::OnAssetDropped, Input, Thumbnail)
	                   [
	                       SNew(SBox)
	                       .WidthOverride(FOptionalSize(ThumbSize))
	                       .HeightOverride(FOptionalSize(ThumbSize))
	                       [
	                           Thumbnail->MakeThumbnailWidget()
	                       ]
	                   ]
	               ]
	           ]

	       ]
	       + SHorizontalBox::Slot()
	       [
	           SNew(SHorizontalBox)
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           .HAlign(HAlign_Center)
	           .VAlign(VAlign_Center)
	           .Padding(2.0f, 1.0f)
	           [
	               PropertyCustomizationHelpers::MakeAssetPickerAnchorButton(
	                   FOnGetAllowedClasses::CreateSP(this, &SSubstanceEditorPanel::OnGetClassesForAssetPicker),
	                   FOnAssetSelected::CreateSP(this, &SSubstanceEditorPanel::OnAssetSelected, Input, Thumbnail))
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           .HAlign(HAlign_Center)
	           .VAlign(VAlign_Center)
	           .Padding(2.0f, 1.0f)
	           [
	               PropertyCustomizationHelpers::MakeUseSelectedButton(FSimpleDelegate::CreateSP(this, &SSubstanceEditorPanel::OnUseSelectedImageInput, Input, Thumbnail))
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           .HAlign(HAlign_Center)
	           .VAlign(VAlign_Center)
	           .Padding(2.0f, 1.0f)
	           [
	               PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP(this, &SSubstanceEditorPanel::OnBrowseImageInput, ImageInput))
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           .HAlign(HAlign_Left)
	           .VAlign(VAlign_Center)
	           .Padding(2.0f, 1.0f)
	           [
	               SNew(SButton)
	               .ToolTipText(NSLOCTEXT("PropertyEditor", "ResetToDefaultToolTip", "Reset to Default"))
	               .ButtonStyle(FEditorStyle::Get(), "NoBorder")
	               .OnClicked(this, &SSubstanceEditorPanel::OnResetImageInput, Input, Thumbnail)
	               .Content()
	               [
	                   SNew(SImage)
	                   .Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
	               ]
	           ]
	           + SHorizontalBox::Slot()
	           .AutoWidth()
	           [
	               SNew(SSpacer)
	           ]
	       ];
}

FString SSubstanceEditorPanel::GetImageInputPath(SubstanceAir::InputInstanceBase* Input)
{
	SubstanceAir::InputInstanceImage* TypedInst = (SubstanceAir::InputInstanceImage*)Input;
	FString PathName;

	if (TypedInst->getImage())
	{
		PathName = reinterpret_cast<UTexture2D*>(TypedInst->getImage()->mUserData)->GetPathName();
	}

	return PathName;
}

FReply SSubstanceEditorPanel::OnResetImageInput(SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail)
{
	OnSetImageInput(nullptr, Input, Thumbnail);
	return FReply::Handled();
}

FReply SSubstanceEditorPanel::OnResetInput(SubstanceAir::InputInstanceBase* Input)
{
	FScopedTransaction Transaction(TEXT("SubstanceEd"), NSLOCTEXT("SubstanceEditor", "SubstanceResetInput", "Substance reset input"), Graph);
	Graph->Modify();

	Substance::Helpers::ResetToDefault(Input);

	for (const auto& OutputItr : Graph->Instance->getOutputs())
	{
		OutputItr->flagAsDirty();
	}

	TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> Graphs;
	Graphs.AddUnique(Graph->Instance);
	Substance::Helpers::RenderAsync(Graphs);

	return FReply::Handled();
}

void SSubstanceEditorPanel::OnSetImageInput(const UObject* InObject, SubstanceAir::InputInstanceBase* Input, TSharedPtr<FAssetThumbnail> Thumbnail)
{
	FScopedTransaction Transaction(TEXT("SubstanceEd"),
	                               InObject ?
	                               NSLOCTEXT("SubstanceEditor", "SubstanceSetInput", "Substance set image input") :
	                               NSLOCTEXT("SubstanceEditor", "SubstanceSetInput", "Substance reset image input"),
	                               Graph);
	Graph->Modify();

	SubstanceAir::InputInstanceImage* InputInst = (SubstanceAir::InputInstanceImage*)Input;

	UObject* NewInput = Cast<UObject>(const_cast<UObject*>(InObject));

	Substance::Helpers::UpdateInput(Graph->Instance, InputInst, NewInput);

	Substance::Helpers::RenderAsync(Graph->Instance);

	Thumbnail->SetAsset(InObject);
	ThumbnailPool->Tick(0.1f);

	CachePresetValues();
}

TSharedRef<SHorizontalBox> SSubstanceEditorPanel::GetInputWidgetString(SubstanceAir::InputInstanceBase* Input)
{
	check(Input);
	check(Input->mDesc.mType == Substance_IOType_String);

	TSharedPtr<SHorizontalBox> InputContainer = SNew(SHorizontalBox);

	TSharedPtr<SEditableTextBox> TextBox =
	    SNew(SEditableTextBox)
	    .Text(this, &SSubstanceEditorPanel::GetInputValue, Input)
	    .OnTextChanged(this, &SSubstanceEditorPanel::UpdateString, Input);

	InputContainer->AddSlot()
	.Padding(0.0f, 1.0f)
	[
	    SNew(SVerticalBox)
	    + SVerticalBox::Slot()
	    [
	        TextBox->AsShared()
	    ]
	];

	TSharedPtr<SHorizontalBox> WidgetsBox =
	    SNew(SHorizontalBox)
	    + SHorizontalBox::Slot()
	    .FillWidth(0.3f)
	    [
	        GetBaseInputWidget(Input)
	    ]
	    + SHorizontalBox::Slot()
	    [
	        InputContainer.ToSharedRef()
	    ];

	return WidgetsBox.ToSharedRef();
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidgetAngle(SubstanceAir::InputInstanceBase* Input)
{
	return GetInputWidgetSlider<float>(Input);
}

TSharedRef<SWidget> SSubstanceEditorPanel::GetInputWidget(SubstanceAir::InputInstanceBase* Input)
{
	// special widget for random seed
	if (Input->mDesc.mIdentifier == "$randomseed")
	{
		return GetInputWidgetRandomSeed(Input);
	}
	if (Input->mDesc.mIdentifier == "$outputsize")
	{
		return GetInputWidgetSizePow2(Input);
	}

	switch (Input->mDesc.mGuiWidget)
	{
	default:
	case SubstanceAir::Input_NoWidget:
	case SubstanceAir::Input_Slider:
	case SubstanceAir::Input_Color:

		switch (Input->mDesc.mType)
		{
		case SubstanceIOType::Substance_IOType_String:
			return GetInputWidgetString(Input);

		case SubstanceIOType::Substance_IOType_Float:
		case SubstanceIOType::Substance_IOType_Float2:
		case SubstanceIOType::Substance_IOType_Float3:
		case SubstanceIOType::Substance_IOType_Float4:
			return GetInputWidgetSlider<float>(Input);

		default:
		case SubstanceIOType::Substance_IOType_Integer:
		case SubstanceIOType::Substance_IOType_Integer2:
		case SubstanceIOType::Substance_IOType_Integer3:
		case SubstanceIOType::Substance_IOType_Integer4:
			return GetInputWidgetSlider<int32>(Input);
		}

	case SubstanceAir::Input_Angle:
		return GetInputWidgetAngle(Input);

	case SubstanceAir::Input_Combobox:
		return GetInputWidgetCombobox(Input);

	case SubstanceAir::Input_Togglebutton:
		return GetInputWidgetTogglebutton(Input);
	}
}

template<typename T> TSharedRef<SHorizontalBox> SSubstanceEditorPanel::GetInputWidgetSlider(SubstanceAir::InputInstanceBase* Input)
{
	check(Input);
	check(Input->mDesc.mType != Substance_IOType_Image);

	int32 SliderCount = 1;

	switch (Input->mDesc.mType)
	{
	case Substance_IOType_Integer2:
	case Substance_IOType_Float2:
		SliderCount = 2;
		break;
	case Substance_IOType_Integer3:
	case Substance_IOType_Float3:
		SliderCount = 3;
		break;
	case Substance_IOType_Integer4:
	case Substance_IOType_Float4:
		SliderCount = 4;
		break;
	}

	TSharedPtr<SHorizontalBox> InputContainer = SNew(SHorizontalBox);

	TSharedPtr<SVerticalBox> SlidersBox = SNew(SVerticalBox);
	for (int32 i = 0; i < SliderCount; ++i)
	{
		SlidersBox->AddSlot()
		[
		    SNew(SVerticalBox)
		    + SVerticalBox::Slot()
		    .Padding(0.0f, 1.0f)
		    [
		        GetInputWidgetSlider_internal<T>(Input, i)
		    ]
		];
	}

	InputContainer->AddSlot()
	[
	    SlidersBox.ToSharedRef()
	];

	if (Input->mDesc.mGuiWidget == SubstanceAir::Input_Color)
	{
		InputContainer->AddSlot()
		.FillWidth(0.1f)
		[
		    SNew(SColorBlock)
		    .Color(this, &SSubstanceEditorPanel::GetColor, Input)
		    .ColorIsHSV(false)
		    .ShowBackgroundForAlpha(true)
		    .ToolTipText(FText::FromString(TEXT("Pick Color")))
		    .OnMouseButtonDown(this, &SSubstanceEditorPanel::PickColor, Input)
		    .UseSRGB(false)
		];
	}

	TSharedPtr<SHorizontalBox> WidgetsBox =
	    SNew(SHorizontalBox)
	    + SHorizontalBox::Slot()
	    .FillWidth(0.3f)
	    [
	        GetBaseInputWidget(Input)
	    ]
	    + SHorizontalBox::Slot()
	    [
	        InputContainer.ToSharedRef()
	    ];

	return WidgetsBox.ToSharedRef();
}

FLinearColor SSubstanceEditorPanel::GetColor(SubstanceAir::InputInstanceBase* Input) const
{
	SubstanceAir::InputInstanceFloat4* InputInst = (SubstanceAir::InputInstanceFloat4*)Input;

	TArray<float> Values;
	Substance::GetNumericalInputValue((SubstanceAir::InputInstanceNumericalBase*)Input, Values);

	FLinearColor InputColor;
	if (Values.Num() == 1)
	{
		InputColor = FLinearColor(Values[0], Values[0], Values[0], 1.0f);
	}
	else if (Values.Num() == 3)
	{
		InputColor = FLinearColor(Values[0], Values[1], Values[2], 1.0f);
	}
	else if (Values.Num() == 4)
	{
		InputColor = FLinearColor(Values[0], Values[1], Values[2], Values[3]);
	}

	return InputColor;
}

void SSubstanceEditorPanel::BeginSliderMovement(SubstanceAir::InputInstanceBase* Input)
{
	FScopedTransaction Transaction(NSLOCTEXT("SubstanceEditor", "SubstanceSetInput", "Substance set input"));
	Graph->Modify();
}

template< typename T, typename U> const U GetMinComponent(const SubstanceAir::InputDescBase* Desc, int Idx, bool& Clamped)
{
	const SubstanceAir::InputDescNumerical<T>* TypedDesc = (SubstanceAir::InputDescNumerical<T>*)Desc;
	const U MinValue = TypedDesc->mMinValue[Idx];
	return MinValue;
}

template< typename T, typename U> const U GetMaxComponent(const SubstanceAir::InputDescBase* Desc, int Idx, bool& Clamped)
{
	SubstanceAir::InputDescNumerical<T>* TypedDesc = (SubstanceAir::InputDescNumerical<T>*)Desc;
	const U MaxValue = TypedDesc->mMaxValue[Idx];
	return MaxValue;
}

template< typename T> void GetMinMaxValues(const SubstanceAir::InputDescBase* Desc, const int32 i, T& Min, T& Max, bool& Clamped)
{
	switch (Desc->mType)
	{
	case Substance_IOType_Integer:
		{
			SubstanceAir::InputDescNumerical<int32>* TypedDesc = (SubstanceAir::InputDescNumerical<int32>*)Desc;
			Min = TypedDesc->mMinValue;
			Max = TypedDesc->mMaxValue;
		}
		break;
	case Substance_IOType_Float:
		{
			SubstanceAir::InputDescNumerical<float>* TypedDesc = (SubstanceAir::InputDescNumerical<float>*)Desc;
			Min = TypedDesc->mMinValue;
			Max = TypedDesc->mMaxValue;
		}
		break;
	case Substance_IOType_Integer2:
		{
			Min = GetMinComponent<SubstanceAir::Vec2Int, int>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec2Int, int>(Desc, i, Clamped);
		}
		break;
	case Substance_IOType_Float2:
		{
			Min = GetMinComponent<SubstanceAir::Vec2Float, float>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec2Float, float>(Desc, i, Clamped);
		}
		break;
	case Substance_IOType_Integer3:
		{
			Min = GetMinComponent<SubstanceAir::Vec3Int, int>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec3Int, int>(Desc, i, Clamped);
		}
		break;
	case Substance_IOType_Float3:
		{
			Min = GetMinComponent<SubstanceAir::Vec3Float, float>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec3Float, float>(Desc, i, Clamped);
		}
		break;
	case Substance_IOType_Integer4:
		{
			Min = GetMinComponent<SubstanceAir::Vec4Int, int>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec4Int, int>(Desc, i, Clamped);
		}
		break;
	case Substance_IOType_Float4:
		{
			Min = GetMinComponent<SubstanceAir::Vec4Float, float>(Desc, i, Clamped);
			Max = GetMaxComponent<SubstanceAir::Vec4Float, float>(Desc, i, Clamped);
		}
		break;
	}
}

template<> TSharedRef<SNumericEntryBox<float>> SSubstanceEditorPanel::GetInputWidgetSlider_internal<float>(SubstanceAir::InputInstanceBase* Input, const int32 SliderIndex)
{
	float delta = 0;
	float SliderMin = 0;
	float SliderMax = 0;
	bool Clamped = false;

	switch (Input->mDesc.mType)
	{
	case Substance_IOType_Float:
		Clamped = static_cast<SubstanceAir::InputInstanceFloat*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Float2:
		Clamped = static_cast<SubstanceAir::InputInstanceFloat2*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Float3:
		Clamped = static_cast<SubstanceAir::InputInstanceFloat3*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Float4:
		Clamped = static_cast<SubstanceAir::InputInstanceFloat4*>(Input)->getDesc().mSliderClamp;
		break;
	default:
		UE_LOG(LogSubstanceEditor, Warning, TEXT("Trying to set clamped value for non float type - Defaults to non-clamped"));
		break;
	}

	GetMinMaxValues(&Input->mDesc, SliderIndex, SliderMin, SliderMax, Clamped);

	float MinValue = SliderMin;
	float MaxValue = SliderMax;

	if (Clamped == false)
	{
		if (Input->mDesc.mGuiWidget == SubstanceAir::Input_Color)
		{
			MinValue = 0.0f;
			MaxValue = 1.0f;
		}
		else
		{
			MinValue = TNumericLimits<float>::Lowest();
			MaxValue = TNumericLimits<float>::Max();
		}
	}

	if (SliderMin == SliderMax || SliderMin > SliderMax)
	{
		if (Input->mDesc.mGuiWidget == SubstanceAir::Input_Color)
		{
			SliderMin = 0.0f;
			SliderMax = 1.0f;
		}
		else
		{
			SliderMin = TNumericLimits<float>::Lowest();
			SliderMax = TNumericLimits<float>::Max();
		}
	}

	return SNew(SNumericEntryBox<float>)
	       .Value(this, &SSubstanceEditorPanel::GetInputValue<float>, Input, SliderIndex)
	       .OnValueChanged(this, &SSubstanceEditorPanel::SetValue<float>, Input, SliderIndex)
	       .OnBeginSliderMovement(this, &SSubstanceEditorPanel::BeginSliderMovement, Input)
	       .ToolTipText(FText::FromString(Input->mDesc.mIdentifier.c_str()))
	       .AllowSpin(true)
	       .Delta(0.001f)
	       .LabelPadding(FMargin(0.0f, 1.1f))
	       .MinValue(MinValue)
	       .MaxValue(MaxValue)
	       .MinSliderValue(SliderMin)
	       .MaxSliderValue(SliderMax);
}

template< > TSharedRef<SNumericEntryBox<int32>> SSubstanceEditorPanel::GetInputWidgetSlider_internal<int32>(SubstanceAir::InputInstanceBase* Input, const int32 SliderIndex)
{
	float delta = 0;
	int32 SliderMin = 0;
	int32 SliderMax = 0;
	bool Clamped = false;

	switch (Input->mDesc.mType)
	{
	case Substance_IOType_Integer:
		Clamped = static_cast<SubstanceAir::InputInstanceInt*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Integer2:
		Clamped = static_cast<SubstanceAir::InputInstanceInt2*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Integer3:
		Clamped = static_cast<SubstanceAir::InputInstanceInt3*>(Input)->getDesc().mSliderClamp;
		break;
	case Substance_IOType_Integer4:
		Clamped = static_cast<SubstanceAir::InputInstanceInt4*>(Input)->getDesc().mSliderClamp;
		break;
	default:
		UE_LOG(LogSubstanceEditor, Warning, TEXT("Trying to set clamped value for non int type - Defaults to non-clamped"));
		break;
	}

	GetMinMaxValues(&Input->mDesc, SliderIndex, SliderMin, SliderMax, Clamped);

	int32 MinValue = SliderMin;
	int32 MaxValue = SliderMax;

	if (Clamped == false)
	{
		MinValue = TNumericLimits<int32>::Lowest();
		MaxValue = TNumericLimits<int32>::Max();
	}

	if (SliderMin == SliderMax || SliderMin > SliderMax)
	{
		SliderMin = TNumericLimits<int32>::Lowest();
		SliderMax = TNumericLimits<int32>::Max();
	}

	return SNew(SNumericEntryBox<int32>)
	       .Value(this, &SSubstanceEditorPanel::GetInputValue<int32>, Input, SliderIndex)
	       .OnValueChanged(this, &SSubstanceEditorPanel::SetValue<int>, Input, SliderIndex)
	       .OnBeginSliderMovement(this, &SSubstanceEditorPanel::BeginSliderMovement, Input)
	       .ToolTipText(FText::FromString(Input->mDesc.mIdentifier.c_str()))
	       .AllowSpin(true)
	       .Delta(0)
	       .LabelPadding(FMargin(0.0f, 1.1f))
	       .MinValue(MinValue)
	       .MaxValue(MaxValue)
	       .MinSliderValue(SliderMin)
	       .MaxSliderValue(SliderMax);
}

#undef LOC_NAMESPACE
