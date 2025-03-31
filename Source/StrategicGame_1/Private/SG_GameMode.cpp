// Fill out your copyright notice in the Description page of Project Settings.


#include "SG_GameMode.h"
#include "SC_PlayerController.h"
//#include "SC_NaiveAIPlayer.h"
// usage of TEXT in HumanPLayer in MoveOnClick
// 


ASG_GameMode::ASG_GameMode()
{
	PlayerControllerClass = ASC_PlayerController::StaticClass();
	DefaultPawnClass = ASC_HumanPlayer::StaticClass();
	gamefield = nullptr;
	IsGameOver = false;
	HumanUnits = nullptr;
	ai_type = -1;
	currentplayer = -1;
	UnitsNotPlaced = 4;
	FirstPlayer = -1;
	GamePhase = -1;
	HCanDeselectUnit = false;
	HCanSelectUnit = false;
	HCanMove = false;
	HCanAttack = false;
	InputEnabled = true;
	Hcontrol = 0;
	AIcontrol = 0;
	AILost = false;
	HLost = false;
	UserHudInstance = nullptr;
	MenuInstance = nullptr;
	HumanPlayer = nullptr;
}

void ASG_GameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("GameMode started."));
	IsGameOver = false;
	ASC_HumanPlayer* humanplayer = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("GetFirstPlayerController 1"));
	humanplayer = GetWorld()->GetFirstPlayerController()->GetPawn<ASC_HumanPlayer>();
	if (!IsValid(humanplayer))
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn of type '%s' was found."), *ASC_HumanPlayer::StaticClass()->GetName());
		return;
	}
	if(GetWorld())
	{
		if (GameFieldClass != nullptr)
		{
			gamefield = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameField non existent."));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed creting world."));
		return;
	}
	float camera_x_pos = ((gamefield->FieldSize * gamefield->TileSize + ((gamefield->FieldSize - 1) * 
		gamefield->CellPadding * gamefield->TileSize)) * 0.5f) - gamefield->TileSize;
	float camera_y_pos = camera_x_pos - 10.85f * gamefield->TileSize;
	float camera_z_pos = camera_x_pos * 2.0f + gamefield->TileSize;
	camera_x_pos = camera_x_pos - gamefield->TileSize;
	// postioning slightly above the cube created by the field, so all the field can be seen
	FVector CameraPos_HPos = FVector(camera_x_pos,camera_y_pos,camera_z_pos);
	humanplayer->SetActorLocationAndRotation(CameraPos_HPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());
	HumanPlayer = humanplayer;

	if (MenuClass)
	{
		MenuInstance = CreateWidget<USG_Menu>(GetWorld(), MenuClass);
		if (MenuInstance)
		{
			MenuInstance->AddToViewport();
		}
	}
	HumanUnits = &humanplayer->ListOfUnits;// passing the address of the array of human units (later used by SmartAi)
	Players.Add(humanplayer);
}

void ASG_GameMode::GenerateField()
{
	gamefield->FieldGenerator();
	this->ChoseAi();
}

void ASG_GameMode::ChoseAi()
{
	// chose ai opponent base on Menu choice
	// ai_type is defined inside OnPlayButtonClicked -> SG_Menu.cpp
	ASC_NaiveAIPlayer* NaiveAIPlayer = nullptr;
	ASG_SmartAIPlayer* SmartAIPlayer = nullptr;
	if (ai_type == 0)
	{
		NaiveAIPlayer = GetWorld()->SpawnActor<ASC_NaiveAIPlayer>(FVector(), FRotator());
		Players.Add(NaiveAIPlayer);
	}
	else
	{
		SmartAIPlayer = GetWorld()->SpawnActor<ASG_SmartAIPlayer>(FVector(),FRotator());
		SmartAIPlayer->ListOfHumanUnits = HumanUnits;// loading reference to list of human units
		Players.Add(SmartAIPlayer);
	}
		if (UserHudClass && UserHudInstance == nullptr)
		{
			// In this case the HUD hasn't been created yet -> first match
			UserHudInstance = CreateWidget<USG_HUD>(GetWorld(), UserHudClass);
			if (UserHudInstance)
			{
				UserHudInstance->AddToViewport();
			}
		}
		else
			// HUD has alredy been created -> it was hidden after Change Settings has been pressed
			UserHudInstance->ShowUHD();
		this->ChosePlayerAndStartGame();
}

int32 ASG_GameMode::Random(int32 dim)
{
	TArray<int32> values; values.Empty();
	for (int32 i = 0; i < 99; i++)
		values.Add(FMath::RandRange(0,dim));
	int32 a = values[FMath::RandRange(0, 98)];
	return a;
}

void ASG_GameMode::ChosePlayerAndStartGame()
{
	int32 dim = Players.Num() - 1;
	currentplayer = ASG_GameMode::Random(Players.Num() - 1);
	FirstPlayer = currentplayer;
	if (FirstPlayer == 0)
		UserHudInstance->SetWinMess(TEXT("Human starts"), true);
	else
	{
		if (ai_type == 0)
			UserHudInstance->SetWinMess(TEXT("Ai(Random) starts"), false);
		else
			UserHudInstance->SetWinMess(TEXT("Ai(Smart) starts"), false);
	}
	for (int32 index = 0; index < Players.Num(); index++)
	{
		Players[index]->playernumber = index;
		Players[index]->gamephase = 0;
	}
	GamePhase = 0;
	Players[currentplayer]->UnitPlacement(UnitsNotPlaced);
}

void ASG_GameMode::SpawnUnit(const int32 playernum, const FVector& SpawnPos, ATile* tileunit)
{
	if (playernum != currentplayer)
		return;
	// first to spawn is brawler
	FVector Location = gamefield->GetActorLocation() + SpawnPos + FVector(0, 0, 10);
	if (playernum == 0)
		// human player is placing
	{
		if (Hcontrol == 0)
			// spawning brawler
		{
			AUnit* brawler_spawned = GetWorld()->SpawnActor<AUnit>(HBrawler, Location, FRotator::ZeroRotator);
			const double UnitScale = (gamefield->ObstacleSize / 100.0f) + 0.3f;
			const double UnitZScale = gamefield->TileZScale;
			brawler_spawned->SetActorScale3D(FVector(UnitScale, UnitScale, UnitZScale));
			brawler_spawned->SetActorRotation(FRotator::ZeroRotator);
			brawler_spawned->SetParameters(tileunit, playernum, 6, 1, 1);
			brawler_spawned->SetDamageRange(1, 6);
			brawler_spawned->SetPointLife(40);
			UserHudInstance->SetHBrawlerHealth(FString::FromInt(40));
			Players[currentplayer]->AddUnitToList(brawler_spawned);
			Players[currentplayer]->BlockedUnits.Add(brawler_spawned, false);
			char pos1 = brawler_spawned->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = brawler_spawned->GetTile()->GetTileId().second;
			FString mess = FString::Printf(TEXT("HBrawler spawned at: %s%d"), *Pos1, pos2);
			UserHudInstance->AddMessageToScrollBox(mess, true);
			Hcontrol++;
			UnitsNotPlaced--;
		}
		else
			//spawning sniper
		{
			AUnit* sniper_spawned = GetWorld()->SpawnActor<AUnit>(HSniper, Location, FRotator::ZeroRotator);
			const double UnitScale = (gamefield->ObstacleSize / 100.0f) + 0.2f;
			const double UnitZScale = gamefield->TileZScale;
			sniper_spawned->SetActorScale3D(FVector(UnitScale, UnitScale, UnitZScale));
			sniper_spawned->SetActorRotation(FRotator::ZeroRotator);
			sniper_spawned->SetParameters(tileunit, playernum, 3, 0, 10);
			sniper_spawned->SetDamageRange(4, 8);
			sniper_spawned->SetPointLife(20);
			UserHudInstance->SetHSniperHealth(FString::FromInt(20));
			Players[currentplayer]->AddUnitToList(sniper_spawned);
			Players[currentplayer]->BlockedUnits.Add(sniper_spawned, false);
			char pos1 = sniper_spawned->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = sniper_spawned->GetTile()->GetTileId().second;
			FString mess = FString::Printf(TEXT("HSniper spawned at: %s%d"), *Pos1, pos2);
			UserHudInstance->AddMessageToScrollBox(mess, true);
			UnitsNotPlaced--;
		}
	}
	else
		// ai player is placing
	{
		if (AIcontrol == 0)
			// spawning brawler
		{
			AUnit* brawler_spawned = GetWorld()->SpawnActor<AUnit>(AIBrawler, Location, FRotator::ZeroRotator);
			const double UnitScale = (gamefield->ObstacleSize / 100.0f) + 0.3f;
			const double UnitZScale = gamefield->TileZScale;
			brawler_spawned->SetActorScale3D(FVector(UnitScale, UnitScale, UnitZScale));
			brawler_spawned->SetActorRotation(FRotator::ZeroRotator);
			brawler_spawned->SetParameters(tileunit, playernum, 6, 1, 1);
			brawler_spawned->SetDamageRange(1, 6);
			brawler_spawned->SetPointLife(40);
			//brawler_spawned->SetPointLife(1);
			UserHudInstance->SetAiBrawlerHealth(FString::FromInt(40));
			Players[currentplayer]->AddUnitToList(brawler_spawned);
			Players[currentplayer]->BlockedUnits.Add(brawler_spawned, false);
			char pos1 = brawler_spawned->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = brawler_spawned->GetTile()->GetTileId().second;
			FString mess = FString::Printf(TEXT("AIBrawler spawned at: %s%d"), *Pos1, pos2);
			UserHudInstance->AddMessageToScrollBox(mess, false);
			AIcontrol++;
			UnitsNotPlaced--;
		}
		else
			//spawning sniper
		{
			AUnit* sniper_spawned = GetWorld()->SpawnActor<AUnit>(AISniper, Location, FRotator::ZeroRotator);
			//const double UnitScale = gamefield->ObstacleSize / 100.0f; 
			const double UnitScale = (gamefield->ObstacleSize / 100.0f) + 0.2f;
			const double UnitZScale = gamefield->TileZScale;
			sniper_spawned->SetActorScale3D(FVector(UnitScale, UnitScale, UnitZScale));
			sniper_spawned->SetActorRotation(FRotator::ZeroRotator);
			sniper_spawned->SetParameters(tileunit, playernum, 3, 0, 10);
			sniper_spawned->SetDamageRange(4, 8);
			sniper_spawned->SetPointLife(20);
			UserHudInstance->SetAiSniperHealth(FString::FromInt(20));
			Players[currentplayer]->AddUnitToList(sniper_spawned);
			Players[currentplayer]->BlockedUnits.Add(sniper_spawned, false);
			char pos1 = sniper_spawned->GetTile()->GetTileId().first;
			FString Pos1 = FString(1, &pos1);
			int32 pos2 = sniper_spawned->GetTile()->GetTileId().second;
			FString mess = FString::Printf(TEXT("AISniper spawned at: %s%d"), *Pos1, pos2);
			UserHudInstance->AddMessageToScrollBox(mess, false);
			UnitsNotPlaced--;
		}
	}
	ASG_GameMode::NextPlayerPlacing(); // executions is blocked after placing 4 units
}

int32 ASG_GameMode::GetNextPlayer(int32 player)
{
	player++;
	if (player > Players.Num() - 1)
	{
		return 0;
	}
	return player;
}

void ASG_GameMode::NextPlayerPlacing()
{
	if (UnitsNotPlaced == 0)
		// all 4 units have been placed
	{
		GamePhase = 1;// next phase: movement/attack
		HCanSelectUnit = true;// selecting unit for a movement
		for (int32 index = 0; index < Players.Num(); index++)
		{
			Players[index]->gamephase = 1;
		}
		currentplayer = FirstPlayer;
		Players[currentplayer]->Move_Attack();
	}
	else
		// not all units have been placed
	{
		currentplayer = ASG_GameMode::GetNextPlayer(currentplayer);
		Players[currentplayer]->UnitPlacement(UnitsNotPlaced);
	}
}

void ASG_GameMode::NextPlayerPlaying()
{
	UserHudInstance->ToggleVisibilityEndTurnButton(false);
	currentplayer = ASG_GameMode::GetNextPlayer(currentplayer);
	Players[currentplayer]->Move_Attack();
}

void ASG_GameMode::GameReset(bool AiLost, bool hLost, bool ResetPressed)
{
	AILost = AiLost;
	HLost = hLost;
	UserHudInstance->ClearHUD();
	UserHudInstance->ToggleVisibilityEndTurnButton(false);
	FTimerHandle Timer1;
	GetWorld()->GetTimerManager().SetTimer(Timer1, [&]()
		{
			ISC_PlayerInterface* HIPlayer = nullptr;
			ISC_PlayerInterface* AIPlayer = nullptr;
			for (auto Interface : Players)
			{
				Interface->ResetValues();
				if (Interface->playernumber == 0)
					HIPlayer = Interface;
				if (Interface->playernumber == 1)
					AIPlayer = Interface;
			}
			if (AILost && !HLost && HIPlayer != nullptr &&
				AIPlayer != nullptr)
				// human wins
			{
				HIPlayer->OnWin();
				AIPlayer->OnLose();
			}
			else if (!AILost && HLost && HIPlayer != nullptr &&
				AIPlayer != nullptr)
				// ai wins
			{
				AIPlayer->OnWin();
				HIPlayer->OnLose();
			}
			else if (HIPlayer == nullptr && AIPlayer == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed Restart"));
			}
			else if (AILost && HLost)
				// tie
			{
				HIPlayer->OnTie();
			}
			else if (!AILost && !HLost)
				// button reset clicked or deadlock
			{
				UE_LOG(LogTemp, Warning, TEXT("Reset clicked or deadlock"));
			}
			// resetting class variables
			UnitsNotPlaced = 4;
			HCanDeselectUnit = false;
			HCanSelectUnit = false;
			HCanMove = false;
			HCanAttack = false;
			InputEnabled = true;
			Hcontrol = 0;
			AIcontrol = 0;
			IsGameOver = false;
			HLost = false;
			AILost = false;
			// deleting ai player since it will be readded in ChoseAi()
			Players.Remove(AIPlayer);
			//AiPlayer->Destroy();
		}, 1.5f, false);

	if(ResetPressed)
		// this way the reset execution is faster (it was deliberately called by a button)
	{
		FTimerHandle Timer3;
		GetWorld()->GetTimerManager().SetTimer(Timer3, [&]()
			{
				for (auto player : Players)
				{
					player->bResetOrChangeSettingsButtonClicked = false;
					// executed in 2.5 seconds since
					// the control returns to OnResetOrChangeSettingsButtonClicked, which terminates, then, 
					// if they have been previously queued different timed functions of human/ai player, they get executed with the bool flag active
					// and then this timed function is the next one to execute, no need to worry about timings since in OnResetOrChangeSettingsButtonClicked
					// all timers have been cleared, the only problem were timed functions already triggered while in OnResetOrChangeSettingsButtonClicked,
					// which will execute as soon as OnResetOrChangeSettingsButtonClicked terminates and are blocked by the bool flag
				}
				gamefield->ResetField();
			}, 1.6f, false);
	}
	else
		// in case the game ended because of a win/lose event,
		// more delay to allow the player to realize the game ended
	{
		FTimerHandle Timer2;
		GetWorld()->GetTimerManager().SetTimer(Timer2, [&]()
			{
				for (auto player : Players)
				{
					player->bResetOrChangeSettingsButtonClicked = false;
					// same thing as above, only difference is that OnResetOrChangeSettingsButtonClicked wasn't executed
				}
				gamefield->ResetField();
			}, 2.0f, false);
	}

}



