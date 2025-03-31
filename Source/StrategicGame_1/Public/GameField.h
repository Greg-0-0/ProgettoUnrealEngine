// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "Obstacle.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

//declaring a delegate to bind actions to the reset event (game over)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class STRATEGICGAME_1_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameField();

	//list of tiles in the gamefield
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	//list of obstacle in the gamefield (used to destroy them after game reset)
	UPROPERTY(Transient)
	TArray<AObstacle*> ObstacleArray;

	//given a position return the corresponding tile
	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	//used to spawn tiles at the right distance
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float NextCellPosMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TileSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CellPadding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	UPROPERTY(BlueprintAssignable)
	FOnReset OnResetEvent;

	// to ensure type safety when spawning tiles
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

	// to ensure type safety when spawning obstacles
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AObstacle> ObstacleClass;

	UPROPERTY(EditDefaultsOnly)
	float PercentageOfObstacles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TileZScale;

	// resetting game field (after game over)
	UFUNCTION(BlueprintCallable)
	void ResetField();

	// Considerations: the first tile of the grid (bottom left) is located in the zero world,
	// by (x, y) position it's meant the coordinates of a tile/obstacle, ranging from 0 to 24 each,
	// starting from bottom left position (0, 0).
	
	// method used to get (x,y) of tile/obstacle given its position realtive to the zero world
	FVector2D GetXYPositionByaRelativePos(const FVector& Location);
	// method used to get the position relative to the zero world of a tile/obstacle given its (x,y) position
	FVector GetRelativePositionByXYPos(const int32 xpos, const int32 ypos,bool Tile_Obs);
	// method used to get (x,y) of an obstacle/tile given its number (from 1 to 625)
	FVector2D GetXYPosByNumberPos(int32 notile);
	// method used to get number (from 1 to 625) of a tile/obstacle given its (x, y) position
	int32 GetNumberPosByXYPos(FVector2D X_Y_pos);
	// called when spawning tiles
	virtual void OnConstruction(const FTransform& Transform) override;
	// method used to get the tile number when a hitclick on the tile is detected
	FVector2D GetGridPosition(const FHitResult& hitresult);
	// Stores array of the tiles
	TArray<ATile*>& GetTileArray();
	// verifies the tile position is in the range size
	bool isValidPositionI(const FVector2D& position) const;
	// Generates the grid (it's called inside GameMode -> GenerateField)
	void FieldGenerator();
	// tile spawner
	void GenerateTile();
	// obstacle spawner
	void GenerateObstacles();
	// called to position obstacle in case PercentageOfObs is under 30%
	void ObsPositioning_1(int32 NoObs);
	// Function to scroll the tiles and check if they are correctly positioned
	void Positioner_Checker(int32 numobs);
	// called to check if all EMPTY tiles are reachable (after executing ObsPositioning_1)
	bool ObsChecker();
private:
	// used to link left and lower tile to the current one
	void LinkLeftNLower(ATile* currenttile,int32 xpos,int32 ypos);
	// used to link current tile and its left-upper one to its previous tile
	void LinkRightNUpper(ATile* currenttile, int32 xpos, int32 ypos);
	// given a current tile it links to it diagonally lower tiles,
	// then the same tiles are linked to the tile two rows lower the "currenttile"
	void LinkLowerDiag(ATile* currenttile, int32 xpos, int32 ypos);
	// aux function  for ObsChecker
	void DFS_Visit(ATile* tile, int32 index, TArray<int32>& marked);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
