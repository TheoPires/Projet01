#include "GridManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Intersection/ContainmentQueries3.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    VerticalLinesComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VerticalLines"));
    VerticalLinesComponent->SetupAttachment(RootComponent);
    VerticalLinesComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HorizontalLinesComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HorizontalLines"));
    HorizontalLinesComponent->SetupAttachment(RootComponent);
    HorizontalLinesComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    InitGrid();
}

void AGridManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    InitGrid();
    ShowGrid();
    // HideGrid();
}

void AGridManager::InitGrid()
{
    Grid.SetNum(GridWidth);
    for (int32 X = 0; X < GridWidth; X++)
    {
        Grid[X].SetNum(GridHeight);
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            Grid[X][Y] = FGridCell();
        }
    }
}

void AGridManager::CreateGridMesh() const
{
    if (!GridStaticMesh) return;

    VerticalLinesComponent->SetStaticMesh(GridStaticMesh);
    HorizontalLinesComponent->SetStaticMesh(GridStaticMesh);

    if (GridMaterial)
    {
        VerticalLinesComponent->SetMaterial(0, GridMaterial);
        HorizontalLinesComponent->SetMaterial(0, GridMaterial);
    }
}

void AGridManager::ShowGrid()
{
    if (!VerticalLinesComponent || !HorizontalLinesComponent || !GridStaticMesh) return;

    VerticalLinesComponent->ClearInstances();
    HorizontalLinesComponent->ClearInstances();

    CreateGridMesh();

    const FBox Box = GridStaticMesh->GetBoundingBox();
    const FVector GridOrigin = GetActorLocation();
    const float TotalWidth  = GridWidth  * CellSize; // X axis
    const float TotalHeight = GridHeight * CellSize; // Y axis

    // Vertical lines (along the X axis, spaced on Y)
    for (int32 i = 0; i <= GridHeight; i++)
    {
        FVector Start  = GridOrigin + FVector(0.f,        i * CellSize, 0.f);
        FVector End    = GridOrigin + FVector(TotalWidth,  i * CellSize, 0.f);
        FVector Center = (Start + End) * 0.5f;

        FTransform T;
        T.SetLocation(Center);
        // No rotation: the line is already aligned with the X axis
        T.SetScale3D(FVector(TotalWidth / (Box.GetExtent().X * 2.f), LineThickness, LineThickness));

        VerticalLinesComponent->AddInstance(T, true);
    }

    // Horizontal lines (along the Y axis, spaced on X)
    for (int32 i = 0; i <= GridWidth; i++)
    {
        FVector Start  = GridOrigin + FVector(i * CellSize, 0.f,         0.f);
        FVector End    = GridOrigin + FVector(i * CellSize, TotalHeight,  0.f);
        FVector Center = (Start + End) * 0.5f;

        FTransform T;
        T.SetLocation(Center);
        T.SetRotation(FRotator(0.f, 90.f, 0.f).Quaternion()); // Rotates around the Y axis
        T.SetScale3D(FVector(TotalHeight / (Box.GetExtent().X * 2.f), LineThickness, LineThickness));

        HorizontalLinesComponent->AddInstance(T, true);
    }
}

void AGridManager::HideGrid()
{
    if (VerticalLinesComponent)
        VerticalLinesComponent->ClearInstances();

    if (HorizontalLinesComponent)
        HorizontalLinesComponent->ClearInstances();
}

FIntPoint AGridManager::WorldToCell(const FVector& WorldPos) const
{
    const FVector LocalPos = WorldPos - GetActorLocation();
    const int32 X = FMath::FloorToInt(LocalPos.X / CellSize);
    const int32 Y = FMath::FloorToInt(LocalPos.Y / CellSize);
    const FIntPoint Cell = FIntPoint(X, Y);
    if (IsValidCell(Cell))
    {
        return FIntPoint(-1, -1);
    }
    return Cell;
}

FVector AGridManager::CellToWorld(const FIntPoint& Cell) const
{
    return GetActorLocation() + FVector(
        Cell.X * CellSize + CellSize * 0.5f,
        Cell.Y * CellSize + CellSize * 0.5f,
        0.f
    );
}

FVector AGridManager::SnapToGrid(const FVector& WorldPos) const
{
    FIntPoint Cell = WorldToCell(WorldPos);
    FVector Snapped = CellToWorld(Cell);
    Snapped.Z = WorldPos.Z; // Keep the original height
    return Snapped;
}

TArray<FIntPoint> AGridManager::GetOccupiedCells(AActor* Actor, const FIntPoint& OriginCell) const
{
    TArray<FIntPoint> Cells;
    if (!Actor) return Cells;

    FVector Origin, Extent;
    Actor->GetActorBounds(false, Origin, Extent);

    // Number of cells occupied in X and Y from the bounding box
    int32 SizeX = FMath::Max(1, FMath::RoundToInt((Extent.X * 2.f) / CellSize));
    int32 SizeY = FMath::Max(1, FMath::RoundToInt((Extent.Y * 2.f) / CellSize));

    for (int32 DX = 0; DX < SizeX; DX++)
    {
        for (int32 DY = 0; DY < SizeY; DY++)
        {
            Cells.Add(FIntPoint(OriginCell.X + DX, OriginCell.Y + DY));
        }
    }

    return Cells;
}

bool AGridManager::IsValidCell(const FIntPoint& Cell) const
{
    return Cell.X >= 0 && Cell.X < GridWidth
        && Cell.Y >= 0 && Cell.Y < GridHeight;
}

bool AGridManager::CanPlaceActor(AActor* Actor, const FIntPoint& OriginCell) const
{
    TArray<FIntPoint> Cells = GetOccupiedCells(Actor, OriginCell);
    for (const FIntPoint& Cell : Cells)
    {
        if (!IsValidCell(Cell) || Grid[Cell.X][Cell.Y].bOccupied)
            return false;
    }
    return true;
}

bool AGridManager::TryPlaceActor(AActor* Actor, const FIntPoint& OriginCell)
{
    if (!CanPlaceActor(Actor, OriginCell))
    {
        return false;
    }
    
    TArray<FIntPoint> Cells = GetOccupiedCells(Actor, OriginCell);
    for (const FIntPoint& Cell : Cells)
    {
        Grid[Cell.X][Cell.Y].bOccupied = true;
        Grid[Cell.X][Cell.Y].OccupyingActor = Actor;
    }

    // Snap on grid
    FVector SnappedPos = CellToWorld(OriginCell);
    SnappedPos.Z = Actor->GetActorLocation().Z;
    Actor->SetActorLocation(SnappedPos);

    return true;
}

void AGridManager::RemoveActor(AActor* Actor)
{
    for (int32 X = 0; X < GridWidth; X++)
    {
        for (int32 Y = 0; Y < GridHeight; Y++)
        {
            if (Grid[X][Y].OccupyingActor == Actor)
            {
                Grid[X][Y].bOccupied = false;
                Grid[X][Y].OccupyingActor = nullptr;
            }
        }
    }
}