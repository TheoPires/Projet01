#include "WorkSpot.h"
#include "StudioGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"

AWorkSpot::AWorkSpot()
{
	PrimaryActorTick.bCanEverTick = false;
}

AStudioGameMode* AWorkSpot::GetStudioGameMode() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetAuthGameMode<AStudioGameMode>() : nullptr;
}

void AWorkSpot::GiveProblem(EMarkerType ProblemType)
{
	if (!CanGetProblem())
	{
		return;
	}

	if (ProblemType == EMarkerType::Question)
	{
		State = EWorkerState::Question;
	}
	else if (ProblemType == EMarkerType::Attention)
	{
		State = EWorkerState::Attention;
	}
	else
	{
		return; // gears are a reward, never a problem
	}

	ShowMarker(ProblemType);
}

void AWorkSpot::Solve()
{
	if (!HasProblem())
	{
		return;
	}

	const bool bWasQuestion = (State == EWorkerState::Question);
	State = EWorkerState::Idle;
	HideMarker();

	if (!bWasQuestion)
	{
		return;
	}

	++AnsweredQuestions;

	const AStudioGameMode* GM = GetStudioGameMode();
	const int32 Needed = GM ? GM->QuestionsForGears : 2;

	if (AnsweredQuestions >= Needed)
	{
		// Worker got their answers -> gears after a short delay
		State = EWorkerState::WaitingForGears;
		const float Delay = GM ? GM->GearsDelay : 15.f;
		GetWorldTimerManager().SetTimer(GearsTimer, this, &AWorkSpot::StartGears, FMath::Max(Delay, 0.01f), false);
	}
}

void AWorkSpot::StartGears()
{
	State = EWorkerState::Gears;
	ShowMarker(EMarkerType::Gears);

	const AStudioGameMode* GM = GetStudioGameMode();
	const float Duration = GM ? GM->GearsDuration : 20.f;
	GetWorldTimerManager().SetTimer(GearsTimer, this, &AWorkSpot::EndGears, FMath::Max(Duration, 0.01f), false);
}

void AWorkSpot::EndGears()
{
	// Back to normal, count starts over
	State = EWorkerState::Idle;
	AnsweredQuestions = 0;
	HideMarker();
}