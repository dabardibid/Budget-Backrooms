// Corentin GUILLAUME 2020  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OpenNewWindow.generated.h"

/**
 * 
 */
UCLASS()
class LDASSISTANT_API UOpenNewWindow : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		UFUNCTION(BlueprintCallable, Category = "ActorFuncions", meta = (WorldContext = WorldContextObject))
		static AActor* OpenWindow();
	//		static AActor* OpenWindow(AActor* InputActor);
};
