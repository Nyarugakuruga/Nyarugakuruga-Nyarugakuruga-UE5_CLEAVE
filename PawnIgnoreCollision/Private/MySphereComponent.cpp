#include "MySphereComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UMySphereComponent::UMySphereComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//初期化処理とSphereCollisionの作成
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Overlap);
	SetGenerateOverlapEvents(true);

	TargetActorClass = ACharacter::StaticClass();

	WallSphere = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InternalWallSphere"));
	if (WallSphere)
	{
		WallSphere->SetupAttachment(this);
		WallSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WallSphere->SetCollisionResponseToAllChannels(ECR_Block);
	}
}

void UMySphereComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateSphereRadii();

	OnComponentBeginOverlap.AddDynamic(this, &UMySphereComponent::OverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UMySphereComponent::OverlapEnd);
}


//内側の円の半径を更新する関数
void UMySphereComponent::UpdateSphereRadii()
{
	SetSphereRadius(TriggerRadius);

	if (WallSphere)
	{
		WallSphere->SetSphereRadius(TriggerRadius * InnerWallRatio);
	}
}

//ビューポート上で大きさが連動して変化するようにするための処理
#if WITH_EDITOR
void UMySphereComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateSphereRadii();
}
#endif

bool UMySphereComponent::IsValidTarget(AActor* Actor) const
{
	if (!Actor || Actor == GetOwner()) return false;

	if (TargetActorClass)
	{
		return Actor->IsA(TargetActorClass);
	}
	return true;
}

void UMySphereComponent::OverlapBegin(
	UPrimitiveComponent* OverlapComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValidTarget(OtherActor) || !OtherComp || !WallSphere) return;

	//すでにIgnore状態のActorは無視
	if (ActiveIgnoredActors.Contains(OtherActor))
	{
		return;
	}

	float TargetSpeed = OtherActor->GetVelocity().Size();

	if (TargetSpeed >= PassSpeedThreshold)
	{
		//500以上の速度で進入してきたキャラはignoreさせる
		OtherComp->IgnoreComponentWhenMoving(WallSphere, true);
		WallSphere->IgnoreComponentWhenMoving(OtherComp, true);

		if (bIgnoreOwnerActorToo && GetOwner())
		{
			OtherComp->IgnoreActorWhenMoving(GetOwner(), true);
		}

		SlowingActors.Remove(OtherActor);
		//外に出た時に解除するためにロックをかける
		ActiveIgnoredActors.Add(OtherActor);
	}
	else
	{
		//50未満の速度で進入してきたキャラは減速させる
		if (TargetSpeed < 50.0f)
		{
			OtherComp->IgnoreComponentWhenMoving(WallSphere, true);
			WallSphere->IgnoreComponentWhenMoving(OtherComp, true);

			if (bIgnoreOwnerActorToo && GetOwner())
			{
				OtherComp->IgnoreActorWhenMoving(GetOwner(), true);
			}

			SlowingActors.Remove(OtherActor);

			ActiveIgnoredActors.Add(OtherActor);

			return;
		}

		//50~500の速度で進入してきたキャラは減速させる
		OtherComp->IgnoreComponentWhenMoving(WallSphere, false);
		WallSphere->IgnoreComponentWhenMoving(OtherComp, false);

		if (bIgnoreOwnerActorToo && GetOwner())
		{
			OtherComp->IgnoreActorWhenMoving(GetOwner(), false);
		}

		SlowingActors.AddUnique(OtherActor);
		SetComponentTickEnabled(true);
	}
}

void UMySphereComponent::OverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValidTarget(OtherActor) || !OtherComp || !WallSphere) return;

	//外側の円を出るまでは完全リセットしない
	float Dist = FVector::Dist(OtherActor->GetActorLocation(), GetComponentLocation());
	if (Dist <= GetScaledSphereRadius())
	{
		return;
	}

	//外側の円を出た時点で完全リセットする
	ActiveIgnoredActors.Remove(OtherActor);

	OtherComp->IgnoreComponentWhenMoving(WallSphere, false);
	WallSphere->IgnoreComponentWhenMoving(OtherComp, false);

	if (bIgnoreOwnerActorToo && GetOwner())
	{
		OtherComp->IgnoreActorWhenMoving(GetOwner(), false);
	}

	SlowingActors.Remove(OtherActor);

	if (SlowingActors.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}
}



void UMySphereComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//500未満で進入してきたキャラの減速処理
	if (SlowingActors.Num() == 0 || !WallSphere)
	{
		SetComponentTickEnabled(false);
		return;
	}

	float OuterRadius = GetScaledSphereRadius();
	float InnerRadius = WallSphere->GetScaledSphereRadius();
	FVector CenterLocation = GetComponentLocation();

	for (int32 i = SlowingActors.Num() - 1; i >= 0; --i)
	{
		AActor* TargetActor = SlowingActors[i];
		if (!TargetActor)
		{
			SlowingActors.RemoveAt(i);
			continue;
		}

		ACharacter* Char = Cast<ACharacter>(TargetActor);
		if (!Char) continue;

		UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
		if (!MoveComp) continue;

		//2つの円の半径に応じて減速させるためのAlpha値を計算する
		float Distance = FVector::Dist(Char->GetActorLocation(), CenterLocation);
		float Alpha = FMath::GetMappedRangeValueClamped(
			FVector2D(InnerRadius, OuterRadius),
			FVector2D(0.0f, 1.0f),
			Distance
		);

		FVector CurrentVel = MoveComp->Velocity;
		FVector TargetVel = CurrentVel * Alpha;

		MoveComp->Velocity = FMath::VInterpTo(CurrentVel, TargetVel, DeltaTime, SlowdownEaseSpeed);
	}
}