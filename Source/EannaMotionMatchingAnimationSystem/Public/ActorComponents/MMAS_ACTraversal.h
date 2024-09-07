// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "MMAS_Utilities.h"
#include "Components/ActorComponent.h"
#include "MMAS_ACTraversal.generated.h"

class UMotionWarpingComponent;
class UChooserTable;
struct FTraversalCheckResult;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API UMMAS_ACTraversal : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMAS_ACTraversal();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "EANNAMMAS|Traversal")
	void PerformJumpOrTraversalAction() const;

	UFUNCTION(BlueprintCallable, Category = "EANNAMMAS|Traversal")
	void SetCharacterJumpData(FCharacterJumpData& InCharacterJumpData) const;
	
protected:
	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "EANNAMMAS|Traversal|Debug")
	bool bDrawDebugHitResults = false;
	UPROPERTY(EditAnywhere, Category = "EANNAMMAS|Traversal|Debug")
	bool bShowDataForTraversal = false;
	UPROPERTY(EditAnywhere, Category = "EANNAMMAS|Traversal|Debug")
	bool bShowChooserSelectedMontages = false;
	UPROPERTY(EditAnywhere, Category = "EANNAMMAS|Traversal|Debug")
	bool bShowMMSelectedMontageDetails = false;
#endif

	UPROPERTY(BlueprintReadOnly, Category = "EANNAMMAS|Traversal")
	TObjectPtr<ACharacter> OwningCharacter;
	UPROPERTY(EditAnywhere, Category = "EANNAMMAS|Traversal")
	TObjectPtr<UChooserTable> TraversalMontagesChooserTable;

	/**
	 * We use mutable to allow us to modify the variable in a const function, we just really need to cache the values
	 * for delegate, we think that it is clearer to use mutable than to make the functions non-const
	 * */
	UPROPERTY(BlueprintReadOnly, Category = "EANNAMMAS|Traversal")
	mutable bool bIsDoingTraversalAction;

	/** Only used for PlayMontage delegates */
	UPROPERTY()
	mutable TObjectPtr<UPrimitiveComponent> TraversalHitComponent;
	mutable ETraversalActionType TraversalActionType;
	
private:
	FTraversalCheckResult TryTraversalAction() const;
	static float GetTraversalForwardTraceDistance(const FTraversalCheckResult& TraversalCheckResult);
	bool IsATraversableObject(const float TraceForwardDistance,  FTraversalCheckResult& TraversalCheckResult) const;
	void PerformLedgesConfigurations(FTraversalCheckResult& TraversalCheckResult) const;
	bool IsRoomOnFrontLedge(FTraversalCheckResult& TraversalCheckResult, FVector& OutFrontLedgeRoom) const;
	void CheckForRoomOnBackLedge(FTraversalCheckResult& TraversalCheckResult, const FVector& InFrontLedgeRoom) const;
	void CheckForFloorBehindBackLedge(FTraversalCheckResult& TraversalCheckResult, const FVector& InBackLedgeRoom) const;

	static void SetAnimInstanceInteractionTransform(const FTraversalCheckResult& TraversalCheckResult);

	void SetMontageToPlay(FTraversalCheckResult& TraversalCheckResult) const;
	void PerformTraversalAction(const FTraversalCheckResult& TraversalCheckResult) const;
	void UpdateWarpTargets(const FTraversalCheckResult& TraversalCheckResult) const;
	static void UpdateBackLedgeWarpTarget(const FTraversalCheckResult& TraversalCheckResult, UMotionWarpingComponent* MotionWarpingComponent, float& OutAnimDistance);
	static void UpdateBackFloorWarpTarget(const FTraversalCheckResult& TraversalCheckResult, UMotionWarpingComponent* MotionWarpingComponent, float InAnimDistance);
	void SetDoingTraversalAction(const bool bDoingTraversalAction) const;

#if WITH_EDITOR
	void ShowDataForTraversal(const FTraversalCheckResult& TraversalCheckResult) const;
#endif
};
