// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Tuple.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM()
enum class TileStatus : uint8
{
	EMPTY UMETA(DisplayName = "Empty tile"),
	UNIT UMETA(DisplayName = "Unit on tile"),
	OBSTACLE UMETA(DisplayName = "Obstacle on tile")
};


UCLASS()
class STRATEGICGAME_1_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();
	// Status can be: EMPTY / OBSTACLE / UNIT
	TileStatus GetTileStatus();
	void SetTileStatus(TileStatus tilestatus);
	// Returns tile position relative to the X axis(from left to right) and 
	// Y axis (from bottom to top) of the grid 
	FVector2D GetTilePos();
	void SetTilePos(double const xpos, double const ypos);
	// Return the owner of the unit on the tile by number (0 -> Human, 1 -> Ai, -1 -> free)
	int32 GetTileOwner();
	// Set the owner (0 -> Human, 1 -> Ai, -1 -> free)
	void setowner(int32 playerowner);
	// Returns the label of the tile: (letter, number),
	// where "letter" goes from A to Y (from left to right)
	// and "number" from 1 to 25 (from bottom to top)
	std::pair<char, int32> GetTileId() const;
	void SetTileId(char Letter, int32 Index);
	// Returns an array containing in order:
	// bottom linked tile, left linked tile, right linked tile, upper linked tile
	const TArray<ATile*>& GetLinkedTiles();
	void LinkTile(ATile* new_tile);
	// Returns an array containing the diagonally linked tiles
	const TArray<ATile*>& GetDLinkedTiles();
	void LinkDTile(ATile* new_dtile);
	FLinearColor GetColorOfTile();

	// Method to dynamically change the color of a tile
	UFUNCTION(BlueprintCallable)
	void ChangeColor(FLinearColor newcolor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	int32 PlayerOwner;// 0 for HumanPlayer, 1 for AiPlayer, -1 means no unit

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TileStatus Tilestatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TilePosition;

	// variabile to represent the directly linked tiles
	UPROPERTY(Transient)
	TArray<ATile*> LinkedTiles;

	// variabile to represent the diagonally linked tiles, 
	// useful to check if obstacles have been spawned properly
	UPROPERTY(Transient)
	TArray<ATile*> DLinkedTiles;

	// pair that defines tile position (index,letter)
	std::pair<char, int32> TileId;

	UMaterialInstanceDynamic* DynamicMaterialInstance;

	FLinearColor ColorOfTile;// stores the color of the tile
};
