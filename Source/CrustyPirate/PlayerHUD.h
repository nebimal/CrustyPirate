#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Components/TextBlock.h"

#include "PlayerHUD.generated.h"


UCLASS()
class CRUSTYPIRATE_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta = (BindWidget)) // Allow us to bind the widget in c++ to the ones we have inside the widget Blueprint.
	UTextBlock* HPText;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) 
	UTextBlock* DiamondsText;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) 
	UTextBlock* LevelText;

	void SetHP(int NewHP);
	void SetDiamonds(int Amount);
	void SetLevel(int Index);
};
