// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrabbableActor.generated.h"

UCLASS()
class FIRSTVR_API AGrabbableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrabbableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FString GetName() const { return Name; }
	bool IsHeld() const { return HeldBy != nullptr; }

private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent * ObjMesh;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent * GhostMesh;


	UPROPERTY(EditAnywhere)
	FString Name;



	class AActor * HeldBy = nullptr;

	//Todo add connection points
	//Todo add highlight component

	
	//state variables
	bool bIsHighlighted = false;
	
	
};
