// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/ShooterGrenade.h"
#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterExplosionEffect.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Bots/ShooterBot.h"
#include "Player/ShooterCharacter.h"
#include "Pickups/PickupText.h"

// Sets default values
AShooterGrenade::AShooterGrenade()
{

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	//CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetupAttachment(RootComponent);

	InteractionComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionComp"));
	InteractionComp->InitSphereRadius(300.0f);
	InteractionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionComp->SetupAttachment(RootComponent);

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
	bIsSticked = false;

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

	InteractionComp->OnComponentBeginOverlap.AddDynamic(this, &AShooterGrenade::OnBeginOverlap);
	InteractionComp->OnComponentEndOverlap.AddDynamic(this, &AShooterGrenade::OnEndOverlap);
}

void AShooterGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPickupText) {
		FVector ShooterGrenadeLoc = this->GetActorLocation();
		FVector LocWithOffset =  ShooterGrenadeLoc + FVector(0.0f, 0.0f, 100.0f);
		CurrentPickupText->SetActorLocation(LocWithOffset);
	}
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
		bIsSticked = true;
		UnsetPickupText(ShooterBot);
	}
	else {
		//CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
		CollisionComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		CollisionComp->SetSimulatePhysics(true);
	}

	FTimerDelegate TimerDel;

	//Binding the function with specific values
	TimerDel.BindUFunction(this, FName("OnExplode"), Hit);
	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().SetTimer(ExplodeTimerHandle, TimerDel, ExplosionTime, false);
}

void AShooterGrenade::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this)) {
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter && (!CurrentPickupText)) {
			SetPickupText(ShooterCharacter);
		}
	}
}

void AShooterGrenade::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this)) {
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter) {
			UnsetPickupText(ShooterCharacter);
		}
	}
}

void AShooterGrenade::OnExplode(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

void AShooterGrenade::SetVelocity(FVector Direction, FVector Velocity)
{
	MovementComp->SetVelocityInLocalSpace(Velocity); //FVector::ForwardVector //Direction * LaunchSpeed
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->Activate();
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

void AShooterGrenade::SetPickupText(class AShooterCharacter* ShooterCharacterParam)
{
	if (bIsSticked) {
		return;
	}

	if (CurrentPickupText) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentPickupText = GetWorld()->SpawnActor<APickupText>(PickupTextClass, this->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
	if (CurrentPickupText && ShooterCharacterParam) {
		ShooterCharacterParam->SetCurrentShooterGrenade(this);
	}
}

void AShooterGrenade::UnsetPickupText(class AShooterCharacter* ShooterCharacterParam)
{
	if (CurrentPickupText && ShooterCharacterParam) {
		ShooterCharacterParam->SetCurrentShooterGrenade(nullptr);
		CurrentPickupText->Destroy();
		CurrentPickupText = nullptr;
	}
}

void AShooterGrenade::GivePickupTo(class AShooterCharacter* Pawn)
{
	GetWorld()->GetTimerManager().ClearTimer(ExplodeTimerHandle);

	AShooterWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);
	if (!Weapon) {
		return;
	}

	AShooterWeapon_Projectile* ShooterWeapon_Projectile = Cast<AShooterWeapon_Projectile>(Weapon);
	if (ShooterWeapon_Projectile){
		ShooterWeapon_Projectile->AddToCurrentAmmoInClip();

		// Fire event for collected ammo
		if (Pawn){
			/*const UWorld* World = GetWorld();
			const IOnlineEventsPtr Events = Online::GetEventsInterface(World);
			const IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);*/

			//if (Events.IsValid() && Identity.IsValid())
			//{
			//	AShooterPlayerController* PC = Cast<AShooterPlayerController>(Pawn->Controller);
			//	if (PC)
			//	{
			//		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player);

			//		if (LocalPlayer)
			//		{
			//			const int32 UserIndex = LocalPlayer->GetControllerId();
			//			TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
			//			if (UniqueID.IsValid())
			//			{
			//				FVector Location = Pawn->GetActorLocation();

			//				FOnlineEventParms Params;

			//				Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
			//				Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			//				Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

			//				Params.Add(TEXT("ItemId"), FVariantData((int32)Weapon->GetAmmoType() + 1)); // @todo come up with a better way to determine item id, currently health is 0 and ammo counts from 1
			//				Params.Add(TEXT("AcquisitionMethodId"), FVariantData((int32)0)); // unused
			//				Params.Add(TEXT("LocationX"), FVariantData(Location.X));
			//				Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
			//				Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));
			//				Params.Add(TEXT("ItemQty"), FVariantData((int32)Qty));

			//				Events->TriggerEvent(*UniqueID, TEXT("CollectPowerup"), Params);
			//			}
			//		}
			//	}
			//}
		}
	}

	this->Destroy();
}