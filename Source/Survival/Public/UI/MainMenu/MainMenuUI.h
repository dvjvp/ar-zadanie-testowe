#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/Object.h"
#include "MainMenuUI.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract)
class SURVIVAL_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget)) UButton* NewGame;
	UPROPERTY(meta=(BindWidget)) UButton* Join;
	UPROPERTY(meta=(BindWidget)) UButton* Exit;

	UPROPERTY(meta=(BindWidget)) UTextBlock* LogOutput;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	virtual void NativeConstruct() override;

	UFUNCTION() void OnNewGame();
	UFUNCTION() void OnJoin();
	UFUNCTION() void OnExit();
	
	void OnCreateSessionComplete(FName Name, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result);

	void Log(const FString& String);
};
