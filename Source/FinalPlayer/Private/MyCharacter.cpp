#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//cameraboomの作成、初期化
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	//cameraboom距離の設定
	CameraBoom->TargetArmLength = BaseCameraLength;

	// cameraboomの地面からの距離
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 20.0f);

	//少しずらす
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, -30.0f);

	//Playerの回転に追従するようにする
	CameraBoom->bUsePawnControlRotation = true;

	//cameraの作成、初期化
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector MyCheckLocation = GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);

	//一番近い敵を探す
	TargetActor = nullptr;
	float ClosestDistance = CameraSensorRadius;
	
	//判定領域の作成
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(CameraSensorRadius);
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;

	// 自分自身を無視するように設定
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		MyCheckLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		SphereShape,
		QueryParams
	);

	//Enemyタグを持つアクターの中で最も近いものを選ぶ
	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (OverlappedActor && OverlappedActor->ActorHasTag(FName("Enemy")))
			{
				FVector EnemyCheckLocation = OverlappedActor->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);
				float Dist = FVector::Dist(MyCheckLocation, EnemyCheckLocation);

				if (Dist < ClosestDistance)
				{
					ClosestDistance = Dist;
					TargetActor = OverlappedActor;
				}
			}
		}
	}

	///armの位置を敵との中間地点にする
	FVector IdealBoomLocation = MyCheckLocation;
	
	float TargetLength = BaseCameraLength;

	if (TargetActor)
	{
		//z軸を入れると画面酔いが起きるので、平面上の中間地点を割り出す
		FVector PlayerPos2D = FVector(MyCheckLocation.X, MyCheckLocation.Y, 0.0f);
		FVector EnemyPos2D = FVector(TargetActor->GetActorLocation().X, TargetActor->GetActorLocation().Y, 0.0f);

		FVector PlayerToEnemy2D = EnemyPos2D - PlayerPos2D;
		float CurrentEnemyDist2D = PlayerToEnemy2D.Size();
		float MiddleEnemyDist = CurrentEnemyDist2D / 2.0f;

		FVector Direction2D = PlayerToEnemy2D.GetSafeNormal();

		// 平面上の真ん中
		FVector MiddlePos2D = PlayerPos2D + (Direction2D * MiddleEnemyDist);

		//高さはPlayerの胸位置に合わせる
		IdealBoomLocation = FVector(MiddlePos2D.X, MiddlePos2D.Y, MyCheckLocation.Z);


		//Playerが写るようにカメラを引く
		float CurrentEnemyDist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

		TargetLength = BaseCameraLength + (CurrentEnemyDist * 0.30f);

		 TargetLength = FMath::Max(TargetLength, 500.0f);
	}

	DesiredArmLength = TargetLength;

	// 滑らか補間でarmの長さを更新
	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		DesiredArmLength,
		DeltaTime,
		8.0f
	);
}



