// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "Weapons/ShooterProjectile.h"
#include "Weapons/ShooterGrenade.h"
#include "Enums/ProjectileType.h"

AShooterWeapon_Projectile::AShooterWeapon_Projectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AShooterWeapon_Projectile::FireWeapon()
{
	FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();

	// trace from camera to check what's under crosshair
	const float ProjectileAdjustRange = 10000.0f;
	const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	if (ProjectileType == EProjectileType::PT_Grenade) {

		FVector OutLaunchVelocity = FVector::ZeroVector;
		FVector StartLocation = Origin;
		FVector HitLocation = Impact.ImpactPoint;
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("HitLocation %s"), *HitLocation.ToString()));
		float LaunchSpeed = 2000.0f;
		FCollisionResponseParams DummyParams;
		TArray<AActor*> DummyIgnores;

		//Calculate the OutLaunchVelocity
		bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity
		(
			this,
			OutLaunchVelocity,
			StartLocation,
			HitLocation,
			LaunchSpeed,
			false,
			0.f,
			10.f,
			ESuggestProjVelocityTraceOption::DoNotTrace
			//, DummyParams, DummyIgnores, true			// comment line to remove Debug DrawLine
		);

		if (bHaveAimSolution) {
			ServerFireProjectile(Origin, ShootDir, LaunchSpeed);
		}

	}else if (ProjectileType == EProjectileType::PT_Projectile) {
		// and adjust directions to hit that actor
		if (Impact.bBlockingHit)
		{
			const FVector AdjustedDir = (Impact.ImpactPoint - Origin).GetSafeNormal();
			bool bWeaponPenetration = false;

			const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
			if (DirectionDot < 0.0f)
			{
				// shooting backwards = weapon is penetrating
				bWeaponPenetration = true;
			}
			else if (DirectionDot < 0.5f)
			{
				// check for weapon penetration if angle difference is big enough
				// raycast along weapon mesh to check if there's blocking hit

				FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 350.0f;
				FVector MuzzleEndTrace = Origin;
				FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

				if (MuzzleImpact.bBlockingHit)
				{
					bWeaponPenetration = true;
				}
			}

			if (bWeaponPenetration)
			{
				// spawn at crosshair position
				Origin = Impact.ImpactPoint - ShootDir * 10.0f;
			}
			else
			{
				// adjust direction to hit
				ShootDir = AdjustedDir;
			}
		}

		ServerFireProjectile(Origin, ShootDir);
	}
	
	
}

bool AShooterWeapon_Projectile::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float LaunchSpeed = 0.0f)
{
	return true;
}

void AShooterWeapon_Projectile::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float LaunchSpeed = 0.0f)
{
	
	if (ProjectileType == EProjectileType::PT_Grenade) {
		FTransform SpawnTM(FRotator::ZeroRotator, Origin);
		AShooterGrenade* Grenade = Cast<AShooterGrenade>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, GrenadeConfig.GrenadeClass, SpawnTM));
		if (Grenade)
		{
			Grenade->SetInstigator(GetInstigator());
			Grenade->SetOwner(this);
			//Grenade->InitVelocity(ShootDir);
			Grenade->SetVelocity(LaunchSpeed, ShootDir);
			UGameplayStatics::FinishSpawningActor(Grenade, SpawnTM);
		}
	}
	else if(ProjectileType == EProjectileType::PT_Projectile) {
		FTransform SpawnTM(ShootDir.Rotation(), Origin);
		AShooterProjectile* Projectile = Cast<AShooterProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetInstigator(GetInstigator());
			Projectile->SetOwner(this);
			Projectile->InitVelocity(ShootDir);

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
		}
	}
}

void AShooterWeapon_Projectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}
