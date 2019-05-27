// Fill out your copyright notice in the Description page of Project Settings.

#include "HandController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_motionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	SetRootComponent(m_motionController);
	m_motionController->SetShowDeviceModel(true);
}

void AHandController::setOtherController(AHandController * otherController)
{
	m_otherController = otherController;
	otherController->m_otherController = this;
}

void AHandController::Grip()
{
	if (m_canClimb)
	{
			if (!m_isClimbing)
			{
				m_climbingStartLocation = GetActorLocation();
				m_isClimbing = true;
				m_otherController->m_isClimbing = false;
				ACharacter* character = Cast<ACharacter>(GetAttachParentActor());
				if (character != nullptr)
				{
					character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
				}
			}
	}
}

void AHandController::Release()
{
	if (m_isClimbing)
	{
		m_isClimbing = false;
		ACharacter* character = Cast<ACharacter>(GetAttachParentActor());
		if (character != nullptr)
		{
			character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);
	TArray<USceneComponent*> motionControllerChildComponents;
	m_motionController->GetChildrenComponents(true, motionControllerChildComponents);
	for (auto child : motionControllerChildComponents)
	{
		UStaticMeshComponent * controllerStaticMesh = Cast<UStaticMeshComponent>(child);
		if (controllerStaticMesh != nullptr)
		{
			m_controllerMesh = controllerStaticMesh;
			
			UE_LOG(LogTemp, Warning, TEXT("Attaching Box collision to static mesh for controller"));
			m_controllerBoxCollision = NewObject<UBoxComponent>(m_controllerMesh);
			m_controllerBoxCollision->SetMobility(EComponentMobility::Movable);
			m_controllerBoxCollision->AttachToComponent(m_controllerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
			m_controllerBoxCollision->RegisterComponent();
			//scale value may need to be modified 
			m_controllerBoxCollision->SetWorldScale3D(FVector(.15f));
			m_controllerBoxCollision->UpdateBodySetup();
			//for debugging
			m_controllerBoxCollision->SetHiddenInGame(false);
			break;
		}
	}
}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_isClimbing)
	{
		FVector handControllerDelta = GetActorLocation() - m_climbingStartLocation;
		GetAttachParentActor()->AddActorWorldOffset(-handControllerDelta);
	}

}

void AHandController::ActorBeginOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	
	bool bNewCanClimb = CanClimb();
	if (!m_canClimb && bNewCanClimb)
	{
		APawn* pawn = Cast<APawn>(GetAttachParentActor());
		if (pawn != nullptr)
		{
			APlayerController* controller = Cast<APlayerController>(pawn->GetController());
			if (controller != nullptr)
			{
				controller->PlayHapticEffect(m_hapticEffect, m_motionController->GetTrackingSource());
			}
		}
	}
	m_canClimb = bNewCanClimb;
}

void AHandController::ActorEndOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	m_canClimb = CanClimb();
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> OverLappingActors;
	m_controllerBoxCollision->GetOverlappingActors(OverLappingActors);
	for (auto actor : OverLappingActors)
	{
		if (actor->ActorHasTag(FName("Climbable")))
		{
			return true;
		}
	}
	return false;
}

