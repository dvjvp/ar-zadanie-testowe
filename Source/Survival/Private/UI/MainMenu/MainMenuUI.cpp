#include "UI/MainMenu/MainMenuUI.h"

#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/TextBlock.h"
#include "Interfaces/OnlineSessionInterface.h"

void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	NewGame->OnClicked.AddDynamic(this, &UMainMenuUI::OnNewGame);
	Join->OnClicked.AddDynamic(this, &UMainMenuUI::OnJoin);
	Exit->OnClicked.AddDynamic(this, &UMainMenuUI::OnExit);
	
	if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMainMenuUI::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMainMenuUI::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMainMenuUI::OnJoinSessionComplete);
		}
	}
}

void UMainMenuUI::OnNewGame()
{
	if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			Log(TEXT("Try New Game"));
			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsLANMatch = true;           // LAN only
			SessionSettings.NumPublicConnections = 4;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUsesPresence = false;

			SessionInterface->CreateSession(0, FName("MyGameSession"), SessionSettings);
		}
	}
}

void UMainMenuUI::OnJoin()
{
	if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			Log(TEXT("Try Join"));
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = true;
			SessionSearch->MaxSearchResults = 10;
			SessionSearch->PingBucketSize = 50;

			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
}

void UMainMenuUI::OnExit()
{
	
}

void UMainMenuUI::OnCreateSessionComplete(FName Name, bool Success)
{
	if (Success)
	{
		Log(TEXT("Create session"));
		GetWorld()->ServerTravel(TEXT("Lvl_Game?Listen"));
	}
	else
	{
		Log(TEXT("Failed to create session"));
	}
}

void UMainMenuUI::OnFindSessionsComplete(bool Success)
{
	if (Success)
	{
		Log(TEXT("Sessions Found"));
		if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
		{
			IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
			if (SessionInterface.IsValid() && SessionSearch.IsValid())
			{
				for (const auto& Result : SessionSearch->SearchResults)
				{
					if (Result.IsValid())
					{
						Log(FString::Printf(TEXT("Trying to join session '%s'"), *Result.Session.OwningUserName));
						SessionInterface->JoinSession(0, FName("MyGameSession"), Result);
						break;
					}
				}
			}
			else
			{
				Log(TEXT("SessionInterface invalid or SessionSearch invalid!"));
			}
		}
	}
	else
	{
		Log(TEXT("Failed to find session"));
	}
}

void UMainMenuUI::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		Log(TEXT("Session Joined"));
		if (auto* OnlineSubsystem = IOnlineSubsystem::Get())
		{
			IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
			if (SessionInterface.IsValid())
			{
				FString ConnectString;
				if (SessionInterface->GetResolvedConnectString(Name, ConnectString))
				{
					Log(FString::Printf(TEXT("SetClientTravel(%s)"), *ConnectString));
					GEngine->SetClientTravel(GetWorld(), *ConnectString, TRAVEL_Absolute);
				}
				else
				{
					Log(TEXT("Could not get connect string."));
				}
			}
			else
			{
				Log(TEXT("Session Interface is invalid"));
			}
		}
	}
	else
	{
		Log(FString::Printf(TEXT("Failed to join session. Result %i"), Result));
	}
}

void UMainMenuUI::Log(const FString& String)
{
	LogOutput->SetText(FText::FromString(String));
	UE_LOG(LogOnline, Error, TEXT("%s"), *String);
}
