#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/ArrowComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "HandController.h"
#include "Curves/CurveFloat.h"
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
	UPROPERTY()
	UCameraComponent*  m_camera;

	UPROPERTY(EditAnywhere)
	AHandController* m_leftMotionController;

	UPROPERTY(EditAnywhere)
	AHandController* m_rightMotionController;

	UPROPERTY()
	USceneComponent* m_VRRoot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* m_destinationMarker;

	UPROPERTY(EditAnywhere)
	float m_teleportProjectileRadius = 10.0f;

	UPROPERTY(EditAnywhere)
	float m_teleportProjectileSpeed = 800.0f;

	UPROPERTY(EditAnywhere)
	float m_teleportSimulationTime = 1.0f;

	UPROPERTY(EditAnywhere)
	USplineComponent* m_teleportationPath;
	
	bool m_canTeleport = false;

	FVector m_teleportLocation;
	
	UPROPERTY(EditAnywhere)
	float m_fadeTime = 1.0f;

	UPROPERTY(EditAnywhere)
	FVector m_teleportProjectionExtent = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY()
	UPostProcessComponent* m_postProcessingComponent;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* m_blinkerMaterialBase;

	UPROPERTY()
	UMaterialInstanceDynamic* m_blinkerMaterialInstance;


	UPROPERTY(EditAnywhere)
	UCurveFloat* m_radiusVSvelocityCurve;



	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* m_teleportArchMesh;
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* m_teleportArchMaterial;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHandController> HandControllerClass;


	UPROPERTY()
	TArray<USplineMeshComponent*> m_pathStaticMeshes;

	UPROPERTY(EditAnywhere)
	UArrowComponent* m_teleportDirection;

	//Rotation Functions and variables
	float m_rotateDirX = 0.0f;
	float m_rotateDirY = 1.0f;
	FRotator m_teleportDirRotation;
	void RotateTeleportDirectionX(float xDir);
	void RotateTeleportDirectionY(float yDir);

	//Teleport Functions
	void UpdateDestinationMarker();
	void HideTeleportPath();
	void DrawTeleportPath(const TArray<FVector> & pathPoints);
	bool FindTeleportDestination(FVector &OutLocation);
	void BeginTeleport();
	void StartFade(float fromAlpha, float toAplpha);
	void EndTeleport();
	void ShowTeleport();


	//Moving with joystick functions
	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();
	void MoveForward(float throttle);
	void MoveRight(float throttle);


	//Climbing functions
	void GripLeft() { if(!m_teleportStarted) m_leftMotionController->Grip(); }
	void ReleaseLeft() { if (!m_teleportStarted)  m_leftMotionController->Release(); }

	void GripRight() { if (!m_teleportStarted) m_rightMotionController->Grip(); }
	void ReleaseRight() { if (!m_teleportStarted) m_rightMotionController->Release(); }
	bool IsClimbing() const { return m_rightMotionController->GetIsClimbing() || m_leftMotionController->GetIsClimbing(); }

	
	//state variabels
	bool m_teleportStarted = false;
	


};

