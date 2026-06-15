// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BaseballPlayerController.generated.h"

class UBasicChatInput;
class UUserWidget;

/**
 * 
 */
UCLASS()
class JHEUM_HW09_API ABaseballPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABaseballPlayerController();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void SetChatMessageString(const FString& InChatMessageString);
	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(BlueprintCallable)
	void RequestStartGame();

	UFUNCTION(BlueprintCallable)
	void RequestResetGame();

	UFUNCTION(Server, Reliable)
	void ServerRPCStartGame();

	UFUNCTION(Server, Reliable)
	void ServerRPCResetGame();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBasicChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UBasicChatInput> ChatInputWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TimerTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TimerTextWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> StartButtonWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> StartButtonWidgetInstance;

	FString ChatMessageString;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;
};
