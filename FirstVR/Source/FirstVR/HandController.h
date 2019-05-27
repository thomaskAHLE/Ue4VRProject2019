// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HandController.generated.h"

UCLASS()
class FIRSTVR_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandController();
	void setHand(EControllerHand hand) { m_motionController->SetTrackingSource(hand); }
	void setOtherController(AHandController * otherController);
	bool GetIsClimbing() const { return m_isClimbing; }
	void Grip();
	void Release();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//default sub object

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* m_motionController;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent * m_controllerBoxCollision;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* m_controllerMesh;

	//Callback function
	UFUNCTION()
	void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
	
	
	//Paramaters
	UPROPERTY(EditDefaultsOnly)
	UHapticFeedbackEffect_Base* m_hapticEffect;

	bool CanClimb() const;

	AHandController* m_otherController;

	//State 
	bool m_canClimb = false;
	bool m_isClimbing = false;
	FVector m_climbingStartLocation;

};
