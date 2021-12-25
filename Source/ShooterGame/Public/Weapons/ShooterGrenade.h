// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Structs/ProjectileWeaponData.h"
#include "GameFramework/Actor.h"
#include "ShooterGrenade.generated.h"

UCLASS()
class SHOOTERGAME_API AShooterGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShooterGrenade();

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	/*UFUNCTION()
	void OnExplode();*/

	void SetVelocity(float LaunchSpeed, FVector Direction);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	/** rendering */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComp;

protected:

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	struct FProjectileWeaponData WeaponConfig;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }

};
