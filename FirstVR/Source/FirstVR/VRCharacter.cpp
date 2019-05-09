#include "VRCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplay.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	m_destinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	m_VRRoot->SetupAttachment(GetRootComponent());
	m_camera->SetupAttachment(m_VRRoot);
	m_destinationMarker->SetupAttachment(m_VRRoot);

	
	m_VRRoot->RelativeLocation.Set(0.0f, 0.0f, -90.15f);

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector cameraOffset = m_camera->GetComponentLocation() - GetActorLocation();
	cameraOffset = FVector::VectorPlaneProject(cameraOffset, m_camera->GetUpVector());
	AddActorWorldOffset(cameraOffset);
	m_VRRoot->AddWorldOffset(-cameraOffset);
	UpdateDestinationMarker();
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector start = m_camera->GetComponentLocation();
	FVector end = start + m_camera->GetForwardVector() * m_maxTeleportDistance;
	FHitResult hitResult;
	bool isHit = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility);
	if (isHit)
	{
		m_destinationMarker->SetVisibility(true);
		m_destinationMarker->SetWorldLocation(hitResult.Location);
	}
	else {
		m_destinationMarker->SetVisibility(false);
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"),IE_Released,this, &AVRCharacter::BeginTeleport);
}

void AVRCharacter::MoveForward(float throttle)
{
	AddMovementInput(throttle* m_camera->GetForwardVector());
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(throttle* m_camera->GetRightVector());
}

void AVRCharacter::BeginTeleport() 
{
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc)
	{
		pc->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, m_fadeTime, FLinearColor::Black, true);
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::EndTeleport, m_fadeTime);
	}
}
void AVRCharacter::EndTeleport() 
{
	SetActorLocation(m_destinationMarker->GetComponentLocation());
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc)
	{
		pc->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, m_fadeTime, FLinearColor::Black, true);
	}
}