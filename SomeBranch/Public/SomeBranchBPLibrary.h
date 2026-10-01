// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SomeBranchBPLibrary.generated.h"

UENUM(BlueprintType)
enum class ESomeBranchResult : uint8
{
	A_And_B UMETA(DisplayName = "A  && B"),
	A_And_Not_B UMETA(DisplayName = "A  && !B"),
	Not_A UMETA(DisplayName = "!A")
};

UCLASS()
class SOMEBRANCH_API USomeBranchBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Some Branch", Keywords = "SomeBranch Multi Condition Branch", ExpandEnumAsExecs = "Branches"), Category = "SomeBranch")
	static void SomeBranch(
		bool A,
		bool B,
		ESomeBranchResult& Branches
	);
};