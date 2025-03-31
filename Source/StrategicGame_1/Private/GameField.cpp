// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "SG_GameMode.h"
#include <random>

// Sets default values
AGameField::AGameField()
{
	PrimaryActorTick.bCanEverTick = false;
	FieldSize = 25;
	CellPadding = 0.2f;
	TileSize = 120.0f;
	ObstacleSize = 110.0f;
	TileZScale = 0.2f;
	NextCellPosMultiplier = 0;
	PercentageOfObstacles = 0;
}

TArray<ATile*>& AGameField::GetTileArray()
{
	return TileArray;
}

bool AGameField::isValidPositionI(const FVector2D& position) const
{
	return position.X <= FieldSize-1 && position.X >= 0 || position.Y >= 0 && position.Y <= FieldSize-1;
}

void AGameField::OnConstruction(const FTransform& Transform)
{ 
	Super::OnConstruction(Transform);
	// considering the first tile located in the zero world:
	NextCellPosMultiplier = (TileSize + CellPadding * TileSize) / TileSize;
}

FVector2D AGameField::GetGridPosition(const FHitResult& hitresult)
{
	return Cast<ATile>(hitresult.GetActor())->GetTilePos();
}

FVector AGameField::GetRelativePositionByXYPos(const int32 xpos, const int32 ypos,bool Tile_Obs)
{
	if(Tile_Obs)
		return NextCellPosMultiplier * TileSize * FVector(xpos, ypos, 0);
	else
		return NextCellPosMultiplier * TileSize * FVector(xpos, ypos, 0.04f);
}

FVector2D AGameField::GetXYPositionByaRelativePos(const FVector& Location)
{
	const double xpos = Location.X / (NextCellPosMultiplier * TileSize);
	const double ypos = Location.Y / (NextCellPosMultiplier * TileSize);
	return FVector2D(xpos, ypos);
}

FVector2D AGameField::GetXYPosByNumberPos(int32 number)
{
	int32 GridRow = number / FieldSize;
	int32 Offset = number - (GridRow * FieldSize);
	if (Offset == 0)
		if(GridRow != 0)
			return FVector2D(FieldSize - 1, GridRow - 1);
		else
			return FVector2D(FieldSize - 1, 0);
	else
		if(GridRow != 0)
			return FVector2D(Offset -1, GridRow);	
		else
			return FVector2D(Offset - 1, 0);
}

int32 AGameField::GetNumberPosByXYPos(FVector2D X_Y_pos)
{
	return X_Y_pos.Y * FieldSize + X_Y_pos.X + 1;
}

void AGameField::BeginPlay()
{
	Super::BeginPlay();
}

void AGameField::FieldGenerator()
{ 
	AGameField::GenerateTile();
	AGameField::GenerateObstacles();
}

void AGameField::GenerateTile()
{
	// checking that TileClass has been correctly created
	if(GetWorld() && TileClass)
	{
		for (int32 IndexY = 0; IndexY < FieldSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < FieldSize; IndexX++)
			{
				FVector Location = AGameField::GetRelativePositionByXYPos(IndexY, IndexX,true);
				Location = FVector(Location.X - TileSize, Location.Y - (TileSize * 10.0f), Location.Z);// offset
				ATile* Obj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
				const double TileScale = TileSize / 100.0f;
				const double Zscaling = TileZScale;
				Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
				Obj->SetActorRotation(FRotator::ZeroRotator);
				Obj->SetTileId(IndexX + 65, IndexY + 1);
				Obj->SetTilePos(IndexX, IndexY);
				TileArray.Add(Obj);
				TileMap.Add(FVector2D(IndexX, IndexY), Obj);
				AGameField::LinkLeftNLower(Obj, IndexX, IndexY);
				AGameField::LinkRightNUpper(Obj, IndexX, IndexY);
				if (IndexY != 0)
					// tile not in the first row
					AGameField::LinkLowerDiag(Obj, IndexX, IndexY);
			}
		
		}
	}
}

void AGameField::LinkLeftNLower(ATile* currenttile, int32 xpos, int32 ypos)
{
	if (ypos != 0)
		// get lower tile
		if (TileMap.Find(FVector2D(xpos, ypos - 1)) != nullptr)
			currenttile->LinkTile(TileMap[FVector2D(xpos, ypos - 1)]);
	if (xpos != 0)
		// get left tile
		if (TileMap.Find(FVector2D(xpos - 1, ypos)) != nullptr)
			currenttile->LinkTile(TileMap[FVector2D(xpos - 1, ypos)]);
}

void AGameField::LinkRightNUpper(ATile* currenttile, int32 xpos, int32 ypos)
{
	if (xpos != 0)
		// previous tile of the current tile (in the same row) links current tile to self as right tile
		if(TileMap.Find(FVector2D(xpos - 1, ypos)) != nullptr)
		{
			ATile* prevtile = TileMap[FVector2D(xpos - 1, ypos)];
			prevtile->LinkTile(currenttile);
		}
	if (ypos != 0)
		// lower tile of the current tiel (in the same column) links current tile to self as upper tile
		if (TileMap.Find(FVector2D(xpos, ypos - 1)) != nullptr)
		{
			ATile* lowertile = TileMap[FVector2D(xpos, ypos - 1)];
			lowertile->LinkTile(currenttile);
		}
}

void AGameField::LinkLowerDiag(ATile* currenttile, int32 xpos, int32 ypos)
{
	if (xpos != 0)
		// tile not in the first coulmn
	{
		currenttile->LinkDTile(TileMap[FVector2D(xpos - 1, ypos - 1)]);// linking diagonal left lower  tile
		if (ypos > 1)
			// tile not in first or second row -> there is a tile in the same column two rows lower
		{
			ATile* prev_tile = TileMap[FVector2D(xpos, ypos - 2)];// getting the two rows lower tile
			prev_tile->LinkDTile(TileMap[FVector2D(xpos - 1, ypos - 1)]);// linking diagonal left lower  tile
		}
	}
	if (xpos != FieldSize - 1)
		// tile not in the last coulmn
	{
		currenttile->LinkDTile(TileMap[FVector2D(xpos + 1, ypos - 1)]);// linking diagonal right lower  tile
		if (ypos > 1)
			// tile not in first or second row -> there is a tile in the same column two rows lower
		{
			ATile* prev_tile = TileMap[FVector2D(xpos, ypos - 2)];
			prev_tile->LinkDTile(TileMap[FVector2D(xpos + 1, ypos - 1)]);// linking diagonal right lower  tile
		}
	}
}

void AGameField::GenerateObstacles()
{
	int32 NObs = ((FieldSize * FieldSize) * PercentageOfObstacles) / 100;
	if (NObs <= 80)// PercentageOfObstacles <= 30%
	{
		AGameField::ObsPositioning_1(NObs);
		int32 loop1 = 0;
		while(!AGameField::ObsChecker())
		{
			loop1++;
			for (ATile* tile : TileArray)
				tile->SetTileStatus(TileStatus::EMPTY);
			AGameField::ObsPositioning_1(NObs);
		}
	}
	else
		AGameField::Positioner_Checker(NObs);
	int32 obs_spawned = 0;

	for (ATile* tile: TileArray)
	{
		if(tile->GetTileStatus() == TileStatus::OBSTACLE)
		{
			obs_spawned++;
			FVector Location = tile->GetActorLocation();
			Location.Z = 10;
			AObstacle* Obj = GetWorld()->SpawnActor<AObstacle>(ObstacleClass, Location, FRotator::ZeroRotator);
			const double ObsScale = ObstacleSize / 100.0f;
			const double Zscaling = TileZScale;
			Obj->SetActorScale3D(FVector(ObsScale, ObsScale, Zscaling));
			Obj->SetActorRotation(FRotator::ZeroRotator);
			ObstacleArray.Add(Obj);
		}
	}
}

void AGameField::ObsPositioning_1(int32 NoObs)
{
	TArray<ATile*> occupied_tiles;
	int32 NObstacles = NoObs;
	srand(time(NULL));
	for (int32 Index = 0; Index < NObstacles; Index++)
	{
		int32 ObsGridNumb = FMath::RandRange(1, (FieldSize * FieldSize));
		FVector2D ObsRelPos = AGameField::GetXYPosByNumberPos(ObsGridNumb);
		ATile* tile = nullptr;
		if (TileMap.Find(ObsRelPos) != nullptr)
		{
			tile = TileMap[ObsRelPos];
			if (tile->GetTileStatus() != TileStatus::OBSTACLE)
				// tile is free
			{
				tile->SetTileStatus(TileStatus::OBSTACLE);
				occupied_tiles.Add(tile);
			}
			else
				// tile is occupied, must choose another number
				Index--;
		}
	}
	return;
}

bool AGameField::ObsChecker()
{
		TArray<int32> marked; marked.Empty();
		int32 NTile = FieldSize * FieldSize; 
		bool called = false;
		for (int32 index = 0; index < NTile; index++)
			marked.Add(0);
		for (int32 index = 0; index < NTile; index++)
		{
			if (called)
				break;
			if (marked[index] == 0 && TileArray[index]->GetTileStatus() != TileStatus::OBSTACLE)
			{
				DFS_Visit(TileArray[index], index, marked);
				called = true;
			}
		}
		// checking if all the tiles have been visited
		for (int32 index = 0; index < NTile; index++)
		{
			if (marked[index] != 2 && TileArray[index]->GetTileStatus() != TileStatus::OBSTACLE)
				return false;
		}
		return true;

}

void AGameField::DFS_Visit(ATile* tile, int32 index, TArray<int32>& marked)
{
	marked[index] = 1;//grey -> visiting
	TArray<ATile*> linked_tiles = tile->GetLinkedTiles();
	for (ATile* linked_t : linked_tiles)
	{
		int32 temp = AGameField::GetNumberPosByXYPos(linked_t->GetTilePos());
		if (marked[temp - 1] == 0 && linked_t->GetTileStatus() != TileStatus::OBSTACLE)
			DFS_Visit(linked_t, temp - 1, marked);
	}
	marked[index] = 2;//black -> visited
}

void AGameField::Positioner_Checker(int32 numobs) {

	FVector2D xy_pos = FVector2D(0, 0);
	for (numobs; numobs != 0;numobs--)
	{
		int32 num_pos = FMath::RandRange(1,625);
		xy_pos = AGameField::GetXYPosByNumberPos(num_pos);
		if (TileMap.Find(xy_pos) != nullptr)
		{
			if (TileMap[xy_pos]->GetTileStatus() != TileStatus::EMPTY)
			{
				numobs++;
				continue;
			}
		}
		else
		{
				numobs++;
				continue;
		}
		TileMap[xy_pos]->SetTileStatus(TileStatus::OBSTACLE);
		if (!AGameField::ObsChecker())
		{
			TileMap[xy_pos]->SetTileStatus(TileStatus::EMPTY);
			numobs++;
		}
	}
}

/*
void AGameField::ObsPositioning_2()
{
	int32 NObstacles = ((FieldSize * FieldSize) * PercentageOfObstacles) / 100;
	//int32 n = TileArray.GetAllocatedSize()-1;// check GetAllocatedSize
	for (ATile* tile : TileArray)
		// all tiles are occupied at first
		tile->SetTileStatus(TileStatus::OBSTACLE);
	if (NObstacles == (FieldSize * FieldSize))
		// field is filled with obstacle
		return;
	// then take a random tile and from there empty the tiles
	//srand(time(NULL));
	//int32 NTile = rand() % (FieldSize * FieldSize) + 1;// <- rand
	int32 NTile = FMath::RandRange(1, (FieldSize * FieldSize));
	FVector2D tile_numb = AGameField::GetXYPosByNumberPos(NTile);
	if (TileMap.Find(tile_numb) != nullptr)
		TileMap[tile_numb]->SetTileStatus(TileStatus::EMPTY);
	if (NObstacles == (FieldSize * FieldSize) - 1)
		// there is only one empty tile
		return;
	TArray<ATile*> empty_tiles;
	empty_tiles.Add(TileMap[tile_numb]);
	int32 loop2 = 0;
	for (int32 i = 0; i < (FieldSize * FieldSize) - 2 - (NObstacles - 1); i++)
	{
		loop2++;
		//int32 n_current_tile = rand() % empty_tiles.Num();// returns randoma number from 0 to size // GetAllocatedSize
		int32 n_current_tile = FMath::RandRange(0, empty_tiles.Num() - 1);
		ATile* current_tile = empty_tiles[n_current_tile];
		TArray<ATile*> aux_ = current_tile->GetLinkedTiles();
		int32 n = aux_.Num();
		int32 n_next_tile = FMath::RandRange(0, n - 1);
		//int32 n_next_tile = rand() % n;// returns randoma number from 0 to n
		int32 counter = n;
		while (aux_[n_next_tile]->GetTileStatus() != TileStatus::OBSTACLE && counter != 0)
		{
			if (n_next_tile == n - 1)
				n_next_tile = 0;
			else
				n_next_tile++;
			counter--;
		}
		if (counter == 0)
		{
			if (n_current_tile != empty_tiles.Num() - 1)
				n_current_tile++;
			else
				n_current_tile = 0;
			i--; continue;
		}
		//ATile* next_tile = aux_[n_next_tile];
		aux_[n_next_tile]->SetTileStatus(TileStatus::EMPTY);
		empty_tiles.Add(aux_[n_next_tile]);
	}
	//return empty_tiles;// real tiles are stored inside
}
*/

void AGameField::ResetField()
{
	// calling all the methods linked to the multicast dynamic delegate OnReset
	OnResetEvent.Broadcast();
	// resetting field
	for (auto obs : ObstacleArray)
		obs->SelfDestroy();
	ObstacleArray.Empty();
	for (auto tile : TileArray)
	{
		tile->setowner(-1);
		tile->SetTileStatus(TileStatus::EMPTY);
	}
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]()
	{
		// must be possible to chose new percentageofobs
		// generating new field
		AGameField::GenerateObstacles();
		// call gamemode and restart a game
		ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
		GameMode->ChoseAi();

	}, 0.75f, false);
	

	// obstacle are erased and respawned, eventually with a new percentageofobstacle -> HUD necessary
	// reset value tiles
}
// Called every frame
//void AGameField::Tick(float DeltaTime){Super::Tick(DeltaTime);}

