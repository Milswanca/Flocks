// Fill out your copyright notice in the Description page of Project Settings.

#include "FlocksInstancedMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AFlocksInstancedMeshActor::AFlocksInstancedMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MESH"));
	Mesh->SetupAttachment(RootComponent);

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 AFlocksInstancedMeshActor::AddInstance(FTransform _trans)
{
	return Mesh->AddInstance(_trans);
}

void AFlocksInstancedMeshActor::SetLocationAndRotation(int32 _id, FVector _location, FRotator _rotation)
{
	FTransform trans;
	Mesh->GetInstanceTransform(_id, trans);
	trans.SetTranslation(_location);
	trans.SetRotation(_rotation.Quaternion());
	Mesh->UpdateInstanceTransform(_id, trans, true, false);
}

void AFlocksInstancedMeshActor::SetMesh(class UStaticMesh* _mesh)
{
	Mesh->SetStaticMesh(_mesh);
}