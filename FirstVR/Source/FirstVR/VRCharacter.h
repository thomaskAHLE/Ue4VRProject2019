#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VRCharacter.generated.h"


UCLASS()
class FIRSTVR_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere)
	UCameraComponent*  m_camera;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* m_VRRoot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* m_destinationMarker;

	UPROPERTY(EditAnywhere)
	float m_maxTeleportDistance = 1000.0f;

	UPROPERTY(EditAnywhere)
	float m_fadeTime = 1.5f;


	void UpdateDestinationMarker();
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void BeginTeleport();
	void EndTeleport();

};

