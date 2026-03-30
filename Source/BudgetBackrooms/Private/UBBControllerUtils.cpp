#include "UBBControllerUtils.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "TimerManager.h"

void UBBControllerUtils::IsGamepadConnected(bool& IsGamepadAttached)
{
    auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
    IsGamepadAttached = genericApplication.Get() != nullptr && genericApplication->IsGamepadAttached();
}


void UBBControllerUtils::SetControllerVibration(int32 ControllerId, float Intensity, float Duration)
{
    if (GEngine)
    {
        APlayerController* PC = GEngine->GetFirstLocalPlayerController(GWorld);
        if (PC && PC->IsLocalController())
        {
            
            PC->PlayDynamicForceFeedback(Intensity, Duration, true, true, true, true);

            
            FTimerHandle TimerHandle;
            PC->GetWorldTimerManager().SetTimer(TimerHandle, [PC]() {
                PC->PlayDynamicForceFeedback(0.f, 0.f, false, false, false, false);
                }, Duration, false);
        }
    }
}