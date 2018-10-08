// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlocksVolume.generated.h"

UENUM(BlueprintType)
enum class EVolumeType : uint8
{
	VT_Restriction,
	VT_Flee,
	VT_Goal
};

UCLASS()
class FLOCKS4UE4_API AFlocksVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlocksVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float InnerRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float OuterRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FleeVolume")
		float Falloff = 1.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetInnerRadius() const { return InnerRadius; }
	float GetOuterRadius() const { return OuterRadius; }
	float GetFalloff() const { return Falloff; }
	virtual EVolumeType GetVolumeType() const { return EVolumeType::VT_Flee; }

	float GetInfluence(FVector _position) const;

private:
	class AFlocksManager* m_flocksManager = nullptr;
	int32 m_id = -1;
	
};
