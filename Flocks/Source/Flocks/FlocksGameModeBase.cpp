// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksGameModeBase.h"

AFlocksGameModeBase::AFlocksGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFlocksGameModeBase::InitGameState()
{
	Super::InitGameState();

	m_flocksManager = GetWorld()->SpawnActor<AFlocksManager>(flocksManagerClass);
}

void AFlocksGameModeBase::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	m_flocksManager->UpdateBoids(_deltaTime);
}

AFlocksManager* AFlocksGameModeBase::GetFlocksManager() const
{
	return m_flocksManager;
}