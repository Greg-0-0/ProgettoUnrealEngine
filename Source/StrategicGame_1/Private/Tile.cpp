// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//a mesh created in the engine is assigned to this actor's mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = Scene;//set as transform of the actor
	Mesh->SetupAttachment(RootComponent);
	PlayerOwner = -1;
	Tilestatus = TileStatus::EMPTY;
	TilePosition = FVector2D(0, 0);
	TileId = std::make_pair(-1, '-');
	DynamicMaterialInstance = nullptr;
	ColorOfTile = FLinearColor::White;
}

TileStatus ATile::GetTileStatus()
{
	return Tilestatus;
}

void ATile::SetTileStatus(TileStatus tilestatus)
{
	Tilestatus = tilestatus;
}

FVector2D ATile::GetTilePos()
{
	return TilePosition;
}

void ATile::SetTilePos(double const xpos, double const ypos)
{
	TilePosition.Set(xpos, ypos);
}

int32 ATile::GetTileOwner()
{
	return PlayerOwner;
}

void ATile::setowner(int32 playerowner){PlayerOwner = playerowner;}

std::pair<char, int32> ATile::GetTileId() const
{
	return TileId;
}

void ATile::SetTileId(char Letter, int32 Index)
{
	TileId.first = Letter; TileId.second = Index;
}

const TArray<ATile*>& ATile::GetLinkedTiles()
{
	return LinkedTiles;
}

const TArray<ATile*>& ATile::GetDLinkedTiles()
{
	return DLinkedTiles;
}

void ATile::LinkDTile(ATile* new_dtile)
{
	DLinkedTiles.Add(new_dtile);
}

FLinearColor ATile::GetColorOfTile()
{
	return ColorOfTile;
}

void ATile::LinkTile(ATile* new_tile)
{
	LinkedTiles.Add(new_tile);
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	if (Mesh)
	{
		UMaterialInterface* BaseMaterial = Mesh->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, 0, TEXT("DynamicMaterialTile"));
			if (DynamicMaterialInstance)
				Mesh->SetMaterial(0, DynamicMaterialInstance);
		}
	}
}

void ATile::ChangeColor(FLinearColor newcolor)
{ 
	if(DynamicMaterialInstance)
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("Color"), newcolor);
	ColorOfTile = newcolor;
}

