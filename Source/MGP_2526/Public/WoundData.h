// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WoundData.generated.h"
UENUM(BlueprintType)
enum class EWoundSeverity : uint8
{
	Minor UMETA(DisplayName = "Minor"),
	Moderate UMETA(DisplayName = "Moderate"),
	Severe UMETA(DisplayName = "Severe")
};
USTRUCT(BlueprintType)
struct FWoundData
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	bool bIsWounded = false;
	//Is there an active Wound?
	UPROPERTY(BlueprintReadOnly)
	EWoundSeverity Severity = EWoundSeverity::Minor;
	UPROPERTY(BlueprintReadOnly)
	float CoveragePercent = 0.f;
	//Normalised 0-1: how much of the wound is wrapped so far?
	UPROPERTY(BlueprintReadOnly)
	int32 WrapsRequired = 4;
	//How many wrap-clicks does this wound require to fully cover it?
	UPROPERTY(BlueprintReadOnly)
	float MaxHealAmount = 50.f;
	//Max health this wound can restore (at 100% coverage)
};