// Fill out your copyright notice in the Description page of Project Settings.


#include "Health.h"

// Sets default values for this component's properties
UHealth::UHealth()
{
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
	if (Damage <= 0.f || HP <= 0.f) return;

	HP = FMath::Clamp(HP - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(HP, -Damage);

	if (HP <= 0.f)
	{
		Died.Broadcast();
	}
}

void UHealth::Heal(float HealAmount)
{
	if (HealAmount <= 0.f || HP <= 0.f) return;
	HP = FMath::Clamp(HP + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(HP, HealAmount);
}
