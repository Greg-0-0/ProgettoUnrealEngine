// Fill out your copyright notice in the Description page of Project Settings.


#include "SG_Menu.h"
#include "SG_GameMode.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"

void USG_Menu::NativeConstruct()
{
	if (AiChosen)
	{
		AiChosen->SetText(FText::FromString("RANDOM"));
	}
	if (ObstaclePercentage)
	{
		ObstaclePercentage->SetText(FText::FromString("0"));
	}
	if (WarningPercentage)
	{
		WarningPercentage->SetText(FText::FromString("Percentage is too high!"));
		WarningPercentage->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		WarningPercentage->SetVisibility(ESlateVisibility::Hidden);
	}
	if (PlayButton)
	{
		FScriptDelegate PlayButtonDelegate;
		PlayButtonDelegate.BindUFunction(this, FName("OnPlayButtonClicked"));
		PlayButton->OnClicked.Add(PlayButtonDelegate);
	}
	if (SmartAIButton)
	{
		FScriptDelegate SmartAIButtonDelegate;
		SmartAIButtonDelegate.BindUFunction(this, FName("OnSmartAIButtonClicked"));
		SmartAIButton->OnClicked.Add(SmartAIButtonDelegate);
	}
	if (NaiveAIButton)
	{
		FScriptDelegate NaiveAIButtonDelegate;
		NaiveAIButtonDelegate.BindUFunction(this, FName("OnNaiveAIButtonClicked"));
		NaiveAIButton->OnClicked.Add(NaiveAIButtonDelegate);
	}
	if (Slider)
	{
		Slider->OnValueChanged.AddDynamic(this, &USG_Menu::OnSliderValueChanged);
	}
}

void USG_Menu::ShowMenu()
{
	SetVisibility(ESlateVisibility::Visible);
}

void USG_Menu::HideMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

int32 USG_Menu::GetSliderValue()
{
	return int32();
}

void USG_Menu::OnPlayButtonClicked()
{
	ASG_GameMode* GameMode = Cast<ASG_GameMode>(GetWorld()->GetAuthGameMode());
	FString temp = ObstaclePercentage->GetText().ToString();
	int32 NObs = FCString::Atoi(*temp);// converting % of obstacles to int
	if (NObs == 100)
	{
		WarningPercentage->SetVisibility(ESlateVisibility::Visible);// warning is shown
		FTimerHandle TImer;
		GetWorld()->GetTimerManager().SetTimer(TImer, [&]() {WarningPercentage->SetVisibility(ESlateVisibility::Hidden);},
			2.0f, false);
		return;// cannot play game
	}
	GameMode->gamefield->PercentageOfObstacles = NObs;// setting number of obstacles
	temp = AiChosen->GetText().ToString();
	if (temp == "RANDOM")
		GameMode->ai_type = 0;
	else if (temp == "SMART")
		GameMode->ai_type = 1;
	HideMenu();
	if(GameMode->UserHudInstance != nullptr)
		// Play button is clicked at start game -> HUD hasn't been created yet
	{
		if (GameMode->UserHudInstance->ChangeSettingsPressed)
		{
			for (auto obs : GameMode->gamefield->ObstacleArray)
				obs->SelfDestroy();
			for (auto player : GameMode->Players)
				player->ResetValues();

			GameMode->GameReset(false, false, true);
		}
		else
			GameMode->GenerateField();
	}
	else
		GameMode->GenerateField();
}

void USG_Menu::OnSmartAIButtonClicked()
{
	if (AiChosen)
	{
		AiChosen->SetText(FText::FromString("SMART"));
	}
}

void USG_Menu::OnNaiveAIButtonClicked()
{
	if (AiChosen)
	{
		AiChosen->SetText(FText::FromString("RANDOM"));
	}
}

void USG_Menu::OnSliderValueChanged(float value)
{
	int32 Percentage = FMath::RoundToInt(value * 100);
	FString temp;
	if (ObstaclePercentage)
	{
		if (Percentage <= 100)
		{
			ObstaclePercentage->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			temp = FString::FromInt(Percentage);
		}
		ObstaclePercentage->SetText(FText::FromString(temp));
	}
}
