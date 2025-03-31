// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "GameFramework/Actor.h"
#include "Unit.generated.h"

UCLASS()
class STRATEGICGAME_1_API AUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUnit();
	// Returns the tile where the unit is positioned
	ATile* GetTile();// this way every information related to the tile is accessibile
	int32 GetUnitOwner();// this way it's easier to retrive this info
	// Returns the mazimum range of movement
	int32 GetMaxMov();
	// Returns the attack type: long range -> 0, short range -> 1
	int32 GetAttackType();
	// Returns the maximum range of attack
	int32 GetMaxAttackR();
	// Returns the Damage Range
	FVector2D GetAttackDR();
	int32 GetHealth();
	// Tells if the unit can move
	bool IsMoving();
	// Tells if the unit has been played
	bool PLayed();
	// function to initialize in order:
	// tile of unit, owner, max movement, short-ranged/long-ranged, Mar range
	void SetParameters(ATile* tileunit, int32 owner, int32 maxmov, int32 attacktype, int32 maxrange);
	void SetDamageRange(int32 minvaldamage, int32 maxvaluedamage);
	void SetPointLife(int32 pointlife);
	void SetTileUnit(ATile* tileunit);
	void SetIfIsMoving(bool ismoving);
	void SetPlayed(bool played);
	// Function used to set the path the unit has to follow
	void SetPathToFollow(TArray<ATile*> pathtoloc);

	UFUNCTION()
	void SelfDestroy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(Transient)
	ATile* TileOfUnit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 UnitOwner;// 0 -> Human Player, 1 -> Ai Player

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxMovement; // calculated starting from the tile where the unit is stationed

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AttackType; // long range = 0, short range = 1

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxAttackRange; // calculated starting from the tile where the unit is stationed

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D AttackDamageRange; // x = minimum attack damage, y = maximum attack damage

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PointLife;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsMoving;

	float Alpha;// value used to interpolate between position to move
	float Speed;// tells the speed at which the unit moves
	bool bPlayed;// tells if the unit already moved/has attacked

	TArray<ATile*> PathToFollow;
	int32 IndexOfPath;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
