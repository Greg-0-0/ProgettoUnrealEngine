// Fill out your copyright notice in the Description page of Project Settings.


#include "SC_GameInstance.h"
#include "SG_GameMode.h"


void USC_GameInstance::IncrementScoreHumanPlayer()
{
	ScoreHumanPlayer++;
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->SetHumanScore(FString::FromInt(ScoreHumanPlayer));
}

void USC_GameInstance::IncrementScoreAiPlayer()
{
	ScoreAiPlayer++;
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->SetAiScore(FString::FromInt(ScoreAiPlayer));
}

void USC_GameInstance::IncrementMatchesPlayed()
{
	MatchesPlayed++;
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->SetNMatches(FString::FromInt(MatchesPlayed));
}

int32 USC_GameInstance::GetScoreHumanPlayer()
{
	return ScoreHumanPlayer;
}

int32 USC_GameInstance::GetScoreAiPlayer()
{
	return ScoreAiPlayer;
}

int32 USC_GameInstance::GetMatchesPlayed()
{
	return MatchesPlayed;
}

FString USC_GameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

void USC_GameInstance::SetTurnMessage(FString Message, bool Hturn)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->SetCurrentPlayer(Message, Hturn);
}

void USC_GameInstance::SetWinMessage(FString Message, bool IsHuman)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->SetWinMess(Message,IsHuman);
}

void USC_GameInstance::SetHealth(FString NewHealth,bool IsHuman, int32 attacktype)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	if(IsHuman)
	{
		if (attacktype == 0)
			GameMode->UserHudInstance->SetHSniperHealth(NewHealth);
		else if (attacktype == 1)
			GameMode->UserHudInstance->SetHBrawlerHealth(NewHealth);
	}
	else
	{
	if (attacktype == 0)
		GameMode->UserHudInstance->SetAiSniperHealth(NewHealth);
	else if (attacktype == 1)
		GameMode->UserHudInstance->SetAiBrawlerHealth(NewHealth);
	}
}

void USC_GameInstance::SetDead(bool IsHuman, int32 attacktype, bool MakeVisible)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsHuman)
	{
		if (attacktype == 0)
			GameMode->UserHudInstance->ToggleVisibilityHS(MakeVisible);
		else if (attacktype == 1)
			GameMode->UserHudInstance->ToggleVisibilityHB(MakeVisible);
	}
	else
	{
		if (attacktype == 0)
			GameMode->UserHudInstance->ToggleVisibilityAiS(MakeVisible);
		else if (attacktype == 1)
			GameMode->UserHudInstance->ToggleVisibilityAiB(MakeVisible);
	}
}

void USC_GameInstance::ChangeVisibilityEndTurnButton(bool MakeVisible)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->ToggleVisibilityEndTurnButton(MakeVisible);
}

void USC_GameInstance::AddMessageToHistory(FString Message, bool IsHuman)
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->UserHudInstance->AddMessageToScrollBox(Message, IsHuman);
}
