
#include "BlasterCharacter.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
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

	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;

	//设置于相机的碰撞关系
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	

	TurningInPlace=ETurningInPlace::ETIP_NotTurning;

	//网络更新频率
	NetUpdateFrequency=66.f;
	MinNetUpdateFrequency=33.f;

	//设置转身的速度
	GetCharacterMovement()->RotationRate=FRotator(0.f,0.f,850.f);
}


void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp,Warning,TEXT("AO_Yaw:%f"),AO_Yaw);
	if(AO_Yaw>90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw<-90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace!=ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw=FMath::FInterpTo(InterpAO_Yaw,0.f,DeltaTime,4.f);
		AO_Yaw=InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace=ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		}
	}
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

	AimOffset( DeltaTime);
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//绑定键Action
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Pressed,this,&ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Released,this,&ABlasterCharacter::AimButtonRelesed);
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

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}else
	{
		Crouch();
	}
	
}

void ABlasterCharacter::AimButtonPressed()
{
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonRelesed()
{
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(Combat&&Combat->EquippedWeapon==nullptr)return;
	FVector Velocity=GetVelocity();
	Velocity.Z=0.f;
	float Speed=Velocity.Size();
	bool bIsAir=GetCharacterMovement()->IsFalling();

	if(Speed==0.f&&!bIsAir)
	{
		FRotator CurrentAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaAimRotation=UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw=DeltaAimRotation.Yaw;
		if(TurningInPlace==ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw=AO_Yaw;
		}
		bUseControllerRotationYaw=true;
		TurnInPlace(DeltaTime);
	}
	if(Speed>0.f||bIsAir)
	{
		StartingAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw=0.f;
		bUseControllerRotationYaw=true;
		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
	}

	//这里源码的处理方式与我们的代码有点错误，需要进行修正，不然网络多人传值不正确,原因在于，源码用了无符号的形式存储pitch
	AO_Pitch=GetBaseAimRotation().Pitch;
	if(AO_Pitch>90.f&&!IsLocallyControlled())
	{
		FVector2D InRange(270.f,360.f);
		FVector2D OutRange(-90,0.f);
		AO_Pitch=FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pitch);
	}
}

void ABlasterCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
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

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat&&Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat&&Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(Combat==nullptr)return nullptr;
	return Combat->EquippedWeapon;
}
