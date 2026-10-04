//#define _CRT_SECURE_NO_WARNINGS

#include "UBBSteamUtils.h"
#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "ThirdParty/Steamworks/Steamv151/sdk/public/steam/steam_api.h"
#include "../BBHashThing.h"

/*
///// WELCOME // PLEASE FUCKING READ ///////////////////////////////////////////////////////
#
# This version of the game is pretentious and I've manually updated the Steam SDK to
# whatever the fucking latest is, just so I can benefit the peakest documentation
# Steam has to offer.
#
# As for updating on your end, go to your Unreal Engine installation folder, then:
# 1. Navigate to Engine/Source/ThirdParty/Steamworks/
# 2. Make a new folder with the version you want to update to, for example "Steamv162"
# 3. Download the Steamworks SDK from https://partner.steamgames.com/downloads/steamworks_sdk.zip
# 4. Extract the SDK and copy the "sdk" folder into your newly created Steamworks version folder
# (make sure to filter out linux maybe ihihihihi)
# 5. Edit Steamworks.build.cs || Edit 'double SteamVersionNumber = 1.41;' to whatever version you updated to.
# 6. Enjoy.
#
#
#
# --------------------------------------------------------------------- 11:06PM 06.09.2025 - NEVER FUCKING MIND WE DO NOT UPDATE ANYMORE THE GAME DOES NOT START.
#
#
////////////////////////////////////////////////////////////////////////////////////////////
*/

namespace BBSteamUtilsPrivate
{
	static ISteamUserStats* GetSteamUserStats()
	{
		// OnlineSubsystemSteam owns SteamAPI_Init / callbacks / shutdown in this project.
		// Checking the named subsystem first prevents direct Steam interface calls too early.
		if (IOnlineSubsystem::Get(FName(TEXT("STEAM"))) == nullptr)
		{
			return nullptr;
		}

		if (SteamUser() == nullptr || !SteamUser()->BLoggedOn())
		{
			return nullptr;
		}

		return SteamUserStats();
	}
}







UBBSteamUtils::UBBSteamUtils()
{
	// yeah, what about it?
}





void UBBSteamUtils::SetSteamRichPresence(const FString& Key, const FString& Value)
{
	if (SteamAPI_Init()) {
        SteamFriends()->SetRichPresence(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam API failed, why??"));
	}
}

void UBBSteamUtils::ClearSteamRichPresence() {
		SteamFriends()->ClearRichPresence();
}


void UBBSteamUtils::OpenSteamOverlayWithURL(const FString& URL)
{
    if (SteamFriends() != nullptr) {  
        const char* SteamURL = TCHAR_TO_UTF8(*URL);
        SteamFriends()->ActivateGameOverlayToWebPage(SteamURL);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam calls r disabled, why??"));
    }
}

// Notification Position stuff because yeah || Thanks to r0neko!! SpectralRift soon?
void UBBSteamUtils::SetSteamOverlayNotificationPosition(ESteamNotificationPosition Position)
{
    if (SteamAPI_Init()) {

        ENotificationPosition steamPosition;
        switch (Position) {

        case ESteamNotificationPosition::TopLeft:
            steamPosition = k_EPositionTopLeft;
            break;
        case ESteamNotificationPosition::TopRight:
            steamPosition = k_EPositionTopRight;
            break;
        case ESteamNotificationPosition::BottomLeft:
            steamPosition = k_EPositionBottomLeft;
            break;
        case ESteamNotificationPosition::BottomRight:
            steamPosition = k_EPositionBottomRight;
            break;
        default:
            steamPosition = k_EPositionTopLeft;
            break;
        }

        SteamUtils()->SetOverlayNotificationPosition(steamPosition);
    }
    else
    {
        //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Steam API Initialization failed."));
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam API failed, why??"));
}
    //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Steam API calls are disabled in the Editor."));
    UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam calls r disabled, why??"));
    
}





//Toggles the player's Steam Overlay as if he pressed SHIFT + TAB.
void UBBSteamUtils::ToggleSteamOverlay()
{
    if (SteamAPI_Init()) {

        SteamFriends()->ActivateGameOverlay("");
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam API failed, why??"));
    }
    UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam calls r disabled, why??"));
}




void UBBSteamUtils::CheckSteamConnection(bool& IsConnected)
{
    if (SteamAPI_Init()) {

        IsConnected = SteamAPI_IsSteamRunning() && SteamUser()->BLoggedOn();
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam API failed, why??"));
    }
}

void UBBSteamUtils::UnlockSteamAchievement(const FString& AchievementID)
{
    ISteamUserStats* UserStats = BBSteamUtilsPrivate::GetSteamUserStats();
    if (UserStats == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam OSS or SteamUserStats is not ready."));
        return;
    }

    const FTCHARToUTF8 AchievementUtf8(*AchievementID);
    if (!UserStats->SetAchievement(AchievementUtf8.Get()) || !UserStats->StoreStats())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not store achievement '%s'. Check its published Steamworks API Name."), *AchievementID);
    }
}

void UBBSteamUtils::ClearSteamAchievement(const FString& AchievementID)
{
#if UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] ClearSteamAchievement is disabled in Shipping."));
    return;
#else
    ISteamUserStats* UserStats = BBSteamUtilsPrivate::GetSteamUserStats();
    if (UserStats == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam OSS or SteamUserStats is not ready."));
        return;
    }

    const FTCHARToUTF8 AchievementUtf8(*AchievementID);
    if (!UserStats->ClearAchievement(AchievementUtf8.Get()) || !UserStats->StoreStats())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not clear achievement '%s'."), *AchievementID);
    }
#endif
}

void UBBSteamUtils::GetSteamAchievement(const FString& AchievementID, bool& IsUnlocked)
{
    IsUnlocked = false;

    ISteamUserStats* UserStats = BBSteamUtilsPrivate::GetSteamUserStats();
    if (UserStats == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam OSS or SteamUserStats is not ready."));
        return;
    }

    const FTCHARToUTF8 AchievementUtf8(*AchievementID);
    if (!UserStats->GetAchievement(AchievementUtf8.Get(), &IsUnlocked))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not read achievement '%s'. Check its published Steamworks API Name."), *AchievementID);
    }
}

void UBBSteamUtils::AddSteamAchievementProgress(
    const FString& AchievementID,
    const FString& ProgressStatID,
    int32 Delta,
    int32 ProgressMaxForToast,
    bool bShowProgressToast,
    int32& NewProgress,
    bool& bSuccess)
{
    NewProgress = 0;
    bSuccess = false;

    ISteamUserStats* UserStats = BBSteamUtilsPrivate::GetSteamUserStats();
    if (UserStats == nullptr || AchievementID.IsEmpty() || ProgressStatID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam stats are not ready, or an Achievement/Stat API Name is empty."));
        return;
    }

    const FTCHARToUTF8 AchievementUtf8(*AchievementID);
    const FTCHARToUTF8 StatUtf8(*ProgressStatID);

    int32 CurrentProgress = 0;
    if (!UserStats->GetStat(StatUtf8.Get(), &CurrentProgress))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not read INT stat '%s'. Check its published Steamworks API Name and type."), *ProgressStatID);
        return;
    }

    const int64 NextProgress = static_cast<int64>(CurrentProgress) + static_cast<int64>(Delta);
    NewProgress = static_cast<int32>(FMath::Clamp<int64>(NextProgress, 0, MAX_int32));

    if (!UserStats->SetStat(StatUtf8.Get(), NewProgress))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not write INT stat '%s'. Check Steamworks constraints."), *ProgressStatID);
        return;
    }

    // This popup is visual feedback only. SetStat above is what writes actual progress.
    if (bShowProgressToast && ProgressMaxForToast > 0 && NewProgress < ProgressMaxForToast)
    {
        bool bAchievementAlreadyUnlocked = false;
        if (UserStats->GetAchievement(AchievementUtf8.Get(), &bAchievementAlreadyUnlocked) && !bAchievementAlreadyUnlocked)
        {
            UserStats->IndicateAchievementProgress(
                AchievementUtf8.Get(),
                static_cast<uint32>(NewProgress),
                static_cast<uint32>(ProgressMaxForToast));
        }
    }

    // Steamworks auto-unlocks when this stat is configured as the achievement's Progress Stat.
    // Do not call SetAchievement here; the backend Unlock Value remains the single source of truth.
    bSuccess = UserStats->StoreStats();
}

void UBBSteamUtils::ResetAllSteamStatsAndAchievements()
{
#if UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] ResetAllSteamStatsAndAchievements is disabled in Shipping."));
    return;
#else
    ISteamUserStats* UserStats = BBSteamUtilsPrivate::GetSteamUserStats();
    if (UserStats == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Steam OSS or SteamUserStats is not ready."));
        return;
    }

    // ResetAllStats(true) already persists the reset through Steam; do not StoreStats a second time.
    if (!UserStats->ResetAllStats(true))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BUDGET STEAM NETWORKING] Could not reset Steam stats and achievements."));
    }
#endif
}


// THANK YOU LORENZOHAPPY19 VERY DEMURE FOR THE GIT HASH THING THANK YOU VERY MUCH GRAZIE CAPAREZZA CALABRESE CARBONARA
FString UBBSteamUtils::GetGitHash()
{
    return FString(PROJECT_GIT_HASH);
}

// THANK YOU LORENZOHAPPY19 VERY DEMURE FOR THE GIT HASH THING THANK YOU VERY MUCH GRAZIE CAPAREZZA CALABRESE CARBONARA
FString UBBSteamUtils::GetGitHashLong()
{
    return FString(PROJECT_GIT_HASH_LONG);
}
