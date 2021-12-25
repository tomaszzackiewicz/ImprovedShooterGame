// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeWeaponData.generated.h"

USTRUCT()
struct FGrenadeWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Grenade)
	TSubclassOf<class AShooterGrenade> GrenadeClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Grenade)
	float GrenadeLife;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = GrenadeStat)
	int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = GrenadeStat)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = GrenadeStat)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FGrenadeWeaponData()
	{
		GrenadeClass = NULL;
		GrenadeLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}

};
