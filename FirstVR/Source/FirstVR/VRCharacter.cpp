#include "VRCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplay.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "AI/Navigation/NavigationSystem.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	m_destinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	m_postProcessingComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessingComponent"));

	m_VRRoot->SetupAttachment(GetRootComponent());
	m_camera->SetupAttachment(m_VRRoot);
	m_destinationMarker->SetupAttachment(m_VRRoot);
	m_postProcessingComponent->SetupAttachment(m_VRRoot);
	
	m_VRRoot->RelativeLocation.Set(0.0f, 0.0f, -90.15f);

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	if (m_blinkerMaterialBase != nullptr)
	{
		m_blinkerMaterialInstance = UMaterialInstanceDynamic::Create(m_blinkerMaterialBase, this);
		m_postProcessingComponent->AddOrUpdateBlendable(m_blinkerMaterialInstance);

	}
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
	UpdateBlinkers();
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector destination;
	
	if (FindTeleportDestination(destination))
	{
		m_destinationMarker->SetVisibility(true);
		m_destinationMarker->SetWorldLocation(destination);
	}
	else
	{
		m_destinationMarker->SetVisibility(false);
	}
}
void AVRCharacter::UpdateBlinkers()
{
	if (m_radiusVSvelocityCurve)
	{
		float speed = GetVelocity().Size();
		float radius = m_radiusVSvelocityCurve->GetFloatValue(speed);
		m_blinkerMaterialInstance->SetScalarParameterValue(TEXT("radius"), radius);
		FVector2D center = GetBlinkerCenter();
		m_blinkerMaterialInstance->SetVectorParameterValue(TEXT("center"), FLinearColor(center.X, center.Y, 0.0f));
	}
	
}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	FVector movementDirection = GetVelocity().GetSafeNormal();
	if (!movementDirection.IsNearlyZero())
	{
		APlayerController* pc = Cast<APlayerController>(GetController());
		if (pc)
		{
			FVector worldStationaryLocation;
			if (FVector::DotProduct(m_camera->GetForwardVector(), movementDirection)> 0.0f)
			{
					worldStationaryLocation = m_camera->GetComponentLocation() + movementDirection * 1000;
			}
			else 
			{
				worldStationaryLocation = m_camera->GetComponentLocation() - movementDirection * 1000;
			}
		    FVector2D screenLocation;
			pc->ProjectWorldLocationToScreen(worldStationaryLocation, screenLocation);
			int32 sizeX;
			int32 sizeY;
			pc->GetViewportSize(sizeX, sizeY);
			//normalize into UV coordinates
			return FVector2D(screenLocation.X / sizeX, screenLocation.Y / sizeY);
				
		}
	}
	return FVector2D(0.5f, 0.5f);
}
bool AVRCharacter :: FindTeleportDestination(FVector & OutLocation)
{
	FVector start = m_camera->GetComponentLocation();
	FVector end = start + m_camera->GetForwardVector() * m_maxTeleportDistance;
	FHitResult hitResult;
	FNavLocation navLocation;
	bool canTeleport = (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility)) &&
		(GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(hitResult.Location, navLocation, m_teleportProjectionExtent));
	if (canTeleport)
	{
		OutLocation = navLocation.Location;
	}
	return canTeleport;
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

void AVRCharacter::StartFade(float fromAlpha, float toAlpha)
{
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc)
	{
		pc->PlayerCameraManager->StartCameraFade(fromAlpha, toAlpha, m_fadeTime, FLinearColor::Black, true);
	}
}

void AVRCharacter::BeginTeleport() 
{
	APlayerController* pc = Cast<APlayerController>(GetController());
	StartFade(0.0f, 1.0f);
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::EndTeleport, m_fadeTime);
	
}
void AVRCharacter::EndTeleport()
{

	SetActorLocation(m_destinationMarker->GetComponentLocation() + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	StartFade(1.0f, 0.0f);
}