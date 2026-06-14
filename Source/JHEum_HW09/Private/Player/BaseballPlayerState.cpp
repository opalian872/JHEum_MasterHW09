// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPlayerState.h"

#include "Net/UnrealNetwork.h"

ABaseballPlayerState::ABaseballPlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
{
	bReplicates = true;
}

void ABaseballPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
}

FString ABaseballPlayerState::GetPlayerInfoString() const
{
	return FString::Printf(TEXT("%s(%d/%d)"), *PlayerNameString, CurrentGuessCount, MaxGuessCount);
}

int32 ABaseballPlayerState::GetCurrentGuessCount() const
{
	return CurrentGuessCount;
}

int32 ABaseballPlayerState::GetMaxGuessCount() const
{
	return MaxGuessCount;
}
