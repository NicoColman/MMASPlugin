// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "MMAS_AnimInstanceBase.h"
#include "MMAS_Utilities.h"
#include "Interfaces/MMAS_InterfaceAnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryTypes.h"
#include "MMAS_AnimInstance3P.generated.h"

struct FAnimNodeReference;
enum class EOffsetRootBoneMode : uint8;
enum class EOrientationWarpingSpace : uint8;
enum class EPoseSearchInterruptMode : uint8;
class UChooserTable;

/**
 * 
 */
UCLASS()
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API UMMAS_AnimInstance3P : public UMMAS_AnimInstanceBase, public IMMAS_InterfaceAnimInstance
{
	GENERATED_BODY()

	public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	/** IEannaMMASInterface */
	virtual void SetInteractionTransform_Implementation(const FTransform& InInteractionTransform) override { InteractionTransform = InInteractionTransform; }
	virtual FTransform GetInteractionTransform_Implementation() const override { return InteractionTransform; }
	/** IEannaMMASInterface */
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	float DeltaTimeX = 0.0f;

	// Essential Values Native
	bool bOffsetRootBoneEnabled = false;
	bool bHasVelocity;
	bool bHasAcceleration;
	
	float PreviousDesiredControllerYaw;
	float AccelerationAmount;

	FVector Velocity;
	FVector FutureVelocity;
	FVector VelocityLastFrame;
	FVector VelocityAcceleration;
	FVector Acceleration;
	FRotator ControlRotation;
	FTransform CharacterTransform;
	
	TArray<FName> CurrentDatabaseTags;
	TArray<FName> LastFrameDatabaseTags;

	// Essential Values Blueprint
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	bool bShouldMove = false;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	int32 MMDatabaseLOD = 0;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	float Speed2D;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	float HeavyLandSpeedThreshold = 700.f;
	UPROPERTY(BlueprintReadWrite, Category = "EannaMMAS|Essential Values")
	FTransform RootTransform;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Essential Values")
	FVector LastNonZeroVelocity;

	// State Values Blueprint
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	ECustomMovementMode MovementMode;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	ECustomMovementMode LastFrameMovementMode;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	ERotationMode RotationMode;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	ERotationMode LastFrameRotationMode;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EMovementState MovementState;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EMovementState LastFrameMovementState;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EGait Gait;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EGait LastFrameGait;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EStance Stance;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|State Values")
	EStance LastFrameStance;
 
	// AimOffset Values Native
	bool bIsWithinAimOffsetThreshold = false;
	float AOMaxAngleThreshold = 115.f;
	FVector2D AOValue;
	FRotator SpineRotation;

	// Layering Values Native
	bool bShouldUpdateLayeringData = false;
	bool bOverlaySettingsConfigTesting = false;

	// Aim Values Native
	bool bWantsToAim = false;
	bool bShouldOverlayAim = false;
	bool bIsWithinAimThreshold = false;
	bool bIsCurrentlyPivoting = false;

	// Motion Matching Values Native
	UPROPERTY(BlueprintReadWrite, Category = "EannaMMAS|Chooser Tables")
	TObjectPtr<UChooserTable> DatabasesChooserTable;
	bool bFeetPlanted = false;
	FPoseSearchTrajectoryData TrajectoryGenerationDataIdle;
	FPoseSearchTrajectoryData TrajectoryGenerationDataMoving;

	// Motion Matching Values Blueprint
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Motion Matching Values")
	TObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Motion Matching Values")
	FPoseSearchQueryTrajectory Trajectory;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Motion Matching Values")
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollisions;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|Motion Matching Values")
	FTransform InteractionTransform;

	UFUNCTION(meta = (BlueprintThreadSafe))
	void UpdateEssentialValues(const float DeltaSeconds);
	UFUNCTION(meta = (BlueprintThreadSafe))
	void UpdateStates();
	UFUNCTION(meta = (BlueprintThreadSafe))
	void UpdateAimOffset();
	UFUNCTION(meta = (BlueprintThreadSafe))
	void UpdateAimValues();

	// Essential Functions
	UFUNCTION(meta = (BlueprintThreadSafe))
	bool CheckUpdateLayeringData(const bool bCurrent, const bool bLast);
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = "EannaMMAS|Essential Functions"))
	FTransform GetRootBoneTransform(const FAnimNodeReference& Node) const;

	// Movement Analysis Functions
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool IsMoving() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool IsStarting() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool ShouldTurnInPlace() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool ShouldLandLightly() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool ShouldLandHeavy() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool WasTraversing() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Movement Analysis")
	bool ShouldSpinTransition() const;

	// RootOffset Functions
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|RootOffset Functions")
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|RootOffset Functions")
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|RootOffset Functions")
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|RootOffset Functions")
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;

	// AimOffset Functions
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|AimOffset Functions")
	bool ShouldEnableAO() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|AimOffset Functions")
	FVector2D GetAimOffsetValue() const;

	UFUNCTION(meta = (BlueprintThreadSafe))
	bool IsWithinAimOffsetThreshold() const;
	UFUNCTION(meta = (BlueprintThreadSafe))
	FRotator GetSpineRotation() const;
	UFUNCTION(meta = (BlueprintThreadSafe))
	bool ShouldInterpolateSpineRotation() const;

	// Additive Lean Functions
	UFUNCTION(meta = (BlueprintThreadSafe))
	FVector GetRelativeAccelerationAmount() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Additive Lean Functions")
	float GetLeanAmount() const;

	// Motion Matching Functions
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Motion Matching Functions")
	void UpdateMotionMatching(const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable ,meta = (BlueprintThreadSafe), Category = "EannaMMAS|Motion Matching Functions")
	void UpdateMotionMatchingPostSelection(const FAnimNodeReference& Node);
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Motion Matching Functions")
	float GetMMBlendTime() const;
	UFUNCTION( BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Motion Matching Functions")
	EPoseSearchInterruptMode GetMMInterruptMode() const;

	UFUNCTION(meta = (BlueprintThreadSafe))
	bool ShouldInterruptCustomIdle() const;
	UFUNCTION(meta = (BlueprintThreadSafe))
	bool ShouldTryToInterruptCustomIdle(const bool bInterrupt) const;
	UFUNCTION(meta = (BlueprintThreadSafe))
	bool IsCustomIdleOrTransitioningOutOfCustomIdle() const;
	void GenerateTrajectory();

	// Steer Functions
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Steer Functions")
	bool ShouldSteer() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Steer Functions")
	FQuat GetDesiredFacingTrajectory() const;

	// Aim Functions
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "EannaMMAS|Aim Functions")
	bool IsPivoting() const;
	UFUNCTION(meta = (BlueprintThreadSafe))
	bool CanOverlayAim() const;
};
