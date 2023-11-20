// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates=true;//?

	WeaponMesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	//设置武器的碰撞属性  ??
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//设置区域？
	AreaSphere=CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//设置提示UI
	PickupWidget=CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	//如果是在服务端上
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Overlap);
	
		//订阅函数
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this,&AWeapon::OnSphereEndOverlap);
	}
	
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlapperComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter=Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		BlasterCharacter->SetOverLappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlapperComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter=Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		BlasterCharacter->SetOverLappingWeapon(nullptr);
	}
}



void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState=State;
	switch ((WeaponState))
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon,WeaponState);
}

void AWeapon::OnRep_WeaponState()
{
	switch ((WeaponState))
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		break;
	}
}