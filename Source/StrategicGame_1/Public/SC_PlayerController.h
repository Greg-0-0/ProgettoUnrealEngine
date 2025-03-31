// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SC_HumanPlayer.h"
#include "SC_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGICGAME_1_API ASC_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASC_PlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* SGContext;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ClickAction;

	// Function executed at a click action (left-mouse button)
	void ClickOnGrid();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
};
