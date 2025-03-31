// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SC_PlayerInterface.h"
#include "SC_GameInstance.h"
#include "Unit.h"
#include "Kismet/GameplayStatics.h"
#include "SC_NaiveAIPlayer.generated.h"

UCLASS()// Class to implment logic of a random Ai -> Movements and Attacks are random, decides randomly wether to attack or move
class STRATEGICGAME_1_API ASC_NaiveAIPlayer : public APawn, public ISC_PlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASC_NaiveAIPlayer();
	USC_GameInstance* GameInst;
	TArray<AUnit*> ListOfUnits;// variable to keep track of owned units
	TArray<ATile*> WalkableTiles;// tile reachable from movement
	TArray<ATile*> TileInRange;// tile reachable from attack
	bool bOtherUnitIsDeadLocked;// tells if other unit couldn't move neither attack

	AUnit* UnitAux;// auxiliary variable
	ATile* TileAux;// auxiliary variable
	TArray<ATile*> ArrayTileAux;// auxiliary variable
	bool bAttackLater;// auxiliary variable
	bool AiLost;// variable used to tell if Ai lost the match
	bool HLost;// variable used to tell if human lost the match

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Auxliary function used to identify correctly walkable tiles by a unit,
	// listoftiles stores the tiles already saved, visitedtile is the current tile visited,
	// maxmov stores the maximum distance reachable, control keeps track of the distance from the tile where there is the unit
	void DfsVisitTiles1(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxmovement, int32& control);
	// Auxliary function used to identify correctly attackable tiles by a unit
	// listoftiles stores the tiles already saved, visitedtile is the current tile visited,
	// maxmov stores the maximum range of attack, control keeps track of the distance from the tile where there is the unit
	void DfsVisitTiles2(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxrange, int32& control);
	// Function to calculate the shortest path from the RootTile to the ClickedTile, 
	// the radius is the maximum distance allowed from the RootTile
	TArray<ATile*> MinRoute(ATile* RootTile, ATile* TargetTile, int32 radius);
	// Function called to select and show the path to walk
	void SelectingPathToWalk(AUnit* UnitToMove);
	// Funtion called to move a unit
	void MoveUnit();
	// Function to manage deadlock situations
	void OnDeadLock();

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Function to add a unit after spawning it
	virtual void AddUnitToList(AUnit* unit) override;
	// Function to remove a unit from the list after destruction
	virtual void RemoveUnitFromList(AUnit* Unit) override;
	// Auxiliary function to check if the list of unit is empty (all unit destroyed)
	virtual void CheckIfListIsEmpty(bool& control) override;
	virtual void GetListOfUnits(TArray<AUnit*>& listofunits) override;
	// Function called to manage placing phase
	virtual void UnitPlacement(int32 NUnitToPlace) override;
	// Function called to manage attack/move phase
	virtual void Move_Attack() override;
	// Function used to color the walkable/attackable tiles when a unit is selected,
	// currunit stores the clicked unit, IsSelecting tells if the function is used for moving/attacking or only attacking
	virtual void SelectUnit(AUnit* currunit, bool IsSelecting) override;
	// Function called when the selected tile is deselected (colored tiles are reverted to initial white color)
	virtual void DeselectUnit() override;
	// Auxiliary method to link unit movement to attack or eventually ending turn
	virtual void AttackingOrEndingTurn() override;
	// The method is virtual since it is called inside Unit.cpp after moving the unit,
	// this ensures that there is a realisitcal minimal delay between actions

	// Function to end human turn (no ai override is necessary)
	virtual void EndTurn() override {};
	
	// Function used to carry out attacks logic
	void AttackUnit();
	// Called when Ai wins
	virtual void OnWin() override;
	// Called when Ai loses
	virtual void OnLose() override;
	//// Function called in case of tie, how evere only the one in human player is used
	virtual void OnTie() override {};
	// Function called to reset all variables and destroy units
	virtual void ResetValues() override;
	// Function called to clear timers of players
	virtual void ClearTimers() override;
};
