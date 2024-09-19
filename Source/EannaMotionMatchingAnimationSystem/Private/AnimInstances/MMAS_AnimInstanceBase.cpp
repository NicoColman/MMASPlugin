// Eanna Entertainment


#include "AnimInstances/MMAS_AnimInstanceBase.h"

#include "GameFramework/Character.h"

void FMMAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	OwningActor = InAnimInstance->GetOwningActor();
	if (!OwningActor) return;
	
	OwningCharacter = Cast<ACharacter>(OwningActor);
	if (!OwningCharacter) return;

	OwningCharacterMovementComponent = OwningCharacter->GetCharacterMovement();
	OwningAnimInstance = InAnimInstance;
}

void FMMAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FMMAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void FMMAnimInstanceProxy::ClearObjects()
{
	FAnimInstanceProxy::ClearObjects();
	OwningCharacter = nullptr;
	OwningCharacterMovementComponent = nullptr;
	OwningAnimInstance = nullptr;
}
