// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ControlledCharacter.generated.h"

UCLASS()
class LAUNCHPAD_API AControlledCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AControlledCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveForwardCharacter(float Value);
	void MoveRightCharacter(float Value);
	void JumpCharacter();
	void RotateCharacter(float Value);

};
