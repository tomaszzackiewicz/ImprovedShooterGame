// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileWeaponData.generated.h"

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AShooterProjectile> ProjectileClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float ProjectileLife;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};
