// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Structs/ProjectileWeaponData.h"
#include "GameFramework/Actor.h"
#include "ShooterGrenade.generated.h"

class APickupText;

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
	void OnExplode(const FHitResult& HitResult);

	/*UFUNCTION()
	void OnExplode();*/

	void SetVelocity(float LaunchSpeed, FVector Direction);

	virtual void Tick(float DeltaTime) override;

	void GivePickupTo(class AShooterCharacter* Pawn);

private:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = "ShooterGrenade")
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = "ShooterGrenade")
	USphereComponent* CollisionComp;

	/** rendering */
	UPROPERTY(VisibleDefaultsOnly, Category = "ShooterGrenade")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "ShooterGrenade")
	USphereComponent* InteractionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "ShooterGrenade")
	UParticleSystemComponent* ParticleComp;

protected:

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category = "ShooterGrenade")
	TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ShooterGrenade")
	TSubclassOf<class APickupText> PickupTextClass;

	UPROPERTY(EditDefaultsOnly, Category = "ShooterGrenade")
	TSubclassOf<AShooterWeapon> WeaponType;

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

	void SetPickupText(class AShooterCharacter* ShooterCharacterParam);

	void UnsetPickupText(class AShooterCharacter* ShooterCharacterParam);

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }

private:

	UPROPERTY(EditDefaultsOnly, Category = "ShooterGrenade", meta = (AllowPrivateAccess = "true"))
	float ExplosionTime = 5.0f;

	UPROPERTY()
	APickupText* CurrentPickupText;

	bool bIsSticked;

	FTimerHandle ExplodeTimerHandle;
};
