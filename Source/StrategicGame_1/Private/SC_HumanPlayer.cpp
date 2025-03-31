// Fill out your copyright notice in the Description page of Project Settings.


#include "SC_HumanPlayer.h"
#include "SG_GameMode.h"
#include "Tile.h"
#include <cmath>
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ASC_HumanPlayer::ASC_HumanPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	// set to be possessed by the first player (lowest number) 
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	SetRootComponent(Camera);
	GameInst = Cast<USC_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	IsMyTurn = false;
	UnitAux = nullptr;
	TileAux = nullptr;
	ArrayTileAux = TArray<ATile*>();
	AiLost = false;
	HLost = false;

}

// Called when the game starts or when spawned
void ASC_HumanPlayer::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASC_HumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASC_HumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASC_HumanPlayer::AddUnitToList(AUnit* unit)
{
	ListOfUnits.Add(unit);
}

void ASC_HumanPlayer::RemoveUnitFromList(AUnit* Unit)
{
	ListOfUnits.Remove(Unit);
}

void ASC_HumanPlayer::CheckIfListIsEmpty(bool& control)
{
	if (ListOfUnits.IsEmpty())
		control = 1;
	else
		control = 0;
}

void ASC_HumanPlayer::GetListOfUnits(TArray<AUnit*>& listofunits)
{
	listofunits = ListOfUnits;
}

void ASC_HumanPlayer::UnitPlacement(int32 NUnitToPlace)
{
	IsMyTurn = true;
	GameInst->SetTurnMessage(TEXT("Human Turn: Place"), true);
}

void ASC_HumanPlayer::Move_Attack()
{
	IsMyTurn = true;
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->HCanSelectUnit = true;
	for (auto unit : ListOfUnits)
		if(unit != nullptr)
			unit->SetPlayed(false);// units are both playable
	GameInst->SetTurnMessage(TEXT("Human Turn: Play"), true);
}

void ASC_HumanPlayer::PlaceUnitOnClick()
{
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked!"));
	FHitResult hit = FHitResult(ForceInit);// structure to store the location hit by the trace of the mouse input
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);// storing the location hit
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (ATile* currtile = Cast<ATile>(hit.GetActor()))
			// checking if the hit landed on a tile
		{
			if (currtile->GetTileStatus() == TileStatus::EMPTY)
				// the tile must be empty	
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawning actor on: %s"), *hit.GetActor()->GetName());
				currtile->SetTileStatus(TileStatus::UNIT);
				currtile->setowner(playernumber);
				FVector spawnpos = currtile->GetActorLocation();
				GameMode->SpawnUnit(playernumber, spawnpos, currtile);
				IsMyTurn = false;
			}
		}
	}
}

void ASC_HumanPlayer::SelectUnitOnClick()
{
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked!"));
	FHitResult hit = FHitResult(ForceInit);// structure to store the location hit by the trace of the mouse input
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);// storing the location hit
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (AUnit* currunit = Cast<AUnit>(hit.GetActor()))
			// checking if a unit has been selected
		{
			if (currunit->GetUnitOwner() == GameMode->currentplayer)
				// checking if the owner is the human player
			{
				UnitAux = currunit;// store the unit in a variable class
				// this way in case of a direct attack, without previous movement, the information is easly accessible
				ASC_HumanPlayer::SelectUnit(currunit,true);
				// checking if any opponent unit can be attacked
				int32 control = 0;
				for (auto tile : TileInRange)
				{
					if (tile->GetColorOfTile() == FLinearColor::Red)
						control = 1;
				}
				if (control == 0 && WalkableTiles.IsEmpty())
					// cannot attack nor move
				{
					BlockedUnits[currunit] = true;
					AUnit* otherunit = nullptr;
					for (auto unit : ListOfUnits)
						if (unit != currunit)
							otherunit = unit;
					if (BlockedUnits[otherunit])
						// also other unit is blocked
					{
						// deadlock
						GameMode->HCanMove = false;
						GameMode->HCanAttack = false;
						GameMode->HCanSelectUnit = false;
						GameMode->HCanDeselectUnit = false;
						ASC_HumanPlayer::OnDeadLock();
					}
					else
					{
						// other unit maybe can move
						// leaving control to the next click of the player
						// this doesn't do anything
					}
				}
				GameMode->HCanMove = true;
				GameMode->HCanAttack = true;
				GameMode->HCanSelectUnit = false;
				GameMode->HCanDeselectUnit = true;
			}
		}
	}
}

void ASC_HumanPlayer::SelectUnit(AUnit* currunit, bool IsSelecting)
{
	// datas are stored in local variables for better handling
	ATile* currtile = currunit->GetTile();
	int32 MaxMov = currunit->GetMaxMov();
	int32 MaxRange = currunit->GetMaxAttackR();
	int32 control = 0;// keeps track of the distance from the currtile (where there's the unit),
	// howevere this value is mostly used inside DfsVisitTiles1/DfsVisitTiles2
	TArray<ATile*> listoftiles;// array to store walkable/attackable tiles
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
	if(IsSelecting)
		// if only attacking and not moving, this doesn't execute
		for (ATile* tile : WalkableTiles)
			// walkable tiles are green
			tile->ChangeColor(FLinearColor::Green);
	for (ATile* tile : TileInRange)
		// attackable tiles are red, but they are shown only if a opponent's unit is on top 
		if (tile->GetTileStatus() == TileStatus::UNIT && tile->GetTileOwner() != playernumber)
			tile->ChangeColor(FLinearColor::Red);
}

void ASC_HumanPlayer::DfsVisitTiles1(TArray<ATile*>& listoftiles, ATile* visitedtile,
	int32 maxmovement, int32& control)
{
	TArray<ATile*> linked_tiles;
	linked_tiles = visitedtile->GetLinkedTiles();
	for (ATile* tile : linked_tiles)
	{
		if(tile->GetTileStatus() != TileStatus::OBSTACLE && control < maxmovement && tile->GetTileStatus() != TileStatus::UNIT)
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

void ASC_HumanPlayer::DfsVisitTiles2(TArray<ATile*>& listoftiles, ATile* visitedtile, int32 maxrange, int32& control)
{
	TArray<ATile*> linked_tiles;
	linked_tiles = visitedtile->GetLinkedTiles();
	for (ATile* tile : linked_tiles)
	{
		if(control < maxrange)
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

void ASC_HumanPlayer::DeselectUnitOnClick()
{
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked!"));
	FHitResult hit = FHitResult(ForceInit);
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (AUnit* currunit = Cast<AUnit>(hit.GetActor()))
		{
			if (currunit->GetTile()->GetColorOfTile() != FLinearColor::Red)
				// deselection with click on unit occurs if the clicked unit isn't on an attackable tiles (red tile)
			{
				ASC_HumanPlayer::DeselectUnit();
				GameMode->HCanMove = false;
				GameMode->HCanAttack = false;
				GameMode->HCanSelectUnit = true;
				GameMode->HCanDeselectUnit = false;
			}
		}
		else if (ATile* currtile = Cast<ATile>(hit.GetActor()))
		{
			if (currtile->GetColorOfTile() != FLinearColor::Green && currtile->GetColorOfTile() != FLinearColor::Red)
				// deselection with click on tile occurs if the clicked tile isn't reachable or attackable
			{
				ASC_HumanPlayer::DeselectUnit();
				GameMode->HCanMove = false;
				GameMode->HCanAttack = false;
				GameMode->HCanSelectUnit = true;
				GameMode->HCanDeselectUnit = false;
			}
		}
		else
			// in case the click hit out from the grid
		{
			ASC_HumanPlayer::DeselectUnit();
			GameMode->HCanMove = false;
			GameMode->HCanAttack = false;
			GameMode->HCanSelectUnit = true;
			GameMode->HCanDeselectUnit = false;
		}

	}
}

void ASC_HumanPlayer::DeselectUnit()
{
	for (AUnit* unit : ListOfUnits)
		if(unit != nullptr)
			unit->GetTile()->ChangeColor(FLinearColor::White);
	for (ATile* tile : WalkableTiles)
		tile->ChangeColor(FLinearColor::White);
	for (ATile* tile : TileInRange)
		tile->ChangeColor(FLinearColor::White);
}

void ASC_HumanPlayer::MoveOnClick()
{
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked!"));
	FHitResult hit = FHitResult(ForceInit);
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (ATile* currtile = Cast<ATile>(hit.GetActor()))
		{
			if (currtile->GetColorOfTile() == FLinearColor::Green)
				// walkable tile must be selected
			{
				GameMode->InputEnabled = false;
				// disabiliting al the inputs, so that the movement gets carried out without interruptions
				ATile* root_tile = nullptr;
				AUnit* MyUnit = nullptr;
				int32 radius = 0;
				for (auto tile : GameMode->gamefield->TileArray)
				{
					if (tile->GetColorOfTile() == FLinearColor::Yellow)
						// the tile where there is the selected unit
						root_tile = tile;
				}
				for (auto unit : ListOfUnits)
				{
					if (unit->GetTile() == root_tile)
						// unit that human is controlling
						MyUnit = unit;
				}
				// Writing the message of movement in the move history 
				char pos1 = MyUnit->GetTile()->GetTileId().first;
				FString Pos1 = FString(1, &pos1);
				int32 pos2 = MyUnit->GetTile()->GetTileId().second;
				char newpos1 = currtile->GetTileId().first;
				FString Newpos1 = FString(1, &newpos1);
				int32 newpos2 = currtile->GetTileId().second;
				int32 type = MyUnit->GetAttackType();
				FString TypeName;
				if (type == 0)
					TypeName = "S";
				else
					TypeName = "B";
				FString mess = FString::Printf(TEXT("HP: %s %s%d -> %s%d "), *TypeName, *Pos1, pos2, *Newpos1, newpos2);
				GameInst->AddMessageToHistory(mess, true);
				MyUnit->SetPlayed(true);// unit has been played
				radius = MyUnit->GetMaxMov();
				TArray<ATile*> minroute = ASC_HumanPlayer::MinRoute(root_tile, currtile, radius);
				ASC_HumanPlayer::DeselectUnit();// showing only the path to walk
				root_tile->ChangeColor(FLinearColor::Yellow);
				for(auto tile: minroute)
					tile->ChangeColor(FLinearColor::Green);
				// using class variables to access more easly info for phase after movement
				TileAux = root_tile;
				UnitAux = MyUnit;
				ArrayTileAux = minroute;
				UnitAux->SetPathToFollow(ArrayTileAux);
				UnitAux->SetIfIsMoving(true);// movement is carried out (Unit.cpp file -> tick())
			}
		}
	}
}

void ASC_HumanPlayer::AttackingOrEndingTurn()
{
	if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
		return;
	FTimerHandle Timer;// this delay allows more realistical behaviour
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
				// ensures that function doesn't do anything if executed after EndTurn -> turn must end
				return;
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			ASC_HumanPlayer::DeselectUnit();// path followed is cancelled
			// updating unit and tile information -> movement concluded
			TileAux->setowner(-1);// previously occupied tile
			TileAux->SetTileStatus(TileStatus::EMPTY);
			// ArrayTileAux store the followed path, the last element(tile) is the final destination of the unit
			UnitAux->SetTileUnit(ArrayTileAux.Last());
			ArrayTileAux.Last()->setowner(playernumber);
			ArrayTileAux.Last()->SetTileStatus(TileStatus::UNIT);
			// from the new position must check if the unit can attack opponents
			ASC_HumanPlayer::SelectUnit(UnitAux, false);
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
			GameMode->InputEnabled = true;// logic steps terminated can accept again inputs (disabled in MoveOnClick)
			if (contr > 0)
				// at least one tile in range has an enemy unit on top -> can attack
			{
				GameInst->ChangeVisibilityEndTurnButton(true);
				GameMode->HCanMove = false;
				GameMode->HCanAttack = true;// possibility to call end turn or attack
				GameMode->HCanSelectUnit = false;
				GameMode->HCanDeselectUnit = false;
			}
			else
				// no attack is possible go to the other unit, or terminate the turn if other unit has already been played
			{
				ASC_HumanPlayer::DeselectUnit();
				// recovering other unit to play, the unit just played is stored in UnitAux
				for (auto unit : ListOfUnits)
				{
					if (unit != UnitAux && unit != nullptr)
						// other unit to play found
					{
						UnitAux = unit;
						break;
					}
				}
				if (!UnitAux->PLayed())
					// checking if other unit hasn't been played yet 
				{
					// unit not played
					UnitAux->SetPlayed(true);
					GameMode->HCanMove = true;
					GameMode->HCanAttack = true;
					GameMode->HCanSelectUnit = false;// last unit to play is forcibly the one to be selected and played
					GameMode->HCanDeselectUnit = false;// previous unit has been already played
					SelectUnit(UnitAux, true);
					int32 control = 0;
					for (auto tile : TileInRange)
					{
						if (tile->GetColorOfTile() == FLinearColor::Red)
							control = 1;
					}
					if (control == 0 && WalkableTiles.IsEmpty())
					{
						// deadlock -> cannot unlock anymore the situation since 
						// I already played the other unit, no unlock is possible
						GameMode->HCanMove = false;
						GameMode->HCanAttack = false;
						GameMode->HCanSelectUnit = false;
						GameMode->HCanDeselectUnit = false;
						ASC_HumanPlayer::OnDeadLock();
					}
					else
					{
						// no deadlock
						BlockedUnits[UnitAux] = false;// in case it was previously deadlocked
					}
				}
				else
					// all units have already been played 
					// ending human turn
				{
					// resetting class variables
					UnitAux = nullptr;
					TileAux = nullptr;
					ArrayTileAux.Empty();
					GameMode->HCanMove = false;
					GameMode->HCanAttack = false;
					GameMode->HCanSelectUnit = false;
					GameMode->HCanDeselectUnit = false;
					IsMyTurn = false;
					GameMode->NextPlayerPlaying();
				}
			}
		}, 0.75f, false);
}

TArray<ATile*>ASC_HumanPlayer::MinRoute(ATile* RootTile, ATile* ClickedTile, int32 radius) {
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
	ShortestPath.Insert(ClickedTile, 0);// the tiles are inserted from the head, starting from the last to be reached,
	// this way the first tile in the array will be also the first right after the RootTile
	int32 distance = Distances[ClickedTile] - 1;// since already the last tile has been added, the distance is decremented by one
	ATile* tile = ClickedTile;

	while (distance > 0)
	{
		tile = Precedent[tile];
		ShortestPath.Insert(tile, 0);
		distance--;
	}
	return ShortestPath;
}

void ASC_HumanPlayer::AttackOnClick()
{
	FPlatformProcess::Sleep(0.5f);
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked attacking!"));
	FHitResult hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (AUnit* attackedunit = Cast<AUnit>(hit.GetActor()))
		{
			if (attackedunit->GetTile()->GetColorOfTile() == FLinearColor::Red)
				// attack can get carried out only on enemy unit in range
			{
				GameInst->ChangeVisibilityEndTurnButton(false);
				UnitAux->SetPlayed(true);// unit is played
				ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
				ASC_HumanPlayer::DeselectUnit();// deselecting red tiles if unit was moved (path was shown),
				// otherwise in case no movement was done just deselecting all the selected units 
				attackedunit->GetTile()->ChangeColor(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f));
				int32 DamageDealt = FMath::RandRange(UnitAux->GetAttackDR().X, UnitAux->GetAttackDR().Y);// extracting the dealt damage
				attackedunit->SetPointLife(attackedunit->GetHealth() - DamageDealt);
				GameInst->SetHealth(FString::FromInt(attackedunit->GetHealth()), false, attackedunit->GetAttackType());
				// printing the action of attack on the move history
				char pos1 = attackedunit->GetTile()->GetTileId().first;
				FString Pos1 = FString(1, &pos1);
				int32 pos2 = attackedunit->GetTile()->GetTileId().second;
				int32 type = UnitAux->GetAttackType();
				FString TypeName;
				if (type == 0)
					TypeName = "S";
				else
					TypeName = "B";
				FString mess = FString::Printf(TEXT("HP: %s %s%d %d "), *TypeName, *Pos1, pos2, DamageDealt);
				GameInst->AddMessageToHistory(mess, true);
				int32 CounterDamage = 0;// if no CounterAttack is carried out CounterDamage is zero
				if(UnitAux->GetAttackType() == 0)
					// unit is a sniper, then the CounterAttack mechanic takes place 
				{
					CounterDamage = FMath::RandRange(1, 3);
				}
				TArray<ATile*> NearTiles = attackedunit->GetTile()->GetLinkedTiles();// storing tiles near the attacked unit
				// used in case attacked unit is a brawler
				NearTiles.Append(attackedunit->GetTile()->GetDLinkedTiles());
				bool IsNear = false;// variable to know wether attacking unit is next to attacked unit
				for (auto tile : NearTiles)
				{
					if (UnitAux->GetTile() == tile)
						// attacking unit is near to attacked unit
						IsNear = true;
				}
				if (attackedunit->GetAttackType() == 0 || IsNear)
					// CounterDamage is applied only if: enemy is a sniper or attacking unit is next to attacked unit (in any case)
				{
					UnitAux->SetPointLife(UnitAux->GetHealth() - CounterDamage);
					GameInst->SetHealth(FString::FromInt(UnitAux->GetHealth()), true, UnitAux->GetAttackType());
				}
				if (attackedunit->GetHealth() <= 0)
					// attacked unit is killed
				{
					GameInst->SetHealth(FString::FromInt(0), false, attackedunit->GetAttackType());
					GameInst->SetDead(false, attackedunit->GetAttackType(), true);
					GameMode->Players[1]->RemoveUnitFromList(attackedunit);
					attackedunit->GetTile()->setowner(-1);// tile is free
					attackedunit->GetTile()->SetTileStatus(TileStatus::EMPTY);// tile is empty
					attackedunit->Destroy();
					bool bControl = false;
					GameMode->Players[1]->CheckIfListIsEmpty(bControl);
					if (bControl)
						// Ai lost all units, game over
					{
						GameMode->IsGameOver = true;
						AiLost = true;
					}
				}
				if (UnitAux->GetHealth() <= 0)
					// attacking unit is killed by counter attack
				{
					GameInst->SetHealth(FString::FromInt(0), true, UnitAux->GetAttackType());
					GameInst->SetDead(true, UnitAux->GetAttackType(), true);
					ListOfUnits.Remove(UnitAux);
					UnitAux->GetTile()->setowner(-1);// tile is free (unit killed)
					UnitAux->GetTile()->SetTileStatus(TileStatus::EMPTY);// tile is empty
					UnitAux->Destroy();
					if(ListOfUnits.IsEmpty())
						// checking if human lost all units
					{
						GameMode->IsGameOver = true;
						HLost = true;
					}
				}
				if(AiLost || HLost)
					// game ended
				{
					if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
						// ensures that function doesn't do anything if executed after EndTurn -> turn must end
						// or after Reset/ChangeSetting -> game must end
						return;
					FTimerHandle Timer;
					GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
						{
							if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
								return;
							ASG_GameMode* GameM = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
							ASC_HumanPlayer::DeselectUnit();
							GameM->GameReset(AiLost, HLost, false);
						}, 0.75f, false);
				}
				else
				{
					// no game over, selecting other unit or ending turn
					GameMode->InputEnabled = false;
					// since a timer is called, it will be executed after some seconds, for the time being no action must be registered
					FTimerHandle Timer;// timer for more realistcal effect, when selecting other not played unit or ending turn
					if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
						return;
					GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
						{
							if (bEndTurnButtonClicked || bResetOrChangeSettingsButtonClicked)
								return;
							ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
							GameMode->InputEnabled = true;// after timer is triggered inputs can be recived again
							bool OtherUnitNotPlayed = false;
							for (auto unit : ListOfUnits)
								if(unit != nullptr)
									if (!unit->PLayed())
										// other unit must still be played
									{
										UnitAux = unit;
										OtherUnitNotPlayed = true;
									}
							ASC_HumanPlayer::DeselectUnit();
							if (OtherUnitNotPlayed)
								// other unit must still be played
							{
								UnitAux->SetPlayed(true);
								GameMode->HCanMove = true;
								GameMode->HCanAttack = true;
								GameMode->HCanSelectUnit = false;// last unit to play is forcibly the one to be selected and played
								GameMode->HCanDeselectUnit = false;// previous unit has been already played
								SelectUnit(UnitAux, true);
							}
							else
								// other unit has already been played, or one of them is destroyed -> ending turn
							{
								GameMode->HCanMove = false;
								GameMode->HCanAttack = false;
								GameMode->HCanSelectUnit = false;
								GameMode->HCanDeselectUnit = false;
								IsMyTurn = false;
								GameMode->NextPlayerPlaying();
							}
						}, 1.5f, false);
				}
			}
		}
	}
}

void ASC_HumanPlayer::EndTurn()
{
	ASC_HumanPlayer::DeselectUnit();
	FTimerHandle Timer;// delay to make the selection of the new unit or the end turn not instantaneous
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
		{
			bEndTurnButtonClicked = false;// variable is reverted to initial value, 
			// no need to block execution of other timed functions anymore
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			bool OtherUnitNotPlayed = false;
			for (auto unit : ListOfUnits)
				if (!unit->PLayed())
					// other unit must still be played
				{
					UnitAux = unit;
					OtherUnitNotPlayed = true;
				}
			if (OtherUnitNotPlayed)
				// other unit must still be played
			{
				UnitAux->SetPlayed(true);
				GameMode->HCanMove = true;
				GameMode->HCanAttack = true;
				GameMode->HCanSelectUnit = false;// last unit to play is forcibly the one to be selected and played
				GameMode->HCanDeselectUnit = false;// previous unit has been already played
				SelectUnit(UnitAux, true);
			}
			else
				// other unit has already been played -> ending turn
			{
				GameMode->HCanMove = false;
				GameMode->HCanAttack = false;
				GameMode->HCanSelectUnit = false;
				GameMode->HCanDeselectUnit = false;
				IsMyTurn = false;
				GameMode->NextPlayerPlaying();
			}
		}, 0.75f, false);
}
/*
void ASC_HumanPlayer::EndTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Clicked!"));
	FHitResult hit = FHitResult(ForceInit);
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, hit);
	if (hit.bBlockingHit)
		UE_LOG(LogTemp, Warning, TEXT("Clicked on: %s"), *hit.GetActor()->GetName());
	if (hit.bBlockingHit && IsMyTurn)
	{
		if (false hit.Location)// ckeck if the area where there is the button has been hit
		{
			ASC_HumanPlayer::DeselectUnit();
			bool OtherUnitNotPlayed = false;
			for (auto unit : ListOfUnits)
				if (!unit->PLayed())
					// other unit must still be played
				{
					UnitAux = unit;
					OtherUnitNotPlayed = true;
				}
			if (OtherUnitNotPlayed)
				// other unit must still be played
			{
				UnitAux->SetPlayed(true);
				GameMode->HCanMove = true;
				GameMode->HCanAttack = true;
				GameMode->HCanSelectUnit = false;// last unit to play is forcibly the one to be selected and played
				GameMode->HCanDeselectUnit = false;// previous unit has been already played
				SelectUnit(UnitAux, true);
			}
			else
				// other unit has already been played -> ending turn
			{
				GameMode->HCanMove = false;
				GameMode->HCanAttack = false;
				GameMode->HCanSelectUnit = false;
				GameMode->HCanDeselectUnit = false;
				IsMyTurn = false;
				GameMode->NextPlayerPlaying();
			}
		}
	}
}
*/

void ASC_HumanPlayer::OnWin()
{
	GameInst->SetWinMessage(TEXT("Human Wins!"), true);
	GameInst->IncrementScoreHumanPlayer();
	GameInst->IncrementMatchesPlayed();
	// clearing "Dead" labels (both Ai and Human)
	GameInst->SetDead(true, 0, false);
	GameInst->SetDead(true, 1, false);
	GameInst->SetDead(false, 0, false);
	GameInst->SetDead(false, 1, false);
}

void ASC_HumanPlayer::OnLose()
{
	GameInst->SetTurnMessage(TEXT("Human Loses!"), true);
}

void ASC_HumanPlayer::OnDeadLock()
{
	// deadlock encountered 
	GameInst->SetWinMessage(TEXT("DeadLock -> Reset field"), true);
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->GameReset(false, false, true);
}


void ASC_HumanPlayer::OnTie()
{
	GameInst->SetWinMessage(TEXT("Game Tie!"), true);
	GameInst->IncrementMatchesPlayed();
}

void ASC_HumanPlayer::ResetValues()
{
	ASC_HumanPlayer::DeselectUnit();
	IsMyTurn = false;
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

void ASC_HumanPlayer::ClearTimers()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

