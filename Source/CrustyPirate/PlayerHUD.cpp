#include "PlayerHUD.h"

void UPlayerHUD::SetHP(int NewHP)
{
	FString Str = FString::Printf(TEXT("HP: %d"), NewHP);
	HPText->SetText(FText::FromString(Str));
}

void UPlayerHUD::SetDiamonds(int Amount)
{
	FString Str = FString::Printf(TEXT("Diamonds: %d"), Amount);
	DiamondsText->SetText(FText::FromString(Str));
}

void UPlayerHUD::SetLevel(int Index)
{
	FString Str = FString::Printf(TEXT("Level: %d"), Index);
	LevelText->SetText(FText::FromString(Str));
}