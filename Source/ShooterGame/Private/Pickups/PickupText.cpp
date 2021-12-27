// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Pickups/PickupText.h"
#include "Components/MaterialBillboardComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
APickupText::APickupText()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MaterialBillboardComponent = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("MaterialBillboardComponent"));
	MaterialBillboardComponent->SetRelativeLocation(FVector(0));
	MaterialBillboardComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickupText::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickupText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

