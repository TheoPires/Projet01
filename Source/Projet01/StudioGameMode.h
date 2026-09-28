#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WorkSpot.h"
#include "StudioGameMode.generated.h"

class UUserWidget;

// One off-screen guide icon for the UI
USTRUCT(BlueprintType)
struct FMarkerScreenIcon
{
	GENERATED_BODY()

	// Position on screen (ready to use in a widget Canvas Panel)
	UPROPERTY(BlueprintReadOnly, Category = "Icon")
	FVector2D Position = FVector2D::ZeroVector;

	// Direction to the marker (0 = right, 90 = down) - for rotating an arrow
	UPROPERTY(BlueprintReadOnly, Category = "Icon")
	float AngleDegrees = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Icon")
	EMarkerType Type = EMarkerType::Question;

	UPROPERTY(BlueprintReadOnly, Category = "Icon")
	int32 DistanceMeters = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectFinished, float, FinishTime, bool, bNewBestTime);

UCLASS()
class PROJET01_API AStudioGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStudioGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// ================= SETTINGS (editable in BP_StudioGameMode) =================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Progress")
	float BaseProjectSeconds = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Progress")
	float QuestionPenalty = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Progress")
	float GearsBonus = 0.2f;

	// Questions can slow things down, but never below this (only ! stops everything)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Progress")
	float MinSpeed = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Gears")
	int32 QuestionsForGears = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Gears")
	float GearsDelay = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Gears")
	float GearsDuration = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Fixing")
	float QuestionHoldTime = 2.5f;

	// 0 = instant tap
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Fixing")
	float AttentionHoldTime = 0.f;

	// How close the boss must be (250 = 2.5 meters)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Fixing")
	float InteractRadius = 250.f;

	// 0.75 = 75% questions, 25% attention
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Problems", meta = (ClampMin = "0", ClampMax = "1"))
	float QuestionChance = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Problems")
	float FirstProblemDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Problems")
	float MinTimeBetweenProblems = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Problems")
	float MaxTimeBetweenProblems = 7.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|Problems")
	int32 MaxProblemsAtOnce = 4;

	// Your Windows 95 widget (Part 4)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Studio|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// ================= LIVE VALUES (read these in the UI) =================

	// 0 to 1
	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	float Progress = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	float SpeedMultiplier = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	bool bFinished = false;

	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	bool bNewBestTime = false;

	// 0 = no best time yet
	UPROPERTY(BlueprintReadOnly, Category = "Studio|Live")
	float BestTime = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "Studio|Live")
	FOnProjectFinished OnProjectFinished;

	// ================= FUNCTIONS =================

	// Call from your E input: Started -> StartFix, Completed -> StopFix
	UFUNCTION(BlueprintCallable, Category = "Studio|Fixing")
	void StartFix();

	UFUNCTION(BlueprintCallable, Category = "Studio|Fixing")
	void StopFix();

	UFUNCTION(BlueprintPure, Category = "Studio|Fixing")
	bool IsFixing() const { return bFixing; }

	// 0 to 1 while holding E on a question
	UFUNCTION(BlueprintPure, Category = "Studio|Fixing")
	float GetFixProgress() const;

	// The problem the boss is standing next to (or None)
	UFUNCTION(BlueprintPure, Category = "Studio|Fixing")
	AWorkSpot* GetClosestProblem() const;

	UFUNCTION(BlueprintPure, Category = "Studio|UI")
	TArray<FMarkerScreenIcon> GetOffscreenIcons(float EdgeMargin = 40.f) const;

	// 83.5 seconds -> "01:23.50"
	UFUNCTION(BlueprintPure, Category = "Studio|UI")
	static FText FormatTime(float Seconds);

	UFUNCTION(BlueprintPure, Category = "Studio")
	int32 CountWorkers(EWorkerState InState) const;

	UFUNCTION(BlueprintCallable, Category = "Studio")
	void RestartRun();

private:
	UPROPERTY()
	TArray<TObjectPtr<AWorkSpot>> Workers;

	UPROPERTY()
	TObjectPtr<AWorkSpot> FixTarget;

	float FixElapsed = 0.f;
	float FixRequired = 0.f;
	bool bFixing = false;

	FTimerHandle ProblemTimer;

	float CalculateSpeed() const;
	void ScheduleNextProblem(float Delay);
	void SpawnProblem();
	bool IsInReach(const AWorkSpot* Spot) const;
	void FinishProject();
	void LoadBestTime();
	void SaveBestTime();
};