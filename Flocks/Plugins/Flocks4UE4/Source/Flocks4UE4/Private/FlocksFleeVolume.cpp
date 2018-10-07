// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksFleeVolume.h"
#include "FlocksManager.h"
#include "EngineUtils.h"

void AFlocksFleeVolume::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AFlocksManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		m_flocksManager = *ActorItr; 
		m_flocksManager->RegisterFleeVolume(this);
	}
}

float AFlocksFleeVolume::GetInfluence(FVector _position) const
{
	float distanceFromEpicenter = FVector::Distance(_position, GetActorLocation());

	float const validatedInnerRadius = FMath::Max(0.f, InnerRadius);
	float const validatedOuterRadius = FMath::Max(OuterRadius, validatedInnerRadius);
	float const validatedDist = FMath::Max(0.f, distanceFromEpicenter);

	if (validatedDist >= validatedOuterRadius)
	{
		// outside the radius, no effect
		return 0.0f;
	}

	if ((Falloff == 0.f) || (validatedDist <= validatedInnerRadius))
	{
		// no falloff or inside inner radius means full effect
		return 1.0f;
	}

	// calculate the interpolated scale
	float influence = 1.f - ((validatedDist - validatedInnerRadius) / (validatedOuterRadius - validatedInnerRadius));
	influence = FMath::Pow(influence, Falloff);

	return influence;
}