// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WoundData.generated.h"

UENUM(BlueprintType)
enum class EWoundSeverity : uint8
{
    Minor     UMETA(DisplayName = "Minor"),
    Moderate  UMETA(DisplayName = "Moderate"),
    Severe    UMETA(DisplayName = "Severe")
};
USTRUCT(BlueprintType)
struct FWoundState
{
    GENERATED_BODY()
    // Is there an active wound?
    UPROPERTY(BlueprintReadOnly)
    bool bIsWounded = false;
    UPROPERTY(BlueprintReadOnly)
    EWoundSeverity Severity = EWoundSeverity::Minor;
    // Normalized 0-1: how much of the wound is wrapped so far
    UPROPERTY(BlueprintReadOnly)
    float CoveragePercent = 0.f;
    // How many wrap-clicks does this wound require to fully cover it?
    UPROPERTY(BlueprintReadOnly)
    int32 WrapsRequired = 4;
    // Max health this wound can restore (at 100% coverage)
    UPROPERTY(BlueprintReadOnly)
    float MaxHealAmount = 50.f;
};