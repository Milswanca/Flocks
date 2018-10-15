// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksAIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "FlocksManager.h"
#include "EngineUtils.h"

void AFlocksAIController::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters spawnInfo;
		spawnInfo.Owner = this;
		spawnInfo.Instigator = Instigator;
		spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APawn* myPawn = GetWorld()->SpawnActor<APawn>(PawnClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);
		Possess(myPawn);
	}
}

void AFlocksAIController::SetID(int32 _id, class AFlocksManager* _manager)
{
	m_id = _id;
	m_flocksManager = _manager;
}

int32 AFlocksAIController::GetID() const
{
	return m_id;
}