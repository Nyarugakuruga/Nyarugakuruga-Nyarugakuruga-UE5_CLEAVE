#include "CharacterDirectionBPLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

// SetDirection: 入力からカメラの移動方向を判定し、実行ピンを分岐させる
//ただプレイヤーの正面とかをカメラで向いた状態で移動するとその場合もカメラ基準に進むためそこは改良する
void UCharacterDirectionBPLibrary::SetDirection(
    APawn* TargetPawn,
    float ActionValueX,
    float ActionValueY,
    EMoveDirection& Branches,
    float Threshold)
{
    APawn* EffectivePawn = TargetPawn;

    //TargetPawnがNullの場合の保険1
    if (!EffectivePawn)
    {
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            UWorld* World = GEngine->GetWorldContexts()[0].World();
            if (World)
            {
                EffectivePawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
            }
        }
    }

    //保険2  保険1でnullならFWDを返す
    if (!EffectivePawn)
    {
        Branches = EMoveDirection::FWD;
        return;
    }

    //カメラ基準の移動ベクトル作成
    //回転取得
    FRotator ControlRot = EffectivePawn->GetControlRotation();
    FRotator YawRotation(0.0f, ControlRot.Yaw, 0.0f);

    //正面と右方向の計算
    FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    //ワールド空間での移動ベクトルを合成
    FVector CombinedVector = (ForwardVector * ActionValueY) + (RightVector * ActionValueX);

    CombinedVector.Normalize(0.0001f);

    //ワールド座標入力をキャラから見たローカル座標系に変換
    FVector LocalInputVector = ControlRot.UnrotateVector(CombinedVector);

    //前後左右の方向判定
    if (LocalInputVector.X >= Threshold)       Branches = EMoveDirection::FWD; 
    else if (LocalInputVector.Y >= Threshold)  Branches = EMoveDirection::R;   
    else if (LocalInputVector.Y <= -Threshold) Branches = EMoveDirection::L;  
    else                                       Branches = EMoveDirection::BWD;
}

//SetDirectionのGet版
void UCharacterDirectionBPLibrary::GetDirection(
    APawn* TargetPawn,
    float ActionValueX,
    float ActionValueY,
    FVector& FWD,
    FVector& R,
    FVector& L,
    FVector& BWD,
    float Threshold)
{
    APawn* EffectivePawn = TargetPawn;
    if (!EffectivePawn)
    {
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            UWorld* World = GEngine->GetWorldContexts()[0].World();
            if (World) EffectivePawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
        }
    }

    //すべての出力ベクトルを0にする
    if (!EffectivePawn)
    {
        FWD = FVector::ZeroVector;
        R = FVector::ZeroVector;
        L = FVector::ZeroVector;
        BWD = FVector::ZeroVector;
        return;
    }

    FRotator ControlRot = EffectivePawn->GetControlRotation();
    FRotator YawRotation(0.0f, ControlRot.Yaw, 0.0f);

    FVector BaseForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    FVector BaseRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    FWD = BaseForward;          
    R = BaseRight;             
    L = BaseRight * -1.0f;       
    BWD = BaseForward * -1.0f;   
}