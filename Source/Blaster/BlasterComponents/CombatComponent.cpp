
#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;//

	BaseWalkSpeed=600.f;
	AimWalkSpeed=450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bAiming);
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);
	if(Character&&Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs((HitResult));
		HitTarget=HitResult.ImpactPoint;
	}
	
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming=bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=bIsAiming?AimWalkSpeed:BaseWalkSpeed;
	}
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon&&Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement=false;
		Character->bUseControllerRotationYaw=true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed=bPressed;
	if(bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
	
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	//获取视口的大小
	FVector2D ViewportSize;
	if(GEngine&&GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X/2.f,ViewportSize.Y/2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld=  UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if(bScreenToWorld)
	{
		FVector Start=CrosshairWorldPosition;
		FVector End=Start+CrosshairWorldDirection*TRACE_LENGTH;
	
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);

		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint=End;
			HitTarget=End;
		}
		else
		{
			HitTarget=TraceHitResult.ImpactPoint;
			//DrawDebugSphere(GetWorld(),TraceHitResult.ImpactPoint,12.f,12,FColor::Red);
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character==nullptr||Character->Controller==nullptr)return;
	
	Controller=Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		HUD=HUD==nullptr?Cast<ABlasterHUD>(Controller->GetHUD()):HUD;
		if(HUD)
		{
			FHUDPackage HUDPackage;
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter=EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft=EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight=EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop=EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom=EquippedWeapon->CrosshairsBottom;
				
			}
			else
			{
				HUDPackage.CrosshairsCenter=nullptr;
				HUDPackage.CrosshairsLeft=nullptr;
				HUDPackage.CrosshairsRight=nullptr;
				HUDPackage.CrosshairsTop=nullptr;
				HUDPackage.CrosshairsBottom=nullptr;
				
			}
			//计算纹理的抖动
			FVector2D walkSpeedRange(0.f,Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f,1.f);
			FVector Velocity=Character->GetVelocity();
			Velocity.Z=0.f;
			CrosshairVelocityFactor= FMath::GetMappedRangeValueUnclamped(walkSpeedRange,VelocityMultiplierRange,Velocity.Size());

			 if(Character->GetCharacterMovement()->IsFalling())
			 {
				 CrosshairInAirFactor=FMath::FInterpTo(CrosshairInAirFactor,2.25f,DeltaTime,2.25f);
			 }
			 else
			 {
				 CrosshairInAirFactor=FMath::FInterpTo(CrosshairInAirFactor,0.f,DeltaTime,30.f);
			 }
			
			HUDPackage.CrosshairSpread=CrosshairVelocityFactor+CrosshairInAirFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultcastFire(TraceHitTarget);
}

void UCombatComponent::MultcastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon==nullptr)return;
	if(Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}



void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming=bIsAiming;
	//要在服务器中设定速度，不然服务器会覆盖本地
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=bIsAiming?AimWalkSpeed:BaseWalkSpeed;
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character==nullptr||WeaponToEquip==nullptr)return;

	EquippedWeapon=WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket= Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}
	//装备后设置武器的所有权
	EquippedWeapon->SetOwner(Character);

	Character->GetCharacterMovement()->bOrientRotationToMovement=false;
	Character->bUseControllerRotationYaw=true;
}

