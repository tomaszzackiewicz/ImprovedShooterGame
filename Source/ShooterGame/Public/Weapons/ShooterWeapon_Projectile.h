// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterWeapon.h"
#include "Structs/ProjectileWeaponData.h"
#include "Structs/GrenadeWeaponData.h"
#include "Enums/ProjectileType.h"
#include "GameFramework/DamageType.h" // for UDamageType::StaticClass()
#include "ShooterWeapon_Projectile.generated.h"

// A weapon that fires a visible projectile
UCLASS(Abstract)
class AShooterWeapon_Projectile : public AShooterWeapon
{
	GENERATED_UCLASS_BODY()

	/** apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::ERocket;
	}

	UPROPERTY(EditDefaultsOnly, Category = Config)
	EProjectileType ProjectileType;

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FProjectileWeaponData ProjectileConfig;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FGrenadeWeaponData GrenadeConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir, FVector Velocity = FVector::ZeroVector);

private:

	UPROPERTY(EditDefaultsOnly, Category = "ShooterWeapon_Projectile", meta = (AllowPrivateAccess = "true"))
	float GrenadeLaunchSpeed;
};
