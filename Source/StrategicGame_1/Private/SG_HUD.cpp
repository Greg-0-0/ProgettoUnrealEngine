// Fill out your copyright notice in the Description page of Project Settings.


#include "SG_HUD.h"
#include "SG_GameMode.h"
#include "SC_HumanPlayer.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void USG_HUD::NativeConstruct()
{
	Super::NativeConstruct();
	if (HBStatus)
	{
		HBStatus->SetVisibility(ESlateVisibility::Hidden);
	}
	if (HSStatus)
	{
		HSStatus->SetVisibility(ESlateVisibility::Hidden);
	}
	if (AiBStatus)
	{
		AiBStatus->SetVisibility(ESlateVisibility::Hidden);
	}
	if (AiSStatus)
	{
		AiSStatus->SetVisibility(ESlateVisibility::Hidden);
	}
	if (HBrawlerHealth)
	{
		HBrawlerHealth->SetText(FText::FromString("-"));
	}
	if (HSniperHealth)
	{
		HSniperHealth->SetText(FText::FromString("-"));
	}
	if (AiBrawlerHealth)
	{
		AiBrawlerHealth->SetText(FText::FromString("-"));
	}
	if (AiSniperHealth)
	{
		AiSniperHealth->SetText(FText::FromString("-"));
	}
	if (CurrentPlayer)
	{
		CurrentPlayer->SetText(FText::FromString(" "));
	}
	if (HumanWins)
	{
		HumanWins->SetText(FText::FromString("0"));
	}
	if (AiWins)
	{
		AiWins->SetText(FText::FromString("0"));
	}
	if (NumberOfMatches)
	{
		NumberOfMatches->SetText(FText::FromString("0"));
	}
	if (WinMessage)
	{
		WinMessage->SetText(FText::FromString(" "));
	}
	if (ResetButton)
	{
		FScriptDelegate ResetButtonDelegate;
		ResetButtonDelegate.BindUFunction(this, FName("OnResetButtonClicked"));
		ResetButton->OnClicked.Add(ResetButtonDelegate);
	}
	if (EndTurnButton)
	{
		FScriptDelegate EndTurnButtonDelegate;
		EndTurnButtonDelegate.BindUFunction(this, FName("OnEndTurnButtonClicked"));
		EndTurnButton->OnClicked.Add(EndTurnButtonDelegate);
		EndTurnButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ChangeSettingsButton)
	{
		FScriptDelegate ChangeSettingsButtonDelegate;
		ChangeSettingsButtonDelegate.BindUFunction(this, FName("OnChangeSettingsButtonClicked"));
		ChangeSettingsButton->OnClicked.Add(ChangeSettingsButtonDelegate);
	}
	ChangeSettingsPressed = false;
	TileToFollow = nullptr;
}

void USG_HUD::SetCurrentPlayer(FString NewPlayer, bool Hturn)
{
	if(CurrentPlayer)
	{
		CurrentPlayer->SetText(FText::GetEmpty());
		if(Hturn)
			CurrentPlayer->SetColorAndOpacity(FSlateColor(FLinearColor(0.025f, 0.0f, 0.3f, 1.0f)));
		else
			CurrentPlayer->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.0f, 0.025f, 1.0f)));
		CurrentPlayer->SetText(FText::FromString(NewPlayer));
	}
}

void USG_HUD::SetWinMess(FString Player, bool IsHuman)
{
	if (WinMessage)
	{
		if(IsHuman)
		{
			WinMessage->SetColorAndOpacity(FSlateColor(FLinearColor(0.025f, 0.0f, 0.3f, 1.0f)));
			WinMessage->SetText(FText::FromString(Player));
		}
		else
		{
			WinMessage->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.0f, 0.025f, 1.0f)));
			WinMessage->SetText(FText::FromString(Player));
		}
	}
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [&]() {WinMessage->SetText(FText::FromString(" ")); }, 3.0f, false);
}

void USG_HUD::SetHumanScore(FString Score)
{
	if (HumanWins)
	{
		HumanWins->SetText(FText::FromString(Score));
	}
}

void USG_HUD::SetAiScore(FString Score)
{
	if (AiWins)
	{
		AiWins->SetText(FText::FromString(Score));
	}
}

void USG_HUD::SetNMatches(FString Number)
{
	if (NumberOfMatches)
	{
		NumberOfMatches->SetText(FText::FromString(Number));
	}
}

void USG_HUD::SetHBrawlerHealth(FString NewHealth)
{
	if (HBrawlerHealth)
	{
		HBrawlerHealth->SetText(FText::FromString(NewHealth));
	}
}

void USG_HUD::SetHSniperHealth(FString NewHealth)
{
	if (HSniperHealth)
	{
		HSniperHealth->SetText(FText::FromString(NewHealth));
	}
}

void USG_HUD::SetAiBrawlerHealth(FString NewHealth)
{
	if (AiBrawlerHealth)
	{
		AiBrawlerHealth->SetText(FText::FromString(NewHealth));
	}
}

void USG_HUD::SetAiSniperHealth(FString NewHealth)
{
	if (AiSniperHealth)
	{
		AiSniperHealth->SetText(FText::FromString(NewHealth));
	}
}

void USG_HUD::ToggleVisibilityHB(bool MakeVisible)
{
	if(MakeVisible)
		HBStatus->SetVisibility(ESlateVisibility::Visible);
	else
		HBStatus->SetVisibility(ESlateVisibility::Hidden);
}

void USG_HUD::ToggleVisibilityHS(bool MakeVisible)
{
	if (MakeVisible)
		HSStatus->SetVisibility(ESlateVisibility::Visible);
	else
		HSStatus->SetVisibility(ESlateVisibility::Hidden);
}

void USG_HUD::ToggleVisibilityAiB(bool MakeVisible)
{
	if (MakeVisible)
		AiBStatus->SetVisibility(ESlateVisibility::Visible);
	else
		AiBStatus->SetVisibility(ESlateVisibility::Hidden);
}

void USG_HUD::ToggleVisibilityAiS(bool MakeVisible)
{
	if (MakeVisible)
		AiSStatus->SetVisibility(ESlateVisibility::Visible);
	else
		AiSStatus->SetVisibility(ESlateVisibility::Hidden);
}

void USG_HUD::ToggleVisibilityEndTurnButton(bool MakeVisible)
{
	if (MakeVisible)
		EndTurnButton->SetVisibility(ESlateVisibility::Visible);
	else
		EndTurnButton->SetVisibility(ESlateVisibility::Collapsed);
}

void USG_HUD::AddMessageToScrollBox(FString Message, bool IsHuman)
{
	if (HistoryBox)
	{
		UTextBlock* NewText = NewObject<UTextBlock>(this);
		if (NewText)
		{
			NewText->SetText(FText::FromString(Message));
			//NewText->SetJustification(ETextJustify::Center);
			if(IsHuman)
				NewText->SetColorAndOpacity(FSlateColor(FLinearColor(0.025f, 0.0f, 0.3f, 1.0f)));
			else
				NewText->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.0f, 0.025f, 1.0f)));
			FSlateFontInfo FontInfo = NewText->GetFont();
			FontInfo.Size = 20;
			NewText->SetFont(FontInfo);
			HistoryBox->AddChild(NewText);
		}
		HistoryBox->ScrollToEnd();
	}
}

void USG_HUD::ClearHUD()
{
	if (HistoryBox)
		HistoryBox->ClearChildren();
	if(CurrentPlayer)
		SetCurrentPlayer(TEXT(" "), false);
	if(AiBrawlerHealth)
		SetAiBrawlerHealth(TEXT("-"));
	if(AiSniperHealth)
		SetAiSniperHealth(TEXT("-"));
	if(HBrawlerHealth)
		SetHBrawlerHealth(TEXT("-"));
	if(HSniperHealth)
		SetHSniperHealth(TEXT("-"));
	if (HBStatus)
		ToggleVisibilityHB(false);
	if(HSStatus)
		ToggleVisibilityHS(false);
	if(AiBStatus)
		ToggleVisibilityAiB(false);
	if(AiSStatus)
		ToggleVisibilityAiS(false);
}

void USG_HUD::ShowUHD()
{
	SetVisibility(ESlateVisibility::Visible);
}

void USG_HUD::HideUHD()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void USG_HUD::OnResetButtonClicked()
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	for (auto player : GameMode->Players)
	{
		player->ClearTimers();// remain only timers ready to be executed (already triggered)
		player->bResetOrChangeSettingsButtonClicked = true;// flag to ensure that these timers don't access variables after resetting
	}
	GameMode->GameReset(false, false, true);// here bEndTurnButtonClicked is reverted to false
}

void USG_HUD::OnEndTurnButtonClicked()
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	for (auto player : GameMode->Players)
	{
		if (player->playernumber == 0)
			// selected player is human
		{
			player->ClearTimers();// remain only timers ready to be executed (already triggered)
			player->bEndTurnButtonClicked = true;// flag to ensure that timers don't access variables
			player->EndTurn();// here bEndTurnButtonClicked is reverted to false
		}
	}
}

void USG_HUD::OnChangeSettingsButtonClicked()
{
	ChangeSettingsPressed = true;
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	for (auto player : GameMode->Players)
	{
		player->ClearTimers();// remain only timers ready to be executed (already triggered)
		player->bResetOrChangeSettingsButtonClicked = true;// flag to ensure that these timers don't access variables after resetting
	}
	HideUHD();
	GameMode->MenuInstance->ShowMenu();
}
