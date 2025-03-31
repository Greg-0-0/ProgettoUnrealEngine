// Fill out your copyright notice in the Description page of Project Settings.


#include "SC_NaiveAIPlayer.h"
#include "Tile.h"
#include "SG_GameMode.h"


// Sets default values
ASC_NaiveAIPlayer::ASC_NaiveAIPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it. 
	PrimaryActorTick.bCanEverTick = true;
	GameInst = Cast<USC_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	UnitAux = nullptr;
	TileAux = nullptr;
	ArrayTileAux = TArray<ATile*>();
	bAttackLater = false;
	AiLost = false;
	HLost = false;
	bOtherUnitIsDeadLocked = false;

}

// Called when the game starts or when spawned
void ASC_NaiveAIPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASC_NaiveAIPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASC_NaiveAIPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/*
* NOTE:
"if (bResetOrChangeSettingsButtonClicked)return;" is inserted very frequently in the code, 
since it can happen that a function interrupts itself following a click on a button,
this may lead to exceptions or undefined behaviours when the execution is resumed 
(variable are resetted when clicking "Reset" or "ChangeSettings" buttons, but they may are accessed again inside Ai functions)
*/

void ASC_NaiveAIPlayer::AddUnitToList(AUnit* unit)
{
	ListOfUnits.Add(unit);
}

void ASC_NaiveAIPlayer::RemoveUnitFromList(AUnit* Unit)
{
	ListOfUnits.Remove(Unit);
}

void ASC_NaiveAIPlayer::CheckIfListIsEmpty(bool& control)
{
	if (ListOfUnits.IsEmpty())
		control = 1;
	else
		control = 0;
}

void ASC_NaiveAIPlayer::GetListOfUnits(TArray<AUnit*>& listofunits)
{
		listofunits = ListOfUnits;
}


void ASC_NaiveAIPlayer::UnitPlacement(int32 NUnitToPlace)
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	GameInst->SetTurnMessage(TEXT("AI (Random) Turn"), false);
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			TArray<ATile*> FreeTiles;
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			for (auto CurrTile : GameMode->gamefield->GetTileArray())
			{
				if (CurrTile->GetTileStatus() == TileStatus::EMPTY)
					FreeTiles.Add(CurrTile);
			}
			if (FreeTiles.Num() > 0)
			{
				int32 TileIndex = FMath::RandRange(0, FreeTiles.Num() - 1);
				FVector spawnpos = FreeTiles[TileIndex]->GetActorLocation(); 
				FreeTiles[TileIndex]->SetTileStatus(TileStatus::UNIT);
				FreeTiles[TileIndex]->setowner(playernumber);
				GameMode->SpawnUnit(playernumber, spawnpos, FreeTiles[TileIndex]);
				
			}
		}, 2.5f, false);
}

void ASC_NaiveAIPlayer::Move_Attack()
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	GameInst->SetTurnMessage(TEXT("AI (Random) Turn"), false);
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			bool bTurnEnded = false;// these variable gurantees that the play of the Ai stops when its turn ends
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			int32 UnitToPlay = -1;
			if (ListOfUnits.Num() > 1)
				// other unit could have been destroyed
				UnitToPlay = FMath::RandRange(0, 1);
			else
				UnitToPlay = 0;
			if (ListOfUnits[UnitToPlay]->PLayed())
				// unit has already been played, switching to next unit
			{
				if (UnitToPlay == 0)
				{
					if (ListOfUnits.Num() > 1)
						// only one unit left to play
						UnitToPlay++;

					if (ListOfUnits[UnitToPlay]->PLayed())
						// both unit have already been played
					{
						// resetting units for next turn
						for (auto unit : ListOfUnits)
							unit->SetPlayed(false);
						{
							bTurnEnded = true;
							UnitAux = nullptr;
							TileAux = nullptr;
							ArrayTileAux.Empty();
							GameMode->NextPlayerPlaying();
						}
					}
				}
				else
				{
					UnitToPlay = 0;
					if (ListOfUnits[UnitToPlay]->PLayed())
						// both unit have already been played
					{
						// resetting units for next turn
						for (auto unit : ListOfUnits)
							unit->SetPlayed(false);
						{
							bTurnEnded = true;
							UnitAux = nullptr;
							TileAux = nullptr;
							ArrayTileAux.Empty();
							GameMode->NextPlayerPlaying();
						}
					}
				}
			}
			if (!bTurnEnded)
			{
				ListOfUnits[UnitToPlay]->SetPlayed(true);// current unit is being played
				ASC_NaiveAIPlayer::SelectUnit(ListOfUnits[UnitToPlay], true);
				int32 MoveOrAttack = FMath::RandRange(0, 1);
				int32 control1 = 0;
				for (auto tile : TileInRange)
				{
					if (tile->GetColorOfTile() == FLinearColor::Red)
						control1 = 1;
				}
				if (control1 == 0 && WalkableTiles.IsEmpty())
					// cannot attack nor move
				{
					BlockedUnits[ListOfUnits[UnitToPlay]] = true;
					AUnit* otherunit = nullptr;
					for (auto unit : ListOfUnits)
						if (unit != ListOfUnits[UnitToPlay])
							otherunit = unit;
					if (BlockedUnits[otherunit] || otherunit->PLayed())
						// other unit is blocked or already played
					{
						// deadlock
						ASC_NaiveAIPlayer::OnDeadLock();
						MoveOrAttack = -1;// this way no other part of the function is executed
					}
					else if (otherunit->PLayed())
					{
						// deadlock
						ASC_NaiveAIPlayer::OnDeadLock();
						MoveOrAttack = -1;// this way no other part of the function is executed
					}
					else
					{
						// other unit wasn't played yet, maybe it can move
						ListOfUnits[UnitToPlay]->SetPlayed(false);
						if (UnitToPlay == 0)
						{
							UnitToPlay++;
						}
						else
							UnitToPlay = 0;
						ListOfUnits[UnitToPlay]->SetPlayed(true);// other unit is being played
						ASC_NaiveAIPlayer::DeselectUnit();// previous colored tile are reverted to white
						ASC_NaiveAIPlayer::SelectUnit(ListOfUnits[UnitToPlay], true);
						int32 control2 = 0;
						for (auto tile : TileInRange)
						{
							if (tile->GetColorOfTile() == FLinearColor::Red)
								// attacks if it can
								control2 = 1;
						}
						if (control2 == 0 && WalkableTiles.IsEmpty())
						{
							// deadlock
							ASC_NaiveAIPlayer::OnDeadLock();
							MoveOrAttack = -1;// this way no other part of the function is executed
						}
						else
							// can attack, move or both -> choosing
						{
							if (control2 != 0 && !WalkableTiles.IsEmpty())
								MoveOrAttack = FMath::RandRange(0, 1);
							else if (control2 != 0 && WalkableTiles.IsEmpty())
								MoveOrAttack = 1;
							else if (control2 == 0 && !WalkableTiles.IsEmpty())
								MoveOrAttack = 0;
						}
					}
				}
				else
				{
					BlockedUnits[ListOfUnits[UnitToPlay]] = false;
				}
				if (MoveOrAttack == 0)
					// moving
				{
					TileAux = ListOfUnits[UnitToPlay]->GetTile();// storing tile where unit is standing
					// this will allow to change its datas once the movement is completed
					ASC_NaiveAIPlayer::SelectingPathToWalk(ListOfUnits[UnitToPlay]);
				}
				else if(MoveOrAttack == 1)
					// attacking right away
				{
					// checking if ai can attack, since AttackUnit() is executed when it is certain that an attack can be carried out
					bool bCanAttack = false;
					for (auto tile : GameMode->gamefield->TileArray)
						if (tile->GetColorOfTile() == FLinearColor::Red)
							bCanAttack = true;
					if (bCanAttack)
					{
						UnitAux = ListOfUnits[UnitToPlay];// storing attacking unit
						ASC_NaiveAIPlayer::AttackUnit();
					}
					else
						// cannot attack now, will try after movement (at least one action per unit must be made)
					{
						bAttackLater = true;// will be used inside AttackingOrEndingTurn() to force attack if possible (no random selection)
						TileAux = ListOfUnits[UnitToPlay]->GetTile();// storing tile where unit is standing
						// this will allow to change its datas once the movement is completed
						ASC_NaiveAIPlayer::SelectingPathToWalk(ListOfUnits[UnitToPlay]);
					}
				}
			}
		}, 1.5f, false);
}

void ASC_NaiveAIPlayer::SelectingPathToWalk(AUnit* UnitToMove)
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	FTimerHandle Timer;
	UnitAux = UnitToMove;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			int32 NWalkableTiles = WalkableTiles.Num();
			int32 TargetTile = FMath::RandRange(0, WalkableTiles.Num() - 1);
			TArray<ATile*> minroute = ASC_NaiveAIPlayer::MinRoute(UnitAux->GetTile(), WalkableTiles[TargetTile], UnitAux->GetMaxMov());
			ASC_NaiveAIPlayer::DeselectUnit();// all the walkable tiles are erased, want to show only path walked
			UnitAux->GetTile()->ChangeColor(FLinearColor::Yellow);
			for (auto tile : minroute)
				tile->ChangeColor(FLinearColor::Green);
			ArrayTileAux = minroute;
			ASC_NaiveAIPlayer::MoveUnit();
		}, 1.5f, false);
}

void ASC_NaiveAIPlayer::MoveUnit()
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			UnitAux->SetPathToFollow(ArrayTileAux);
			// Writing the message of movement in the move history 
			char pos1 = UnitAux->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = UnitAux->GetTile()->GetTileId().second;
			char newpos1 = ArrayTileAux.Last()->GetTileId().first;
			FString Newpos1 = FString(1, &newpos1);
			int32 newpos2 = ArrayTileAux.Last()->GetTileId().second;
			int32 type = UnitAux->GetAttackType();
			FString TypeName;
			if (type == 0)
				TypeName = "S";
			else
				TypeName = "B";
			FString mess = FString::Printf(TEXT("AI: %s %s%d -> %s%d "), *TypeName, *Pos1, pos2, *Newpos1, newpos2);
			GameInst->AddMessageToHistory(mess, false);
			UnitAux->SetIfIsMoving(true);// movement is carried out (Unit.cpp file -> tick())
		}, 1.0f, false);
}

void ASC_NaiveAIPlayer::DeselectUnit()
{
	for (AUnit* unit : ListOfUnits)
		unit->GetTile()->ChangeColor(FLinearColor::White);
	for (ATile* tile : WalkableTiles)
		tile->ChangeColor(FLinearColor::White);
	for (ATile* tile : TileInRange)
		tile->ChangeColor(FLinearColor::White);
}

void ASC_NaiveAIPlayer::AttackingOrEndingTurn()
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			ASC_NaiveAIPlayer::DeselectUnit();// path followed is cancelled
			// updating unit and tile information
			TileAux->setowner(-1);// previously occupied tile
			TileAux->SetTileStatus(TileStatus::EMPTY);
			// ArrayTileAux store the followed path, the last element(tile) is the final destination of the unit
			UnitAux->SetTileUnit(ArrayTileAux.Last());
			ArrayTileAux.Last()->setowner(playernumber);
			ArrayTileAux.Last()->SetTileStatus(TileStatus::UNIT);
			// from the new position must check if the unit can attack opponents
			ASC_NaiveAIPlayer::SelectUnit(UnitAux, false);
			int32 contr = 0;
			for (auto tile : TileInRange)
			{
				if (tile->GetTileStatus() == TileStatus::UNIT && tile->GetTileOwner() != playernumber)
					// checking if any enemy unit is in range
				{
					tile->ChangeColor(FLinearColor::Red);
					contr++;
				}
			}
			if (contr > 0)
				// at least one tile in range has an enemy unit on top -> can attack
			{
				if (bAttackLater)
					// couldn't attack before even if chosen (in Move_Attack between moving or attacking), doing it now
				{
					bAttackLater = false;// restoring value
					ASC_NaiveAIPlayer::AttackUnit();
				}
				else
					// attack wasn't choose before -> random selection
				{
					// randomly deciding if Ai will attack
					int32 AttackDecision = FMath::RandRange(0, 1);
					if (AttackDecision == 0)
						// no attack is carried out, checking if the other unit hasn't been played yet
					{
						if (bResetOrChangeSettingsButtonClicked)
							return;
						FTimerHandle Timer;
						GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
							{
								if (bResetOrChangeSettingsButtonClicked)// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
									return;
								ASC_NaiveAIPlayer::DeselectUnit();
								ASC_NaiveAIPlayer::Move_Attack();// checking is done here
							}, 1.0f, false);
					}
					else
						// attacking an enemy unit
					{
						ASC_NaiveAIPlayer::AttackUnit();
					}
				}
			}
			else
				// no attack is possible, terminating the turn if other unit has already been played
			{
				ASC_NaiveAIPlayer::DeselectUnit();
				ASC_NaiveAIPlayer::Move_Attack();// checking if other unit has been played is done here
			}

		}, 0.75f, false);
}

void ASC_NaiveAIPlayer::AttackUnit()
{
	// Premise: this function is called when at least a unit can be attacked
	if (bResetOrChangeSettingsButtonClicked)
		return;
	UnitAux->SetPlayed(true);
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
				return;
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			TArray<AUnit*> ListOfEnemyUnits;
			AUnit* AttackedUnit = nullptr;
			TArray<ATile*> AttackableTiles;
			for (auto tile : TileInRange)
			{
				if (tile->GetColorOfTile() == FLinearColor::Red)
					AttackableTiles.Add(tile);
			}
			int32 AttackedTile = FMath::RandRange(0, AttackableTiles.Num() - 1);// randomly choosing a tile to attack
			ASC_NaiveAIPlayer::DeselectUnit();// red tiles are reverted to white
			// changing color to orange to point out the chosen tile
			AttackableTiles[AttackedTile]->ChangeColor(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f));
			// recovering the attacked unit 
			{
				for (auto HPlayer : GameMode->Players)
				{
					if (HPlayer->playernumber == 0)
						// that's human player
					{
						HPlayer->GetListOfUnits(ListOfEnemyUnits);
					}
				}
				for (auto unit : ListOfEnemyUnits)
					// after attack the tile are reverted to white
				{
					if (unit->GetTile()->GetColorOfTile() != FLinearColor::White)
						AttackedUnit = unit;
				}
			}
			// attack logic
			int32 DamageDealt = FMath::RandRange(UnitAux->GetAttackDR().X, UnitAux->GetAttackDR().Y);// extracting the dealt damage
			AttackedUnit->SetPointLife(AttackedUnit->GetHealth() - DamageDealt);// applying the damage
			GameInst->SetHealth(FString::FromInt(AttackedUnit->GetHealth()), true, AttackedUnit->GetAttackType());
			// printing the action of attack on the move history
			char pos1 = AttackedUnit->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = AttackedUnit->GetTile()->GetTileId().second;
			int32 type = UnitAux->GetAttackType();
			FString TypeName;
			if (type == 0)
				TypeName = "S";
			else
				TypeName = "B";
			FString mess = FString::Printf(TEXT("AI: %s %s%d %d "), *TypeName, *Pos1, pos2, DamageDealt);
			GameInst->AddMessageToHistory(mess, false);
			int32 CounterDamage = 0;// if no CounterAttack is carried out CounterDamage is zero
			if (UnitAux->GetAttackType() == 0)
				// unit is a sniper, then the CounterAttack mechanic takes place
			{
				CounterDamage = FMath::RandRange(1, 3);
			}
			TArray<ATile*> NearTiles = AttackedUnit->GetTile()->GetLinkedTiles();// storing tiles near the attacked unit
			// used in case attacked unit is a brawler
			NearTiles.Append(AttackedUnit->GetTile()->GetDLinkedTiles());
			bool IsNear = false;// variable to know wether attacking unit is next to attacked unit
			for (auto tile : NearTiles)
			{
				if (UnitAux->GetTile() == tile)
					// attacking unit is near to attacked unit
					IsNear = true;
			}
			if (AttackedUnit->GetAttackType() == 0 || IsNear)
				// CounterDamage is applied only if: enemy is a sniper or attacking unit is next to attacked unit (in any case)
			{
				UnitAux->SetPointLife(UnitAux->GetHealth() - CounterDamage);
				GameInst->SetHealth(FString::FromInt(UnitAux->GetHealth()), false, UnitAux->GetAttackType());
			}
			if (AttackedUnit->GetHealth() <= 0)
				// attacked human unit is killed
			{
				GameInst->SetHealth(FString::FromInt(0), true, AttackedUnit->GetAttackType());
				GameInst->SetDead(true, AttackedUnit->GetAttackType(), true);
				GameMode->Players[0]->RemoveUnitFromList(AttackedUnit);
				AttackedUnit->GetTile()->setowner(-1);// tile is free
				AttackedUnit->GetTile()->SetTileStatus(TileStatus::EMPTY);// tile is empty
				AttackedUnit->Destroy();
				bool bControl = false;
				GameMode->Players[0]->CheckIfListIsEmpty(bControl);
				if (bControl)
					// Ai lost all units, game over
				{
					GameMode->IsGameOver = true;
					HLost = true;
				}
			}
			if (UnitAux->GetHealth() <= 0)
				// attacking unit is killed by counter attack
			{
				GameInst->SetHealth(FString::FromInt(0), false, UnitAux->GetAttackType());
				GameInst->SetDead(false, UnitAux->GetAttackType(), true);
				ListOfUnits.Remove(UnitAux);
				UnitAux->GetTile()->setowner(-1);// tile is free
				UnitAux->GetTile()->SetTileStatus(TileStatus::EMPTY);// tile is empty
				UnitAux->Destroy();
				if (ListOfUnits.IsEmpty())
					// checking if human lost all units
				{
					GameMode->IsGameOver = true;
					AiLost = true;
				}
			}
			if (AiLost || HLost)
				// game ended -> remember to reset all the variables (eg ListOfUnits) of both players
			{
				if (bResetOrChangeSettingsButtonClicked)
					return;
				FTimerHandle Timer;
				GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
					{
						if (bResetOrChangeSettingsButtonClicked)
							//  ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
							return;
						ASG_GameMode* GameM = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
						ASC_NaiveAIPlayer::DeselectUnit();
						GameM->GameReset(AiLost, HLost, false);
					}, 0.75f, false);
			}
			else
			{
				// no game over, attack has ended, timer to make realistic eventual selection of the other unit
				if (bResetOrChangeSettingsButtonClicked)
					return;
				FTimerHandle Timer;
				GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
					{
						if (bResetOrChangeSettingsButtonClicked)
							// ensures that function doesn't do anything if executed after Reset/ChangeSettings -> game must end
							return;
						// attack has ended
						ASC_NaiveAIPlayer::DeselectUnit();
						ASC_NaiveAIPlayer::Move_Attack();// checking if other unit can be played is done here
					}, 0.5f, false);
			}
		}, 1.5f, false);
}

void ASC_NaiveAIPlayer::SelectUnit(AUnit* currunit, bool IsSelecting)
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	// datas are stored in local variables for better handling
	ATile* currtile = currunit->GetTile();
	int32 MaxMov = currunit->GetMaxMov();
	int32 MaxRange = currunit->GetMaxAttackR();
	int32 control = 0;// keeps track of the distance from the currtile (where there's the unit),
	// howevere this value is mostly used inside DfsVisitTiles1/DfsVisitTiles2
	TArray<ATile*> listoftiles;// tiles saved as walkable/attackable
	TArray<ATile*> linkedtiles;
	linkedtiles = currtile->GetLinkedTiles();
	// tiles are visited using a depth-first strategy (Dfs)
	for (ATile* tile : linkedtiles)
	{
		if (tile->GetTileStatus() != TileStatus::OBSTACLE && tile->GetTileStatus() != TileStatus::UNIT)
		{
			control++;
			if (!listoftiles.Contains(tile))
				listoftiles.Add(tile);
			DfsVisitTiles1(listoftiles, tile, MaxMov, control);
			control = 0;
		}
	}
	WalkableTiles = listoftiles;
	listoftiles.Empty();// variable is reused for attackable tiles
	linkedtiles = currtile->GetLinkedTiles();
	control = 0;
	if (MaxRange == 1)
		// in case the unit is a brawler the range extends to only tiles around unit
	{
		listoftiles = currtile->GetLinkedTiles();
	}
	else
		// in case the unit is a sniper
	{
		for (ATile* tile : linkedtiles)
		{
			// no control is necessary since attacks can pass through obstacles and units
			control++;
			if (!listoftiles.Contains(tile))
				listoftiles.Add(tile);
			DfsVisitTiles2(listoftiles, tile, MaxRange, control);
			control = 0;
		}
	}
	TileInRange = listoftiles;
	currtile->ChangeColor(FLinearColor::Yellow);
	if (IsSelecting)
		// if only attacking and not moving, this doesn't execute
		for (ATile* tile : WalkableTiles)
			tile->ChangeColor(FLinearColor::Green);
	for (ATile* tile : TileInRange)
		if (tile->GetTileStatus() == TileStatus::UNIT && tile->GetTileOwner() != playernumber)
			tile->ChangeColor(FLinearColor::Red);
}

void ASC_NaiveAIPlayer::DfsVisitTiles1(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxmovement, int32& control)
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	TArray<ATile*> linked_tiles;
	linked_tiles = visitedtile->GetLinkedTiles();
	for (ATile* tile : linked_tiles)
	{
		if (tile->GetTileStatus() != TileStatus::OBSTACLE && control < maxmovement && tile->GetTileStatus() != TileStatus::UNIT)
		{
			control++;
			if (!listoftiles.Contains(tile))
				// checking if tile has been already visited
				listoftiles.Add(tile);
			if (control < maxmovement)
				// tile isn't the last one in range
			{
				int32 tm = control - 1;
				DfsVisitTiles1(listoftiles, tile, maxmovement, control);
				control = tm;
			}
			else
				// tile is the last one in range, thus it is saved but no recursion occurs
				control--;
		}
	}
}

void ASC_NaiveAIPlayer::DfsVisitTiles2(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxrange, int32& control)
{
	if (bResetOrChangeSettingsButtonClicked)
		return;
	TArray<ATile*> linked_tiles;
	linked_tiles = visitedtile->GetLinkedTiles();
	for (ATile* tile : linked_tiles)
	{
		if (control < maxrange)
			// must only check if tile is in range
		{
			control++;
			if (!listoftiles.Contains(tile))
				// checking if tile has been already visited
				listoftiles.Add(tile);
			if (control < maxrange)
				// tile isn't the last one in range
			{
				int32 tm = control - 1;
				DfsVisitTiles2(listoftiles, tile, maxrange, control);
				control = tm;
			}
			else
				// tile is the last one in range, thus it is saved but no recursion occurs
				control--;
		}
	}
}

TArray<ATile*> ASC_NaiveAIPlayer::MinRoute(ATile* RootTile, ATile* TargetTile, int32 radius)
{
	TMap<ATile*, int32> Distances;// structure to store the distances between every reachable tile and the RootTile
	TMap<ATile*, ATile*> Precedent;// strucuture that pairs a tile (value) and its parent (key),
	// this enables to walk the various paths (tree-like-structure)
	Distances.Add(RootTile, 0);
	Precedent.Add(RootTile, nullptr);
	TQueue<ATile*> TileQueue;
	TileQueue.Enqueue(RootTile);
	// the tiles are visited using a breadth-first strategy (Bfs)
	while (!TileQueue.IsEmpty())
	{
		ATile* CurrTile;
		TileQueue.Dequeue(CurrTile);
		int32 CurrentDistance = Distances[CurrTile];
		for (auto tile : CurrTile->GetLinkedTiles())
		{
			if (!Distances.Contains(tile) && tile->GetTileStatus() != TileStatus::OBSTACLE && tile->GetTileStatus() != TileStatus::UNIT &&
				abs(tile->GetTilePos().X - RootTile->GetTilePos().X) + abs(tile->GetTilePos().Y - RootTile->GetTilePos().Y) <= radius)
				// the condition is met if:
				// the tile hasn't been already visited, doesn't have an obstacle/unit on top,
				// isn't farther away than the maximum movement allowed by the unit (radius)
			{
				Distances.Add(tile, CurrentDistance + 1);// new tile is visited, hence distance is incremented
				Precedent.Add(tile, CurrTile);// storing the tile parent
				TileQueue.Enqueue(tile);
			}
		}
	}

	// the shortest path is stored
	TArray<ATile*> ShortestPath;
	ShortestPath.Insert(TargetTile, 0);// the tiles are inserted from the head, starting from the last to be reached,
	// this way the first tile in the array will be also the first right after the RootTile
	int32 distance = Distances[TargetTile] - 1;// since already the last tile has been added, the distance is decremented by one
	ATile* tile = TargetTile;

	while (distance > 0)
	{
		tile = Precedent[tile];
		ShortestPath.Insert(tile, 0);
		distance--;
	}
	return ShortestPath;
}

void ASC_NaiveAIPlayer::OnWin()
{
	GameInst->SetWinMessage(TEXT("AI Wins!"), false);
	GameInst->IncrementScoreAiPlayer();
	GameInst->IncrementMatchesPlayed();
	// clearing "Dead" labels (both Ai and Human)
	GameInst->SetDead(true, 0, false);
	GameInst->SetDead(true, 1, false);
	GameInst->SetDead(false, 0, false);
	GameInst->SetDead(false, 1, false);
}

void ASC_NaiveAIPlayer::OnLose()
{
	GameInst->SetTurnMessage(TEXT("AI Loses!"), false);
}

void ASC_NaiveAIPlayer::OnDeadLock()
{
	// deadlock encountered
	GameInst->SetWinMessage(TEXT("DeadLock -> Reset field"), false);
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->GameReset(false, false, true);
}


void ASC_NaiveAIPlayer::ResetValues()
{
	ASC_NaiveAIPlayer::DeselectUnit();
	bAttackLater = false;
	TileAux = nullptr;
	UnitAux = nullptr;
	AiLost = false;
	HLost = false;
	ArrayTileAux.Empty();
	WalkableTiles.Empty();
	TileInRange.Empty();
	for (auto unit : ListOfUnits)
		if (unit != nullptr)
			unit->SelfDestroy();
	ListOfUnits.Empty();
}

void ASC_NaiveAIPlayer::ClearTimers()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}
