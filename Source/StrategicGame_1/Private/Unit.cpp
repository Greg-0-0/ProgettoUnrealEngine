// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "SG_GameMode.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//a mesh created in the engine is assigned to this actor's mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(Scene);
	Mesh->SetupAttachment(RootComponent);
	TileOfUnit = nullptr;
	MaxMovement = -1;
	AttackType = -1;
	MaxAttackRange = -1;
	AttackDamageRange = FVector2D(0, 0);
	PointLife = -1;
	UnitOwner = -1;
	bIsMoving = false;
	bPlayed = false;
	Alpha = 0.0f;
	Speed = 0.3f;
	PathToFollow = TArray<ATile*>();
	IndexOfPath = 0;
}

ATile* AUnit::GetTile()
{
	return TileOfUnit;
}

int32 AUnit::GetUnitOwner()
{
	return UnitOwner;
}

int32 AUnit::GetMaxMov()
{
	return MaxMovement;
}

int32 AUnit::GetAttackType()
{
	return AttackType;
}

int32 AUnit::GetMaxAttackR()
{
	return MaxAttackRange;
}

FVector2D AUnit::GetAttackDR()
{
	return AttackDamageRange;
}

int32 AUnit::GetHealth()
{
	return PointLife;
}

bool AUnit::IsMoving()
{
	return bIsMoving;
}

bool AUnit::PLayed()
{
	return bPlayed;
}

void AUnit::SetParameters(ATile* tileunit, int32 owner, int32 maxmov, int32 attacktype, int32 maxrange)
{
	TileOfUnit = tileunit; UnitOwner = owner; MaxMovement = maxmov;
	AttackType = attacktype; MaxAttackRange = maxrange;
}

void AUnit::SetDamageRange(int32 minvaldamage, int32 maxvaluedamage)
{
	AttackDamageRange = FVector2D(minvaldamage, maxvaluedamage);
}

void AUnit::SetPointLife(int32 pointlife)
{
	PointLife = pointlife;
}

void AUnit::SetTileUnit(ATile* tileunit)
{
	TileOfUnit = tileunit;
}

//void AUnit::SetUnitOwner(int32 owner){UnitOwner = owner;}

void AUnit::SetIfIsMoving(bool ismoving)
{
	bIsMoving = ismoving;
}

void AUnit::SetPlayed(bool played)
{
	bPlayed = played;
}

void AUnit::SetPathToFollow(TArray<ATile*> pathtoloc)
{
	PathToFollow = pathtoloc;
}

void AUnit::SelfDestroy()
{
	Destroy();
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsMoving)
	{
		if(IndexOfPath < PathToFollow.Num())
			// following the path
		{
			FVector TargetLoc = FVector(PathToFollow[IndexOfPath]->GetActorLocation().X,
				PathToFollow[IndexOfPath]->GetActorLocation().Y,GetActorLocation().Z);
			FVector CurrentLocation = GetActorLocation();
			Alpha = FMath::Clamp(Alpha + (DeltaTime * Speed), 0.0f, 1.0f);
			// this value allows to interpolates between the current position of the unit and the targeted one
			// the new position is calculate as a percentage of the current location and the targeted one
			FVector NewPos = FMath::Lerp(GetActorLocation(), TargetLoc, Alpha);
			SetActorLocation(NewPos);
			if (FVector::Dist(CurrentLocation, TargetLoc) < 2.0f && IndexOfPath < PathToFollow.Num())
			{
				IndexOfPath++;
				Alpha = 0.0f;
			}
		}
		else if(IndexOfPath == PathToFollow.Num() || Alpha >= 1.0f)
		{
			bIsMoving = false;
			IndexOfPath = 0;
			Alpha = 0.0f;
			PathToFollow.Empty();
			bool bCalledFunction = false;// variable to guarantee that AttackingOrEndingTurn() isn't executed unecessary times 
			// while waiting for a timer
			ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
			for (auto player : GameMode->Players)
			{
				if (player->playernumber == GameMode->currentplayer && !bCalledFunction)
					// human player or ai player
					// resuming logic after movement ended
				{
					if (player->bResetOrChangeSettingsButtonClicked)
						return;
					player->AttackingOrEndingTurn();
					bCalledFunction = true;
				}
			}
		}
	}
}

