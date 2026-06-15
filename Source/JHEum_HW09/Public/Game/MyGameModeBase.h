// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class ABaseballPlayerController;

/**
 * 
 */
UCLASS()
class JHEUM_HW09_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber() const;
	bool IsGuessNumberString(const FString& InNumberString) const;
	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString) const;
	void ProcessChatMessage(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	bool JudgeGame(ABaseballPlayerController* InChattingPlayerController, bool bIsWinningGuess);
	void ResetGame();

	void StartTurn();
	void AdvanceTurn();
	void OnTurnTimerElapsed();

	bool IsCurrentTurnPlayer(
		ABaseballPlayerController* InPlayerController
	) const;

	void StartGame();

protected:
	FString GetGuessValidationMessage(const FString& InNumberString) const;

	FString SecretNumberString;

	FTimerHandle TurnTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	int32 TurnTimeLimit = 10;

	int32 RemainingTurnTime = 10;
	int32 CurrentTurnIndex = 0;

	bool bIsGameRunning = false;

	UPROPERTY()
	TArray<TObjectPtr<ABaseballPlayerController>> AllPlayerControllers;
};
