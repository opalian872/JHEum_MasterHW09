// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPawn.h"

#include "JHEum_HW09.h"

// Sets default values
ABaseballPawn::ABaseballPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseballPawn::BeginPlay()
{
	Super::BeginPlay();

	// const FString NetRoleString = FBaseballNetworkDebug::GetRoleString(this);
	// const FString CombinedString = FString::Printf(
	// 	TEXT("ABaseballPawn::BeginPlay() %s [%s]"),
	// 	*FBaseballNetworkDebug::GetNetModeString(this),
	// 	*NetRoleString);
	// FBaseballNetworkDebug::PrintString(this, CombinedString, 10.f);
}

void ABaseballPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// const FString NetRoleString = FBaseballNetworkDebug::GetRoleString(this);
	// const FString CombinedString = FString::Printf(
	// 	TEXT("ABaseballPawn::PossessedBy() %s [%s]"),
	// 	*FBaseballNetworkDebug::GetNetModeString(this),
	// 	*NetRoleString);
	// FBaseballNetworkDebug::PrintString(this, CombinedString, 10.f);
}

// Called every frame
void ABaseballPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseballPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

