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

	void ShowGrid();
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<AStaticMeshActor> PlaneMesh = nullptr;

	FVector GridToWorld(int32 X, int32 Y);

	bool CanPlace(int32 X, int32 Y, int32 ActorWidth, int32 ActorHeight);
	void PlaceObject(int32 X, int32 Y, const TObjectPtr<APlaceableActor>& Actor);

	float CellSize = 100.0f;
	
	FVector PlaneOrigin;

	FVector PlaneExtent;

	TArray<TArray<FGridCell>> Grid;
};
