#include "GridPlaceableComponent.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

UGridPlaceableComponent::UGridPlaceableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGridPlaceableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bFollowMouse)
	{
		return;
	}
	
	FVector WorldPos;
	if (!GetMouseGridPosition(WorldPos))
	{
		return;
	}

	FIntPoint Cell = GridManager->WorldToCell(WorldPos);
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, FString::Printf(TEXT("UGridPlaceableComponent::TickComponent - Cursor on X:%d Y:%d"), Cell.X, Cell.Y));

	FVector Snapped = GridManager->SnapToGrid(WorldPos);
	GetOwner()->SetActorLocation(Snapped);
	DrawDebugSphere(GetWorld(), Snapped, 5, 30, FColor::Red, false, .1, 2, 1); 

	if (GridManager->CanPlaceActor(GetOwner(), Cell))
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, FString::Printf(TEXT("UGridPlaceableComponent::TickComponent - Valid place actor")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, FString::Printf(TEXT("UGridPlaceableComponent::TickComponent - NOT Valid place actor")));
	}
}

bool UGridPlaceableComponent::GetMouseGridPosition(FVector& OutPos) const
{
	if (!bHasScreenPosition)
	{
		return false;
	}
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return false;
	
	FVector2D PixelPosition, ViewportPosition;
	USlateBlueprintLibrary::AbsoluteToViewport(
		GetWorld(),
		LastScreenPosition,
		PixelPosition,
		ViewportPosition
		);

	// UE_LOG(LogTemp, Warning, TEXT("LastScreen: %s | Pixel: %s | Viewport: %s"),
	// 	*LastScreenPosition.ToString(),
	// 	*PixelPosition.ToString(),
	// 	*ViewportPosition.ToString());
	
	FVector Origin, Direction;
	// "Deprojects" from the screen position provided by the widget
	// UE_LOG(LogTemp, Warning, TEXT("Deproject with : %s"), *LastScreenPosition.ToString());
	UGameplayStatics::DeprojectScreenToWorld(
		PC,
		PixelPosition,
		Origin,
		Direction
	);

	if (FMath::IsNearlyZero(Direction.Z))
	{
		return false;
	}

	float GridZ = GridManager->GetActorLocation().Z;
	float T = (GridZ - Origin.Z) / Direction.Z;
	if (T < 0.f) return false;
	
	OutPos = Origin + Direction * T;
	return true;
}

void UGridPlaceableComponent::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Does not find GridManager"));
		return;
	}
}

void UGridPlaceableComponent::StartFollowMouse()
{
	bFollowMouse = true;
	SetComponentTickEnabled(true);
	GetOwner()->SetActorEnableCollision(false);
}

void UGridPlaceableComponent::StopFollowMouse()
{
	bFollowMouse = false;
	SetComponentTickEnabled(false);
	GetOwner()->SetActorEnableCollision(true);
}

void UGridPlaceableComponent::SnapPreview()
{
	if (!GridManager) return;
	AActor* Owner = GetOwner();
	FVector Snapped = GridManager->SnapToGrid(Owner->GetActorLocation());
	Snapped.Z = Owner->GetActorLocation().Z;
	Owner->SetActorLocation(Snapped);
}

void UGridPlaceableComponent::UpdateScreenPosition(const FVector2D ScreenPosition)
{
	if (ScreenPosition.IsZero())
	{
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("UpdateAbsolutePosition reçoit : %s"), *ScreenPosition.ToString());
	
	LastScreenPosition = ScreenPosition;
	bHasScreenPosition = true;
}

bool UGridPlaceableComponent::CanPlaceAtCurrentPosition() const
{
	if (!GridManager) return false;
	FIntPoint Cell = GridManager->WorldToCell(GetOwner()->GetActorLocation());
	return GridManager->CanPlaceActor(GetOwner(), Cell);
}

bool UGridPlaceableComponent::Place()
{
	if (!GridManager) return false;

	AActor* Owner = GetOwner();
	FIntPoint Cell = GridManager->WorldToCell(Owner->GetActorLocation());

	if (bIsPlaced) Remove(); // Moves if already placed

	bool bSuccess = GridManager->TryPlaceActor(Owner, Cell);
	if (bSuccess)
	{
		CurrentCell = Cell;
		bIsPlaced = true;
	}
	return bSuccess;
}

void UGridPlaceableComponent::Remove()
{
	if (!GridManager || !bIsPlaced) return;
	GridManager->RemoveActor(GetOwner());
	bIsPlaced = false;
}