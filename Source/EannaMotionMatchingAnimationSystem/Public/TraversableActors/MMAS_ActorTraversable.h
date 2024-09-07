// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/MMAS_InterfaceTraversableActor.h"
#include "MMAS_ActorTraversable.generated.h"

class UStaticMeshComponent;
class USplineComponent;

UCLASS()
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API AMMAS_ActorTraversable : public AActor, public IMMAS_InterfaceTraversableActor
{
	GENERATED_BODY()

public:
	AMMAS_ActorTraversable();
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

	// IEannaMMASInterface
	virtual void GetLedgeTransforms_Implementation(FTraversalCheckResult& TraversalCheckResult) const override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Traversable")
	TArray<USplineComponent*> LedgeComponents;
	UPROPERTY(BlueprintReadWrite, Category = "Traversable")
	TMap<USplineComponent*, USplineComponent*> LedgeComponentPairs;
	
	/** Traversal Components */
	UPROPERTY(VisibleAnywhere, Category = "Traversable")
	TObjectPtr<UStaticMeshComponent> TraversableMesh;
	UPROPERTY(VisibleAnywhere, Category = "Traversable")
	TObjectPtr<USplineComponent> Ledge1Component;
	UPROPERTY(VisibleAnywhere, Category = "Traversable")
	TObjectPtr<USplineComponent> Ledge2Component;
	UPROPERTY(VisibleAnywhere, Category = "Traversable")
	TObjectPtr<USplineComponent> Ledge3Component;
	UPROPERTY(VisibleAnywhere, Category = "Traversable")
	TObjectPtr<USplineComponent> Ledge4Component;

	/** Traversal Variables */
	float MinLedgeWidth;
	
	/** Traversal Functions */
	USplineComponent* FindLedgeClosestToCharacter(const FVector& CharacterLocation) const;
	void TryFindFrontLedge(const USplineComponent* const InSpline, FTraversalCheckResult& TraversalCheckResult) const;
	void TryFindBackLedge(const USplineComponent* const InSpline, FTraversalCheckResult& TraversalCheckResult) const;
};
