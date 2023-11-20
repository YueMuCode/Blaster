
#include "BlasterCharacter.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//对弹簧臂进行管理
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength=600.f;
	CameraBoom->bUsePawnControlRotation=true;

	//对相机进行管理
	FollowCamera=CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);//?
	FollowCamera->bUsePawnControlRotation=false;

	bUseControllerRotationYaw=false;
	GetCharacterMovement()->bOrientRotationToMovement=true;
	
	OverHeadWidget=CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat=CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);//？？？
}



void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//当碰撞到武器时，设置显示UI的条件（服务器还是客户端）
	DOREPLIFETIME_CONDITION(ABlasterCharacter,OverlappingWeapon,COND_OwnerOnly);//
}



void ABlasterCharacter::SetOverLappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	//如果是在服务器上
	OverlappingWeapon=Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//绑定键Action
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&ABlasterCharacter::EquipButtonPressed);
	
	//绑定输入键Value
	PlayerInputComponent->BindAxis("MoveForward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&ABlasterCharacter::LookUp);

	
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character=this;
	}
}


void ABlasterCharacter::MoveForward(float Value)
{
	if(Controller!=nullptr&&Value!=0.0f)
	{
		//获取前进的方向
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		//最终让角色动起来，改变速度的话要改变组件中的参数。
		AddMovementInput(Direction,Value);
	}
	
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(Controller!=nullptr&&Value!=0.0f)
	{
		//获取前进的方向
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		//最终让角色动起来，改变速度的话要改变组件中的参数。
		AddMovementInput(Direction,Value);
	}
	
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//正在从客户端调用这个函数
		{
			ServerEquipButtonPressed();
		}
	}
}

//RPC?what that mean?
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}