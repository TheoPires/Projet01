// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"

#include "Engine/StaticMeshActor.h"

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

	float NbHorizontalCells = PlaneWidth / CellSize;
	float NbVerticalCells = PlaneHeight / CellSize;
	UE_LOG(LogTemp, Warning, TEXT("NbHorizontalCells | NbVerticalCells: %.2f x %.2f"), NbHorizontalCells, NbVerticalCells);
	
	Grid.SetNum(NbVerticalCells);
	for (int i = 0; i < NbVerticalCells; i++)
	{
		Grid[i].SetNum(NbHorizontalCells);
	}

	ShowGrid();
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

