// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksRestrictionVolume.h"
#include "GameFramework/Pawn.h"
#include "FlocksAIController.h"
#include "FlocksManager.h"
#include "EngineUtils.h"

void AFlocksRestrictionVolume::BeginPlay()
{
	Super::BeginPlay();
}

void AFlocksRestrictionVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (APawn* pawn = Cast<APawn>(OtherActor))
	{
		if (pawn->GetController())
		{
			if (AFlocksAIController* controller = Cast<AFlocksAIController>(pawn->GetController()))
			{
				controller->MovedIntoRestriction(this);
			}
		}
	}
}

void AFlocksRestrictionVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (APawn* pawn = Cast<APawn>(OtherActor))
	{
		if (pawn->GetController())
		{
			if (AFlocksAIController* controller = Cast<AFlocksAIController>(pawn->GetController()))
			{
				controller->MovedOutOfRestriction(this);
			}
		}
	}
}