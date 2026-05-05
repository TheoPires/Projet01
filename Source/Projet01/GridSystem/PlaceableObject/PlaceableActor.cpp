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

void APlaceableActor::UpdateRectangle(float NewWidth, float NewHeight)
{
	if (NewWidth > 0.0f && NewHeight > 0.0f && MeshComponent)
	{
		Width = NewWidth;
		Height = NewHeight;
 
		// Scale du cube : le cube par défaut fait 200x200x200 unités
		// On veut : Width en X, Height en Y, 1 cm en Z
		float ScaleX = Width;
		float ScaleY = Height;
		float ScaleZ = 1.0f;  // 1 unité = 100 cm, donc 1 cm = 0.01 unité
 
		MeshComponent->SetRelativeScale3D(FVector(ScaleX, ScaleY, ScaleZ));
	}
}
