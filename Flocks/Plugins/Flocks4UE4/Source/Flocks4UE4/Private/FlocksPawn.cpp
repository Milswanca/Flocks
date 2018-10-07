// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksPawn.h"


// Sets default values
AFlocksPawn::AFlocksPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFlocksPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFlocksPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFlocksPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

