

#include "StudioGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* SaveSection = TEXT("StudioBoss");
	const TCHAR* SaveKey = TEXT("BestTime");
}

AStudioGameMode::AStudioGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AStudioGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Find every worker in the level
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, AWorkSpot::StaticClass(), Found);
	for (AActor* Actor : Found)
	{
		if (AWorkSpot* Worker = Cast<AWorkSpot>(Actor))
		{
			Workers.Add(Worker);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("StudioGameMode: found %d workers"), Workers.Num());

	LoadBestTime();

	if (HUDWidgetClass)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (UUserWidget* HUD = CreateWidget<UUserWidget>(PC, HUDWidgetClass))
			{
				HUD->AddToViewport();
			}
		}
	}

	ScheduleNextProblem(FirstProblemDelay);
}

void AStudioGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFinished)
	{
		return;
	}

	ElapsedTime += DeltaSeconds;
	SpeedMultiplier = CalculateSpeed();
	Progress = FMath::Min(1.f, Progress + DeltaSeconds * SpeedMultiplier / BaseProjectSeconds);

	// Holding E on a question
	if (bFixing)
	{
		if (!IsValid(FixTarget) || !FixTarget->HasProblem() || !IsInReach(FixTarget))
		{
			StopFix(); // walked away or problem gone
		}
		else
		{
			FixElapsed += DeltaSeconds;
			if (FixElapsed >= FixRequired)
			{
				AWorkSpot* Target = FixTarget;
				StopFix();
				Target->Solve();
			}
		}
	}

	if (Progress >= 1.f)
	{
		FinishProject();
	}
}

int32 AStudioGameMode::CountWorkers(EWorkerState InState) const
{
	int32 Count = 0;
	for (const AWorkSpot* Worker : Workers)
	{
		if (IsValid(Worker) && Worker->GetWorkerState() == InState)
		{
			++Count;
		}
	}
	return Count;
}

float AStudioGameMode::CalculateSpeed() const
{
	// Any ! = everything stops
	if (CountWorkers(EWorkerState::Attention) > 0)
	{
		return 0.f;
	}

	const float Speed = 1.f
		+ CountWorkers(EWorkerState::Gears) * GearsBonus
		- CountWorkers(EWorkerState::Question) * QuestionPenalty;

	return FMath::Max(Speed, MinSpeed);
}

void AStudioGameMode::ScheduleNextProblem(float Delay)
{
	GetWorldTimerManager().SetTimer(ProblemTimer, this, &AStudioGameMode::SpawnProblem, FMath::Max(Delay, 0.01f), false);
}

void AStudioGameMode::SpawnProblem()
{
	if (bFinished)
	{
		return;
	}

	const int32 ActiveProblems = CountWorkers(EWorkerState::Question) + CountWorkers(EWorkerState::Attention);

	if (ActiveProblems < MaxProblemsAtOnce)
	{
		TArray<AWorkSpot*> Available;
		for (AWorkSpot* Worker : Workers)
		{
			if (IsValid(Worker) && Worker->CanGetProblem())
			{
				Available.Add(Worker);
			}
		}

		if (Available.Num() > 0)
		{
			AWorkSpot* Chosen = Available[FMath::RandRange(0, Available.Num() - 1)];
			Chosen->GiveProblem(FMath::FRand() < QuestionChance ? EMarkerType::Question : EMarkerType::Attention);
		}
	}

	ScheduleNextProblem(FMath::FRandRange(MinTimeBetweenProblems, MaxTimeBetweenProblems));
}

bool AStudioGameMode::IsInReach(const AWorkSpot* Spot) const
{
	const APawn* Boss = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Boss || !Spot)
	{
		return false;
	}

	const FVector A = Boss->GetActorLocation();
	const FVector B = Spot->GetActorLocation();
	return FVector::Dist2D(A, B) <= InteractRadius && FMath::Abs(A.Z - B.Z) <= 400.f;
}

AWorkSpot* AStudioGameMode::GetClosestProblem() const
{
	const APawn* Boss = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Boss)
	{
		return nullptr;
	}

	AWorkSpot* Closest = nullptr;
	float ClosestDist = TNumericLimits<float>::Max();

	for (AWorkSpot* Worker : Workers)
	{
		if (!IsValid(Worker) || !Worker->HasProblem() || !IsInReach(Worker))
		{
			continue;
		}

		const float Dist = FVector::Dist2D(Boss->GetActorLocation(), Worker->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			Closest = Worker;
		}
	}
	return Closest;
}

void AStudioGameMode::StartFix()
{
	if (bFinished)
	{
		return;
	}

	AWorkSpot* Target = GetClosestProblem();
	if (!Target)
	{
		return;
	}

	const float Required = Target->GetWorkerState() == EWorkerState::Question ? QuestionHoldTime : AttentionHoldTime;

	if (Required <= 0.f)
	{
		Target->Solve(); // instant
		return;
	}

	FixTarget = Target;
	FixElapsed = 0.f;
	FixRequired = Required;
	bFixing = true;
}

void AStudioGameMode::StopFix()
{
	bFixing = false;
	FixTarget = nullptr;
	FixElapsed = 0.f;
}

float AStudioGameMode::GetFixProgress() const
{
	return (bFixing && FixRequired > 0.f) ? FMath::Clamp(FixElapsed / FixRequired, 0.f, 1.f) : 0.f;
}

void AStudioGameMode::FinishProject()
{
	bFinished = true;
	StopFix();
	GetWorldTimerManager().ClearTimer(ProblemTimer);

	bNewBestTime = BestTime <= 0.f || ElapsedTime < BestTime;
	if (bNewBestTime)
	{
		BestTime = ElapsedTime;
		SaveBestTime();
	}

	OnProjectFinished.Broadcast(ElapsedTime, bNewBestTime);
}

void AStudioGameMode::RestartRun()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

FText AStudioGameMode::FormatTime(float Seconds)
{
	const int32 Minutes = FMath::FloorToInt(Seconds / 60.f);
	const float Rest = Seconds - Minutes * 60.f;
	return FText::FromString(FString::Printf(TEXT("%02d:%05.2f"), Minutes, Rest));
}

TArray<FMarkerScreenIcon> AStudioGameMode::GetOffscreenIcons(float EdgeMargin) const
{
	TArray<FMarkerScreenIcon> Icons;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return Icons;
	}

	int32 SizeX = 0, SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0)
	{
		return Icons;
	}

	const float DPIScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(PC), 0.01f);
	const float Margin = EdgeMargin * DPIScale;
	const FVector2D Center(SizeX * 0.5f, SizeY * 0.5f);

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const APawn* Boss = PC->GetPawn();
	const FVector DistanceFrom = Boss ? Boss->GetActorLocation() : CamLoc;

	for (const AWorkSpot* Worker : Workers)
	{
		if (!IsValid(Worker) || !Worker->HasProblem())
		{
			continue;
		}

		const FVector WorldLoc = Worker->GetActorLocation();
		const FVector Local = CamRot.UnrotateVector(WorldLoc - CamLoc); // X forward, Y right, Z up

		FVector2D Dir(Local.Y, -Local.Z);

		if (Local.X > 0.f)
		{
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(WorldLoc, ScreenPos))
			{
				const bool bOnScreen = ScreenPos.X >= Margin && ScreenPos.X <= SizeX - Margin
					&& ScreenPos.Y >= Margin && ScreenPos.Y <= SizeY - Margin;
				if (bOnScreen)
				{
					continue; // visible - no guide icon needed
				}
				Dir = ScreenPos - Center;
			}
		}

		if (Dir.IsNearlyZero())
		{
			Dir = FVector2D(0.f, 1.f);
		}
		Dir.Normalize();

		// Push the icon to the edge of the screen in that direction
		const float HalfW = Center.X - Margin;
		const float HalfH = Center.Y - Margin;
		const float EdgeDist = FMath::Min(
			HalfW / FMath::Max(FMath::Abs(Dir.X), KINDA_SMALL_NUMBER),
			HalfH / FMath::Max(FMath::Abs(Dir.Y), KINDA_SMALL_NUMBER));

		FMarkerScreenIcon Icon;
		Icon.Position = (Center + Dir * EdgeDist) / DPIScale;
		Icon.AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		Icon.Type = Worker->GetWorkerState() == EWorkerState::Question ? EMarkerType::Question : EMarkerType::Attention;
		Icon.DistanceMeters = FMath::RoundToInt(FVector::Dist(DistanceFrom, WorldLoc) / 100.f);
		Icons.Add(Icon);
	}

	return Icons;
}

void AStudioGameMode::LoadBestTime()
{
	if (GConfig)
	{
		GConfig->GetFloat(SaveSection, SaveKey, BestTime, GGameUserSettingsIni);
	}
}

void AStudioGameMode::SaveBestTime()
{
	if (GConfig)
	{
		GConfig->SetFloat(SaveSection, SaveKey, BestTime, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}





}