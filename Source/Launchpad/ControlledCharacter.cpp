// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlledCharacter.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AControlledCharacter::AControlledCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AControlledCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AControlledCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AControlledCharacter::MoveForwardCharacter(float Value)
{
	AddMovementInput(GetActorForwardVector()*Value);
	//FireEvent();
}

void AControlledCharacter::MoveRightCharacter(float Value)
{
	AddMovementInput(GetActorRightVector()*Value);
	//FireEvent();
}

void AControlledCharacter::JumpCharacter()
{
	Jump();
}

void AControlledCharacter::RotateCharacter(float Value)
{
	AddControllerYawInput(Value);
}

