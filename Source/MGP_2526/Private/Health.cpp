// Fill out your copyright notice in the Description page of Project Settings.


#include "Health.h"

// Sets default values for this component's properties
UHealth::UHealth()
{
	//Disable the update as we dont need it
	//PrimaryComponentTick.bCanEverTick = true;
	HP = MaxHealth;
}


// Called when the game starts
void UHealth::BeginPlay()
{
	Super::BeginPlay();
}


void UHealth::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHealth::TakeDamage(float Damage)
{
	HP -= Damage;
	if (HP < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("I am hit"));
		HP = 0;
		// Tell interested classes we have died, as we are polite 
		Died.Broadcast();
	}
}

void UHealth::Heal(float Heal)
{
	HP += Heal;
	if (HP > MaxHealth)
	{
		HP = MaxHealth;
	}
}
