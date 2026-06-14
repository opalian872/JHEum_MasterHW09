// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPawn.h"

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

