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
}

void FMMAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FMMAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}