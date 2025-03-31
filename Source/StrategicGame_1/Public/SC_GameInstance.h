// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SC_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGICGAME_1_API USC_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	// score value for human player
	UPROPERTY(EditAnywhere)
	int32 ScoreHumanPlayer = 0;

	// score value for AI player
	UPROPERTY(EditAnywhere)
	int32 ScoreAiPlayer = 0;

	// number of matches played
	UPROPERTY(EditAnywhere)
	int32 MatchesPlayed = 0;

	// message to show every turn
	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	// increment the score for human player
	void IncrementScoreHumanPlayer();

	// increment the score for AI player
	void IncrementScoreAiPlayer();

	// increment matches played
	void IncrementMatchesPlayed();

	// get the score for human player
	UFUNCTION(BlueprintCallable)
	int32 GetScoreHumanPlayer();

	// get the score for AI player
	UFUNCTION(BlueprintCallable)
	int32 GetScoreAiPlayer();

	// get the number of matches played
	UFUNCTION(BlueprintCallable)
	int32 GetMatchesPlayed();

	// get the current turn message
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();

	// set the turn message
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message, bool Hturn);

	// set the win message
	UFUNCTION(BlueprintCallable)
	void SetWinMessage(FString Message, bool IsHuman);

	// set health of a unit given its type and player
	UFUNCTION(BlueprintCallable)
	void SetHealth(FString NewHealth, bool IsHuman, int32 attacktype);

	// set label "Dead"
	UFUNCTION(BlueprintCallable)
	void SetDead(bool IsHuman, int32 attacktype, bool MakeVisible);

	// changes visibility of EndTurnButton
	UFUNCTION(BlueprintCallable)
	void ChangeVisibilityEndTurnButton(bool MakeVisible);

	// add message to history
	UFUNCTION(BlueprintCallable)
	void AddMessageToHistory(FString Message, bool IsHuman);
};
