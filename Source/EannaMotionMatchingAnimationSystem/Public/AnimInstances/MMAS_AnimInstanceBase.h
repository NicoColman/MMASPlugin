// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "MMAS_AnimInstanceBase.generated.h"

struct FGameplayTag;
class UCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FMMAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

public:
	FMMAnimInstanceProxy() : FAnimInstanceProxy()
	{
	}
	FMMAnimInstanceProxy(UAnimInstance* InAnimInstance) : FAnimInstanceProxy(InAnimInstance) {}

	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;
	virtual void ClearObjects() override;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwningActor = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwningCharacter = nullptr;
	UPROPERTY(Transient)
	UCharacterMovementComponent* OwningCharacterMovementComponent = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> OwningAnimInstance = nullptr;
};

/**
 * 
 */
UCLASS()
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API UMMAS_AnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Example")
	FMMAnimInstanceProxy MMAnimInstanceProxy;

private:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &MMAnimInstanceProxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}
	friend FMMAnimInstanceProxy;
};
