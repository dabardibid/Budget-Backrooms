//#define _CRT_SECURE_NO_WARNINGS

#include "UBBSteamUtils.h"
#include "CoreMinimal.h"
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