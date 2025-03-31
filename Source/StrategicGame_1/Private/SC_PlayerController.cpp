// Fill out your copyright notice in the Description page of Project Settings.


#include "SC_PlayerController.h"
#include "Components/InputComponent.h"
#include "InputMappingContext.h"
#include "SG_GameMode.h"

ASC_PlayerController::ASC_PlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;

	/*
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> ContextAsset(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_SG_2.IMC_SG_2'"));
	if (ContextAsset.Succeeded())
	{
		SGContext = ContextAsset.Object;
	}*/
}

void ASC_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("PlayerController started"));
	// changing the default mapping context of the player controller
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Setting AddMappingContext"));
		Subsystem->AddMappingContext(SGContext, 0);
		UE_LOG(LogTemp, Warning, TEXT("Setted AddMappingContext"));
	}
	/*
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);*/
}

void ASC_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent"));
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &ASC_PlayerController::ClickOnGrid);
	}
	UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent() executed correctly"));
}

void ASC_PlayerController::ClickOnGrid()
{
	ASG_GameMode* GMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	const auto humanplayer = Cast<ASC_HumanPlayer>(GetPawn());
	if(IsValid(humanplayer) && GMode->InputEnabled)
	{
		if(GMode->GamePhase == 0)
			humanplayer->PlaceUnitOnClick();
		else
		{
			if (GMode->HCanSelectUnit)
				humanplayer->SelectUnitOnClick();
			else 
			{
				if (GMode->HCanDeselectUnit)
					humanplayer->DeselectUnitOnClick();
				if (GMode->HCanAttack)
					humanplayer->AttackOnClick();
				if (GMode->HCanMove)
					humanplayer->MoveOnClick();
			}
		}
	}
}




