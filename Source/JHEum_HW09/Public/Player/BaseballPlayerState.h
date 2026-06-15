// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BaseballPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class JHEUM_HW09_API ABaseballPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABaseballPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetPlayerInfoString() const;
	int32 GetCurrentGuessCount() const;
	int32 GetMaxGuessCount() const;

public:
	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bHasGuessed;
};
