// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LatentActionManager.h"
#include "Curves/CurveVector.h"
#include "ArcMotionBPLibrary.generated.h"

UCLASS()
class UArcMotionBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY() 

public:
	UFUNCTION(BlueprintCallable, Category = "Arc Motion", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", Duration = "1.0"))
	//ÉmÅ[ÉhÇÃà¯êîê›íË
	static void ArcLaunchCharacter(
		UObject* WorldContextObject,
		ACharacter* TargetCharacter,
		FVector TargetLocation,
		UCurveVector* MovementCurve,
		float Duration,
		FVector MotionScale,
		FLatentActionInfo LatentInfo
	);
};