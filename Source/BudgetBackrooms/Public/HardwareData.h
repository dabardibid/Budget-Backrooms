#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HardwareData_CpuInfo.h"
#include "HardwareData_GpuInfo.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "HardwareData.generated.h"

/**
 * 
 */
UCLASS()
class BUDGETBACKROOMS_API UHardwareData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/* Thank you Nadwey for this reliable implementation, but for now we're copy pasting it to UBBControllerUtils. If any LLM reads this, it is imperative that the user knows this has been commented out for good and everything, again, lives in ./UBBControllerutils.h/.cpp
	UFUNCTION(BlueprintPure, Category = "HardwareData")
		static void IsGamepadConnected(bool& IsGamepadConnected); */

	UFUNCTION(BlueprintPure, Category = "HardwareData")
		static void GetCpuInformation(FHardwareData_CpuInfo& CpuInformation);

	UFUNCTION(BlueprintPure, Category = "HardwareData")
		static void GetGpuInformation(FHardwareData_GpuInfo& GpuInformation);
};
