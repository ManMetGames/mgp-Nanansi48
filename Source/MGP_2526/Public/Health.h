// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Health.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FYouDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Delta);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MGP_2526_API UHealth : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const { return MaxHealth; }
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthChanged OnHealthChanged;
private:
	// make our variables private only the health script should deal with the health values
	UPROPERTY(VisibleAnywhere, Category = "Health|MaxHealth")
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, Category = "Health|HP")
	float HP= 0.f;


public:
	// Sets default values for this component's properties
	UHealth();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	// function to take damage
	virtual void TakeDamage(float damage);
	UFUNCTION(BlueprintCallable)
	virtual void Heal(float Heal);
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FYouDied Died;

};