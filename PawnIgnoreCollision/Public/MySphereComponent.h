#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MySphereComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "IgnoreSphereCollision"))
class PAWNIGNORECOLLISION_API UMySphereComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UMySphereComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
	TSubclassOf<AActor> TargetActorClass;

	//Ignoreする速度の値
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
	float PassSpeedThreshold = 500.0f;

	//500未満のときに減速させるための外側の半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
	float TriggerRadius = 150.0f;

	//TriggerRadius に対する内側の半径の割合
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System", meta = (ClampMin = "0.1", ClampMax = "0.99"))
	float InnerWallRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
	bool bIgnoreOwnerActorToo = true;

	// 500未満のときの減速の滑らかさ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
	float SlowdownEaseSpeed = 10.0f;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision System")
	USphereComponent* WallSphere;

	// 減速処理中の対象リスト
	UPROPERTY()
	TArray<AActor*> SlowingActors;

	UPROPERTY()
	TSet<AActor*> ActiveIgnoredActors;

	UFUNCTION()
	void OverlapBegin(
		UPrimitiveComponent* OverlapComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	void UpdateSphereRadii();
	bool IsValidTarget(AActor* Actor) const;
};