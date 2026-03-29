#include "UBBWindowsUtils.h"
#include "Misc/MessageDialog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/WindowsPlatformMisc.h"
#endif

void UBBWindowsUtils::LockPC() {
    #if PLATFORM_WINDOWS
    ::LockWorkStation();
    #else

    #endif
};

void UBBWindowsUtils::REDACTED() {
    UE_LOG(LogTemp, Fatal, TEXT("LogScript: Warning: █████G███████████████████O██████████"));
}

bool UBBWindowsUtils::IsGameMinimized() {
#if PLATFORM_WINDOWS
    HWND hwnd = GetForegroundWindow();
    if (hwnd)
    {
        DWORD processID;
        GetWindowThreadProcessId(hwnd, &processID);
        return processID != GetCurrentProcessId();
    }
    return true;
#else
    return false;
#endif
}


void UBBWindowsUtils::ForceEnableHDR(bool Enabled) {
#if PLATFORM_WINDOWS
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmFields = DM_DISPLAYFIXEDOUTPUT;

        if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode))
        {
            if (Enabled)
            {
                devMode.dmDisplayFixedOutput = DMDFO_DEFAULT;
            }
            else
            {
                devMode.dmDisplayFixedOutput = DMDFO_CENTER;
            }

            ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY);
        }
        ReleaseDC(NULL, hdc);
    }
#else

#endif
};


int32 UBBWindowsUtils::ShowWindowsMessageBox(FString Message, FString Title, EWindowsMessageBoxButtons ButtonType) {
#if PLATFORM_WINDOWS
    UINT uType;

    switch (ButtonType)
    {
    case EWindowsMessageBoxButtons::OK:
        uType = MB_OK;
        break;
    case EWindowsMessageBoxButtons::OKCancel:
        uType = MB_OKCANCEL;
        break;
    case EWindowsMessageBoxButtons::YesNo:
        uType = MB_YESNO;
        break;
    case EWindowsMessageBoxButtons::YesNoCancel:
        uType = MB_YESNOCANCEL;
        break;
    case EWindowsMessageBoxButtons::RetryCancel:
        uType = MB_RETRYCANCEL;
        break;
    case EWindowsMessageBoxButtons::AbortRetryIgnore:
        uType = MB_ABORTRETRYIGNORE;
        break;
    default:
        uType = MB_OK;
        break;
    }

    int32 Result = MessageBox(
        NULL,
        *Message,
        *Title,
        uType | MB_ICONINFORMATION
    );

    return Result;
    #else
    // Fallback for non-Windows platforms
    FText Msg = FText::FromString(Message);
    FText TitleText = FText::FromString(Title);
    FMessageDialog::Open(EAppMsgType::Ok, Msg, &TitleText);
    return 0; // Default fallback value
    #endif
}
