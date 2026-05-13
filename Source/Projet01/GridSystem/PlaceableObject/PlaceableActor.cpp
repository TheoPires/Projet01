// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaceableActor.h"

// Sets default values
APlaceableActor::APlaceableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Width = 1;
	Height = 1;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshObject(TEXT("/Engine/BasicShapes/Cube"));
	if (StaticMeshObject.Succeeded())
	{
		MeshComponent->SetStaticMesh(StaticMeshObject.Object);
	}

	UpdateRectangle(Width, Height);
}

APlaceableActor::APlaceableActor(int32 Width, int32 Height)
{
	this->Width = Width;
	this->Height = Height;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshObject(TEXT("/Engine/BasicShapes/Cube"));
	if (StaticMeshObject.Succeeded())
	{
		MeshComponent->SetStaticMesh(StaticMeshObject.Object);
	}

	UpdateRectangle(Width, Height);
}

int32 APlaceableActor::GetWidth()
{
	return Width;
}

int32 APlaceableActor::GetHeight()
{
	return Height;
}

// Called when the game starts or when spawned
void APlaceableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlaceableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlaceableActor::UpdateRectangle(float NewWidth, float NewHeight, float GridCellSize)
{
	if (NewWidth > 0.0f && NewHeight > 0.0f && MeshComponent)
	{
		Width = NewWidth;
		Height = NewHeight;

		// Mesh size of unreal default cube
		constexpr float BaseMeshSize = 100.0f;

		// Taille cible en unités Unreal
		const float TargetWidth = Width * GridCellSize;
		const float TargetHeight = Height * GridCellSize;

		// Scale relatif
		const float ScaleX = TargetWidth / BaseMeshSize;
		const float ScaleY = TargetHeight / BaseMeshSize;

		MeshComponent->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
	}
}
