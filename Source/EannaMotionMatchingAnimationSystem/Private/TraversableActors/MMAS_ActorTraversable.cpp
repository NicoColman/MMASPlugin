// Eanna Entertainment


#include "TraversableActors/MMAS_ActorTraversable.h"
#include "MMAS_Utilities.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"


AMMAS_ActorTraversable::AMMAS_ActorTraversable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	TraversableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TraversableMesh"));
	RootComponent = TraversableMesh.Get();
	TraversableMesh->SetIsReplicated(true);

	Ledge1Component = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge1Component"));
	Ledge1Component->SetupAttachment(TraversableMesh);
	Ledge2Component = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge2Component"));
	Ledge2Component->SetupAttachment(TraversableMesh);
	Ledge3Component = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge3Component"));
	Ledge3Component->SetupAttachment(TraversableMesh);
	Ledge4Component = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge4Component"));
	Ledge4Component->SetupAttachment(TraversableMesh);

	MinLedgeWidth = 60.f;
}

void AMMAS_ActorTraversable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	LedgeComponents.Add(Ledge1Component);
	LedgeComponents.Add(Ledge2Component);
	LedgeComponents.Add(Ledge3Component);
	LedgeComponents.Add(Ledge4Component);

	LedgeComponentPairs.Add(Ledge1Component, Ledge2Component);
	LedgeComponentPairs.Add(Ledge2Component, Ledge1Component);
	LedgeComponentPairs.Add(Ledge3Component, Ledge4Component);
	LedgeComponentPairs.Add(Ledge4Component, Ledge3Component);
	
}

void AMMAS_ActorTraversable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMMAS_ActorTraversable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMMAS_ActorTraversable::GetLedgeTransforms_Implementation(FTraversalCheckResult& TraversalCheckResult) const
{
	const USplineComponent* const ClosestLedge = FindLedgeClosestToCharacter(TraversalCheckResult.ActorLocation);

	TryFindFrontLedge(ClosestLedge, TraversalCheckResult);
	if (!TraversalCheckResult.bHasFrontLedge) return;

	TryFindBackLedge(ClosestLedge, TraversalCheckResult);
	if (!TraversalCheckResult.bHasBackLedge) return;
}

USplineComponent* AMMAS_ActorTraversable::FindLedgeClosestToCharacter(const FVector& CharacterLocation) const
{
	if (LedgeComponents.IsEmpty()) return nullptr;

	float ClosestDistance = MAX_FLT;
	USplineComponent* ClosestLedge = nullptr;
	for (USplineComponent* Ledge : LedgeComponents)
	{
		const FVector LedgeLocation = Ledge->FindLocationClosestToWorldLocation(CharacterLocation, ESplineCoordinateSpace::World);
		const float Distance = FVector::Distance(LedgeLocation, CharacterLocation);

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestLedge = Ledge;
		}
	}
	return ClosestLedge;
}

void AMMAS_ActorTraversable::TryFindFrontLedge(const USplineComponent* const InSpline,
                                                    FTraversalCheckResult& TraversalCheckResult) const
{
	if (!InSpline || InSpline->GetSplineLength() < MinLedgeWidth)
	{
		TraversalCheckResult.bHasFrontLedge = false;
		return;
	}

	const float DistanceAlongSpline = InSpline->GetDistanceAlongSplineAtLocation(
		InSpline->FindLocationClosestToWorldLocation(TraversalCheckResult.HitLocation, ESplineCoordinateSpace::Local),
		ESplineCoordinateSpace::Local);

	const float ClampedDistance = FMath::Clamp(
		DistanceAlongSpline, MinLedgeWidth / 2.f, InSpline->GetSplineLength() - MinLedgeWidth / 2.f);

	const FTransform TransformAlongSpline = InSpline->GetTransformAtDistanceAlongSpline(
		ClampedDistance, ESplineCoordinateSpace::World);

	TraversalCheckResult.bHasFrontLedge = true;
	TraversalCheckResult.FrontLedgeLocation = TransformAlongSpline.GetLocation();
	TraversalCheckResult.FrontLedgeNormal = TransformAlongSpline.GetRotation().GetUpVector();

}

void AMMAS_ActorTraversable::TryFindBackLedge(const USplineComponent* const InSpline,
	FTraversalCheckResult& TraversalCheckResult) const
{
	const USplineComponent* const* const PairedLedge = LedgeComponentPairs.Find(InSpline);
	if (!PairedLedge || !(*PairedLedge))
	{
		TraversalCheckResult.bHasBackLedge = false;
		return;
	}

	const FTransform ClosestTransform = (*PairedLedge)->FindTransformClosestToWorldLocation(TraversalCheckResult.FrontLedgeLocation, ESplineCoordinateSpace::World);
	TraversalCheckResult.bHasBackLedge = true;
	TraversalCheckResult.BackLedgeLocation = ClosestTransform.GetLocation();
	TraversalCheckResult.BackLedgeNormal = ClosestTransform.GetRotation().GetUpVector();
}


