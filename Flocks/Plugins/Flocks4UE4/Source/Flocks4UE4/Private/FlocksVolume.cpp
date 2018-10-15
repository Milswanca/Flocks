// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksVolume.h"
#include "FlocksManager.h"
#include "EngineUtils.h"

// Sets default values
AFlocksVolume::AFlocksVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFlocksVolume::BeginPlay()
{
	Super::BeginPlay();
	
	m_lastPosition = GetActorLocation();

	for (TActorIterator<AFlocksManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		m_flocksManager = *ActorItr;
		m_id = m_flocksManager->RegisterVolume(this);
	}
}

// Called every frame
void AFlocksVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector newLoc = GetActorLocation();

	if (m_lastPosition != newLoc)
	{
		m_flocksManager->FlockVolumeMoved(m_id, newLoc);
	}

	m_lastPosition = newLoc;
}

float AFlocksVolume::GetVolumeInfluence(FVector _position) const
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

float AFlocksVolume::GetInfluence(FVector _position, FVector _volumePos, float _innerRadius, float _outerRadius, float _falloff)
{
	float distanceFromEpicenter = FVector::Distance(_position, _volumePos);

	float const validatedInnerRadius = FMath::Max(0.f, _innerRadius);
	float const validatedOuterRadius = FMath::Max(_outerRadius, validatedInnerRadius);
	float const validatedDist = FMath::Max(0.f, distanceFromEpicenter);

	if (validatedDist >= validatedOuterRadius)
	{
		// outside the radius, no effect
		return 0.0f;
	}

	if ((_falloff == 0.f) || (validatedDist <= validatedInnerRadius))
	{
		// no falloff or inside inner radius means full effect
		return 1.0f;
	}

	// calculate the interpolated scale
	float influence = 1.f - ((validatedDist - validatedInnerRadius) / (validatedOuterRadius - validatedInnerRadius));
	influence = FMath::Pow(influence, _falloff);

	return influence;
}