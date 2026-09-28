#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorkSpot.generated.h"

class AStudioGameMode;

// Which marker Blueprint should appear
UENUM(BlueprintType)
enum class EMarkerType : uint8
{
	Question  UMETA(DisplayName = "Question (?)"),
	Attention UMETA(DisplayName = "Attention (!)"),
	Gears     UMETA(DisplayName = "Gears")
};

// What the worker is doing right now
UENUM(BlueprintType)
enum class EWorkerState : uint8
{
	Idle            UMETA(DisplayName = "Idle"),
	Question        UMETA(DisplayName = "Has a Question"),
	Attention       UMETA(DisplayName = "Needs Attention"),
	WaitingForGears UMETA(DisplayName = "Waiting For Gears"),
	Gears           UMETA(DisplayName = "In Flow (Gears)")
};

UCLASS()
class PROJET01_API AWorkSpot : public AActor
{
	GENERATED_BODY()

public:
	AWorkSpot();

	UFUNCTION(BlueprintPure, Category = "Worker")
	EWorkerState GetWorkerState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Worker")
	bool HasProblem() const { return State == EWorkerState::Question || State == EWorkerState::Attention; }

	UFUNCTION(BlueprintPure, Category = "Worker")
	bool CanGetProblem() const { return State == EWorkerState::Idle; }

	UFUNCTION(BlueprintPure, Category = "Worker")
	int32 GetAnsweredQuestions() const { return AnsweredQuestions; }

	// Called by the game mode
	void GiveProblem(EMarkerType ProblemType);
	void Solve();

	// These two appear as events in BP_Test_Marker.
	// Connect them to your Activate Marker / Deactivate Marker functions.
	UFUNCTION(BlueprintImplementableEvent, Category = "Worker")
	void ShowMarker(EMarkerType Type);

	UFUNCTION(BlueprintImplementableEvent, Category = "Worker")
	void HideMarker();

private:
	EWorkerState State = EWorkerState::Idle;
	int32 AnsweredQuestions = 0;
	FTimerHandle GearsTimer;

	void StartGears();
	void EndGears();
	AStudioGameMode* GetStudioGameMode() const;
};