// Eanna Entertainment

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Chooser.h"

#include "MMAS_Utilities.generated.h"

class UChooserTable;
class ACharacter;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	Grounded,
	InAir,
	Swimming
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	OrientToMovement,
	Strafe
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle,
	Moving
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class EStance : uint8
{
	Standing,
	Crouching,
	Prone
};

USTRUCT(BlueprintType)
struct EANNAMOTIONMATCHINGANIMATIONSYSTEM_API FCharacterJumpData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EannaMMAS|CharacterJumpData")
	bool bJustLanded = false;
	UPROPERTY(BlueprintReadWrite, Category = "EannaMMAS|CharacterJumpData")
	FVector LandVelocity = FVector::ZeroVector;
};

/**
 * Traversal
 */

UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	None,
	Hurdle,
	Vault,
	Mantle
};

USTRUCT(BlueprintType)
struct FTraversalCheckResult
{
	GENERATED_BODY()

public:
	FTraversalCheckResult() = default;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	ETraversalActionType TraversalActionType = ETraversalActionType::None;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	bool bHasFrontLedge = false;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector FrontLedgeLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	bool bHasBackLedge = false;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector BackLedgeLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector BackLedgeNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	bool bHasBackFloor = false;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector BackFloorLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float ObstacleHeight = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float ObstacleDepth = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float BackLedgeHeight = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	TObjectPtr<const ACharacter> Character = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector ActorLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector ActorForwardVector = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float CapsuleRadius = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float CapsuleHalfHeight = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	FVector HitLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	TObjectPtr<const UAnimMontage> MontageToPlay = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float StartTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	float PlayRate = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	bool bTraversalCheckFailed = false;
	UPROPERTY(BlueprintReadOnly, Category = "EannaMMAS|TraversalCheckResult")
	bool bMontageSelectionFailed = false;

	bool SetInitialValues(const ACharacter* const InCharacter);
	void CalculateTraversalActionType();
};

UCLASS()
class EANNAMOTIONMATCHINGANIMATIONSYSTEM_API UMMAS_Utilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the results of a chooser table that contains multiple objects of a specific type. Utility function to easily
	 *	get the results of a chooser table that contains multiple objects of a specific type.
	 * @param ChooserTable The chooser table to evaluate.
	 * @param ContextObject The context object to evaluate the chooser table with.
	 * @param OutResults The array to store the results in.
	 */
	template<typename T>
	static void GetUObjectChooserMultiResults(UChooserTable* ChooserTable, UObject* ContextObject, TArray<T*>& OutResults);
	template<typename T, typename TResult>
	static void GetStructChooserMultiResults(UChooserTable* ChooserTable, T& ContextStruct, TArray<TResult*>& OutResults);

private:
	template<typename T>
	static void GetChooserMultiResults(UChooserTable* ChooserTable, FChooserEvaluationContext& EvaluationContext, TArray<T*>& OutResults);
};

template <typename T>
void UMMAS_Utilities::GetUObjectChooserMultiResults(UChooserTable* ChooserTable, UObject* ContextObject, TArray<T*>& OutResults)
{
	if (!IsValid(ChooserTable) || !IsValid(ContextObject)) return;
	
	FChooserEvaluationContext EvaluationContext;
	EvaluationContext.AddObjectParam(ContextObject);

	GetChooserMultiResults(ChooserTable, EvaluationContext, OutResults);
}

template <typename T, typename TResult>
void UMMAS_Utilities::GetStructChooserMultiResults(UChooserTable* ChooserTable, T& ContextStruct,
	TArray<TResult*>& OutResults)
{
	FChooserEvaluationContext EvaluationContext;
	EvaluationContext.AddStructParam(ContextStruct);

	GetChooserMultiResults(ChooserTable, EvaluationContext, OutResults);
}

template <typename T>
void UMMAS_Utilities::GetChooserMultiResults(UChooserTable* ChooserTable,
		FChooserEvaluationContext& EvaluationContext, TArray<T*>& OutResults)
{
	FEvaluateChooser EvaluateChooser;
	EvaluateChooser.Chooser = ChooserTable;
	
	FObjectChooserBase::FObjectChooserIteratorCallback Callback;
	Callback.BindLambda([&OutResults](UObject* Object) -> FObjectChooserBase::EIteratorStatus
	{
		if (!Object) return FObjectChooserBase::EIteratorStatus::Stop;
		
		if (T* CastedObject = Cast<T>(Object))
		{
			OutResults.Add(CastedObject);
			return FObjectChooserBase::EIteratorStatus::Continue;
		}

		return FObjectChooserBase::EIteratorStatus::Stop;
	});

	EvaluateChooser.ChooseMulti(EvaluationContext, Callback);
}

USTRUCT(BlueprintType)
struct FTraversalChooserParams
{
	GENERATED_BODY()

public:
	void MakeTraversalChooserParams(const FTraversalCheckResult& TraversalCheckResult);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	ETraversalActionType TraversalActionType = ETraversalActionType::None;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	EGait Gait = EGait::Walk;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float Speed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth = 0.f;
};
