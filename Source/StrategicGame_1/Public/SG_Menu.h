// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SG_Menu.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGICGAME_1_API USG_Menu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ShowMenu();

	UFUNCTION()
	void HideMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	int32 GetSliderValue();

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ObstaclePercentage;// Percentage of obstacle shown under slider

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarningPercentage;// Message shown under play button if Percentage == 100 
	// (no unit can be spawned -> game cannot be played)

	UPROPERTY(meta = (BindWidget))
	class USlider* Slider;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiChosen;// Text to show which Ai has been chosen

	UPROPERTY(meta = (BindWidget))
	class UButton* PlayButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* SmartAIButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* NaiveAIButton;

	// Click play
	UFUNCTION()
	void OnPlayButtonClicked();

	// Selecting Smart Ai
	UFUNCTION()
	void OnSmartAIButtonClicked();

	// Selecting Naive Ai
	UFUNCTION()
	void OnNaiveAIButtonClicked();

	// Changing values of the slider for obstacles percentage 
	UFUNCTION()
	void OnSliderValueChanged(float value);
};
