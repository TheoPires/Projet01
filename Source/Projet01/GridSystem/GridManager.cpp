// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"

#include "Engine/StaticMeshActor.h"
#include "PlaceableObject/PlaceableActor.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (PlaneMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Plane Mesh should not be NULL"));
		return;
	}

	PlaneMesh->GetActorBounds(false,PlaneOrigin, PlaneExtent);

	UE_LOG(LogTemp, Warning, TEXT("Plane location: %s"), *PlaneOrigin.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Plane extent: %s"), *PlaneExtent.ToString());

	float PlaneWidth = PlaneExtent.X * 2.0f;
	float PlaneHeight = PlaneExtent.Y * 2.0f;
	UE_LOG(LogTemp, Warning, TEXT("Plane dimensions: %.2f x %.2f"), PlaneWidth, PlaneHeight);

	int NbHorizontalCells = FMath::FloorToInt(PlaneWidth / CellSize);
	int NbVerticalCells = FMath::FloorToInt(PlaneHeight / CellSize);
	UE_LOG(LogTemp, Warning, TEXT("NbHorizontalCells | NbVerticalCells: %.2d x %.2d"), NbHorizontalCells, NbVerticalCells);
	
	Grid.SetNum(NbVerticalCells);
	for (int i = 0; i < NbVerticalCells; i++)
	{
		Grid[i].SetNum(NbHorizontalCells);
	}
	
	ShowGrid();

	for (int i = 0; i < 10; i++)
	{
		TObjectPtr<APlaceableActor> PlaceableActor = GetWorld()->SpawnActor<APlaceableActor>(FVector(0.f,0.f,0.f), FRotator::ZeroRotator);
		int32 PlaceX = FMath::Rand() % NbVerticalCells;
		int32 PlaceY = FMath::Rand() % NbHorizontalCells;;
		UE_LOG(LogTemp, Warning, TEXT("PlaceX: %d | PlaceY: %d"), PlaceX, PlaceY);
		
		if (CanPlace(PlaceX, PlaceY, 1, 1))
		{
			TObjectPtr<APlaceableActor> PlaceableActor2 = GetWorld()->SpawnActor<APlaceableActor>(FVector(0.f,0.f,0.f), FRotator::ZeroRotator);
			if (i == 2)
			{
				PlaceableActor2->UpdateRectangle(2, 1);
			}
			PlaceObject(PlaceX, PlaceY, PlaceableActor2);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CanPlace failed for PlaceX %d PlaceY %d"), PlaceX, PlaceY);
		}
	}
	

}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridManager::ShowGrid()
{
	FVector CornerPointReference = PlaneOrigin + PlaneExtent;

	for (int i = 0; i < PlaneExtent.Y * 2.0f / CellSize ; i++)
	{
		DrawDebugLine(GetWorld(), CornerPointReference - FVector(0.0f, CellSize, .0f) * i,-1 * FVector(0.0f, CellSize, .0f) * i + CornerPointReference - FVector::ForwardVector * PlaneExtent.X * 2, FColor::Red, true, 1.0f, 5, 2.0f);
	}

	for (int i = 0; i < PlaneExtent.X * 2.0f / CellSize ; i++)
	{
		FVector HorizontalLineStart = CornerPointReference - FVector(CellSize, .0f, .0f) * i;
		FVector LineEnd = HorizontalLineStart + -1 * FVector::RightVector * PlaneExtent.Y * 2;
		DrawDebugLine(GetWorld(), HorizontalLineStart, LineEnd , FColor::Red, true, 1.0f, 5, 2.0f);
	}
}

FVector AGridManager::GridToWorld(int32 X, int32 Y)
{
	FVector BottomLeft = PlaneOrigin - PlaneExtent;

	FVector result = BottomLeft + FVector(
			(X + 0.5f) * CellSize,
			(Y + 0.5f) * CellSize,
			100.f
		);
	
	DrawDebugSphere(GetWorld(), result, 10.0f, 12, FColor::Yellow, true, 1.0f, 5.0f, 2.0f);
	
	return result;
}

bool AGridManager::CanPlace(const int32 X, const int32 Y, const int32 ActorWidth, const int32 ActorHeight)
{
	for (int i = 0; i < ActorWidth; i++)
	{
		for (int j = 0; j < ActorHeight; j++)
		{
			int CheckX = X + i;
			int CheckY = Y + j;

			if (!Grid.IsValidIndex(CheckY) || !Grid[CheckY].IsValidIndex(CheckX))
				return false;

			if (Grid[CheckY][CheckX].bOccupied)
				return false;
		}
	}

	return true;
}

void AGridManager::PlaceObject(int32 X, int32 Y, const TObjectPtr<APlaceableActor>& Actor)
{
	int32 ActorWidth = Actor->Width;
	int32 ActorHeight = Actor->Height;
	
	for (int i = 0; i < ActorWidth; i++)
	{
		for (int j = 0; j < ActorHeight; j++)
		{
			Grid[Y + j][X + i].bOccupied = true;
			Grid[Y + j][X + i].OccupyingActor = Actor;
		}
	}

	FVector WorldPos = GridToWorld(X, Y);

	FVector Offset = FVector(
		(ActorWidth - 1) * CellSize * 0.5f,
		(ActorHeight - 1) * CellSize * 0.5f,
		0.f
	);

	FVector FinalPos = WorldPos + Offset;

	Actor->SetActorLocation(FinalPos);
	// Actor->SetActorLocation(WorldPos);
}
