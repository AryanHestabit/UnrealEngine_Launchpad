// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerController.h"
#include "ControlledCharacter.h"

void ACustomPlayerController::SetupInputComponent()
{

	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ACustomPlayerController::CharacterMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACustomPlayerController::CharacterMoveRight);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACustomPlayerController::CharacterJump);
	InputComponent->BindAxis("Rotate", this, &ACustomPlayerController::CharacterRotate);
}

void ACustomPlayerController::CharacterMoveForward(float Value)
{
	AControlledCharacter* cc = Cast<AControlledCharacter>(GetPawn());
	if (cc)
	{
		cc->MoveForwardCharacter(Value);
	}
}

void ACustomPlayerController::CharacterMoveRight(float Value)
{
	AControlledCharacter* cc = Cast<AControlledCharacter>(GetPawn());
	if (cc)
	{
		cc->MoveRightCharacter(Value);
	}
}

void ACustomPlayerController::CharacterJump()
{
	AControlledCharacter* cc = Cast<AControlledCharacter>(GetPawn());
	if (cc)
	{
		cc->JumpCharacter();
	}
}

void ACustomPlayerController::CharacterRotate(float Value)
{
	AControlledCharacter* cc = Cast<AControlledCharacter>(GetPawn());
	if (cc)
	{
		cc->RotateCharacter(Value);
	}
}
