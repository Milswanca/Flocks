// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FlocksAIController.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS4UE4_API AFlocksAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		TSubclassOf<APawn> PawnClass;

	void SetID(int32 _id, class AFlocksManager* _manager);

	UFUNCTION(BlueprintPure, Category = "Flocks")
		int32 GetID() const;

private:
	UPROPERTY()
		int32 m_id;

	UPROPERTY()
		class AFlocksManager* m_flocksManager;
};
