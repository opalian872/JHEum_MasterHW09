// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Player/BaseballPlayerController.h"

void AMyGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PlayerController) == true)
		{
			ABaseballPlayerController* BaseballPlayerController = Cast<ABaseballPlayerController>(PlayerController);
			if (IsValid(BaseballPlayerController) == true)
			{
				const FString NotificationString = InNameString + TEXT(" has joined the game.");
				BaseballPlayerController->PrintChatMessageString(NotificationString);
			}
		}
	}
}
