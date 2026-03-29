// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimeClock.h"
#include "TimeClockCore.generated.h"

#define LINE_DELIMITER TEXT("_END_OF_LINE_")
#define VALUE_DELIMITER TEXT("_VALUE_")

USTRUCT(BlueprintType)
struct TIMECLOCK_API FTimeClockData
{
	GENERATED_BODY()

	// Name of the project these stats were taken from.
	UPROPERTY(BlueprintReadWrite, Category = "Time Clock")
		FString ProjectName;
	// The date at which the stats started being recorded.
	UPROPERTY(BlueprintReadWrite, Category = "Time Clock")
		FDateTime StartDate;
	// Last time this project's stats were saved.
	UPROPERTY(BlueprintReadWrite, Category = "Time Clock")
		FDateTime LastSaved;
	// The total second spent with project opened.
	UPROPERTY(BlueprintReadWrite, Category = "Time Clock")
		int TotalSecondsSpent;
	// Last time this project's stats were saved before today. Only used to calculate time spent today.
	UPROPERTY(BlueprintReadOnly, Category = "Time Clock")
		FDateTime LastSavedBeforeToday;	
	// The total second spent with project opened before today. Only used to calculate time spent today.
	UPROPERTY(BlueprintReadOnly, Category = "Time Clock")
		int SecondsSpentBeforeToday;

	FTimeClockData()
	{
		ProjectName = "Unknown";
		StartDate = FDateTime();
		LastSaved = FDateTime();
		TotalSecondsSpent = 0;
		LastSavedBeforeToday = FDateTime();
		SecondsSpentBeforeToday = 0;
	}

	FTimeClockData(FString InProjectName, FDateTime InStartDate, FDateTime InLastModified, int InMinutesSpent, FDateTime InLastSavedBeforeToday, int InSecondsSpentBeforeToday)
	{
		ProjectName = InProjectName;
		StartDate = InStartDate;
		LastSaved = InLastModified;
		TotalSecondsSpent = InMinutesSpent;
		LastSavedBeforeToday = InLastSavedBeforeToday;
		SecondsSpentBeforeToday = InSecondsSpentBeforeToday;
	}

};

UCLASS(BlueprintType, Blueprintable)
class TIMECLOCK_API UTimeClockCore : public UObject
{

	GENERATED_BODY()

public:
	UTimeClockCore();
	~UTimeClockCore();

	// The runtime TimeClock data for the opened project. Its value might differ to the saved value.
	FTimeClockData ActiveProjectData;

	//The date at which the current session started (date at which the project was opened).
	UPROPERTY(BlueprintReadWrite, Category = "Time Clock")
	FDateTime ActiveSessionStartDate;

	// Initialise a session which will track TimeClockData for the opened project.
	void InitialiseTimeClockCore();

	void BindTimeClockEvents();

	// Manually update the data for the current project. This will update the time spent today. Does not save the updated data (use SaveCurrentProjectData() instead).
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	void RefreshWorkTime();

	// Returns the currently opened project's name.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	FString GetProjectName();

	// Returns the full date from the computer's time.
	FDateTime GetDateNow();




	
	FTimerHandle AlarmTimerHandle;

	// Save the ActiveTimeClockData.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool SaveCurrentProjectData();

	// Returns the TimeClock data for the current project.
	UFUNCTION(BlueprintPure, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool GetCurrentProjectData(FTimeClockData& ProjectData);
	
	// Saves the input TimeClockData (useful to update closed project).
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool SaveProjectData(FTimeClockData TimeClockData);

	// Deletes the saved data for a project.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool DeleteSavedProjectData(FString ProjectName);

	// Returns the saved data project for the input project name if any.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool GetSavedDataForProject(FString ProjectName, FTimeClockData& ProjectData);
	
	// Returns all saved project data.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	TArray<FTimeClockData> GetAllSavedProjectData();
	



	/* ===== File I/O - Save/Load ===== */

	// Returns the filename of the TimeClock save file (Usually in my Documents).
	FString GetSavePath();

	// Loads the Save File.
	bool LoadSaveFile(FString &FileContent);

	// Writes the Save File (replaces its content).
	bool WriteSaveFile(FString FileContent);

	// Used to write the Save File's content (converts FTimeClockData array to string, applying the delimiters).
	FString SerialiseFileContent(TArray<FTimeClockData> TimeClockData);

	// Used to read the loaded Save File's content (converts its content to FTimeClockData array, taking in account the delimiters).
	TArray<FTimeClockData> DeserialiseFileContent(FString FileContent);

	// Converts TimeClockData to string, applying the ValueDelimiter.
	bool SerialiseTimeClockData(FTimeClockData StructData, FString& Success);

	// Converts string to TimeClockData, taking in account the ValueDelimiter.
	bool DeserialiseTimeClockData(FString StringData, FTimeClockData& Success);


	/* ===== File I/O - Export ===== */

	// Formats TimeClock Data struct for export as CSV
	bool FormatProjectDataForExport(FTimeClockData StructData, FString& Data);

	// Export the input data in a CSV format.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
	bool ExportDataToCSV(TArray<FTimeClockData> Data, FString Directory, bool AsTextFile);
};
