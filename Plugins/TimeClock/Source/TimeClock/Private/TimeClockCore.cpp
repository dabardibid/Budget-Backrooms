// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.


#include "TimeClockCore.h"
#include "Misc/ConfigCacheIni.h"
#include "Editor.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

UTimeClockCore::UTimeClockCore()
{
}

UTimeClockCore::~UTimeClockCore()
{
	UObjectBaseUtility::RemoveFromRoot();
}

void UTimeClockCore::InitialiseTimeClockCore()
{
	// Necessary to avoid random GC
	UObjectBaseUtility::AddToRoot();

	// Bindings to save the current stats
	BindTimeClockEvents();

	FString CurrentProjectName = GetProjectName();

	ActiveSessionStartDate = GetDateNow();

	// Check if there is any saved data for this project, load it if true.
	if (GetSavedDataForProject(CurrentProjectName, ActiveProjectData))
	{
		FDateTime CurrentTime = GetDateNow();

		// Checks if it is the first time the project is opened today.
		if (ActiveProjectData.LastSaved.GetDate() < CurrentTime.GetDate())
		{
			// If it is, update the BeforeToday values
			ActiveProjectData.LastSavedBeforeToday = ActiveProjectData.LastSaved;
			ActiveProjectData.SecondsSpentBeforeToday = ActiveProjectData.TotalSecondsSpent;
		}

		ActiveProjectData.LastSaved = CurrentTime;

		RefreshWorkTime();

		UE_LOG(TimeClock, Display, TEXT("Starting TimeClock for known project named: %s"), *CurrentProjectName);
		return;
	}

	// If not, start a new entry for this project.
	{
		UE_LOG(TimeClock, Display, TEXT("Starting TimeClock for new project named: %s"), *CurrentProjectName);

		ActiveProjectData.ProjectName = GetProjectName();
		ActiveProjectData.StartDate = GetDateNow();
		ActiveProjectData.LastSaved = GetDateNow();
		ActiveProjectData.TotalSecondsSpent = 0;
		ActiveProjectData.LastSavedBeforeToday = GetDateNow();
		ActiveProjectData.SecondsSpentBeforeToday = 0;

		SaveProjectData(ActiveProjectData);
		return;
	}
}

void UTimeClockCore::BindTimeClockEvents()
{
	// These are when Time Clock will save the current stats it gathered.

	FEditorDelegates::PreSaveWorld.AddUFunction(this, FName("SaveCurrentProjectData"));
	FEditorDelegates::PreBeginPIE.AddUFunction(this, FName("SaveCurrentProjectData"));
	FCoreDelegates::OnPreExit.AddUFunction(this, FName("SaveCurrentProjectData"));
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUFunction(this, FName("SaveCurrentProjectData"));
}

void UTimeClockCore::RefreshWorkTime()
{
	int NewTimeSpent;
	FDateTime CurrentTime = GetDateNow();

	// Compare the time difference (in seconds) between now and the last time the data was saved
	NewTimeSpent = CurrentTime.ToUnixTimestamp() - ActiveProjectData.LastSaved.ToUnixTimestamp();

	// This only happens if the data was last saved with a date in the future (should not happen).
	if (NewTimeSpent < 0)
	{
		UE_LOG(TimeClock, Warning, TEXT("It seems the stats for this project were saved in the future. I don't see any flying cars though? Dates were adjusted, time spent is unchanged. Please restart the editor or save TimeClockData for current project."));
		ActiveProjectData.LastSaved = CurrentTime;
		ActiveProjectData.LastSavedBeforeToday = ActiveProjectData.StartDate;
		ActiveProjectData.SecondsSpentBeforeToday = ActiveProjectData.TotalSecondsSpent;
		return;
	}


	// Update the struct values. This still needs to be saved!
	ActiveProjectData.TotalSecondsSpent = ActiveProjectData.TotalSecondsSpent + NewTimeSpent;
	ActiveProjectData.LastSaved = CurrentTime;
}


bool UTimeClockCore::SaveCurrentProjectData()
{
	RefreshWorkTime();
	return SaveProjectData(ActiveProjectData);

}

bool UTimeClockCore::SaveProjectData(FTimeClockData TimeClockData)
{
	// Load all saved projects
	TArray<FTimeClockData> tempAllProjectData;
	tempAllProjectData = GetAllSavedProjectData();
	bool Success;

	// Check if other projects were already saved.
	if (tempAllProjectData.Num() > 0)
	{
		// If so look for an entry that matches the input name.
		for (int i = 0; i < tempAllProjectData.Num(); i++)
		{
			if (tempAllProjectData[i].ProjectName == TimeClockData.ProjectName)
			{
				// Update the loaded data with the input data, and sets a new LastUpdated value.
				tempAllProjectData[i] = TimeClockData;
				tempAllProjectData[i].LastSaved = GetDateNow();

				// Checks if the input data corresponds to the currently opened project, if so refreshes the active data.
				if (TimeClockData.ProjectName == ActiveProjectData.ProjectName)
				{
					ActiveProjectData = tempAllProjectData[i];
				}

				// Saves the updated data 
				Success = WriteSaveFile(SerialiseFileContent(tempAllProjectData));
				UE_LOG(TimeClock, Display, TEXT("TimeClock stats updated for project named: %s"), *TimeClockData.ProjectName);
				return Success;
			}
		}
	}

	// If this is the first project to be saved, write the entry to the file.
	tempAllProjectData.Add(ActiveProjectData);
	Success = WriteSaveFile(SerialiseFileContent(tempAllProjectData));
	UE_LOG(TimeClock, Display, TEXT("TimeClock stats created for project named: %s"), *TimeClockData.ProjectName);
	return Success;
}

bool UTimeClockCore::GetCurrentProjectData(FTimeClockData& ProjectData)
{
	// This returns the active values, not the saved ones!
	RefreshWorkTime();

	ProjectData = ActiveProjectData;
	return true;
}

bool UTimeClockCore::GetSavedDataForProject(FString ProjectName, FTimeClockData &ProjectData)
{
	// Loads project data from file
	TArray<FTimeClockData> tempAllProjectData;
	tempAllProjectData = GetAllSavedProjectData();

	// Checks if there are any entries saved.
	if (tempAllProjectData.Num() > 0)
	{
		// Look for the saved project that matches the input name.
		for (int i = 0; i < tempAllProjectData.Num(); i++)
		{
			if (tempAllProjectData[i].ProjectName == ProjectName)
			{
				ProjectData = tempAllProjectData[i];
				return true;
			}
		}
	}

	UE_LOG(TimeClock, Warning, TEXT("No saved TimeClock data for project named: %s"), *ProjectName);
	ProjectData = FTimeClockData();
	return false;
}

TArray<FTimeClockData> UTimeClockCore::GetAllSavedProjectData()
{
	FString SaveFileContent;
	LoadSaveFile(SaveFileContent);

	return DeserialiseFileContent(SaveFileContent);
}


bool UTimeClockCore::DeleteSavedProjectData(FString ProjectName)
{
	TArray<FTimeClockData> tempAllProjectData;
	tempAllProjectData = GetAllSavedProjectData();

	if (tempAllProjectData.Num() > 0)
	{
		// Look for the saved project that matches the input name.
		for (int i = tempAllProjectData.Num() - 1; i >= 0; i--)
		{
			if (tempAllProjectData[i].ProjectName == ProjectName)
			{
				tempAllProjectData.RemoveAt(i);

				WriteSaveFile(SerialiseFileContent(tempAllProjectData));
				UE_LOG(TimeClock, Display, TEXT("TimeClock stats deleted for project named: %s"), *ProjectName);
				return true;
			}
		}
	}

	UE_LOG(TimeClock, Warning, TEXT("Couldn't delete TimeClock data for project named: %s"), *ProjectName);
	return false;

}


FString UTimeClockCore::GetProjectName()
{
	// This isn't the most elegant way to get the Project's name, but it works for now.
	FString ProjectName = FString(FPaths::GetBaseFilename(FPaths::GetProjectFilePath()));

	return ProjectName;
}

FDateTime UTimeClockCore::GetDateNow()
{
	return FDateTime::Now();
}



/* ===== Saving & Loading ===== */

FString UTimeClockCore::GetSavePath()
{
	FString tempPath = FPlatformProcess::UserDir();
	tempPath.Append(TEXT("UE4_TimeClockData.txt"));
	return tempPath;
}


bool UTimeClockCore::LoadSaveFile(FString &FileContent)
{
	return FFileHelper::LoadFileToString(FileContent, *GetSavePath(), FFileHelper::EHashOptions::None, 0);
}

bool UTimeClockCore::WriteSaveFile(FString FileContent)
{
	return FFileHelper::SaveStringToFile(*FileContent, *GetSavePath(), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), 0);
}

FString UTimeClockCore::SerialiseFileContent(TArray<FTimeClockData> TimeClockData)
{
	FString SerialisedFileContent;


	for (int i = 0; i < TimeClockData.Num(); i++)
	{
		FString SerialisedData;
		
		if (SerialiseTimeClockData(TimeClockData[i], SerialisedData))
		{
			SerialisedFileContent.Append(SerialisedData + LINE_DELIMITER);
		}
	}

	return SerialisedFileContent;
}

TArray<FTimeClockData> UTimeClockCore::DeserialiseFileContent(FString FileContent)
{
	TArray<FTimeClockData> DeserialisedFileContent;
	TArray<FString> ParsedFileContent;

	FileContent.ParseIntoArray(ParsedFileContent, LINE_DELIMITER, true);

	for (int i = 0; i < ParsedFileContent.Num(); i++)
	{
		FTimeClockData LineData;

		if (DeserialiseTimeClockData(ParsedFileContent[i], LineData))
		{
			DeserialisedFileContent.Add(LineData);
		}
	}

	return DeserialisedFileContent;
}

 bool UTimeClockCore::SerialiseTimeClockData(FTimeClockData StructData, FString& Data)
{
	FString ReturnData=
		StructData.ProjectName 
		+ VALUE_DELIMITER + StructData.StartDate.ToString() 
		+ VALUE_DELIMITER + StructData.LastSaved.ToString() 
		+ VALUE_DELIMITER + FString::FromInt(StructData.TotalSecondsSpent) 
		+ VALUE_DELIMITER + StructData.LastSavedBeforeToday.ToString() 
		+ VALUE_DELIMITER + FString::FromInt(StructData.SecondsSpentBeforeToday);

	Data = ReturnData;
	return true;
}

bool UTimeClockCore::DeserialiseTimeClockData(FString StringData, FTimeClockData& Data)
{
	FString TempStringData = StringData;
	TempStringData.RemoveFromEnd(LINE_DELIMITER);

	TArray<FString> ParsedLineContent;
	TempStringData.ParseIntoArray(ParsedLineContent, VALUE_DELIMITER, false);

	FTimeClockData LineData;

	if (ParsedLineContent.Num() == 6)
	{
		LineData.ProjectName = ParsedLineContent[0];
		FDateTime::Parse(ParsedLineContent[1], LineData.StartDate);
		FDateTime::Parse(ParsedLineContent[2], LineData.LastSaved);
		LineData.TotalSecondsSpent = FCString::Atoi(*ParsedLineContent[3]);
		FDateTime::Parse(ParsedLineContent[4], LineData.LastSavedBeforeToday);
		LineData.SecondsSpentBeforeToday = FCString::Atoi(*ParsedLineContent[5]);

		if (LineData.ProjectName != "" || LineData.ProjectName != "Unknown")
		{
			Data = LineData;
			return true;
		}
	}

	UE_LOG(TimeClock, Warning, TEXT("Cannot deserialise input string into TimeClockData: %s"), *StringData);
	Data = LineData;
	return false;
}

bool UTimeClockCore::FormatProjectDataForExport(FTimeClockData StructData, FString& Data)
{
	FString ReturnData =
		StructData.ProjectName
		+ "," + StructData.StartDate.ToString()
		+ "," + StructData.LastSaved.ToString()
		+ "," + FString::FromInt(StructData.TotalSecondsSpent);

	Data = ReturnData;
	return true;
}

bool UTimeClockCore::ExportDataToCSV(TArray<FTimeClockData> Data, FString Directory, bool AsTextFile)
{
	TArray<FString> ExportString;
	ExportString.Add(FString("ProjectName,StartDate,LastSaved,TotalTimeSpent"));


	for (int i = 0; i < Data.Num(); i++)
	{
		FString itemData;
		if (FormatProjectDataForExport(Data[i], itemData))
		{
			ExportString.Add(itemData);
		}

		else
		{
			UE_LOG(TimeClock, Error, TEXT("Failed to export, invalid TimeClockData."));
			return false;
		}
	}

	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*Directory))
	{
		UE_LOG(TimeClock, Error, TEXT("Failed to export, directory does not exist."));
		return false;
	}

	FString FileExtension = ".csv";
	if (AsTextFile)
	{
		FileExtension = ".txt";
	}

	FString FilePath = Directory + "TimeClockExport" + FileExtension;

	int DuplicateFileIndex = 0;
	while (PlatformFile.FileExists(*FilePath))
	{
		DuplicateFileIndex++;
		FilePath = Directory + "TimeClockExport" + FString::FromInt(DuplicateFileIndex) + FileExtension;
	}

	FFileHelper::SaveStringArrayToFile(ExportString, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), 0);

	UE_LOG(TimeClock, Display, TEXT("TimeClockData exported at: %s"), *FilePath);
	return true;
}

