
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
}

