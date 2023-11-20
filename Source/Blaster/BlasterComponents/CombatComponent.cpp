
#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;//禁用tick
}
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
}

