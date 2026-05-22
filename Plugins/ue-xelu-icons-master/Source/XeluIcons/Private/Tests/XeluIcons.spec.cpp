// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "InputMappingContext.h"
#include "XeluIconsBlueprintLibrary.h"
#include "XeluIconsDeveloperSettings.h"
#include "Engine/Texture2D.h"
#include "GameFramework/InputSettings.h"
#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"
 
#if UE_VERSION_OLDER_THAN(5, 5, 0)
 
// 5.4.x and down
inline constexpr uint8 EAutomationTestFlags_ApplicationContextMask = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext;
 
#endif

BEGIN_DEFINE_SPEC(FXeluIconsSpec, "XeluIcons", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)

	UInputAction* InputAction01 = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/IA_Xelu_Icons_Test_01.IA_Xelu_Icons_Test_01")));
	UInputAction* InputAction02 = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/IA_Xelu_Icons_Test_02.IA_Xelu_Icons_Test_02")));
	UInputAction* InputAction03 = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/IA_Xelu_Icons_Test_03.IA_Xelu_Icons_Test_03")));
	UInputAction* InputAction04 = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/IA_Xelu_Icons_Test_04.IA_Xelu_Icons_Test_04")));
	UInputAction* InputAction05 = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/IA_Xelu_Icons_Test_05.IA_Xelu_Icons_Test_05")));

	UInputMappingContext* Context = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/XeluIcons/Tests/MC_Xelu_Icons_Test.MC_Xelu_Icons_Test")));

	FInputActionKeyMapping TotallyATest_A = FInputActionKeyMapping(FName("TotallyATest"), EKeys::A, true, true, true, true);
	FInputActionKeyMapping TotallyATest_B = FInputActionKeyMapping(FName("TotallyATest"), EKeys::B, true, true, true, true);

	FInputActionKeyMapping TotallyATest_NoModifier_A = FInputActionKeyMapping(FName("TotallyATest_NoModifier"), EKeys::A);
	FInputActionKeyMapping TotallyATest_NoModifier_B = FInputActionKeyMapping(FName("TotallyATest_NoModifier"), EKeys::B);

END_DEFINE_SPEC(FXeluIconsSpec)

void FXeluIconsSpec::Define()
{
	Describe("UXeluIconsBlueprintLibrary", [this]()
	{
		BeforeEach([this]()
		{
			// Setup tests
		});

		It("GetPluginSettings()", [this]()
		{
			const UXeluIconsDeveloperSettings* Settings = UXeluIconsBlueprintLibrary::GetPluginSettings();
			TestTrue("Returns developer settings", Settings != nullptr);

			const TSoftObjectPtr<UDataTable> DataTable = Settings->IconsDataTable;
			TestTrue("Datatable is not null", !DataTable.IsNull());
		});

		It("GetIconsDatatable()", [this]()
		{
			const UDataTable* DataTable = UXeluIconsBlueprintLibrary::GetIconsDatatable();
			TestTrue("Resolved Datatable is valid", DataTable != nullptr);
		});

		Describe("Enhanced Input", [this]()
		{
			It("GetSoftIconForInputAction()", [this]()
			{
				TMap<UInputAction*, FString> InputsToPath;
				InputsToPath.Add(InputAction01, "/XeluIcons/Icons/Keyboard_Icons/Dark/Mouse_Left_Key_Dark.Mouse_Left_Key_Dark");
				InputsToPath.Add(InputAction02, "/XeluIcons/Icons/Keyboard_Icons/Dark/Mouse_Right_Key_Dark.Mouse_Right_Key_Dark");
				InputsToPath.Add(InputAction03, "/XeluIcons/Icons/Keyboard_Icons/Dark/Space_Key_Dark.Space_Key_Dark");
				InputsToPath.Add(InputAction04, "/XeluIcons/Icons/Keyboard_Icons/Dark/U_Key_Dark.U_Key_Dark");
				InputsToPath.Add(InputAction05, "/XeluIcons/Icons/Keyboard_Icons/Dark/E_Key_Dark.E_Key_Dark");

				for (const auto Entry : InputsToPath)
				{
					FString ExpectedPath = Entry.Value;
					UInputAction* InputAction = Entry.Key;

					const TSoftObjectPtr<UTexture2D> Texture = UXeluIconsBlueprintLibrary::GetSoftIconForInputAction(InputAction, Context, EXeluIconsIconType::Keyboard, false);

					TestTrue(FString::Printf(TEXT("Texture for %s is valid"), *GetNameSafe(InputAction)), Texture.IsNull() == false);
					TestEqual(FString::Printf(TEXT("Texture path for %s is matching"), *GetNameSafe(InputAction)), Texture.ToSoftObjectPath().ToString(), ExpectedPath);
				}

				InputsToPath.Empty();
				InputsToPath.Add(InputAction01, "/XeluIcons/Icons/PS4_Icons/PS4_Square.PS4_Square");
				InputsToPath.Add(InputAction02, "/XeluIcons/Icons/PS4_Icons/PS4_Triangle.PS4_Triangle");
				InputsToPath.Add(InputAction03, "/XeluIcons/Icons/PS4_Icons/PS4_Circle.PS4_Circle");
				InputsToPath.Add(InputAction04, "/XeluIcons/Icons/PS4_Icons/PS4_Cross.PS4_Cross");
				InputsToPath.Add(InputAction05, "/XeluIcons/Icons/PS4_Icons/PS4_R1.PS4_R1");

				for (const auto Entry : InputsToPath)
				{
					FString ExpectedPath = Entry.Value;
					UInputAction* InputAction = Entry.Key;

					const TSoftObjectPtr<UTexture2D> Texture = UXeluIconsBlueprintLibrary::GetSoftIconForInputAction(InputAction, Context, EXeluIconsIconType::PS4, false);

					TestTrue(FString::Printf(TEXT("Texture for %s is valid"), *GetNameSafe(InputAction)), Texture.IsNull() == false);
					TestEqual(FString::Printf(TEXT("Texture path for %s is matching"), *GetNameSafe(InputAction)), Texture.ToSoftObjectPath().ToString(), ExpectedPath);
				}
			});

			It("GetIconForInputAction()", [this]()
			{
				UTexture2D* Texture = UXeluIconsBlueprintLibrary::GetIconForInputAction(InputAction01, Context);

				TestTrue(FString::Printf(TEXT("Texture for %s is valid"), *GetNameSafe(InputAction01)), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s is matching"), *GetNameSafe(InputAction01)), *GetNameSafe(Texture), "Mouse_Left_Key_Dark");
			});
		});

		Describe("Input Settings", [this]()
		{
			BeforeEach([this]()
			{
				UInputSettings* InputSettings = UInputSettings::GetInputSettings();
				InputSettings->AddActionMapping(TotallyATest_A);
				InputSettings->AddActionMapping(TotallyATest_B);
				InputSettings->AddActionMapping(TotallyATest_NoModifier_A);
				InputSettings->AddActionMapping(TotallyATest_NoModifier_B);
			});

			It("GetSoftIconForActionMapping()", [this]()
			{

				TMap<FName, FString> ActionKeyToPath;
				ActionKeyToPath.Add(TotallyATest_A.ActionName, "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");
				ActionKeyToPath.Add(TotallyATest_NoModifier_A.ActionName, "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");

				for (const auto Entry : ActionKeyToPath)
				{
					FName ActionName = Entry.Key;
					FString ExpectedPath = Entry.Value;

					const TSoftObjectPtr<UTexture2D> Texture = UXeluIconsBlueprintLibrary::GetSoftIconForActionMapping(ActionName, EXeluIconsIconType::Keyboard, false);
					const FString ActionString = ActionName.ToString();

					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), Texture.IsNull() == false);
					TestEqual(FString::Printf(TEXT("Texture path Action name '%s' is valid"), *ActionString), Texture.ToSoftObjectPath().ToString(), ExpectedPath);
				}
			});

			It("GetSoftIconsForActionMapping()", [this]()
			{
				TArray<TSoftObjectPtr<UTexture2D>> Icons = UXeluIconsBlueprintLibrary::GetSoftIconsForActionMapping(TotallyATest_A.ActionName, EXeluIconsIconType::Keyboard, false);
				TestEqual(TEXT("Number of icons for TotallyATest Action Name is correct"), Icons.Num(), 2);

				const FString ActionString = TotallyATest_A.ActionName.ToString();

				if (Icons.IsValidIndex(0))
				{
					const TSoftObjectPtr<UTexture2D> Texture = Icons[0];
					const FString ExpectedPath = "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark";
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), Texture.IsNull() == false);
					TestEqual(FString::Printf(TEXT("Texture path Action name '%s' is valid"), *ActionString), Texture.ToSoftObjectPath().ToString(), ExpectedPath);
				}
				else
				{
					TestTrue(TEXT("Icons {0} is invalid"), false);
				}

				if (Icons.IsValidIndex(1))
				{
					const TSoftObjectPtr<UTexture2D> Texture = Icons[1];
					const FString ExpectedPath = "/XeluIcons/Icons/Keyboard_Icons/Dark/B_Key_Dark.B_Key_Dark";
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), Texture.IsNull() == false);
					TestEqual(FString::Printf(TEXT("Texture path Action name '%s' is valid"), *ActionString), Texture.ToSoftObjectPath().ToString(), ExpectedPath);
				}
				else
				{
					TestTrue(TEXT("Icons {1} is invalid"), false);
				}
			});

			It("GetSoftIconForActionMappingWithModifiers()", [this]()
			{
				const FXeluIconsActionMappingIcons TestIcons_A = UXeluIconsBlueprintLibrary::GetSoftIconForActionMappingWithModifiers(TotallyATest_A.ActionName, EXeluIconsIconType::Keyboard, false);
				TestEqual("TestIcons_A bAlt", TestIcons_A.bAlt, true);
				TestEqual("TestIcons_A bCmd", TestIcons_A.bCmd, true);
				TestEqual("TestIcons_A bCtrl", TestIcons_A.bCtrl, true);
				TestEqual("TestIcons_A bShift", TestIcons_A.bShift, true);

				TestEqual("TestIcons_A KeyIcon", TestIcons_A.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");
				TestEqual("TestIcons_A AltIcon", TestIcons_A.AltIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Alt_Key_Dark.Alt_Key_Dark");
				TestEqual("TestIcons_A CmdIcon", TestIcons_A.CmdIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Command_Key_Dark.Command_Key_Dark");
				TestEqual("TestIcons_A CtrlIcon", TestIcons_A.CtrlIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Ctrl_Key_Dark.Ctrl_Key_Dark");
				TestEqual("TestIcons_A ShiftIcon", TestIcons_A.ShiftIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Shift_Alt_Key_Dark.Shift_Alt_Key_Dark");

				const FXeluIconsActionMappingIcons TestNoModifier_A = UXeluIconsBlueprintLibrary::GetSoftIconForActionMappingWithModifiers(TotallyATest_NoModifier_A.ActionName, EXeluIconsIconType::Keyboard, false);
				TestEqual("TestNoModifier_A bAlt", TestNoModifier_A.bAlt, false);
				TestEqual("TestNoModifier_A bCmd", TestNoModifier_A.bCmd, false);
				TestEqual("TestNoModifier_A bCtrl", TestNoModifier_A.bCtrl, false);
				TestEqual("TestNoModifier_A bShift", TestNoModifier_A.bShift, false);

				TestEqual("TestNoModifier_A KeyIcon", TestNoModifier_A.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");
				TestEqual("TestNoModifier_A AltIcon", TestNoModifier_A.AltIcon.ToSoftObjectPath().ToString(), "");
				TestEqual("TestNoModifier_A CmdIcon", TestNoModifier_A.CmdIcon.ToSoftObjectPath().ToString(), "");
				TestEqual("TestNoModifier_A CtrlIcon", TestNoModifier_A.CtrlIcon.ToSoftObjectPath().ToString(), "");
				TestEqual("TestNoModifier_A ShiftIcon", TestNoModifier_A.ShiftIcon.ToSoftObjectPath().ToString(), "");
			});

			It("GetSoftIconsForActionMappingWithModifiers() - Should return modifier icons for keys defining them", [this]()
			{
				TArray<FXeluIconsActionMappingIcons> ActionMappingIcons = UXeluIconsBlueprintLibrary::GetSoftIconsForActionMappingWithModifiers(TotallyATest_A.ActionName, EXeluIconsIconType::Keyboard, false);

				TestEqual(TEXT("Number of icons for TotallyATest Action Name is correct"), ActionMappingIcons.Num(), 2);

				const FString ActionString = TotallyATest_A.ActionName.ToString();

				if (ActionMappingIcons.IsValidIndex(0))
				{
					const FXeluIconsActionMappingIcons MappingIcons = ActionMappingIcons[0];
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), MappingIcons.KeyIcon.IsNull() == false);
					TestEqual("MappingIcons 0 KeyIcon", MappingIcons.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");
					TestEqual("MappingIcons 0 AltIcon", MappingIcons.AltIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Alt_Key_Dark.Alt_Key_Dark");
					TestEqual("MappingIcons 0 CmdIcon", MappingIcons.CmdIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Command_Key_Dark.Command_Key_Dark");
					TestEqual("MappingIcons 0 CtrlIcon", MappingIcons.CtrlIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Ctrl_Key_Dark.Ctrl_Key_Dark");
					TestEqual("MappingIcons 0 ShiftIcon", MappingIcons.ShiftIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Shift_Alt_Key_Dark.Shift_Alt_Key_Dark");
				}
				else
				{
					TestTrue(TEXT("Icons {0} is invalid"), false);
				}

				if (ActionMappingIcons.IsValidIndex(1))
				{
					const FXeluIconsActionMappingIcons MappingIcons = ActionMappingIcons[1];
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), MappingIcons.KeyIcon.IsNull() == false);
					TestEqual("MappingIcons 0 KeyIcon", MappingIcons.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/B_Key_Dark.B_Key_Dark");
					TestEqual("MappingIcons 0 AltIcon", MappingIcons.AltIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Alt_Key_Dark.Alt_Key_Dark");
					TestEqual("MappingIcons 0 CmdIcon", MappingIcons.CmdIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Command_Key_Dark.Command_Key_Dark");
					TestEqual("MappingIcons 0 CtrlIcon", MappingIcons.CtrlIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Ctrl_Key_Dark.Ctrl_Key_Dark");
					TestEqual("MappingIcons 0 ShiftIcon", MappingIcons.ShiftIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/Shift_Alt_Key_Dark.Shift_Alt_Key_Dark");
				}
				else
				{
					TestTrue(TEXT("Icons {1} is invalid"), false);
				}
			});

			It("GetSoftIconsForActionMappingWithModifiers() - Should return no modifier icons for keys not defining them", [this]()
			{
				TArray<FXeluIconsActionMappingIcons> ActionMappingIcons = UXeluIconsBlueprintLibrary::GetSoftIconsForActionMappingWithModifiers(TotallyATest_NoModifier_A.ActionName, EXeluIconsIconType::Keyboard, false);

				TestEqual(TEXT("Number of icons for TotallyATest_NoModifier_A Action Name is correct"), ActionMappingIcons.Num(), 2);

				const FString ActionString = TotallyATest_NoModifier_A.ActionName.ToString();

				if (ActionMappingIcons.IsValidIndex(0))
				{
					const FXeluIconsActionMappingIcons MappingIcons = ActionMappingIcons[0];
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), MappingIcons.KeyIcon.IsNull() == false);
					TestEqual("MappingIcons 0 KeyIcon", MappingIcons.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/A_Key_Dark.A_Key_Dark");
					TestEqual("MappingIcons 0 AltIcon", MappingIcons.AltIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 CmdIcon", MappingIcons.CmdIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 CtrlIcon", MappingIcons.CtrlIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 ShiftIcon", MappingIcons.ShiftIcon.ToSoftObjectPath().ToString(), "");
				}
				else
				{
					TestTrue(TEXT("Icons {0} is invalid"), false);
				}

				if (ActionMappingIcons.IsValidIndex(1))
				{
					const FXeluIconsActionMappingIcons MappingIcons = ActionMappingIcons[1];
					TestTrue(FString::Printf(TEXT("Texture for Action name '%s' is valid"), *ActionString), MappingIcons.KeyIcon.IsNull() == false);
					TestEqual("MappingIcons 0 KeyIcon", MappingIcons.KeyIcon.ToSoftObjectPath().ToString(), "/XeluIcons/Icons/Keyboard_Icons/Dark/B_Key_Dark.B_Key_Dark");
					TestEqual("MappingIcons 0 AltIcon", MappingIcons.AltIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 CmdIcon", MappingIcons.CmdIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 CtrlIcon", MappingIcons.CtrlIcon.ToSoftObjectPath().ToString(), "");
					TestEqual("MappingIcons 0 ShiftIcon", MappingIcons.ShiftIcon.ToSoftObjectPath().ToString(), "");
				}
				else
				{
					TestTrue(TEXT("Icons {1} is invalid"), false);
				}
			});

			AfterEach([this]()
			{
				UInputSettings* InputSettings = UInputSettings::GetInputSettings();
				InputSettings->RemoveActionMapping(TotallyATest_A, false);
				InputSettings->RemoveActionMapping(TotallyATest_B, false);
				InputSettings->RemoveActionMapping(TotallyATest_NoModifier_A, false);
				InputSettings->RemoveActionMapping(TotallyATest_NoModifier_B, false);
			});
		});

		Describe("GetIconForKey()", [this]()
		{
			It("Should return valid textures", [this]()
			{
				FKey Key = EKeys::LeftMouseButton;

				UTexture2D* Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "Mouse_Left_Key_Dark");

				Key = EKeys::Gamepad_FaceButton_Left;

				Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::PS4);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "PS4_Square");

				Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::PS5);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "PS5_Square");

				Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::XboxOne);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "XboxOne_X");

				Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::XboxSeries);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "XboxSeriesX_X");
			});

			It("Should fallback to default IconPreference when a wrong one is provided", [this]()
			{
				FKey Key = EKeys::LeftMouseButton;

				UTexture2D* Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::XboxOne);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "Mouse_Left_Key_Dark");

				Key = EKeys::Gamepad_FaceButton_Left;

				Texture = UXeluIconsBlueprintLibrary::GetIconForKey(Key, EXeluIconsIconType::Keyboard);
				TestTrue(FString::Printf(TEXT("Texture for %s key is valid"), *Key.GetDisplayName().ToString()), Texture != nullptr);
				TestEqual(FString::Printf(TEXT("Texture path for %s key is matching"), *Key.GetDisplayName().ToString()), *GetNameSafe(Texture), "XboxOne_X");
			});
		});

		AfterEach([this]()
		{
			// Teardown
		});
	});
}
