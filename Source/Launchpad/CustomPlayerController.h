// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LAUNCHPAD_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;
	void CharacterMoveForward(float Value);
	void CharacterMoveRight(float Value);
	void CharacterJump();
	void CharacterRotate(float Value);
	
};
