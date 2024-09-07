// Eanna Entertainment


#include "ActorComponents/MMAS_ACTraversal.h"
#include "AnimationWarpingLibrary.h"
#include "MMAS_Utilities.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/MMAS_InterfaceAnimInstance.h"
#include "Interfaces/MMAS_InterfaceCharacter.h"
#include "Interfaces/MMAS_InterfaceTraversableActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "TimerManager.h"


UMMAS_ACTraversal::UMMAS_ACTraversal()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsDoingTraversalAction = false;
	TraversalActionType = ETraversalActionType::None;
}

void UMMAS_ACTraversal::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UMMAS_ACTraversal::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
}

void UMMAS_ACTraversal::PerformJumpOrTraversalAction() const
{
	if (bIsDoingTraversalAction || !OwningCharacter->GetCharacterMovement()->IsMovingOnGround()) return;
	if (TryTraversalAction().bTraversalCheckFailed)
	{
		OwningCharacter->Jump();
	}
}

void UMMAS_ACTraversal::SetCharacterJumpData(FCharacterJumpData& InCharacterJumpData) const
{
	InCharacterJumpData.LandVelocity = OwningCharacter->GetCharacterMovement()->Velocity;
	InCharacterJumpData.bJustLanded = true;

	FTimerHandle TimerHandle;
	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, &TimerHandle, &InCharacterJumpData]()
	{
		TimerHandle.Invalidate();
		InCharacterJumpData.bJustLanded = false;
	});
	OwningCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.3f, false);
}

FTraversalCheckResult UMMAS_ACTraversal::TryTraversalAction() const
{
	FTraversalCheckResult TraversalCheckResult = FTraversalCheckResult();

	// Cache the character's location, forward vector, capsule radius, and capsule half height
	if (!TraversalCheckResult.SetInitialValues(OwningCharacter)) return TraversalCheckResult;
	
	// Check if the actor is in front of a traversable object
	if (!IsATraversableObject(GetTraversalForwardTraceDistance(TraversalCheckResult), TraversalCheckResult)) return TraversalCheckResult;

	// Check For Valid Front Ledge
	if (!TraversalCheckResult.bHasFrontLedge) return TraversalCheckResult;
	
	// Perform ledge configurations, checks for front ledge, back ledge, and back floor
	PerformLedgesConfigurations(TraversalCheckResult);
	if (TraversalCheckResult.bTraversalCheckFailed) return TraversalCheckResult;

	// Calculate the traversal action type based on the ledge configurations
	TraversalCheckResult.CalculateTraversalActionType();

#if WITH_EDITOR
	if (bShowDataForTraversal) ShowDataForTraversal(TraversalCheckResult);
#endif
	
	if (TraversalCheckResult.TraversalActionType == ETraversalActionType::None) return TraversalCheckResult;

	// Set the interaction transform for the animation instance through the interface
	SetAnimInstanceInteractionTransform(TraversalCheckResult);

	// Set the montage to play based on the traversal action type and the character's animation history
	SetMontageToPlay(TraversalCheckResult);
	if (TraversalCheckResult.bMontageSelectionFailed) return TraversalCheckResult;

	// Perform the traversal action based on the selected montage
	PerformTraversalAction(TraversalCheckResult);
	
	return TraversalCheckResult;
}

float UMMAS_ACTraversal::GetTraversalForwardTraceDistance(const FTraversalCheckResult& TraversalCheckResult)
{
	const float UnRotateVector = TraversalCheckResult.Character->GetActorRotation().UnrotateVector(TraversalCheckResult.Character->GetVelocity()).X;
	return FMath::GetMappedRangeValueClamped(FVector2D(0.f, 500.f), FVector2D(75.f, 350.f), UnRotateVector);
}

bool UMMAS_ACTraversal::IsATraversableObject(const float TraceForwardDistance, FTraversalCheckResult& TraversalCheckResult) const
{
	const FVector EndLocation = TraversalCheckResult.ActorLocation + (TraversalCheckResult.ActorForwardVector * TraceForwardDistance);
	constexpr float Radius = 30.f;
	constexpr float HalfHeight = 60.f;
	FHitResult OutHit;
	bool bDrawDebugTrace = false;

#if WITH_EDITOR
	bDrawDebugTrace = bDrawDebugHitResults;
#endif
	
	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		TraversalCheckResult.ActorLocation,
		EndLocation,
		Radius,
		HalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHit,
		true
		);

	if (!OutHit.bBlockingHit || !OutHit.GetActor()->Implements<UMMAS_InterfaceTraversableActor>())
	{
		TraversalCheckResult.bTraversalCheckFailed = true;
		return false;
	}
	
	TraversalCheckResult.HitComponent = OutHit.GetComponent();
	TraversalCheckResult.HitLocation = OutHit.ImpactPoint;
	IMMAS_InterfaceTraversableActor::Execute_GetLedgeTransforms(OutHit.GetActor(), TraversalCheckResult);

	return bHit;
}

void UMMAS_ACTraversal::PerformLedgesConfigurations(FTraversalCheckResult& TraversalCheckResult) const
{
	FVector FrontLedgeRoom = FVector::ZeroVector;
	if (!IsRoomOnFrontLedge(TraversalCheckResult, FrontLedgeRoom)) return;
	CheckForRoomOnBackLedge(TraversalCheckResult, FrontLedgeRoom);
}

bool UMMAS_ACTraversal::IsRoomOnFrontLedge(FTraversalCheckResult& TraversalCheckResult, FVector& OutFrontLedgeRoom) const
{
	OutFrontLedgeRoom = TraversalCheckResult.FrontLedgeLocation +
			(TraversalCheckResult.CapsuleRadius + 2.f) * TraversalCheckResult.FrontLedgeNormal +
			FVector(0.f, 0.f, TraversalCheckResult.CapsuleHalfHeight + 2.f);

	bool bDrawDebugTrace = false;

#if WITH_EDITOR
	bDrawDebugTrace = bDrawDebugHitResults;
#endif
	
	FHitResult OutHit;
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		TraversalCheckResult.ActorLocation,
		OutFrontLedgeRoom,
		TraversalCheckResult.CapsuleRadius,
		TraversalCheckResult.CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHit,
		true
		);
	
	if (/**OutHit.bBlockingHit ||*/ OutHit.bStartPenetrating)
	{
		TraversalCheckResult.bHasFrontLedge = false;
		TraversalCheckResult.bTraversalCheckFailed = true;
		return false;
	}
	
	TraversalCheckResult.ObstacleHeight = FMath::Abs(
		TraversalCheckResult.ActorLocation.Z - TraversalCheckResult.CapsuleHalfHeight - TraversalCheckResult.FrontLedgeLocation.Z);

	return true;
}

void UMMAS_ACTraversal::CheckForRoomOnBackLedge(FTraversalCheckResult& TraversalCheckResult, const FVector& InFrontLedgeRoom) const
{
	const FVector RoomBackLedge = TraversalCheckResult.BackLedgeLocation +
			(TraversalCheckResult.CapsuleRadius + 2.f) * TraversalCheckResult.BackLedgeNormal +
			FVector(0.f, 0.f, TraversalCheckResult.CapsuleHalfHeight + 2.f);
	
	FHitResult OutHit;
	bool bDrawDebugTrace = false;

#if WITH_EDITOR
	bDrawDebugTrace = bDrawDebugHitResults;
#endif
	
	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		InFrontLedgeRoom,
		RoomBackLedge,
		TraversalCheckResult.CapsuleRadius,
		TraversalCheckResult.CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHit,
		true
		);

	if (bHit)
	{
		TraversalCheckResult.ObstacleDepth = (OutHit.ImpactPoint - TraversalCheckResult.FrontLedgeLocation).Size2D();
		TraversalCheckResult.bHasBackLedge = false;
		return;
	}

	TraversalCheckResult.ObstacleDepth = (TraversalCheckResult.FrontLedgeLocation - TraversalCheckResult.BackLedgeLocation).Size2D();

	CheckForFloorBehindBackLedge(TraversalCheckResult, RoomBackLedge);
}

void UMMAS_ACTraversal::CheckForFloorBehindBackLedge(FTraversalCheckResult& TraversalCheckResult,
	const FVector& InBackLedgeRoom) const
{
	FVector EndLocation = TraversalCheckResult.BackLedgeLocation + TraversalCheckResult.BackLedgeNormal * (TraversalCheckResult.CapsuleRadius + 2.f);
	const float EndLocationZ = TraversalCheckResult.ObstacleHeight - TraversalCheckResult.CapsuleHalfHeight + 50.f;
	EndLocation.Z = EndLocation.Z - EndLocationZ;

	FHitResult OutHit;
	bool bDrawDebugTrace = false;

#if WITH_EDITOR
	bDrawDebugTrace = bDrawDebugHitResults;
#endif
	
	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		InBackLedgeRoom,
		EndLocation,
		TraversalCheckResult.CapsuleRadius,
		TraversalCheckResult.CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		OutHit,
		true
		);

	if (!bHit) return;
	
	TraversalCheckResult.bHasBackFloor = true;
	TraversalCheckResult.BackFloorLocation = OutHit.ImpactPoint;
	TraversalCheckResult.BackLedgeHeight = FMath::Abs(OutHit.ImpactPoint.Z - TraversalCheckResult.BackLedgeLocation.Z);
}

void UMMAS_ACTraversal::SetAnimInstanceInteractionTransform(
	const FTraversalCheckResult& TraversalCheckResult)
{
	const UAnimInstance* AnimInstance = TraversalCheckResult.Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Implements<UMMAS_InterfaceAnimInstance>()) return;
	FTransform InteractionTransform;
	InteractionTransform.SetLocation(TraversalCheckResult.FrontLedgeLocation);
	InteractionTransform.SetRotation(FRotationMatrix::MakeFromZ(TraversalCheckResult.FrontLedgeNormal).Rotator().Quaternion());
	InteractionTransform.SetScale3D(FVector::OneVector);
	IMMAS_InterfaceAnimInstance::Execute_SetInteractionTransform(
		TraversalCheckResult.Character->GetMesh()->GetAnimInstance() , InteractionTransform);
}

void UMMAS_ACTraversal::SetMontageToPlay(FTraversalCheckResult& TraversalCheckResult) const
{
	FTraversalChooserParams Params;
	Params.MakeTraversalChooserParams(TraversalCheckResult);

	TArray<UObject*> ChosenMontages;
	UMMAS_Utilities::GetStructChooserMultiResults(TraversalMontagesChooserTable, Params,ChosenMontages);

#if WITH_EDITOR
	if (bShowChooserSelectedMontages)
	{
		for (UObject* Montage : ChosenMontages)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Montage->GetName());
		}
	}
#endif
	
	FPoseSearchBlueprintResult MotionMatchResult;
	UPoseSearchLibrary::MotionMatch(
		TraversalCheckResult.Character->GetMesh()->GetAnimInstance(),
		ChosenMontages,
		FName("PoseHistory"),
		FPoseSearchFutureProperties(),
		MotionMatchResult,
		42
	);

	const UAnimMontage* MontageToPlay = Cast<UAnimMontage>(MotionMatchResult.SelectedAnimation);
	if (!MontageToPlay)
	{
		TraversalCheckResult.bMontageSelectionFailed = true;
#if WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Montage Selection Failed"));
#endif
		
		return;
	}
	
	TraversalCheckResult.MontageToPlay = Cast<UAnimMontage>(MotionMatchResult.SelectedAnimation);
	TraversalCheckResult.StartTime = MotionMatchResult.SelectedTime;
	TraversalCheckResult.PlayRate = MotionMatchResult.WantedPlayRate;

#if WITH_EDITOR
	if (bShowMMSelectedMontageDetails)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Montage Selected: ") + MontageToPlay->GetName() +
			TEXT(", Start Time: ") + FString::SanitizeFloat(TraversalCheckResult.StartTime) +
			TEXT(", Play Rate: ") + FString::SanitizeFloat(TraversalCheckResult.PlayRate));
	}
#endif
}

void UMMAS_ACTraversal::PerformTraversalAction(const FTraversalCheckResult& TraversalCheckResult) const
{
	UpdateWarpTargets(TraversalCheckResult);
}

void UMMAS_ACTraversal::UpdateWarpTargets(const FTraversalCheckResult& TraversalCheckResult) const
{
	UMotionWarpingComponent* MotionWarpingComponent = IMMAS_InterfaceCharacter::Execute_GetCharacterMotionWarpingComponent(TraversalCheckResult.Character);
	if (!MotionWarpingComponent)
	{
#if WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Motion Warping Component Not Found"));
#endif
		return;
	}
	
	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("FrontLedge"),
		TraversalCheckResult.FrontLedgeLocation,
		FRotationMatrix::MakeFromX(-TraversalCheckResult.FrontLedgeNormal).Rotator()
		);
	
	float AnimDistanceFromFrontLedgeToBackLedge;
	UpdateBackLedgeWarpTarget(TraversalCheckResult, MotionWarpingComponent, AnimDistanceFromFrontLedgeToBackLedge);
	UpdateBackFloorWarpTarget(TraversalCheckResult, MotionWarpingComponent, AnimDistanceFromFrontLedgeToBackLedge);

	/**
	 * Need to const_cast the montage to play because the Montage_Play function is not const and we are getting a const
	 * UObject in SetMontageToPlay, MotionMatchResult.SelectedAnimation
	 */
	UAnimInstance* AnimInstance =TraversalCheckResult. Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	UAnimMontage* Montage = const_cast<UAnimMontage*>(TraversalCheckResult.MontageToPlay.Get());
	AnimInstance->Montage_Play(
		Montage,
		TraversalCheckResult.PlayRate,
		EMontagePlayReturnType::MontageLength,
		TraversalCheckResult.StartTime
		);

	auto Lambda = [this](UAnimMontage* Montage, bool bInterrupted)
	{
		SetDoingTraversalAction(false);
	};
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindLambda(Lambda);
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindLambda(Lambda);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate,Montage);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);

	TraversalHitComponent = TraversalCheckResult.HitComponent;
	TraversalActionType = TraversalCheckResult.TraversalActionType;
	SetDoingTraversalAction(true);
}

void UMMAS_ACTraversal::UpdateBackLedgeWarpTarget(const FTraversalCheckResult& TraversalCheckResult,
	UMotionWarpingComponent* MotionWarpingComponent, float& OutAnimDistance)
{
	if (TraversalCheckResult.TraversalActionType != ETraversalActionType::Vault && TraversalCheckResult.TraversalActionType != ETraversalActionType::Hurdle)
	{
		MotionWarpingComponent->RemoveWarpTarget(FName("BackLedge"));
		return;
	}

	TArray<FMotionWarpingWindowData> OutWindows;
	UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalCheckResult.MontageToPlay, FName("BackLedge"), OutWindows);
	if (OutWindows.IsEmpty())
	{
		MotionWarpingComponent->RemoveWarpTarget(FName("BackLedge"));
		return;
	}
	
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(
		TraversalCheckResult.MontageToPlay,
		FName("Distance_From_Ledge"),
		OutWindows[0].EndTime,
		OutAnimDistance
		); 

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("BackLedge"),
		TraversalCheckResult.BackLedgeLocation,
		FRotator::ZeroRotator
		);
}

void UMMAS_ACTraversal::UpdateBackFloorWarpTarget(const FTraversalCheckResult& TraversalCheckResult,
	UMotionWarpingComponent* MotionWarpingComponent, float InAnimDistance)
{
	if (TraversalCheckResult.TraversalActionType != ETraversalActionType::Hurdle)
	{
		MotionWarpingComponent->RemoveWarpTarget(FName("BackFloor"));
		return;
	}

	TArray<FMotionWarpingWindowData> OutWindows;
	UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalCheckResult.MontageToPlay, FName("BackFloor"), OutWindows);
	if (OutWindows.IsEmpty())
	{
		MotionWarpingComponent->RemoveWarpTarget(FName("BackFloor"));
		return;
	}

	float AnimatedDistanceFromBackLedgeToBackFloor;
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(
		TraversalCheckResult.MontageToPlay,
		FName("Distance_From_Ledge"),
		OutWindows[0].EndTime,
		AnimatedDistanceFromBackLedgeToBackFloor
		);

	const FVector BackLedgeNormal = TraversalCheckResult.BackLedgeNormal * (FMath::Abs(InAnimDistance - AnimatedDistanceFromBackLedgeToBackFloor));
	const FVector LocationXY = TraversalCheckResult.BackLedgeLocation + BackLedgeNormal;
	const FVector TargetLocation = FVector(LocationXY.X, LocationXY.Y, TraversalCheckResult.BackFloorLocation.Z);

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("BackFloor"),
		TargetLocation,
		FRotator::ZeroRotator
		);
}

void UMMAS_ACTraversal::SetDoingTraversalAction(const bool bDoingTraversalAction) const
{
	OwningCharacter->GetCapsuleComponent()->IgnoreComponentWhenMoving(TraversalHitComponent, bDoingTraversalAction);
	bIsDoingTraversalAction = bDoingTraversalAction;
	if (bDoingTraversalAction)
	{
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		return;
	}

	if (TraversalActionType == ETraversalActionType::Vault)
	{
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
	else
	{
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

#if WITH_EDITOR
void UMMAS_ACTraversal::ShowDataForTraversal(const FTraversalCheckResult& TraversalCheckResult) const
{
	const FString TraversalStructResult = FString::Printf(TEXT("Traversal Struct Result:\nHas Front Ledge: %s\nHas Back Ledge: %s\nHas Back Floor: %s\nObstacle Height: %.2f\nObstacle Depth: %.2f\nBack Ledge Height: %.2f\nTraversal Action Type: %s"),
		TraversalCheckResult.bHasFrontLedge ? TEXT("True") : TEXT("False"),
		TraversalCheckResult.bHasBackLedge ? TEXT("True") : TEXT("False"),
		TraversalCheckResult.bHasBackFloor ? TEXT("True") : TEXT("False"),
		TraversalCheckResult.ObstacleHeight,
		TraversalCheckResult.ObstacleDepth,
		TraversalCheckResult.BackLedgeHeight,
		*StaticEnum<ETraversalActionType>()->GetNameStringByValue(static_cast<int64>(TraversalActionType))
		);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TraversalStructResult);
}
#endif
