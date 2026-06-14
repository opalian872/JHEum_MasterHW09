// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyGameModeBase.h"

#include "EngineUtils.h"
#include "Game/MyGameStateBase.h"
#include "Player/BaseballPlayerController.h"
#include "Player/BaseballPlayerState.h"

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
}

void AMyGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABaseballPlayerController* BaseballPlayerController = Cast<ABaseballPlayerController>(NewPlayer);
	if (IsValid(BaseballPlayerController) == true)
	{
		BaseballPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(BaseballPlayerController);

		ABaseballPlayerState* BaseballPlayerState = BaseballPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BaseballPlayerState) == true)
		{
			BaseballPlayerState->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());

			AMyGameStateBase* MyGameStateBase = GetGameState<AMyGameStateBase>();
			if (IsValid(MyGameStateBase) == true)
			{
				MyGameStateBase->MulticastRPCBroadcastLoginMessage(BaseballPlayerState->PlayerNameString);
			}
		}
	}
}

FString AMyGameModeBase::GenerateSecretNumber() const
{
	TArray<int32> Numbers;
	for (int32 Number = 1; Number <= 9; ++Number)
	{
		Numbers.Add(Number);
	}

	FString Result;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const int32 RandomIndex = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[RandomIndex]));
		Numbers.RemoveAt(RandomIndex);
	}

	return Result;
}

bool AMyGameModeBase::IsGuessNumberString(const FString& InNumberString) const
{
	return GetGuessValidationMessage(InNumberString).IsEmpty();
}

FString AMyGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString) const
{
	int32 StrikeCount = 0;
	int32 BallCount = 0;

	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (InSecretNumberString[Index] == InGuessNumberString[Index])
		{
			++StrikeCount;
		}
		else if (InSecretNumberString.Contains(FString::Chr(InGuessNumberString[Index])))
		{
			++BallCount;
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AMyGameModeBase::ProcessChatMessage(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	if (IsValid(InChattingPlayerController) == false)
	{
		return;
	}

	ABaseballPlayerState* BaseballPlayerState = InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();
	if (IsValid(BaseballPlayerState) == false)
	{
		return;
	}

	FString GuessInputString = InChatMessageString;
	const int32 SeparatorIndex = InChatMessageString.Find(TEXT(": "));
	if (SeparatorIndex != INDEX_NONE)
	{
		GuessInputString = InChatMessageString.Mid(SeparatorIndex + 2);
	}

	bool bContainsDigit = false;
	for (const TCHAR Character : GuessInputString)
	{
		if (FChar::IsDigit(Character) == true)
		{
			bContainsDigit = true;
			break;
		}
	}

	if (bContainsDigit == false)
	{
		for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
		{
			ABaseballPlayerController* BaseballPlayerController = *It;
			if (IsValid(BaseballPlayerController) == true)
			{
				BaseballPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
		return;
	}

	const FString ValidationMessage = GetGuessValidationMessage(GuessInputString);
	if (ValidationMessage.IsEmpty() == false)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(ValidationMessage);
		return;
	}

	if (BaseballPlayerState->CurrentGuessCount >= BaseballPlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("기회를 모두 사용했습니다."));
		return;
	}

	++BaseballPlayerState->CurrentGuessCount;

	const FString JudgeResultString = JudgeResult(SecretNumberString, GuessInputString);
	const FString ResultMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
	for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
	{
		ABaseballPlayerController* BaseballPlayerController = *It;
		if (IsValid(BaseballPlayerController) == true)
		{
			BaseballPlayerController->ClientRPCPrintChatMessageString(ResultMessageString);
		}
	}

	JudgeGame(InChattingPlayerController, JudgeResultString.StartsWith(TEXT("3S")));
}

FString AMyGameModeBase::GetGuessValidationMessage(const FString& InNumberString) const
{
	if (InNumberString.Len() != 3)
	{
		return TEXT("3자리 숫자를 입력해주세요.");
	}

	TSet<TCHAR> UniqueDigits;
	for (const TCHAR Character : InNumberString)
	{
		if (FChar::IsDigit(Character) == false)
		{
			return TEXT("숫자만 입력해주세요.");
		}

		if (Character == '0')
		{
			return TEXT("1부터 9까지의 숫자만 입력해주세요.");
		}

		UniqueDigits.Add(Character);
	}

	if (UniqueDigits.Num() != 3)
	{
		return TEXT("중복되지 않은 숫자를 입력해주세요.");
	}

	return FString();
}

void AMyGameModeBase::JudgeGame(ABaseballPlayerController* InChattingPlayerController, bool bIsWinningGuess)
{
	if (bIsWinningGuess == true)
	{
		ABaseballPlayerState* WinnerPlayerState = InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(WinnerPlayerState) == true)
		{
			const FText WinnerMessage = FText::FromString(WinnerPlayerState->PlayerNameString + TEXT(" has won the game."));
			for (const TObjectPtr<ABaseballPlayerController>& BaseballPlayerController : AllPlayerControllers)
			{
				if (IsValid(BaseballPlayerController) == true)
				{
					BaseballPlayerController->NotificationText = WinnerMessage;
				}
			}
			ResetGame();
		}

		return;
	}

	bool bIsDraw = AllPlayerControllers.Num() > 0;
	for (const TObjectPtr<ABaseballPlayerController>& BaseballPlayerController : AllPlayerControllers)
	{
		if (IsValid(BaseballPlayerController) == false)
		{
			continue;
		}

		ABaseballPlayerState* BaseballPlayerState = BaseballPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BaseballPlayerState) == false || BaseballPlayerState->CurrentGuessCount < BaseballPlayerState->MaxGuessCount)
		{
			bIsDraw = false;
			break;
		}
	}

	if (bIsDraw == true)
	{
		const FText DrawMessage = FText::FromString(TEXT("Draw..."));
		for (const TObjectPtr<ABaseballPlayerController>& BaseballPlayerController : AllPlayerControllers)
		{
			if (IsValid(BaseballPlayerController) == true)
			{
				BaseballPlayerController->NotificationText = DrawMessage;
			}
		}
		ResetGame();
	}
}

void AMyGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const TObjectPtr<ABaseballPlayerController>& BaseballPlayerController : AllPlayerControllers)
	{
		if (IsValid(BaseballPlayerController) == false)
		{
			continue;
		}

		ABaseballPlayerState* BaseballPlayerState = BaseballPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BaseballPlayerState) == true)
		{
			BaseballPlayerState->CurrentGuessCount = 0;
		}
	}
}
