// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameField.h"
#include "Unit.h"
#include "SC_PlayerInterface.h"
#include "SG_HUD.h"
#include "SG_Menu.h"
#include "SC_HumanPlayer.h"
#include "SC_NaiveAIPlayer.h"
#include "SG_SmartAIPlayer.h"
#include "GameFramework/GameMode.h"
#include "SG_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGICGAME_1_API ASG_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	bool IsGameOver;
	TArray<ISC_PlayerInterface*> Players;// Array that stores the interfaces of the players
	TArray<AUnit*>* HumanUnits;// Array to access human units (used by SmartAi to initialize its corresponding array)
	int32 ai_type;// 0 -> naive, 1 -> smart
	int32 currentplayer;// tells the current player playing
	int32 UnitsNotPlaced;// tells the number of units not placed yet
	int32 FirstPlayer;// variable used to keep track of the first player for the second phase of the game
	int32 GamePhase;// tells in which phase the game is (placing -> 1, attack/movement -> 2)
	bool HCanDeselectUnit;// tells if the human player can deselect units
	bool HCanSelectUnit;// tells if the human player can select units
	bool HCanMove;// tells if the human can move units
	bool HCanAttack;// tells if the human can attack
	bool InputEnabled;// variable to disable/enable inputs (used in functions called by inputs)
	int32 Hcontrol;// tells which unit the human player has to spawn
	int32 AIcontrol;// tells which unit the ai player has to spawn
	bool AILost;// tells if ai lost
	bool HLost;// tell if human lost
	// could be both -> tie, if they are both false that means the reset button has been clicked or deadlock scenario


	USG_HUD* UserHudInstance;

	USG_Menu* MenuInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(VisibleAnywhere)
	AGameField* gamefield;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AUnit> HBrawler;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AUnit> HSniper;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AUnit> AIBrawler;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AUnit> AISniper;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USG_HUD> UserHudClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USG_Menu> MenuClass;

	ASC_HumanPlayer* HumanPlayer;

	ASG_GameMode();
	virtual void BeginPlay() override;
	// Function called to generate field (called from OnPlayButtonClicked -> SG_Menu.cpp)
	void GenerateField();
	// Function called at the start of the game or after clicking Play button
	void ChoseAi();
	void ChosePlayerAndStartGame();
	// Function called when a click event to spawn a unit is triggered or when an Ai spawns a unit
	void SpawnUnit(const int32 playernum, const FVector& SpawnPos,ATile* tileunit);
	// Returns the number of the next player placing/playing (0 -> human, 1 -> Ai)
	int32 GetNextPlayer(int32 player);
	// Function for fisrt phase of the game -> placing
	void NextPlayerPlacing();
	// Function for second phase of the game -> playing
	void NextPlayerPlaying();
	// Function called on game reset
	void GameReset(bool AILost, bool HLost, bool ResetPressed);
protected:
	// Function used to ensure the randomness of the first player to play
	int32 Random(int32 dim);
};
