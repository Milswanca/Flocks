// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlocksManagerThread.h"
#include "FlocksAIController.h"
#include "GameFramework/Actor.h"
#include "FlocksPawn.h"
#include "FlocksComputeShaderWrapper.h"
#include "FlocksManager.generated.h"

UENUM(BlueprintType)
enum class ComputeType : uint8
{
	CT_CPU,
	CT_MultiThreaded,
	CT_GPU
};

UENUM(BlueprintType)
enum class FlockType : uint8
{
	FT_Flying,
	FT_Herding
};

USTRUCT(BlueprintType)
struct FSpawnData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		FGroupData GroupData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		float SpawnRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		float MinSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		FVector Scale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		FRotator RotOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		int32 NumSpawns = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		bool bUseInstancedMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks", meta = (EditCondition = "!bUseInstancedMesh"))
		TSubclassOf<AFlocksAIController> ControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks", meta = (EditCondition = "bUseInstancedMesh"))
		class UStaticMesh* FishMesh = nullptr;

	UPROPERTY()
		class AFlocksInstancedMeshActor* InstancedMesh;

	UPROPERTY()
		TArray<int32> FishIDs;

	UPROPERTY()
		TArray<class AFlocksAIController*> FlocksControllers;

};

USTRUCT(BlueprintType)
struct FSpawnGroup
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		TArray<FSpawnData> SpawnData;
};

/**
 *
 */
UCLASS()
class FLOCKS4UE4_API AFlocksManager : public AActor
{
	GENERATED_BODY()

public:
	AFlocksManager();
	virtual void BeginPlay() override;
	virtual void Tick(float _deltaTime) override;

	virtual void DeregisterBoid(int32 _boidId);

	virtual int32 RegisterVolume(class AFlocksVolume* _volume);
	virtual void DeregisterVolume(class AFlocksVolume* _volume);

	UFUNCTION(BlueprintPure, Category = "Flocks")
		FVector GetBoidPosition(int32 _boidID);

	void BoidChangedAcceleration(int32 _id, float _acceleration);
	void BoidChangedMaxVelocity(int32 _id, float _maxVelocity);

	void FlockVolumeMoved(int32 _volumeID, FVector _newPosition);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flocks")
		FSpawnGroup SpawnGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		ComputeType ComputeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocks")
		FlockType FlockType;

	virtual void UpdateGPU(float _deltaTime);
	virtual void UpdateCPU(float _deltaTime);
	virtual void UpdateFromStates(float _deltaTime);

private:
	TSharedPtr<class FlocksComputeShaderWrapper> m_flockShader;

	TArray<BoidState> m_boidData;
	TArray<FlocksVolumeData> m_volumeData;
	TArray<FSpawnData> m_groups;

	static FVector ToVector(float _fromArr[3])
	{
		return FVector(_fromArr[0], _fromArr[1], _fromArr[2]);
	}

	UPROPERTY()
		TArray<class AFlocksVolume*> m_flockVolumes;

	static int32 currentGroupID;
};
