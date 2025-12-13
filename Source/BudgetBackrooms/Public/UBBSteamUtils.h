#pragma once

#include "OnlineSubsystemSteam.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "ThirdParty/Steamworks/Steamv151/sdk/public/steam/steam_api.h"
#include "UBBSteamUtils.generated.h"


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
# 11:06PM 06.09.2025 - NEVER FUCKING MIND WE DO NOT UPDATE ANYMORE THE GAME DOES NOT START.
#
#
////////////////////////////////////////////////////////////////////////////////////////////
*/




UENUM(BlueprintType)
enum class ESteamNotificationPosition : uint8 { 
    TopLeft UMETA(DisplayName = "Top Left"),
    TopRight UMETA(DisplayName = "Top Right"),
    BottomLeft UMETA(DisplayName = "Bottom Left"),
    BottomRight UMETA(DisplayName = "Bottom Right")
};

UCLASS()
class BUDGETBACKROOMS_API UBBSteamUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UBBSteamUtils();

    UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)", meta = (Keywords = "Set Steam Rich Presence", ToolTip = "Sets Steam Rich Presence with the desired args. It can be seen from the Friends List or other menus in the Steam Community.")) 
    static void SetSteamRichPresence(const FString& Key, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)", meta = (Keywords = "Steam Overlay Position", ToolTip = "Changes in which corner to show Steam Overlay notifications"))
    static void SetSteamOverlayNotificationPosition(ESteamNotificationPosition Position);

    UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)", meta = (ToolTip = "Self-explanatory, toggles the Steam Overlay as if the user pressed SHIFT + TAB"))
    static void ToggleSteamOverlay();

    UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)", meta = (Keywords = "Steam Connection", ToolTip = "Checks whether the player has Steam active, and if the game can do a handshake with the Steam API (aka Initialization)"))
    static void CheckSteamConnection(bool& IsConnected);

    UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)", meta = (Keywords = "Steam Overlay", ToolTip = "Self-explanatory; Opens the user's Steam Overlay with a defined target link."))
    static void OpenSteamOverlayWithURL(const FString& URL);

	UFUNCTION(BlueprintCallable, Category = "Steam (Budget Backrooms // Custom BP Expose)")
	static void ClearSteamRichPresence();
    
    UFUNCTION(BlueprintPure, Category = "Budget Other Utils", meta = (CompactNodeTitle = "Git Hash"))
    static FString GetGitHash();

    UFUNCTION(BlueprintPure, Category = "Budget Other Utils", meta = (CompactNodeTitle = "Git Hash Long, Very Long"))
    static FString GetGitHashLong();
};
