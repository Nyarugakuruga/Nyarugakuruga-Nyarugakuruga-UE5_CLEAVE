#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h" 
#include "MyCharacter.generated.h" 

UCLASS()
class FINALPLAYER_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	float DesiredArmLength = 450.0f; 

	AActor* TargetActor = nullptr;

	// カメラが敵を探す半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float CameraSensorRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float BaseCameraLength = 230.0f;

	// 敵に近づいたときに最大まで引くカメラの長さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MaxCloseArmLength = 500.0f;

	// カメラが引き始める敵との距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float NormalDistThreshold = 800.0f;

	// これ以上近づいたらカメラが引ききる敵との距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float CloseDistThreshold = 1000.0f;

public:
	virtual void Tick(float DeltaTime) override;
};
