// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"

#include "Components/InstancedStaticMeshComponent.h"
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

	NbHorizontalCells = FMath::FloorToInt(PlaneWidth / CellSize);
	NbVerticalCells = FMath::FloorToInt(PlaneHeight / CellSize);
	UE_LOG(LogTemp, Warning, TEXT("NbHorizontalCells | NbVerticalCells: %.2d x %.2d"), NbHorizontalCells, NbVerticalCells);
	
	Grid.SetNum(NbVerticalCells);
	for (int i = 0; i < NbVerticalCells; i++)
	{
		Grid[i].SetNum(NbHorizontalCells);
	}

	CreateGridMesh();
	// ShowGrid();

	for (int i = 0; i < 10; i++)
	{
		int32 PlaceX = FMath::RandRange(0,  NbHorizontalCells - 1);
		int32 PlaceY = FMath::RandRange(0,  NbVerticalCells - 1);
		UE_LOG(LogTemp, Warning, TEXT("PlaceX: %d | PlaceY: %d"), PlaceX, PlaceY);

		TObjectPtr<APlaceableActor> PlaceableActor = GetWorld()->SpawnActor<APlaceableActor>(FVector(0.f,0.f,0.f), FRotator::ZeroRotator);
		PlaceableActor->UpdateRectangle(FMath::RandRange(1,  4), FMath::RandRange(1,  4));
		if (CanPlace(PlaceX, PlaceY, PlaceableActor->Width, PlaceableActor->Height))
		{
			PlaceObject(PlaceX, PlaceY, PlaceableActor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CanPlace failed for PlaceX %d PlaceY %d"), PlaceX, PlaceY);
			PlaceableActor->Destroy();
		}
	}
	

}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridManager::CreateGridMesh()
{
    VerticalLinesComponent = NewObject<UInstancedStaticMeshComponent>(this);
    VerticalLinesComponent->RegisterComponent();
    VerticalLinesComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    HorizontalLinesComponent = NewObject<UInstancedStaticMeshComponent>(this);
    HorizontalLinesComponent->RegisterComponent();
    HorizontalLinesComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (GridStaticMesh)
    {
        VerticalLinesComponent->SetStaticMesh(GridStaticMesh);
        HorizontalLinesComponent->SetStaticMesh(GridStaticMesh);
        
        if (GridMaterial)
        {
            VerticalLinesComponent->SetMaterial(0, GridMaterial);
            HorizontalLinesComponent->SetMaterial(0, GridMaterial);
        }
    }
}

void AGridManager::ShowGrid() const
{
    if (!VerticalLinesComponent ||
    	!HorizontalLinesComponent)
    {
        return;
    }
    
    VerticalLinesComponent->ClearInstances();
    HorizontalLinesComponent->ClearInstances();
    
    FVector CornerPointReference = PlaneOrigin + PlaneExtent;

	FBox Box = GridStaticMesh->GetBoundingBox();
	
    // VERTICAL LINES (parallel to the Y-axis)
    int32 VerticalLineCount = FMath::RoundToInt(PlaneExtent.Y * 2.0f / CellSize);
    for (int i = 0; i <= VerticalLineCount; i++)
    {
        FVector LineStart = CornerPointReference - FVector(0.0f, CellSize, 0.0f) * i;
        FVector LineEnd = LineStart - FVector::ForwardVector * PlaneExtent.X * 2;
        FVector LineCenter = (LineStart + LineEnd) * 0.5f;
        
        FTransform LineTransform;
        LineTransform.SetLocation(LineCenter);
        LineTransform.SetScale3D(FVector(PlaneExtent.X / Box.GetExtent().X, LineThickness, LineThickness));
        
        VerticalLinesComponent->AddInstance(LineTransform, true);
    }
	
    // HORIZONTAL LINES (parallel to the X-axis)
    int32 HorizontalLineCount = FMath::RoundToInt(PlaneExtent.X * 2.0f / CellSize);
    for (int i = 0; i <= HorizontalLineCount; i++)
    {
        FVector HorizontalLineStart = CornerPointReference - FVector(CellSize, 0.0f, 0.0f) * i;
        FVector LineEnd = HorizontalLineStart - FVector::RightVector * PlaneExtent.Y * 2;
        FVector LineCenter = (HorizontalLineStart + LineEnd) * 0.5f;
        
        FTransform LineTransform;
        LineTransform.SetLocation(LineCenter);
        LineTransform.SetRotation(FRotator(.0f, 90.0f, .0f).Quaternion());
        LineTransform.SetScale3D(FVector(PlaneExtent.Y / Box.GetExtent().Y, LineThickness, LineThickness));
        
        HorizontalLinesComponent->AddInstance(LineTransform);
    }
}

void AGridManager::HideGrid()
{
    if (VerticalLinesComponent)
    {
        VerticalLinesComponent->ClearInstances();
    }
    if (HorizontalLinesComponent)
    {
        HorizontalLinesComponent->ClearInstances();
    }
}

FVector AGridManager::GridToWorld(int32 X, int32 Y)
{
	FVector BottomLeft = PlaneOrigin - PlaneExtent;
	
	return BottomLeft + FVector(
			(X + 0.5f) * CellSize,
			(Y + 0.5f) * CellSize,
			100.f);
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
			if (Y + j <= NbVerticalCells && X + i <= NbHorizontalCells)
			{
				Grid[Y + j][X + i].bOccupied = true;
				Grid[Y + j][X + i].OccupyingActor = Actor;
			}
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
}
