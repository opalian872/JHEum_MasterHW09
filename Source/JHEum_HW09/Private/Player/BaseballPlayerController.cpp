// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPlayerController.h"

#include "Game/MyGameModeBase.h"
#include "JHEum_HW09.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/BaseballPlayerState.h"
#include "UI/BasicChatInput.h"

ABaseballPlayerController::ABaseballPlayerController()
{
	bReplicates = true;
}

void ABaseballPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UBasicChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ABaseballPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		ABaseballPlayerState* BaseballPlayerState = GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BaseballPlayerState) == true)
		{
			const FString CombinedMessageString = BaseballPlayerState->GetPlayerInfoString() + TEXT(": ") + ChatMessageString;
			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ABaseballPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	// const FString NetModeString = FBaseballNetworkDebug::GetNetModeString(this);
	// const FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	// FBaseballNetworkDebug::PrintString(this, CombinedMessageString, 10.f);

	UKismetSystemLibrary::PrintString(this, InChatMessageString, true, true, FLinearColor::Red, 5.0f);
}

void ABaseballPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ABaseballPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	AMyGameModeBase* MyGameModeBase = Cast<AMyGameModeBase>(GameMode);
	if (IsValid(MyGameModeBase) == true)
	{
		MyGameModeBase->ProcessChatMessage(this, InChatMessageString);
	}
}

void ABaseballPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}
