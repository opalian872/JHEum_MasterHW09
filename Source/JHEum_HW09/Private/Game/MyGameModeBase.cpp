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

	if (bIsGameRunning == false)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("게임을 먼저 시작해주세요.")
		);
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
	if (IsCurrentTurnPlayer(InChattingPlayerController) == false)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(
			TEXT("현재 당신의 턴이 아닙니다.")
		);
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
	BaseballPlayerState->bHasGuessed = true;

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

	bool bGameEnded = JudgeGame(InChattingPlayerController, JudgeResultString.StartsWith(TEXT("3S")));

	if (bGameEnded == false)
	{
		AdvanceTurn();
	}
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

bool AMyGameModeBase::JudgeGame(ABaseballPlayerController* InChattingPlayerController, bool bIsWinningGuess)
{
	if (bIsWinningGuess == true)
	{
		ABaseballPlayerState* WinnerPlayerState =
			InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();

		if (IsValid(WinnerPlayerState) == true)
		{
			const FText WinnerMessage = FText::FromString(
				WinnerPlayerState->PlayerNameString +
				TEXT(" has won the game.")
			);

			for (const TObjectPtr<ABaseballPlayerController>& PlayerController :
				AllPlayerControllers)
			{
				if (IsValid(PlayerController) == true)
				{
					PlayerController->NotificationText = WinnerMessage;
				}
			}

			ResetGame();
			return true;
		}
	}

	bool bIsDraw = AllPlayerControllers.Num() > 0;

	for (const TObjectPtr<ABaseballPlayerController>& PlayerController :
		AllPlayerControllers)
	{
		if (IsValid(PlayerController) == false)
		{
			continue;
		}

		ABaseballPlayerState* PlayerState =
			PlayerController->GetPlayerState<ABaseballPlayerState>();

		if (IsValid(PlayerState) == false ||
			PlayerState->CurrentGuessCount < PlayerState->MaxGuessCount)
		{
			bIsDraw = false;
			break;
		}
	}

	if (bIsDraw == true)
	{
		const FText DrawMessage = FText::FromString(TEXT("Draw..."));

		for (const TObjectPtr<ABaseballPlayerController>& PlayerController :
			AllPlayerControllers)
		{
			if (IsValid(PlayerController) == true)
			{
				PlayerController->NotificationText = DrawMessage;
			}
		}

		ResetGame();
		return true;
	}

	return false;
}


void AMyGameModeBase::ResetGame()
{
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	bIsGameRunning = false;
	AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();
	if (IsValid(MyGameState) == true)
	{
		MyGameState->bIsGameRunning = false;
	}
	CurrentTurnIndex = 0;
	RemainingTurnTime = TurnTimeLimit;
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
			BaseballPlayerState->bHasGuessed = false;
		}
	}
	if (IsValid(MyGameState) == true)
	{
		MyGameState->CurrentTurnPlayerName = TEXT("");
		MyGameState->RemainingTurnTime = RemainingTurnTime;
	}


}

void AMyGameModeBase::StartTurn()
{
	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex) == false)
	{
		return;
	}

	ABaseballPlayerController* CurrentController =
		AllPlayerControllers[CurrentTurnIndex];

	if (IsValid(CurrentController) == false)
	{
		AdvanceTurn();
		return;
	}

	ABaseballPlayerState* CurrentPlayerState =
		CurrentController->GetPlayerState<ABaseballPlayerState>();

	if (IsValid(CurrentPlayerState) == false)
	{
		AdvanceTurn();
		return;
	}

	if (CurrentPlayerState->CurrentGuessCount >=
		CurrentPlayerState->MaxGuessCount)
	{
		AdvanceTurn();
		return;
	}

	CurrentPlayerState->bHasGuessed = false;
	RemainingTurnTime = TurnTimeLimit;

	AMyGameStateBase* MyGameState =
		GetGameState<AMyGameStateBase>();

	if (IsValid(MyGameState) == true)
	{
		MyGameState->CurrentTurnPlayerName =
			CurrentPlayerState->PlayerNameString;

		MyGameState->RemainingTurnTime =
			RemainingTurnTime;
	}

	GetWorldTimerManager().ClearTimer(TurnTimerHandle);

	GetWorldTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ThisClass::OnTurnTimerElapsed,
		1.0f,
		true
	);
}

void AMyGameModeBase::AdvanceTurn()
{
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);

	if (AllPlayerControllers.IsEmpty() == true)
	{
		return;
	}

	for (int32 Index = 0; Index < AllPlayerControllers.Num(); ++Index)
	{
		CurrentTurnIndex =
			(CurrentTurnIndex + 1) % AllPlayerControllers.Num();

		ABaseballPlayerController* NextController =
			AllPlayerControllers[CurrentTurnIndex];

		if (IsValid(NextController) == false)
		{
			continue;
		}

		ABaseballPlayerState* NextPlayerState =
			NextController->GetPlayerState<ABaseballPlayerState>();

		if (IsValid(NextPlayerState) == true &&
			NextPlayerState->CurrentGuessCount <
			NextPlayerState->MaxGuessCount)
		{
			StartTurn();
			return;
		}
	}
}

void AMyGameModeBase::OnTurnTimerElapsed()
{
	--RemainingTurnTime;

	AMyGameStateBase* MyGameState =
		GetGameState<AMyGameStateBase>();

	if (IsValid(MyGameState) == true)
	{
		MyGameState->RemainingTurnTime =
			RemainingTurnTime;
	}

	if (RemainingTurnTime > 0)
	{
		return;
	}

	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex) == false)
	{
		return;
	}

	ABaseballPlayerController* CurrentController =
		AllPlayerControllers[CurrentTurnIndex];

	if (IsValid(CurrentController) == true)
	{
		ABaseballPlayerState* CurrentPlayerState =
			CurrentController->GetPlayerState<ABaseballPlayerState>();

		if (IsValid(CurrentPlayerState) == true)
		{
			CurrentPlayerState->bHasGuessed = false;
			++CurrentPlayerState->CurrentGuessCount;

			CurrentController->ClientRPCPrintChatMessageString(
				TEXT("Time Out! 턴을 종료합니다.")
			);
		}
	}

	bool bGameEnded = JudgeGame(CurrentController, false);

	if (bGameEnded == false)
	{
		AdvanceTurn();
	}
}

bool AMyGameModeBase::IsCurrentTurnPlayer(ABaseballPlayerController* InPlayerController) const
{
	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex) == false)
	{
		return false;
	}

	return AllPlayerControllers[CurrentTurnIndex] == InPlayerController;
}

void AMyGameModeBase::StartGame()
{
	if (bIsGameRunning == true)
	{
		return;
	}

	if (AllPlayerControllers.Num() < 2)
	{
		for (const TObjectPtr<ABaseballPlayerController>& PlayerController :
			AllPlayerControllers)
		{
			if (IsValid(PlayerController) == true)
			{
				PlayerController->ClientRPCPrintChatMessageString(TEXT("플레이할 인원이 부족합니다"));
			}
		}
		return;
	}

	bIsGameRunning = true;
	AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();
	if (IsValid(MyGameState) == true)
	{
		MyGameState->bIsGameRunning = true;
	}

	CurrentTurnIndex = 0;
	RemainingTurnTime = TurnTimeLimit;
	SecretNumberString = GenerateSecretNumber();

	for (const TObjectPtr<ABaseballPlayerController>& PlayerController : AllPlayerControllers)
	{
		if (IsValid(PlayerController) == true)
		{
			PlayerController->NotificationText = FText::GetEmpty();
		}
	}

	StartTurn();
}

