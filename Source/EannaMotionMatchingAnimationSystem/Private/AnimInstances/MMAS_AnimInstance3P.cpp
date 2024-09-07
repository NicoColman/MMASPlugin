// Eanna Entertainment


#include "AnimInstances/MMAS_AnimInstance3P.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimationWarpingLibrary.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "Interfaces/MMAS_InterfaceCharacter.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchResult.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"


void UMMAS_AnimInstance3P::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UMMAS_AnimInstance3P::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MMAnimInstanceProxy.OwningCharacter) return;
	GenerateTrajectory();
	// bOffsetRootBoneEnabled = UKismetSystemLibrary::GetConsoleVariableBoolValue(FString("a.animnode.offsetrootbone.enable"));
	// MMDatabaseLOD = UKismetSystemLibrary::GetConsoleVariableIntValue(FString("DDCvar.MMDatabaseLOD"));
}

void UMMAS_AnimInstance3P::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!MMAnimInstanceProxy.OwningCharacter) return;
	DeltaTimeX = DeltaSeconds;
	UpdateEssentialValues(DeltaSeconds);
	UpdateStates();
	UpdateAimOffset();
	UpdateLayeringValues();
	UpdateAimValues();
	
}

void UMMAS_AnimInstance3P::UpdateEssentialValues(const float DeltaSeconds)
{
	CharacterTransform = MMAnimInstanceProxy.OwningCharacter->GetActorTransform();
	bShouldMove = CheckUpdateLayeringData(IsMoving(), bShouldMove);

	Acceleration = MMAnimInstanceProxy.OwningCharacterMovementComponent->GetCurrentAcceleration();
	AccelerationAmount = Acceleration.Size() / MMAnimInstanceProxy.OwningCharacterMovementComponent->GetMaxAcceleration();
	bHasAcceleration = AccelerationAmount > 0.f;

	VelocityLastFrame = Velocity;
	Velocity = MMAnimInstanceProxy.OwningCharacterMovementComponent->Velocity;
	Speed2D = Velocity.Size2D();
	bHasVelocity = Speed2D > 5.f;
	VelocityAcceleration = (Velocity - VelocityLastFrame) / FMath::Max(DeltaTimeX, 0.001f);
	if (bHasVelocity) LastNonZeroVelocity = Velocity;

	// TODO: Make it PreEvent
	if (!CurrentSelectedDatabase) return;
	LastFrameDatabaseTags = CurrentDatabaseTags;
	if (!CurrentSelectedDatabase->Tags.IsEmpty()) CurrentDatabaseTags = CurrentSelectedDatabase->Tags;
}

void UMMAS_AnimInstance3P::UpdateStates()
{
	LastFrameMovementMode = MovementMode;
	switch (MMAnimInstanceProxy.OwningCharacterMovementComponent->MovementMode)
	{
		case MOVE_Walking:
		case MOVE_NavWalking:
		case MOVE_None:
		{
			MovementMode = ECustomMovementMode::Grounded;
			break;
		}
		case MOVE_Falling:
		{
			MovementMode = ECustomMovementMode::InAir;
			break;
		}
		default:
			break;
	}

	LastFrameRotationMode = RotationMode;
	RotationMode = MMAnimInstanceProxy.OwningCharacterMovementComponent->bOrientRotationToMovement ? ERotationMode::OrientToMovement : ERotationMode::Strafe;

	LastFrameMovementState = MovementState;
	MovementState = /** TODO: bShouldMove*/ IsMoving() ? EMovementState::Moving : EMovementState::Idle;

	LastFrameGait = Gait;
	CheckUpdateLayeringData(LastFrameGait != Gait, false);
	Gait = IMMAS_InterfaceCharacter::Execute_GetGait(MMAnimInstanceProxy.OwningCharacter);

	LastFrameStance = Stance;
	Stance = MMAnimInstanceProxy.OwningCharacterMovementComponent->IsCrouching() ? EStance::Crouching : EStance::Standing;

	// TODO: Implement Object State
}

void UMMAS_AnimInstance3P::UpdateAimOffset()
{
	AOValue = GetAimOffsetValue();
	SpineRotation = GetSpineRotation();
}

void UMMAS_AnimInstance3P::UpdateLayeringValues()
{
	if (!bShouldUpdateLayeringData && !CurrentSelectedDatabase) return;

	
}

void UMMAS_AnimInstance3P::UpdateAimValues()
{
	bShouldOverlayAim = CanOverlayAim();
	bIsWithinAimOffsetThreshold = IsWithinAimOffsetThreshold();
	bIsCurrentlyPivoting = IsPivoting();
}

bool UMMAS_AnimInstance3P::CheckUpdateLayeringData(const bool bCurrent, const bool bLast)
{
	if (bCurrent != bLast)
	{
		bShouldUpdateLayeringData = true;
		return true;
	}
	return false;
}

FTransform UMMAS_AnimInstance3P::GetRootBoneTransform(const FAnimNodeReference& Node) const
{
	if (!bOffsetRootBoneEnabled) return CharacterTransform;
	
	const FTransform OffSetRootTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);
	
	FTransform RootBoneTransform;
	RootBoneTransform.SetLocation(OffSetRootTransform.GetLocation());
	UE::Math::TQuat<double> Rotator = OffSetRootTransform.GetRotation();
	Rotator.Z += 90.f;
	RootBoneTransform.SetRotation(Rotator);
	RootBoneTransform.SetScale3D(UE::Math::TVector<double>(1.f, 1.f, 1.f));
	return RootBoneTransform;
}

bool UMMAS_AnimInstance3P::IsMoving() const
{
	return	!Velocity.Equals(FVector::ZeroVector, 0.1f) &&
			!FutureVelocity.Equals(FVector::ZeroVector, 0.1f);
}

bool UMMAS_AnimInstance3P::IsStarting() const
{
	const bool bFutureVelocityGreater = FutureVelocity.Size2D() >= Velocity.Size2D() + 100.f;
	return IsMoving() && bFutureVelocityGreater && !CurrentDatabaseTags.Contains(FName("Pivots"));
}

bool UMMAS_AnimInstance3P::ShouldTurnInPlace() const
{
	const FRotator NormalizedRotation = UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.GetRotation().Rotator(), RootTransform.GetRotation().Rotator());
	const bool AbsoluteYawGreater = FMath::Abs(NormalizedRotation.Yaw) >= 50.f;
	return AbsoluteYawGreater && (/** TODO: bWantsToAim || */ MovementState == EMovementState::Idle && LastFrameMovementState == EMovementState::Moving);
}

bool UMMAS_AnimInstance3P::ShouldLandLightly() const
{
	if (!MMAnimInstanceProxy.OwningCharacter) return false;
	const auto& [bJustLanded, LandVelocity] = IMMAS_InterfaceCharacter::Execute_GetCharacterJumpData(MMAnimInstanceProxy.OwningCharacter);
	return bJustLanded && FMath::Abs(LandVelocity.Z) < FMath::Abs(HeavyLandSpeedThreshold);
}

bool UMMAS_AnimInstance3P::ShouldLandHeavy() const
{
	if (!MMAnimInstanceProxy.OwningCharacter) return false;
	const auto& [bJustLanded, LandVelocity] = IMMAS_InterfaceCharacter::Execute_GetCharacterJumpData(MMAnimInstanceProxy.OwningCharacter);
	return bJustLanded && FMath::Abs(LandVelocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold);
}

bool UMMAS_AnimInstance3P::WasTraversing() const
{
	return !IsSlotActive(FName("DefaultSlot")) && GetCurveValue(FName("MovingTraversal")) > 0.f;
}

bool UMMAS_AnimInstance3P::ShouldSpinTransition() const
{
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.GetRotation().Rotator(), RootTransform.GetRotation().Rotator());
	const bool AbsoluteYawGreater = FMath::Abs(DeltaRotator.Yaw) >= 130.f;
	return AbsoluteYawGreater && Speed2D >= 150.f && !CurrentDatabaseTags.Contains(FName("Pivots"));
}

EOffsetRootBoneMode UMMAS_AnimInstance3P::GetOffsetRootRotationMode() const
{
	return IsSlotActive(FName("DefaultSlot")) ? EOffsetRootBoneMode::Release : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UMMAS_AnimInstance3P::GetOffsetRootTranslationMode() const
{
	EOffsetRootBoneMode OffsetRootBoneMode = EOffsetRootBoneMode::Release;
	switch (MovementMode)
	{
		case ECustomMovementMode::Grounded:
		{
			OffsetRootBoneMode = bShouldMove ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;
			break;
		}
		case ECustomMovementMode::InAir:
		{
			OffsetRootBoneMode = EOffsetRootBoneMode::Release;
			break;
		}
		case ECustomMovementMode::Swimming:
		default:
			break;
	}
	return IsSlotActive(FName("DefaultSlot")) ? EOffsetRootBoneMode::Release : OffsetRootBoneMode;
}

float UMMAS_AnimInstance3P::GetOffsetRootTranslationHalfLife() const
{
	return MovementState == EMovementState::Idle ? 0.1f : 0.2f;
}

EOrientationWarpingSpace UMMAS_AnimInstance3P::GetOrientationWarpingSpace() const
{
	return bOffsetRootBoneEnabled ? EOrientationWarpingSpace::RootBoneTransform : EOrientationWarpingSpace::ComponentTransform;
}

bool UMMAS_AnimInstance3P::ShouldEnableAO() const
{
	return bIsWithinAimOffsetThreshold && RotationMode == ERotationMode::Strafe &&
		GetSlotMontageLocalWeight(FName("DefaultSlot"));
}

FVector2D UMMAS_AnimInstance3P::GetAimOffsetValue() const
{
	if (!MMAnimInstanceProxy.OwningCharacter) return FVector2D();

	const FRotator ControlRotation = MMAnimInstanceProxy.OwningCharacter->GetControlRotation();
	const FRotator RootRotation = RootTransform.GetRotation().Rotator();

	UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, RootRotation);
	return FVector2D(ControlRotation.Yaw, ControlRotation.Pitch);
}

bool UMMAS_AnimInstance3P::IsWithinAimOffsetThreshold() const
{
	return FMath::Abs(AOValue.X) <= AOMaxAngleThreshold;
}

FRotator UMMAS_AnimInstance3P::GetSpineRotation() const
{
	const FRotator Rotator = FRotator(0.f, 0.f, FMath::ClampAngle(AOValue.X, -90.f, 90.f / 6.f));
	const FRotator InterpRotator = FMath::RInterpConstantTo(SpineRotation, Rotator, DeltaTimeX, 150.f);
	return ShouldInterpolateSpineRotation() ? InterpRotator : Rotator;
	
}

bool UMMAS_AnimInstance3P::ShouldInterpolateSpineRotation() const
{
	const float Rotator = FMath::Abs(AOValue.X - SpineRotation.Yaw);
	const float FloatRotationMode = RotationMode == ERotationMode::OrientToMovement ? 25.f : AOMaxAngleThreshold;
	return Rotator > FloatRotationMode;
}

FVector UMMAS_AnimInstance3P::GetRelativeAccelerationAmount() const
{
	if (!MMAnimInstanceProxy.OwningCharacterMovementComponent) return FVector::ZeroVector;
	
	const float MaxAcceleration = MMAnimInstanceProxy.OwningCharacterMovementComponent->GetMaxAcceleration();
	const float MaxBrakingDeceleration = MMAnimInstanceProxy.OwningCharacterMovementComponent->GetMaxBrakingDeceleration();

	if (MaxAcceleration <= 0.0f || MaxBrakingDeceleration <= 0.0f) return FVector::ZeroVector;

	return FVector::DotProduct(Acceleration, Velocity) > 0.0f ?
		CharacterTransform.GetRotation().Rotator().UnrotateVector(VelocityAcceleration.GetClampedToMaxSize(MaxAcceleration) / MaxAcceleration) :
		CharacterTransform.GetRotation().Rotator().UnrotateVector(VelocityAcceleration.GetClampedToMaxSize(MaxBrakingDeceleration) / MaxBrakingDeceleration);
}

float UMMAS_AnimInstance3P::GetLeanAmount() const
{
	return GetRelativeAccelerationAmount().Y * FMath::GetMappedRangeValueClamped(FVector2D(200.f, 500.f), FVector2D(0.5f, 1.f), Speed2D);
}

void UMMAS_AnimInstance3P::UpdateMotionMatching(const FAnimNodeReference& Node)
{
	FMotionMatchingAnimNodeReference MotionMatchingAnimNodeReference;
	bool bResult = false;
	UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNodePure(Node, MotionMatchingAnimNodeReference, bResult);

	TArray<UPoseSearchDatabase*> Databases;
	UMMAS_Utilities::GetUObjectChooserMultiResults(DatabasesChooserTable, this,Databases);

	UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(MotionMatchingAnimNodeReference, Databases, GetMMInterruptMode());
}

void UMMAS_AnimInstance3P::UpdateMotionMatchingPostSelection(const FAnimNodeReference& Node)
{
	FMotionMatchingAnimNodeReference MotionMatchingAnimNodeReference;
	bool bResult = false;
	FPoseSearchBlueprintResult PoseSearchBlueprintResult;
	bool IsValidResult = false;
	
	UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNodePure(Node, MotionMatchingAnimNodeReference, bResult);
	UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MotionMatchingAnimNodeReference, PoseSearchBlueprintResult, IsValidResult);
	CurrentSelectedDatabase = PoseSearchBlueprintResult.SelectedDatabase.Get();
}

float UMMAS_AnimInstance3P::GetMMBlendTime() const
{
	switch (MovementMode)
	{
	case ECustomMovementMode::Grounded:
		{
			return LastFrameMovementMode == ECustomMovementMode::Grounded ? 0.4f : 0.2f;
		}
	case ECustomMovementMode::InAir:
		{
			return Velocity.Z > 100.f ? 0.15f : 0.5;
		}
	case ECustomMovementMode::Swimming:
	default:
		return 0.2f;
	}
}

EPoseSearchInterruptMode UMMAS_AnimInstance3P::GetMMInterruptMode() const
{
	const bool bDifferentLastFrameMovementMode = LastFrameMovementMode != MovementMode;
	const bool DifferentEnums = LastFrameMovementState != MovementState || LastFrameGait != Gait || LastFrameStance != Stance;
	const bool bDifferentEnumsAndGrounded = DifferentEnums && MovementMode == ECustomMovementMode::Grounded;
	
	const bool bIndexFalse = bDifferentEnumsAndGrounded || bDifferentLastFrameMovementMode;
	const bool bIndexTrue = bDifferentEnumsAndGrounded || bDifferentLastFrameMovementMode || ShouldInterruptCustomIdle();

	const bool SelectedIndex = IsCustomIdleOrTransitioningOutOfCustomIdle() ? bIndexTrue : bIndexFalse;
	return SelectedIndex ? EPoseSearchInterruptMode::InterruptOnDatabaseChange : EPoseSearchInterruptMode::DoNotInterrupt; 
}

bool UMMAS_AnimInstance3P::ShouldInterruptCustomIdle() const
{
	return ShouldTryToInterruptCustomIdle(MovementState == EMovementState::Idle && !CurrentDatabaseTags.Contains(FName("TurnInPlace")));
}

bool UMMAS_AnimInstance3P::ShouldTryToInterruptCustomIdle(const bool bInterrupt) const
{
	return (CurrentDatabaseTags.Contains(FName("Stop")) && bFeetPlanted) ||
		(CurrentDatabaseTags.Contains(FName("Idle")) && !CurrentDatabaseTags.Contains(FName("Stop"))) ?
			bInterrupt : false;
}

bool UMMAS_AnimInstance3P::IsCustomIdleOrTransitioningOutOfCustomIdle() const
{
	// TODO: Implement
	return false;
}

void UMMAS_AnimInstance3P::GenerateTrajectory()
{
	FPoseSearchQueryTrajectory OutTrajectory;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTrajectory
	(
		MMAnimInstanceProxy.OwningCharacter->GetMesh()->GetAnimInstance(),
		Speed2D > 0.f ? TrajectoryGenerationDataMoving : TrajectoryGenerationDataIdle,
		MMAnimInstanceProxy.GetDeltaSeconds(),
		Trajectory,
		PreviousDesiredControllerYaw,
		OutTrajectory,
		-1.f,
		30,
		0.1f,
		15
	);

	FPoseSearchTrajectory_WorldCollisionResults WorldCollisionResults;
	UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions
	(
		this,
		this,
		OutTrajectory,
		true,
		0.01f,
		OutTrajectory,
		WorldCollisionResults,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		true,
		150.f
		);

	Trajectory = OutTrajectory;
	TrajectoryCollisions = WorldCollisionResults;

	FPoseSearchQueryTrajectorySample TrajectorySample;
	UPoseSearchTrajectoryLibrary::GetTrajectorySampleAtTime(Trajectory, 0.5f, TrajectorySample);
	FPoseSearchQueryTrajectorySample TrajectorySample2;
	UPoseSearchTrajectoryLibrary::GetTrajectorySampleAtTime(Trajectory, 0.4f, TrajectorySample2);

	FutureVelocity = (TrajectorySample.Position - TrajectorySample2.Position) * 10.f;
}


bool UMMAS_AnimInstance3P::ShouldSteer() const
{
	return MovementState == EMovementState::Moving || MovementMode ==  ECustomMovementMode::InAir;
}

FQuat UMMAS_AnimInstance3P::GetDesiredFacingTrajectory() const
{
	FPoseSearchQueryTrajectorySample TrajectorySample;
	UPoseSearchTrajectoryLibrary::GetTrajectorySampleAtTime(Trajectory, 0.5f, TrajectorySample);
	return TrajectorySample.Facing;
}

bool UMMAS_AnimInstance3P::CanOverlayAim() const
{
	return bWantsToAim && !(ShouldLandHeavy() && ShouldLandLightly());
}

bool UMMAS_AnimInstance3P::IsPivoting() const
{
	const FRotator VelocityRotation = Velocity.ToOrientationRotator();
	const FRotator FutureVelocityRotation = FutureVelocity.ToOrientationRotator();
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(FutureVelocityRotation, VelocityRotation);
	const float AbsoluteDelta = FMath::Abs(DeltaRotator.Yaw);

	return AbsoluteDelta >= (RotationMode == ERotationMode::OrientToMovement ? 60.f : 40.f);
}