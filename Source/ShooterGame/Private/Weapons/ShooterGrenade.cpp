// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/ShooterGrenade.h"
#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterExplosionEffect.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Bots/ShooterBot.h"

// Sets default values
AShooterGrenade::AShooterGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComp->SetupAttachment(RootComponent);

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = false;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->SetupAttachment(RootComponent);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	MovementComp->bAutoActivate = false;
	/*MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 1000.0f;
	MovementComp->MaxSpeed = 1000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;*/

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);

}

void AShooterGrenade::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//MovementComp->OnProjectileStop.AddDynamic(this, &AShooterGrenade::OnImpact);
	CollisionComp->MoveIgnoreActors.Add(GetInstigator());

	AShooterWeapon_Projectile* OwnerWeapon = Cast<AShooterWeapon_Projectile>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}

	SetLifeSpan(WeaponConfig.ProjectileLife);
	MyController = GetInstigatorController();

	CollisionComp->OnComponentHit.AddDynamic(this, &AShooterGrenade::OnHit);
}

void AShooterGrenade::InitVelocity(FVector& ShootDirection)
{
	if (MovementComp)
	{
		MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
	}
}

void AShooterGrenade::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{

	AShooterBot* ShooterBot = Cast<AShooterBot>(OtherActor);
	if (ShooterBot) {
		FVector ImpactPointLoc = Hit.ImpactPoint;// -(this->GetActorForwardVector() * 50.0f);
		FName BoneName = Hit.BoneName;
		this->AttachToComponent(ShooterBot->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, BoneName);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionProfileName(TEXT("NoCollision"));
	}
	else {
		CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
		CollisionComp->SetCollisionProfileName(TEXT("BlockAll"));
		CollisionComp->SetSimulatePhysics(true);
	}

	/*FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &AShooterGrenade::OnExplode, 3.0f, false);*/

	FTimerDelegate TimerDel;
	FTimerHandle TimerHandle;

	//Binding the function with specific values
	TimerDel.BindUFunction(this, FName("OnImpact"), Hit);
	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 3.0f, false);
}

void AShooterGrenade::OnImpact(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

//void AShooterGrenade::OnExplode(){
//
//	if (GetLocalRole() == ROLE_Authority && !bExploded){
//		Explode(HitResult);
//		DisableAndDestroy();
//	}
//}

void AShooterGrenade::SetVelocity(float LaunchSpeed, FVector Direction)
{
	//MovementComp->Velocity = VelocityParam;
	MovementComp->SetVelocityInLocalSpace(Direction* LaunchSpeed); //FVector::ForwardVector
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->Activate();
	//LaunchBlast->Activate();
}

void AShooterGrenade::Explode(const FHitResult& Impact)
{
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	// effects and damage origin shouldn't be placed inside mesh at impact point
	//const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
	const FVector NudgedImpactLocation = this->GetActorLocation() + Impact.ImpactNormal * 10.0f;

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, NudgedImpactLocation, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}

	if (ExplosionTemplate)
	{
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), NudgedImpactLocation);
		AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}

	this->SetActorHiddenInGame(true);
	this->SetActorEnableCollision(false);
	this->SetActorTickEnabled(false);

	bExploded = true;
}

void AShooterGrenade::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	// give clients some time to show explosion
	SetLifeSpan(2.0f);
}

///CODE_SNIPPET_START: AActor::GetActorLocation AActor::GetActorRotation
void AShooterGrenade::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}
///CODE_SNIPPET_END

void AShooterGrenade::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AShooterGrenade::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGrenade, bExploded);
}

