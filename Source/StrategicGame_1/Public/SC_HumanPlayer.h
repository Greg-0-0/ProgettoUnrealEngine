// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SC_PlayerInterface.h"
#include "Camera/CameraComponent.h"
#include "SC_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Unit.h"
#include "SC_HumanPlayer.generated.h"

UCLASS()// Class that represents the human player
class STRATEGICGAME_1_API ASC_HumanPlayer : public APawn, public ISC_PlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASC_HumanPlayer();
	UCameraComponent* Camera;
	USC_GameInstance* GameInst;
	TArray<AUnit*> ListOfUnits;// variable to keep track of owned units
	TArray<ATile*> WalkableTiles;// tile reachable from movement
	TArray<ATile*> TileInRange;// tile reachable from attack

	AUnit* UnitAux;// auxiliary variable
	ATile* TileAux;// auxiliary variable
	TArray<ATile*> ArrayTileAux;// auxiliary variable
	bool AiLost;// variable used to tell if Ai lost the match
	bool HLost;// variable used to tell if human lost the match

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool IsMyTurn;

	// Auxliary function used to identify correctly walkable tiles by a unit,
	// listoftiles stores the tiles already saved as walkable, visitedtile is the current tile visited,
	// maxmov stores the maximum distance reachable by movement, control keeps track of the distance from the tile where there is the unit
	void DfsVisitTiles1(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxmovement, int32& control);	
	// Auxliary function used to identify correctly attackable tiles by a unit
	// listoftiles stores the tiles already saved as attackable, visitedtile is the current tile visited,
	// maxmov stores the maximum range of attack, control keeps track of the distance from the tile where there is the unit
	void DfsVisitTiles2(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxrange, int32& control);
	// Function to calculate the shortest path from the RootTile to the ClickedTile, 
	// the radius is the maximum distance allowed from the RootTile
	TArray<ATile*> MinRoute(ATile* RootTile, ATile* ClickedTile, int32 radius);

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
	virtual void CheckIfListIsEmpty(bool& control);
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
	// Called when Ai wins
	virtual void OnWin() override;
	// Called when Ai loses
	virtual void OnLose() override;
	// Function called in case of tie
	virtual void OnTie() override;
	// Function called to reset all variables and destroy units
	virtual void ResetValues() override;
	// Function called to clear timers of players
	virtual void ClearTimers() override;
	// Function to manage deadlock situations
	void OnDeadLock();

	// Method used to manage the placing phase
	UFUNCTION()
	void PlaceUnitOnClick();

	// Method used to select a unit for movement/attack on click
	UFUNCTION()
	void SelectUnitOnClick();

	// Method used to deselect a previously selected unit
	UFUNCTION()
	void DeselectUnitOnClick();

	// Method called to manage the movement on click
	UFUNCTION()
	void MoveOnClick();

	// Auxiliary method to link unit movement to attack or eventually ending turn (called by tick in Unit.cpp)
	virtual void AttackingOrEndingTurn() override;
	// The method has been overridden since it is called inside Unit.cpp after moving the unit,
	// this ensures that there is a realisitcal minimal delay between actions

	// Method used to manage attacks triggered on click
	UFUNCTION()
	void AttackOnClick();

	// Method used to end the turn for the current unit or the human turn (if second unit) -> called with a button
	UFUNCTION()
	virtual void EndTurn() override;

};
