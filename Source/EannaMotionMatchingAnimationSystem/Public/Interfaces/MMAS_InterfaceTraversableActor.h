// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MMAS_InterfaceTraversableActor.generated.h"

struct FTraversalCheckResult;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UMMAS_InterfaceTraversableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API IMMAS_InterfaceTraversableActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Eanna MMAS Interface")
	void GetLedgeTransforms(FTraversalCheckResult& TraversalCheckResult) const;
};
