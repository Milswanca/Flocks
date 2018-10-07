// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksManager.h"
#include "GameFramework/GameModeBase.h"
#include "FlocksGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKS_API AFlocksGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AFlocksGameModeBase();
	virtual void InitGameState() override;
	virtual void Tick(float _deltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Flocks")
		class AFlocksManager* GetFlocksManager() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocks")
		TSubclassOf<AFlocksManager> flocksManagerClass;

private:
	UPROPERTY()
		class AFlocksManager* m_flocksManager;
};
