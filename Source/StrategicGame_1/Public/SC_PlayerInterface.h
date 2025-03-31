// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "UObject/Interface.h"
#include "SC_PlayerInterface.generated.h"

UINTERFACE(MinimalAPI)
class USC_PlayerInterface : public UInterface
{
    GENERATED_BODY()
};

class STRATEGICGAME_1_API ISC_PlayerInterface
{
    GENERATED_BODY()

public:
    int32 playernumber;// 0 for Human, 1 for Ai
    int32 gamephase;// tells in which phase the game is (placing -> 0, attack/movement -> 1)
    TMap<AUnit*, bool> BlockedUnits;// variable used to know if a unit cannot move/attack
    bool bResetOrChangeSettingsButtonClicked = false;// this variable is used to ensure that timers don't get executed after a Reset/ChangeSettings button is clicked
    bool bEndTurnButtonClicked = false;// this variable is used to ensure that timers don't get executed after a EndTurn button is clicked
    // it can happen that, before clicking the button, a timed function was queued and
    // it might executed after the function bound to the button
    
    // EndTurnButton shoudn't block processes in Ai logic, but is safe since it can be clicked only during human turns, 
    // instead ReseButton must be always checked
    
    // function to place Units (abilitation of onclik events)
    virtual void UnitPlacement(int32 NUnitToPlace) {};
    //function to manage Move/Attack phase
    virtual void Move_Attack() {};
    // Function to retrieve an array that stores the units owned by a certain player
    virtual void GetListOfUnits(TArray<AUnit*>& listofunits) {};
    // Function to add a unit to ListOfUnits array after spawning it (helps to keep track of the units owned by every player)
    virtual void AddUnitToList(AUnit* unit) {};
    // Function to remove a unit from ListOfUnits after destruction
    virtual void RemoveUnitFromList(AUnit* Unit) {};
    // Auxiliary function to check if ListOfUnits is empty (all units destroyed)
    virtual void CheckIfListIsEmpty(bool& control) {};
    // called to mark up walkable/attackable tiles
    virtual void SelectUnit(AUnit* currunit, bool IsSelecting) {};
    // called to deselect unit
    virtual void DeselectUnit() {};
    // Auxiliary method to link unit movement to attack or eventually ending turn
    virtual void AttackingOrEndingTurn() {};
    // Function to end human turn (no ai override is necessary)
    virtual void EndTurn() {};
    // Called on win
    virtual void OnWin() {};
    // Called on lose
    virtual void OnLose() {};
    // Function called in case of tie
    virtual void OnTie() {};
    // Called to reset all class variables and despawn units
    virtual void ResetValues() {};
    // Function called to clear timers of players
    virtual void ClearTimers() {};
};
