// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WoundData.h"
#include "BandageComponent.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class UDecalComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBandagingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBandagingEnded, float, HealingApplied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrapApplied, float, NewCoverage);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MGP_2526_API UBandageComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBandageComponent();
    //Public Interface
    //Call this to inflict a wound (e.g. from damage logic)
    UFUNCTION(BlueprintCallable, Category = "Bandage")
    void InflictWound(EWoundSeverity Severity);
    //Player pressed the heal key
    UFUNCTION(BlueprintCallable, Category = "Bandage")
    void TryStartBandaging();
    //Player clicked during bandaging — places one wrap
    UFUNCTION(BlueprintCallable, Category = "Bandage")
    void ApplyWrap();
    //Force-cancel bandaging (e.g. player sprints away)
    UFUNCTION(BlueprintCallable, Category = "Bandage")
    void CancelBandaging();
    UFUNCTION(BlueprintPure, Category = "Bandage")
    bool IsBandaging() const { return bIsBandaging; }
    UFUNCTION(BlueprintPure, Category = "Bandage")
    const FWoundData& GetWoundData() const { return CurrentWound; }
    UFUNCTION(BlueprintPure, Category = "Bandage")
    int32 GetBandageCount() const { return BandageInventory; }
    //Configuration
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    int32 BandageInventory = 5;
    //The arm mesh shown in first-person view
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    TObjectPtr<USkeletalMeshComponent> FirstPersonArmMesh;
    //The dedicated first-person camera
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    TObjectPtr<UCameraComponent> FirstPersonBandageCamera;
    //Decal material showing the wound on the arm
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    TObjectPtr<UMaterialInterface> WoundDecalMaterial;
    //Decal material showing a wrap segment
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    TObjectPtr<UMaterialInterface> WrapDecalMaterial;
    //HUD widget class to spawn
    UPROPERTY(EditAnywhere, Category = "Bandage|Setup")
    TSubclassOf<UUserWidget> BandageHUDClass;
    //Healing per severity at full coverage
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    float MinorHealAmount = 25.f;
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    float ModerateHealAmount = 50.f;
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    float SevereHealAmount = 80.f;
    //Wraps required per severity
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    int32 MinorWrapsRequired = 3;
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    int32 ModerateWrapsRequired = 5;
    UPROPERTY(EditAnywhere, Category = "Bandage|Healing")
    int32 SevereWrapsRequired = 7;
    //Delegates
    UPROPERTY(BlueprintAssignable)
    FOnBandagingStarted OnBandagingStarted;
    UPROPERTY(BlueprintAssignable)
    FOnBandagingEnded OnBandagingEnded;
    UPROPERTY(BlueprintAssignable)
    FOnWrapApplied OnWrapApplied;
protected:
    virtual void BeginPlay() override;
private:
    FWoundData CurrentWound;
    bool bIsBandaging = false;
    int32 WrapsApplied = 0;
    //Cached refs
    UPROPERTY()
    TObjectPtr<APlayerController> OwnerController;
    UPROPERTY()
    TObjectPtr<UCameraComponent> OriginalCamera;
    UPROPERTY()
    TObjectPtr<UUserWidget> BandageHUDInstance;
    //Spawned decals
    UPROPERTY()
    TObjectPtr<UDecalComponent> WoundDecal;
    UPROPERTY()
    TArray<TObjectPtr<UDecalComponent>> WrapDecals;
    void EnterBandagingMode();
    void ExitBandagingMode(bool bCompleted);
    void SpawnWoundDecal();
    void SpawnWrapDecal(int32 WrapIndex);
    float CalculateHealing() const;
    void CleanupDecals();
    //Maps severity to config values
    void ConfigureWoundFromSeverity(EWoundSeverity Severity);
};