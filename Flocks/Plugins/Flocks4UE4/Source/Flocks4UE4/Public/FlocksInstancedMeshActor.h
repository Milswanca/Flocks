// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlocksInstancedMeshActor.generated.h"

UCLASS()
class FLOCKS4UE4_API AFlocksInstancedMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlocksInstancedMeshActor();

	int32 AddInstance(FTransform _trans);
	void SetLocationAndRotation(int32 _id, FVector _location, FRotator _rotation);
	void SetMesh(class UStaticMesh* _mesh);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flocks", Meta = (AllowPrivateAccess = "True"))
	class UInstancedStaticMeshComponent* Mesh;

public:
	FORCEINLINE class UInstancedStaticMeshComponent* GetInstancedMesh() const { return Mesh; }
};
