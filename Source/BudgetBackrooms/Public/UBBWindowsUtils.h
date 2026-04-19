#pragma once
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#include "Windows/WindowsHWrapper.h"
#endif

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UBBWindowsUtils.generated.h"




UENUM(BlueprintType)
enum class EWindowsMessageBoxButtons : uint8
{
    OK UMETA(DisplayName = "OK"),
    OKCancel UMETA(DisplayName = "OK Cancel"),
    YesNo UMETA(DisplayName = "Yes No"),
    YesNoCancel UMETA(DisplayName = "Yes No Cancel"),
    RetryCancel UMETA(DisplayName = "Retry Cancel"),
    AbortRetryIgnore UMETA(DisplayName = "Abort Retry Ignore")
};

UCLASS()
class BUDGETBACKROOMS_API UBBWindowsUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
    static int32 ShowWindowsMessageBox(FString Message, FString Title, EWindowsMessageBoxButtons ButtonType);

    UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
    static void LockPC();

	UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
	static void ForceEnableHDR(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
	static bool IsGameMinimized();

	UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
    static void REDACTED();

    UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
    static void GetMonitorHDRSpecs(bool& bSupportsHDR, float& MaxLuminance, float& MinLuminance);

    UFUNCTION(BlueprintCallable, Category = "Budget Windows Utils")
    static void AutoConfigureUE4HDR();
};
