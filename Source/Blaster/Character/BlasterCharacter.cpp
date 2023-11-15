
#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

	
	//绑定输入键Value
	PlayerInputComponent->BindAxis("MoveForward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&ABlasterCharacter::LookUp);
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

