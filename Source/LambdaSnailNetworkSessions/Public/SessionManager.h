// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionManager.generated.h"

class FOnlineSessionSearch;
class IOnlineSession;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionManagerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSessionManagerOnFindSessionsComplete, TArray<FOnlineSessionSearchResult> const& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FSessionManagerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionManagerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionManagerStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class LAMBDASNAILNETWORKSESSIONS_API USessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Creates a new online session. Closes any existing sessions first. 
	 * @param NumPublicConnections The number of players that can join the session.
	 * @param MatchType The type of the match.
	 */
	UFUNCTION(BlueprintCallable)
	void CreateGameSession(int32 NumPublicConnections, FString const& MatchType);
	void FindGameSessions(int32 MaxSessionCount);
	void JoinGameSession(FOnlineSessionSearchResult const& Result);
	void DestroyGameSession();
	void StartGameSession();

	FSessionManagerOnCreateSessionComplete OnCreateSessionComplete_Delegate;
	FSessionManagerOnFindSessionsComplete OnFindSessionsComplete_Delegate;
	FSessionManagerOnJoinSessionComplete OnJoinSessionComplete_Delegate;
	FSessionManagerOnDestroySessionComplete OnDestroySessionComplete_Delegate;
	FSessionManagerStartSessionComplete OnStartSessionComplete_Delegate;
	
private:
	USessionManager();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	TSharedPtr<IOnlineSession, ESPMode::ThreadSafe> SessionInterface;

	/**
	 * Stores the session settings of the current session, or the last session that was created if no session is active.
	 */
	TSharedPtr<FOnlineSessionSettings> LatestSessionSettings;

	// Create Session
	FDelegateHandle OnCreateSessionComplete_Handle;
	FOnCreateSessionCompleteDelegate OnCreateSessionComplete;
	void SessionManager_OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// Find Session
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	FDelegateHandle OnFindSessionsComplete_Handle;
	FOnFindSessionsCompleteDelegate OnFindSessionsComplete;
	void SessionManager_OnFindSessionsComplete(bool bWasSuccessful);

	// Join Session
	FDelegateHandle OnJoinSessionComplete_Handle;
	FOnJoinSessionCompleteDelegate OnJoinSessionComplete;
	void SessionManager_OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type ResultType);

	// Start Session
	FDelegateHandle OnStartSessionComplete_Handle;
	FOnStartSessionCompleteDelegate OnStartSessionComplete;
	void SessionManager_OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
	// Destroy Session
	FDelegateHandle OnDestroySessionComplete_Handle;
	FOnDestroySessionCompleteDelegate OnDestroySessionComplete;
	void SessionManager_OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	FName const MatchTypeKey { "MatchType" };
	FString const ListenServerUrlParameter { "?listen" };

	// Used to create a session after failing due to previous session not being destroyed yet
	bool bCreateSessionOnDestroy { false };
	int32 LastNumPublicConnections; 
	FString LastMatchType;
};
