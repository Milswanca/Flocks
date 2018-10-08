// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksAIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "FlocksManager.h"
#include "EngineUtils.h"

void AFlocksAIController::BeginPlay()
{
	Super::BeginPlay();

	SetDirection(UKismetMathLibrary::RandomUnitVector());
}

void AFlocksAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	for (TActorIterator<AFlocksManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		m_flocksManager = *ActorItr;
		m_id = m_flocksManager->RegisterBoid(this);

		break;
	}
}

void AFlocksAIController::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	if (GetPawn())
	{
		m_direction = FMath::VInterpTo(m_direction, m_desiredDirection, _deltaTime, rotationInterpSpeed);
		GetPawn()->AddMovementInput(m_direction);
	}
}

float AFlocksAIController::GetNeighbourDistanceSqr() const
{
	return neighbourDist * neighbourDist;
}

void AFlocksAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (m_flocksManager)
	{
		m_flocksManager->DeregisterBoid(this);
	}
}

void AFlocksAIController::SetDirection(FVector _direction)
{
	m_desiredDirection = _direction;
}

FVector AFlocksAIController::GetDirection() const
{
	return m_desiredDirection;
}

int32 AFlocksAIController::GetID() const
{
	return m_id;
}

void AFlocksAIController::MovedIntoRestriction(AFlocksVolume* _volume)
{
	m_restrictionVolumes.AddUnique(_volume);
}

void AFlocksAIController::MovedOutOfRestriction(AFlocksVolume* _volume)
{
	m_restrictionVolumes.Remove(_volume);
}

bool AFlocksAIController::IsInRestrictionVolume() const
{
	return m_restrictionVolumes.Num() > 0;
}

void AFlocksAIController::SetAcceleration(float _acceleration)
{
	maxAcceleration = _acceleration;

	if (m_flocksManager)
	{
		m_flocksManager->BoidChangedAcceleration(m_id, maxAcceleration);
	}
}

void AFlocksAIController::SetMaxVelocity(float _maxVelocity)
{
	maxVelocity = _maxVelocity;

	if (m_flocksManager)
	{
		m_flocksManager->BoidChangedMaxVelocity(m_id, _maxVelocity);
	}
}