#include "UBBWindowsUtils.h"
#include "Misc/MessageDialog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/WindowsPlatformMisc.h"

// Allow inclusion of Windows APIs
#include "Windows/AllowWindowsPlatformTypes.h"
#include <dxgi1_6.h>
#include "Windows/HideWindowsPlatformTypes.h"
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

void UBBWindowsUtils::GetMonitorHDRSpecs(bool& bSupportsHDR, float& MaxLuminance, float& MinLuminance)
{
    bSupportsHDR = false;
    MaxLuminance = 0.0f;
    MinLuminance = 0.0f;

#if PLATFORM_WINDOWS
    IDXGIFactory1* dxgiFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
    {
        IDXGIAdapter1* dxgiAdapter = nullptr;
        // Just grab the first adapter (primary GPU)
        if (SUCCEEDED(dxgiFactory->EnumAdapters1(0, &dxgiAdapter)))
        {
            IDXGIOutput* dxgiOutput = nullptr;
            // Grab the primary monitor
            if (SUCCEEDED(dxgiAdapter->EnumOutputs(0, &dxgiOutput)))
            {
                IDXGIOutput6* dxgiOutput6 = nullptr;
                if (SUCCEEDED(dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput6))))
                {
                    DXGI_OUTPUT_DESC1 desc1;
                    if (SUCCEEDED(dxgiOutput6->GetDesc1(&desc1)))
                    {
                        bSupportsHDR = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
                        MaxLuminance = desc1.MaxLuminance;
                        MinLuminance = desc1.MinLuminance;
                    }
                    dxgiOutput6->Release();
                }
                dxgiOutput->Release();
            }
            dxgiAdapter->Release();
        }
        dxgiFactory->Release();
    }
#endif
}

void UBBWindowsUtils::AutoConfigureUE4HDR()
{
    bool bSupportsHDR = false;
    float MaxLuminance = 0.0f;
    float MinLuminance = 0.0f;

    GetMonitorHDRSpecs(bSupportsHDR, MaxLuminance, MinLuminance);

    if (GEngine)
    {
        if (bSupportsHDR)
        {
            GEngine->Exec(nullptr, TEXT("r.HDR.EnableHDROutput 1"));
            
            // In UE4:
            // OutputDevice 3 = 1000 nits Rec2020
            // OutputDevice 4 = 2000 nits Rec2020
            // OutputDevice 5 = 1000 nits scRGB
            // OutputDevice 6 = 2000 nits scRGB

            // We default to scRGB which Windows usually handles better dynamically, 
            // and choose peak nits based on hardware
            if (MaxLuminance > 1000.0f)
            {
                GEngine->Exec(nullptr, TEXT("r.HDR.Display.OutputDevice 6")); // 2000 nits
            }
            else
            {
                GEngine->Exec(nullptr, TEXT("r.HDR.Display.OutputDevice 5")); // 1000 nits
            }

            GEngine->Exec(nullptr, TEXT("r.HDR.Display.ColorGamut 2")); // Rec2020
        }
        else
        {
            // Fallback to standard SDR
            GEngine->Exec(nullptr, TEXT("r.HDR.EnableHDROutput 0"));
            GEngine->Exec(nullptr, TEXT("r.HDR.Display.OutputDevice 0")); // sRGB
        }
    }
}
