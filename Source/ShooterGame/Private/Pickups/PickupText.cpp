// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Pickups/PickupText.h"

// Sets default values
APickupText::APickupText()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MaterialBillboardComponent = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("MaterialBillboardComponent"));
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

