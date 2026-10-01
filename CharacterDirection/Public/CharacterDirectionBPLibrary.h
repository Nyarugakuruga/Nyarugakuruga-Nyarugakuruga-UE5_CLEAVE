#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharacterDirectionBPLibrary.generated.h"

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
    FWD UMETA(DisplayName = "Nyaru_Forward"),
    R UMETA(DisplayName = "Nyaru_Right"),
    L UMETA(DisplayName = "Nyaru_Left"),
    BWD UMETA(DisplayName = "Nyaru_Back")

};


UCLASS()
class CHARACTERDIRECTION_API UCharacterDirectionBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Direction", Keywords = "SetCharacterDirection", ExpandEnumAsExecs = "Branches", DefaultToSelf = "TargetPawn"), Category = "Utilities|Transformation")
    static void SetDirection(
        APawn* TargetPawn,
        float ActionValueX,
        float ActionValueY,
        EMoveDirection& Branches,
        float Threshold = 0.5f
    );

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Direction", Keywords = "GetCharacterDirection", DefaultToSelf = "TargetPawn"), Category = "Utilities|Transformation")
    static void GetDirection(
        APawn* TargetPawn,
        float ActionValueX,
        float ActionValueY,
        FVector& FWD,
        FVector& R,
        FVector& L,
        FVector& BWD,
        float Threshold = 0.5f
    );
};