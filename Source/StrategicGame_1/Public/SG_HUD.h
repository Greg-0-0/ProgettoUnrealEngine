// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "Blueprint/UserWidget.h"
#include "SG_HUD.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGICGAME_1_API USG_HUD : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	// Current player
	UFUNCTION(Category = "UI")
	void SetCurrentPlayer(FString NewPlayer, bool Hturn);

	// Used to set win message, game tie message or deadlock message
	UFUNCTION(Category = "UI")
	void SetWinMess(FString NewPlayer, bool IsHuman);

	UFUNCTION(Category = "UI")
	void SetHumanScore(FString Score);

	UFUNCTION(Category = "UI")
	void SetAiScore(FString Score);

	UFUNCTION(Category = "UI")
	void SetNMatches(FString Number);

	UFUNCTION(Category = "UI")
	void SetHBrawlerHealth(FString NewHealth);

	UFUNCTION(Category = "UI")
	void SetHSniperHealth(FString NewHealth);

	UFUNCTION(Category = "UI")
	void SetAiBrawlerHealth(FString NewHealth);

	UFUNCTION(Category = "UI")
	void SetAiSniperHealth(FString NewHealth);

	// Function used to show the label "Dead" 
	UFUNCTION(Category = "UI")
	void ToggleVisibilityHB(bool MakeVisible);

	// Function used to show the label "Dead" 
	UFUNCTION(Category = "UI")
	void ToggleVisibilityHS(bool MakeVisible);

	// Function used to show the label "Dead" 
	UFUNCTION(Category = "UI")
	void ToggleVisibilityAiB(bool MakeVisible);

	// Function used to show the label "Dead" 
	UFUNCTION(Category = "UI")
	void ToggleVisibilityAiS(bool MakeVisible);

	UFUNCTION(Category = "UI")
	void ToggleVisibilityEndTurnButton(bool MakeVisible);

	UFUNCTION(Category = "UI")
	void AddMessageToScrollBox(FString Message, bool IsHuman);

	// Function used to clear move history, "Dead" labels, units health
	UFUNCTION(Category = "UI")
	void ClearHUD();

	UFUNCTION(Category = "UI")
	void ShowUHD();

	UFUNCTION(Category = "UI")
	void HideUHD();

	bool ChangeSettingsPressed;// flag used by Menu to eventually reset the field inside OnPlayButtonClicked

	UFUNCTION()
	void OnResetButtonClicked();

protected:

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* HistoryBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HBStatus;// Text to show label "Dead"

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HSStatus;// Text to show label "Dead"

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiBStatus;// Text to show label "Dead"

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiSStatus;// Text to show label "Dead"

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HBrawlerHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HSniperHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiBrawlerHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiSniperHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WinMessage;

	UPROPERTY(meta =(BindWidget))
	class UTextBlock* CurrentPlayer;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HumanWins;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AiWins;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NumberOfMatches;// stores number of matches played

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EndTurnButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ChangeSettingsButton;

	ATile* TileToFollow;

	UFUNCTION()
	void OnEndTurnButtonClicked();

	// Function called to change setting -> Percentage of obstacle and Ai type
	UFUNCTION()
	void OnChangeSettingsButtonClicked();
};
