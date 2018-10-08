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

}

float AFlocksVolume::GetInfluence(FVector _position) const
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