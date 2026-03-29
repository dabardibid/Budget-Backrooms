// ImageResizUEr
// Plugin code
// Copyright 2015-2022 Turfster / NT Entertainment
// All Rights Reserved.

#pragma once

#include "SImageResizeWidget.h"

#define LOCTEXT_NAMESPACE "SImageResizeWidget"

void SImageResizeWidget::Construct(const FArguments& InArgs)
{
	edgeList.Empty();
	edgeList.Add(MakeShareable(new FString("Clamp")));
	edgeList.Add(MakeShareable(new FString("Reflect")));
	edgeList.Add(MakeShareable(new FString("Wrap")));
	edgeList.Add(MakeShareable(new FString("Zero")));

	filterList.Empty();
	filterList.Add(MakeShareable(new FString("Box")));
	filterList.Add(MakeShareable(new FString("Triangle")));
	filterList.Add(MakeShareable(new FString("Cubic B-Spline")));
	filterList.Add(MakeShareable(new FString("Catmull Rom Cubic Spline")));
	filterList.Add(MakeShareable(new FString("Mitchell-Netrevalli")));

	selectedEdge = 0;
	selectedFilter = 4;
	initialEdgeSelected = edgeList[selectedEdge];
	initialFilterSelected = filterList[selectedFilter];

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(3.0f, 1.0f)
		[
			SNew(SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(91)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Import_DirectoryTitle", "Work directory: "))
						]
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(230)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(3.f, 0.f)
							[
								SNew(SBox)
								.HeightOverride(18)
								.WidthOverride(204)
								.VAlign(VAlign_Center)
								[
									SAssignNew(importDirBlock, SEditableTextBox)
									.BackgroundColor(FSlateColor(FLinearColor(FColor::White)))
									.OnTextChanged(this, &SImageResizeWidget::OnImportDirNameChanged)
									.Text(this, &SImageResizeWidget::GetImportDirName)
									.ToolTipText(LOCTEXT("Work_DirectoryTitle_tooltip", "Location in content browser to work on"))
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Right)
							[
								SNew(SButton)
								.OnClicked(this, &SImageResizeWidget::GetImportHeader)
								.IsEnabled(true)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Text(LOCTEXT("WorkDots", "..."))
								.ToolTipText(FText::FromString(TEXT("Select a root Content Browser directory to work on (including all subdirectories)")))

							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(133)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("res", "Target: "))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(40)
								[
									SNew(SEditableTextBox)
									.OnTextCommitted(this, &SImageResizeWidget::SetWidth)
									.Text(this, &SImageResizeWidget::GetWidth)
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(3.f, 0.f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("x","x"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(40)
								[
									SNew(SEditableTextBox)
									.OnTextCommitted(this, &SImageResizeWidget::SetHeight)
									.Text(this, &SImageResizeWidget::GetHeight)
								]
							]
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(1.f, 0.f)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(85)
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(1.f,0.f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Up","Up"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SCheckBox)
								.IsChecked(this, &SImageResizeWidget::GetUpSample)
								.OnCheckStateChanged(this, &SImageResizeWidget::OnUpSampleChanged)
								.ToolTipText(LOCTEXT("upsampleTooltip", "When checked, upsample smaller images"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(1.f, 0.f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Down", "Down"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SCheckBox)
								.IsChecked(this, &SImageResizeWidget::GetDownSample)
								.OnCheckStateChanged(this, &SImageResizeWidget::OnDownSampleChanged)
								.ToolTipText(LOCTEXT("downsampleTooltip", "When checked, downsample larger images"))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(100)
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("inPlace", "In place?: "))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.IsChecked(this, &SImageResizeWidget::GetInPlace)
								.OnCheckStateChanged(this, &SImageResizeWidget::OnInPlaceChanged)
								.ToolTipText(LOCTEXT("replaceTooltip", "When checked, replaces the texture. Otherwise, writes it to a new directory called resized_images"))
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Edge","Edges: "))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&edgeList)
						.InitiallySelectedItem(initialEdgeSelected)
						.OnSelectionChanged(this, &SImageResizeWidget::HandleEdgeSelectionChanged)
						.OnGenerateWidget(this, &SImageResizeWidget::HandleComboGenerateWidget)
						[
							SNew(STextBlock)
							.Text(this, &SImageResizeWidget::HandleEdgeText)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Empty","    "))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)	// does not actually align right, but fuck it
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Filter","Scaling: "))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&filterList)
							.InitiallySelectedItem(initialFilterSelected)
							.OnSelectionChanged(this, &SImageResizeWidget::HandleFilterSelectionChanged)
							.ToolTipText(this, &SImageResizeWidget::GetFilterTooltip)
							.OnGenerateWidget(this, &SImageResizeWidget::HandleComboGenerateWidget)
							[
								SNew(SBox)
								.WidthOverride(140)
								[
									SNew(STextBlock)
									.Text(this, &SImageResizeWidget::HandleFilterText)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.MaxWidth(160)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ignorelist", "Ignore textures that contain: "))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SEditableTextBox)
						.BackgroundColor(FSlateColor(FLinearColor(FColor::White)))
						.OnTextCommitted(this, &SImageResizeWidget::OnIgnoreChanged)
						.Text(this, &SImageResizeWidget::GetIgnoreList)
						.ToolTipText(FText::FromString(TEXT("Comma separated list of segments")))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.OnClicked(this, &SImageResizeWidget::DoDirectory)
					.IsEnabled(true)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("Do_Stuff", "Go to work"))
				]
			]
		]
	];

	LoadConfig();
}
#undef LOCTEXT_NAMESPACE