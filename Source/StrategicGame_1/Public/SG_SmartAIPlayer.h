// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SC_PlayerInterface.h"
#include "SC_GameInstance.h"
#include "Unit.h"
#include "Kismet/GameplayStatics.h"
#include "SG_SmartAIPlayer.generated.h"

UCLASS()// Class to implment logic of a "smart" Ai -> Attacks are always carried out when possible and always tries to reach enemy unit 
class STRATEGICGAME_1_API ASG_SmartAIPlayer : public APawn, public ISC_PlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASG_SmartAIPlayer();

	USC_GameInstance* GameInst;
	TArray<AUnit*> ListOfUnits;// variable to keep track of owned units
	TArray<ATile*> WalkableTiles;// tile reachable from movement
	TArray<ATile*> TileInRange;// tile reachable from attack
	TArray<AUnit*>* ListOfHumanUnits;// variable to track where opponent's units are
	bool UnitInReach;// tells if at least an opponent's unit is in the movement range

	AUnit* UnitAux;// auxiliary variable
	ATile* TileAux;// auxiliary variable
	TArray<ATile*> ArrayTileAux;// auxiliary variable
	bool bAttackLater;// auxiliary variable
	bool AiLost;// variable used to tell if Ai lost the match
	bool HLost;// variable used to tell if Human lost the match
	
	int32 temp = 0;


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
	// Function to calculate the shortest path from the TargetTile to the ClickedTile, 
	TArray<ATile*> MinRoute(ATile* RootTile, ATile* TargetTile, TMap<ATile*, int32>& Dist, TMap<ATile*, ATile*>& Prec);
	// Function to execute breadth-first visit of the tiles
	void BFSVisit(ATile* RootTile, TMap<ATile*, int32>& Distances, TMap<ATile*, ATile*>& Precedent);
	// Function called to select and show the path to walk
	void SelectingPathToWalk(AUnit* UnitToMove);
	// Funtion called to move a unit
	void MoveUnit();
	// Function to manage deadlock situations
	void OnDeadLock();
	// Selects an alternative paths when neither human(enemy) units can be directly reached,
	// the path returned ends on the nearest reachable tile to the nearest human(enemy) unit
	TArray<ATile*> SelectPathToWalkAlternative(TMap<ATile*, int32> Distances, TMap<ATile*, ATile*> Precedent);

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

	// Function to end human turn (no ai override is necessary)+
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
