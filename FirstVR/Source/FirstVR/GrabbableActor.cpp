// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"


// Sets default values
AGrabbableActor::AGrabbableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ObjMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjMesh"));
	RootComponent = ObjMesh;
	GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostMesh"));
	GhostMesh->SetupAttachment(ObjMesh);

	Tags.Add(FName("Grabbable"));
	bReplicates = true;
	bReplicateMovement = true;
}

// Called when the game starts or when spawned
void AGrabbableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrabbableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

