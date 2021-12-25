// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "ProjectileType.generated.h"

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	PT_Grenade = 0			UMETA(DisplayName = "Grenade"),
	PT_Projectile = 1		UMETA(DisplayName = "Projectile"),
	
};

