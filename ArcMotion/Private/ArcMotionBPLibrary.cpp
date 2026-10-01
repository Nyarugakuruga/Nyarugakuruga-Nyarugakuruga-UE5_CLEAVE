// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArcMotionBPLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "ArcMotion.h"

class FArcLaunchLatentAction : public FPendingLatentAction
{
public:
	ACharacter* TargetChar;
	FVector StartLoc;
	FVector EndLoc;
	UCurveVector* Curve;
	float TotalDuration;
	FVector Scale;
	float ElapsedTime;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FArcLaunchLatentAction(ACharacter* InChar, FVector InTargetLoc, UCurveVector* InCurve, float InDuration, FVector InScale, const FLatentActionInfo& LatentInfo)
		: TargetChar(InChar)
		, EndLoc(InTargetLoc)
		, Curve(InCurve)
		, TotalDuration(InDuration)
		, Scale(InScale)
		, ElapsedTime(0.0f)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
		if (TargetChar)
		{
			StartLoc = TargetChar->GetActorLocation();
			//放物線移動させる際に地面に引っかかったりしないように、飛行モードにする
			if (UCharacterMovementComponent* MoveComp = TargetChar->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Flying);
			}
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (!TargetChar || !Curve || TotalDuration <= 0.0f)
		{
			Response.DoneIf(true);
			Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
			return;
		}

		ElapsedTime += Response.ElapsedTime();
		float TimeRatio = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);

		//カーブからオフセット値を取り出す
		FVector CurveValue = Curve->GetVectorValue(TimeRatio);

		//移動計算
		FVector CurrentLinearPos = FMath::Lerp(StartLoc, EndLoc, TimeRatio);
		FRotator LaunchRotation = (EndLoc - StartLoc).Rotation();
		FVector WorldCurveOffset = LaunchRotation.RotateVector(CurveValue * Scale);

		TargetChar->SetActorLocation(CurrentLinearPos + WorldCurveOffset, true);

		//移動完了時
		if (TimeRatio >= 1.0f)
		{
			if (UCharacterMovementComponent* MoveComp = TargetChar->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Falling); //通常モードに戻す
			}

			Response.DoneIf(true);
			Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

void UArcMotionBPLibrary::ArcLaunchCharacter(
	UObject* WorldContextObject,
	ACharacter* TargetCharacter,
	FVector TargetLocation,
	UCurveVector* MovementCurve,
	float Duration,
	FVector MotionScale,
	FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FArcLaunchLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			LatentActionManager.AddNewAction(
				LatentInfo.CallbackTarget,
				LatentInfo.UUID,
				new FArcLaunchLatentAction(TargetCharacter, TargetLocation, MovementCurve, Duration, MotionScale, LatentInfo)
			);
		}
	}
}