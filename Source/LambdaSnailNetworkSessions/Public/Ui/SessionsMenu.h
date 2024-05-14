// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"

#include "SessionsMenu.generated.h"

class UButton;
class USessionManager;

/**
 * 
 */
UCLASS()
class LAMBDASNAILNETWORKSESSIONS_API USessionsMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupMenu();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void SessionManager_OnCreateSessionComplete(bool bWasSuccessful);
	void SessionManager_OnFindSessionComplete(TArray<FOnlineSessionSearchResult> const& SessionResults, bool bWasSuccessful);
	void SessionManager_OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void SessionManager_OnDestroySessionComplete(bool bWasSuccessful);
	UFUNCTION()
	void SessionManager_OnStartSessionComplete(bool bWasSuccessful);

	UPROPERTY(EditDefaultsOnly)
	FString PathToLobby;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	FName const MatchTypeKey { "MatchType" };
	FName const MatchType_FreeForAll { "FreeForAll" };
	
	TWeakObjectPtr<USessionManager> SessionManager;
};
