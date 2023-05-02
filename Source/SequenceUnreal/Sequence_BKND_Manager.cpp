// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequence_BKND_Manager.h"

// Sets default values
ASequence_BKND_Manager::ASequence_BKND_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASequence_BKND_Manager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASequence_BKND_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*
	Simple Testing Function for setting up bi directional communication
*/
int32 ASequence_BKND_Manager::Setup(int32 setup_prop)
{
	return setup_prop + 1;//this ensures that stuff is going back and forth properly
}
