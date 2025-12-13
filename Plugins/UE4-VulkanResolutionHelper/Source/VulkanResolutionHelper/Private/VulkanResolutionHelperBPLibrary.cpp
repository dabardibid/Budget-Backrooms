// Copyright Epic Games, Inc. All Rights Reserved.

#include "VulkanResolutionHelperBPLibrary.h"
#include "VulkanResolutionHelper.h"

UVulkanResolutionHelperBPLibrary::UVulkanResolutionHelperBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

TArray<FIntPoint> UVulkanResolutionHelperBPLibrary::GetSupportedResolutions(int32 MonitorIndex)
{
    // Get the display metrics
    FDisplayMetrics DisplayMetrics;
    FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);

    if (MonitorIndex >= DisplayMetrics.MonitorInfo.Num()) {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Monitor Index %d Out of Bounds"), MonitorIndex));
        MonitorIndex = DisplayMetrics.MonitorInfo.Num() - 1;
    }

    // Array to return possible resolutions
    TArray<FIntPoint> PossibleResolutions;

    // Get Monitor's Width & Height
    int32 Width = DisplayMetrics.MonitorInfo[MonitorIndex].NativeWidth;
    int32 Height = DisplayMetrics.MonitorInfo[MonitorIndex].NativeHeight;

    // Find GCD to get the aspect ratio
    int32 Divisor = FMath::GreatestCommonDivisor(Width, Height);
    int32 WidthRatio = Width / Divisor;
    int32 HeightRatio = Height / Divisor;

    // Add Calculated Resolutions
    for (int k = 52; k <= Divisor; k += 4) {
        PossibleResolutions.AddUnique(FIntPoint(k * WidthRatio, k * HeightRatio));
    }

    // Add Current Monitor's Resolution
    PossibleResolutions.AddUnique(FIntPoint(Width, Height));

    return PossibleResolutions;
}

TArray<FIntPoint> UVulkanResolutionHelperBPLibrary::GetPrimaryMonitorSupportedResolutions()
{
    return GetSupportedResolutions(0);
}

int32 UVulkanResolutionHelperBPLibrary::GetNumberOfConnectedMonitors()
{
    FDisplayMetrics DisplayMetrics;
    FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
    return DisplayMetrics.MonitorInfo.Num();
}

void UVulkanResolutionHelperBPLibrary::GetMonitorInfo(int32 MonitorIndex, FString &Name, FString &ID, int32 &NativeWidth,
    int32 &NativeHeight, FIntPoint &MaxResolution, bool &bIsPrimary, int32 &DPI)
{
    FDisplayMetrics DisplayMetrics;
    FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);

    if (MonitorIndex >= DisplayMetrics.MonitorInfo.Num()) {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Monitor Index %d Out of Bounds"), MonitorIndex));
        MonitorIndex = DisplayMetrics.MonitorInfo.Num() - 1;
    }

    Name = DisplayMetrics.MonitorInfo[MonitorIndex].Name;
    ID = DisplayMetrics.MonitorInfo[MonitorIndex].ID;
    NativeWidth = DisplayMetrics.MonitorInfo[MonitorIndex].NativeWidth;
    NativeHeight = DisplayMetrics.MonitorInfo[MonitorIndex].NativeHeight;
    MaxResolution = DisplayMetrics.MonitorInfo[MonitorIndex].MaxResolution;
    bIsPrimary = DisplayMetrics.MonitorInfo[MonitorIndex].bIsPrimary;
    DPI = DisplayMetrics.MonitorInfo[MonitorIndex].DPI;
}
