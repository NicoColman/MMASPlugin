// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "MMAS_Utilities.h"
#include "UObject/Interface.h"
#include "MMAS_InterfaceCharacter.generated.h"

class UMotionWarpingComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UMMAS_InterfaceCharacter : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API IMMAS_InterfaceCharacter
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS Interface")
	EGait GetGait() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS Interface")
	FCharacterJumpData GetCharacterJumpData() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS Interface")
	UMotionWarpingComponent* GetCharacterMotionWarpingComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS Interface")
	void TryTraversalAction();
};
