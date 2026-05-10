#include "UBBWindowsUtils.h"
#include "Misc/MessageDialog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/WindowsPlatformMisc.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <dxgi1_6.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

//I apologize for whoever reads this, I just couldn't figure out Unreal's CPP. My bad.

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

void UBBWindowsUtils::GetMonitorHDRSpecs(bool& SupportsHDR, float& MaxLuminance, float& MinLuminance)
{
    SupportsHDR = false;
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
                        SupportsHDR = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
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

void UBBWindowsUtils::HardRestart(FName TargetMap)
{
    UGameplayStatics::OpenLevel(GEngine->GetWorld(), TargetMap, true);
}

void UBBWindowsUtils::IntegratedGraphics(const UObject* WorldContextObject)
{
    FString Name = GRHIAdapterName.ToLower();

    bool isIntegrated =
        Name.Contains(TEXT("intel uhd")) ||
        Name.Contains(TEXT("intel hd")) ||
        Name.Contains(TEXT("intel iris")) ||
        Name.Contains(TEXT("radeon graphics")) ||
        Name.Contains(TEXT("radeon (tm) graphics")) ||
        Name.Contains(TEXT("vega 8")) ||
        Name.Contains(TEXT("vega 7")) ||
        Name.Contains(TEXT("vega 6")) ||
        Name.Contains(TEXT("780m")) ||
        Name.Contains(TEXT("760m")) ||
        Name.Contains(TEXT("radeon 890m")) ||
        Name.Contains(TEXT("radeon 880m"));

#if PLATFORM_WINDOWS
    if (isIntegrated)
    {
        FString Message = FString::Printf(
            TEXT("Budget Backrooms seems to be running on an integrated GPU (Game running under %s).\n\n")
            TEXT("Friendly reminder from the dev, the game is supposed to be played on a dedicated GPU.\n")
            TEXT("For a better experience, please switch the game to a dedicated GPU (NVIDIA GTX/RTX or AMD Radeon RX).\n\n")
            TEXT("If your current system does not have a dedicated GPU, expect underwhelming performance. You can disable this message next time from the settings or by using the -ignoreIGPU launch option.\n\n")
            TEXT("Game will start up anyway;\n")
            TEXT("Click OK to continue."),
            *GRHIAdapterName
        );

        MessageBoxW(
            nullptr,
            *Message,
            TEXT("Hold up"),
            MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST
        );
    }
#endif
}