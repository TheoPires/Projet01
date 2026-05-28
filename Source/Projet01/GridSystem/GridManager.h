// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlaceableObject/PlaceableActor.h"
#include "GridManager.generated.h"

USTRUCT()
struct FGridCell
{
	GENERATED_BODY()

	bool bOccupied = false;
	UPROPERTY()
	TObjectPtr<AActor> OccupyingActor;
};

UCLASS()
class PROJET01_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void CreateGridMesh();

	UFUNCTION(BlueprintCallable)
	void ShowGrid() const;
	UFUNCTION(BlueprintCallable)
	void HideGrid();


	FVector GridToWorld(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	FIntPoint WorldToGrid(const FVector& WorldPosition) const;

	UFUNCTION(BlueprintCallable)
	bool CanPlace(int32 X, int32 Y, int32 ActorWidth, int32 ActorHeight);

	UFUNCTION(BlueprintCallable)
	void PlaceObject(int32 X, int32 Y, APlaceableActor* Actor);

	UPROPERTY(EditAnywhere, Category = "Setup")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Setup")
	float LineThickness = 0.01f;
	
	FVector PlaneOrigin;

	FVector PlaneExtent;

	TArray<TArray<FGridCell>> Grid;

	int NbHorizontalCells = 0;

	int NbVerticalCells = 0;
	
	UPROPERTY(EditAnywhere, Category = "Setup")
	TObjectPtr<AStaticMeshActor> PlaneMesh = nullptr;
	
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> VerticalLinesComponent;
    
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> HorizontalLinesComponent;

	UPROPERTY(EditAnywhere, Category = "Setup")
	TObjectPtr<UStaticMesh> GridStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Setup")
	TObjectPtr<UMaterial> GridMaterial = nullptr;
};
