#include "VRCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplay.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	m_destinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	m_postProcessingComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessingComponent"));

	

	m_teleportationPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportationPath"));
	m_teleportDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("directionalArrow"));



	m_VRRoot->SetupAttachment(GetRootComponent());
	m_camera->SetupAttachment(m_VRRoot);
	m_destinationMarker->SetupAttachment(m_VRRoot);
	m_postProcessingComponent->SetupAttachment(m_VRRoot);
	m_teleportationPath->SetupAttachment(m_VRRoot);
	m_teleportDirection->SetupAttachment(m_destinationMarker);

	m_VRRoot->RelativeLocation.Set(0.0f, 0.0f, -90.15f);
	
	m_teleportDirection->SetWorldLocation(m_teleportDirection->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight()* GetActorUpVector());
	m_teleportDirection->bHiddenInGame = false;
	m_teleportDirection->ArrowSize = 2;
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	if (HandControllerClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("hand controller class doesn't exists"));
	}
	if (m_blinkerMaterialBase != nullptr)
	{
		m_blinkerMaterialInstance = UMaterialInstanceDynamic::Create(m_blinkerMaterialBase, this);
		m_postProcessingComponent->AddOrUpdateBlendable(m_blinkerMaterialInstance);

	}
	
	m_leftMotionController  = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	if (m_leftMotionController != nullptr)
	{
		
		m_leftMotionController->AttachToComponent(m_VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		m_leftMotionController->setHand(EControllerHand::Left);
	}
	m_rightMotionController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	if (m_rightMotionController != nullptr)
	{
		m_rightMotionController->AttachToComponent(m_VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		m_rightMotionController->setHand(EControllerHand::Right);
	}
	if (m_leftMotionController != nullptr && m_rightMotionController != nullptr)
	{
		m_leftMotionController->setOtherController(m_rightMotionController);
	}
	
	m_destinationMarker->SetVisibility(false);
	m_teleportDirection->SetVisibility(false);

}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector cameraOffset = m_camera->GetComponentLocation() - GetActorLocation();
	cameraOffset = FVector::VectorPlaneProject(cameraOffset, m_camera->GetUpVector());
	AddActorWorldOffset(cameraOffset);
	m_VRRoot->AddWorldOffset(-cameraOffset);
	HideTeleportPath();
	if (m_teleportStarted)
	{
		UpdateDestinationMarker();
	}
	UpdateBlinkers();
	
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector destination;
	
	if (FindTeleportDestination(destination))
	{
		m_destinationMarker->SetVisibility(true);
		m_teleportDirection->SetVisibility(true);
		//UE_LOG(LogTemp, Warning, TEXT("rotate %f, rotateX %f, rotateY %f"), acosf(m_rotateDirX / sqrt(m_rotateDirX*m_rotateDirX + m_rotateDirY * m_rotateDirY)), m_rotateDirX, m_rotateDirY);
		float dirHypotenuse = sqrtf(m_rotateDirX*m_rotateDirX + m_rotateDirY * m_rotateDirY);
		if (abs(dirHypotenuse) > .000002f)
		{
			m_teleportDirection->AddLocalRotation(FRotator(0.0f, acosf(m_rotateDirX / dirHypotenuse), 0.0f));
		}
		m_destinationMarker->SetWorldLocation(destination);
	}
	else
	{
		m_destinationMarker->SetVisibility(false);
		m_teleportDirection->SetVisibility(false);
	}
}
void AVRCharacter::UpdateBlinkers()
{
	//update radius of blinker based on character velocity, calculated by unreal curve
	if (m_radiusVSvelocityCurve)
	{
		
		float speed = GetVelocity().Size();
		float radius = m_radiusVSvelocityCurve->GetFloatValue(speed);
		m_blinkerMaterialInstance->SetScalarParameterValue(TEXT("radius"), radius);
		FVector2D center = GetBlinkerCenter();
		m_blinkerMaterialInstance->SetVectorParameterValue(TEXT("center"), FLinearColor(center.X, center.Y, 0.0f));
	}
	
}

void AVRCharacter::HideTeleportPath()
{
	for (USplineMeshComponent* splineMesh : m_pathStaticMeshes)
	{
		splineMesh->SetVisibility(false);
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
			//if moving forwards add movement direction, if moving backwards subtract movement direction 
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
	m_canTeleport = false;
	
	FVector start = m_rightMotionController->GetActorLocation();
	FVector look = m_rightMotionController->GetActorForwardVector();

	FPredictProjectilePathParams params(
		m_teleportProjectileRadius, start, look* m_teleportProjectileSpeed,
		m_teleportSimulationTime, ECollisionChannel::ECC_Camera, this
		);
	params.bTraceComplex = true;
	FPredictProjectilePathResult result;
	m_canTeleport = UGameplayStatics::PredictProjectilePath(this, params, result);
	if (m_canTeleport) 
	{
		//use navMesh to constrain teleportation to only the floor area 
		FNavLocation navLocation;
		m_canTeleport = (GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(result.HitResult.Location, navLocation, m_teleportProjectionExtent));
		if (m_canTeleport)
		{
			TArray<FVector> splinePath;
			for (FPredictProjectilePathPointData point : result.PathData)
			{
				splinePath.Emplace(point.Location);
			}
			DrawTeleportPath(splinePath);
			OutLocation = navLocation.Location;
		}
	}
	return m_canTeleport;
}


void AVRCharacter::DrawTeleportPath(const TArray<FVector>& pathPoints)
{
	//set spline points from path points
	m_teleportationPath->SetSplinePoints(pathPoints, ESplineCoordinateSpace::World);
	
	//dynamically draw path by iterating over segments
	for ( int32 i = 0; i < pathPoints.Num() - 1; i++)
	{
		if (m_pathStaticMeshes.Num() <= i)
		{

			m_pathStaticMeshes.Emplace(NewObject<USplineMeshComponent>(this));
			m_pathStaticMeshes[i]->SetMobility(EComponentMobility::Movable);
			m_pathStaticMeshes[i]->AttachToComponent(m_teleportationPath, FAttachmentTransformRules::KeepRelativeTransform);
			m_pathStaticMeshes[i]->SetStaticMesh(m_teleportArchMesh);
			m_pathStaticMeshes[i]->SetMaterial(0, m_teleportArchMaterial);
			m_pathStaticMeshes[i]->RegisterComponent();
		}
		
		FVector startPos, startTan, endPos, endTan;
		m_teleportationPath->GetLocationAndTangentAtSplinePoint(i, startPos, startTan, ESplineCoordinateSpace::Local);
		m_teleportationPath->GetLocationAndTangentAtSplinePoint(i + 1, endPos, endTan, ESplineCoordinateSpace::Local);
		m_pathStaticMeshes[i]->SetStartAndEnd(startPos, startTan, endPos, endTan);
		m_pathStaticMeshes[i]->SetVisibility(true);
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//movement with thumbstick
	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);

	//teleportation
	PlayerInputComponent->BindAction(TEXT("Teleport"),IE_Released,this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Pressed, this, &AVRCharacter::ShowTeleport);
	PlayerInputComponent->BindAxis(TEXT("RotateTeleportDirX"),this, &AVRCharacter::RotateTeleportDirectionX);
	PlayerInputComponent->BindAxis(TEXT("RotateTeleportDirY"), this, &AVRCharacter::RotateTeleportDirectionY);

	//climbing
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRCharacter::ReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRCharacter::ReleaseRight);
}

void AVRCharacter::MoveForward(float throttle)
{
	if (!IsClimbing())
	{
		AddMovementInput(throttle* m_camera->GetForwardVector());
	}
}

void AVRCharacter::MoveRight(float throttle)
{
	if (!IsClimbing())
	{
		AddMovementInput(throttle* m_camera->GetRightVector());
	}
}


void AVRCharacter::RotateTeleportDirectionX(float dirX)
{
	if (m_teleportStarted)
	{
		m_rotateDirX = dirX;
	}
}

void AVRCharacter::RotateTeleportDirectionY(float dirY)
{
	if (m_teleportStarted)
	{
		m_rotateDirY = dirY;
	}
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
	if (m_canTeleport)
	{
		m_teleportLocation = m_destinationMarker->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight()* GetActorUpVector();
		APlayerController* pc = Cast<APlayerController>(GetController());
		StartFade(0.0f, 1.0f);
		m_teleportDirRotation = FRotator(m_teleportDirection->GetComponentTransform().GetRotation());
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::EndTeleport, m_fadeTime);
	}
}
void AVRCharacter::EndTeleport()
{
	if (m_canTeleport && m_teleportStarted)
	{
		m_VRRoot->SetWorldRotation(m_teleportDirRotation);
		SetActorLocation(m_teleportLocation);
		StartFade(1.0f, 0.0f);
		m_teleportStarted = false;
		m_destinationMarker->SetVisibility(false);
		m_teleportDirection->SetVisibility(false);
	}
}

void AVRCharacter::ShowTeleport()
{
	if (!IsClimbing())
	{
		m_teleportStarted = true;
		FRotator cameraRotator = m_camera->GetComponentRotation();
		m_teleportDirection->SetWorldRotation(FRotator(0.0f, cameraRotator.Yaw, 0.0f));
		m_destinationMarker->SetVisibility(true);
		m_teleportDirection->SetVisibility(true);
	}
}
