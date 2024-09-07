// Eanna Entertainment


#include "MMAS_Utilities.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/MMAS_InterfaceCharacter.h"

bool FTraversalCheckResult::SetInitialValues(const ACharacter* const InCharacter)
{
	if (!InCharacter)
	{
		bTraversalCheckFailed = true;
		return false;
	}

	Character = InCharacter;
	ActorLocation = Character->GetActorLocation();
	ActorForwardVector = Character->GetActorForwardVector();
	CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	return true;
}

void FTraversalCheckResult::CalculateTraversalActionType()
{
	constexpr float MinObstacleHeight = 50.f;
	constexpr float MaxVaultHurdleHeight = 125.f;
	constexpr float MaxMantleHeight = 257.f;
	constexpr float MaxVaultHurdleDepth = 59.f;
	constexpr float MinHurdleBackLedgeHeight = 50.f;

	const bool bIsVaultHurdleHeight = FMath::IsWithinInclusive(ObstacleHeight, MinObstacleHeight, MaxVaultHurdleHeight);
	const bool bIsMantleHeight = FMath::IsWithinInclusive(ObstacleHeight, MinObstacleHeight, MaxMantleHeight);
	const bool bIsShallowObstacle = ObstacleDepth < MaxVaultHurdleDepth;

	if (bHasFrontLedge && bHasBackLedge && bIsVaultHurdleHeight && bIsShallowObstacle)
	{
		TraversalActionType = !bHasBackFloor ? ETraversalActionType::Vault :
			(BackLedgeHeight > MinHurdleBackLedgeHeight ? ETraversalActionType::Hurdle : ETraversalActionType::None);
	}
	else if (bHasFrontLedge && bIsMantleHeight && !bIsShallowObstacle)
	{
		TraversalActionType = ETraversalActionType::Mantle;
	}
	else
	{
		TraversalActionType = ETraversalActionType::None;
		bTraversalCheckFailed = true;
	}
}

void FTraversalChooserParams::MakeTraversalChooserParams(const FTraversalCheckResult& TraversalCheckResult)
{
	TraversalActionType = TraversalCheckResult.TraversalActionType;
	Gait = IMMAS_InterfaceCharacter::Execute_GetGait(TraversalCheckResult.Character);
	Speed = TraversalCheckResult.Character->GetVelocity().Size2D();
	ObstacleHeight = TraversalCheckResult.ObstacleHeight;
	ObstacleDepth = TraversalCheckResult.ObstacleDepth;
}
