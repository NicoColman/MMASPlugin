// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MMAS_InterfaceAnimInstance.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UMMAS_InterfaceAnimInstance : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API IMMAS_InterfaceAnimInstance
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS AnimInstance Interface")
	void SetInteractionTransform(const FTransform& InInteractionTransform);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (BlueprintThreadSafe),Category = "Eanna MMAS AnimInstance Interface")
	FTransform GetInteractionTransform() const;
};
