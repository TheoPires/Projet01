#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../GridManager.h"
#include "GridPlaceableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJET01_API UGridPlaceableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridPlaceableComponent();

	// Enables mouse tracking (called right after spawn)
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void StartFollowMouse();

	// Disables mouse tracking
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void StopFollowMouse();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsPlaced() const { return bIsPlaced; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Called to confirm placement at the Actor's current position
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool Place();

	// Removes the object from the grid
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void Remove();

	// Preview: checks whether the object can be placed at its current position
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool CanPlaceAtCurrentPosition() const;

	// Visually snaps the actor to the grid without placing it
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SnapPreview();

	// Called every frame from the widget with the cursor screen position
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void UpdateScreenPosition(FVector2D ScreenPosition);
	
protected:
	virtual void BeginPlay() override;

private:
	// Reference to the GridManager (to be assigned in the BP or via GetActorOfClass)
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;
	
	// Current origin cell of the object
	UPROPERTY(BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	FIntPoint CurrentCell;

	UPROPERTY(BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	bool bIsPlaced = false;

	bool GetMouseGridPosition(FVector& OutPos) const;
	
	bool bFollowMouse = false;

	FVector2D LastScreenPosition;
	
	bool bHasScreenPosition = false;
};