#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
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
	UMotionControllerComponent* m_leftMotionController;

	UPROPERTY(EditAnywhere)
	UMotionControllerComponent* m_rightMotionController;

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

	UPROPERTY()
	TArray<USplineMeshComponent*> m_pathStaticMeshes;

	void UpdateDestinationMarker();
	void UpdateBlinkers();
	void HideTeleportPath();
	void DrawTeleportPath(const TArray<FVector> & pathPoints);
	FVector2D GetBlinkerCenter();
	bool FindTeleportDestination(FVector &OutLocation);
	void StartFade(float fromAlpha, float toAplpha);
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void BeginTeleport();
	void EndTeleport();

};

