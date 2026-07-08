#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bOccupied = false;

    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> OccupyingActor;
};

UCLASS()
class PROJET01_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

    // === Editor settings ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridWidth = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridHeight = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSize = 100.f; // (1 UE = 1 cm)

    // Converts a world position → cell coordinate
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FIntPoint WorldToCell(const FVector& WorldPos) const;

    // Converts a cell coordinate → snapped world center
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector CellToWorld(const FIntPoint& Cell) const;

    // Snaps a world position to the grid
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector SnapToGrid(const FVector& WorldPos) const;

    // Attempts to place an actor on the grid
    // Returns true if the placement is valid
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool TryPlaceActor(AActor* Actor, const FIntPoint& OriginCell);

    // Frees the cells occupied by an actor
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void RemoveActor(AActor* Actor);

    // Checks if the cells are free
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool CanPlaceActor(AActor* Actor, const FIntPoint& OriginCell) const;

    // Computes the cells occupied by an actor from its bounding box
    UFUNCTION(BlueprintCallable, Category = "Grid")
    TArray<FIntPoint> GetOccupiedCells(AActor* Actor, const FIntPoint& OriginCell) const;

    bool IsValidCell(const FIntPoint& Cell) const;
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Visual")
    void  ShowGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Visual")
    void HideGrid();
    
protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    TArray<TArray<FGridCell>> Grid;

    void InitGrid();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Visual", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMesh> GridStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Visual", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInterface> GridMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Visual", meta=(AllowPrivateAccess = "true"))
    float LineThickness = 0.1f;

    void CreateGridMesh() const;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> VerticalLinesComponent;

    UPROPERTY()
    TObjectPtr<UInstancedStaticMeshComponent> HorizontalLinesComponent;
};