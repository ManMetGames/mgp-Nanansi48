// Fill out your copyright notice in the Description page of Project Settings.


#include "BandageComponent.h"
#include "Health.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
UBandageComponent::UBandageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}
void UBandageComponent::BeginPlay()
{
    Super::BeginPlay();
    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
        OwnerController = Cast<APlayerController>(Owner->GetController());
        // Cache the character's current active camera
        // Third-person spring arm camera component
        OriginalCamera = Owner->FindComponentByClass<UCameraComponent>();
    }
}

// Public Interface
void UBandageComponent::InflictWound(EWoundSeverity Severity)
{
    if (CurrentWound.bIsWounded) return; // already wounded — extend or stack as needed
    ConfigureWoundFromSeverity(Severity);
    CurrentWound.bIsWounded = true;
    CurrentWound.CoveragePercent = 0.f;
}
void UBandageComponent::TryStartBandaging()
{

    if (bIsBandaging) return;
    if (!CurrentWound.bIsWounded)
    {
        UE_LOG(LogTemp, Warning, TEXT("BandageComponent: No wound to treat."));
        return;
    }
    if (BandageInventory <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BandageComponent: No bandages left."));
        return;
    }
    EnterBandagingMode();
}
void UBandageComponent::ApplyWrap()
{
    if (!bIsBandaging) return;
    if (BandageInventory <= 0)
    {
        ExitBandagingMode(false);
        return;
    }
    WrapsApplied++;
    BandageInventory--;
    // Update coverage — each wrap covers an equal fraction of the wound
    float CoveragePerWrap = 1.f / static_cast<float>(CurrentWound.WrapsRequired);
    CurrentWound.CoveragePercent = FMath::Clamp(
        WrapsApplied * CoveragePerWrap, 0.f, 1.f);
    SpawnWrapDecal(WrapsApplied - 1);
    OnWrapApplied.Broadcast(CurrentWound.CoveragePercent);
    UE_LOG(LogTemp, Log, TEXT("Wrap %d applied. Coverage: %.0f%%"),
        WrapsApplied, CurrentWound.CoveragePercent * 100.f);
    // End automatically when wound is fully covered OR bandages run out
    bool bFullyCovered = WrapsApplied >= CurrentWound.WrapsRequired;
    bool bOutOfBandages = BandageInventory <= 0;
    if (bFullyCovered || bOutOfBandages)
    {
        ExitBandagingMode(true);
    }
}
void UBandageComponent::CancelBandaging()
{
    if (!bIsBandaging) return;
    ExitBandagingMode(false);
}

// Private — Mode Transitions
void UBandageComponent::EnterBandagingMode()
{
    bIsBandaging = true;
    WrapsApplied = 0;

    // Switch to first-person bandage camera
    if (OwnerController && FirstPersonBandageCamera)
    {
        OwnerController->SetViewTargetWithBlend(
            GetOwner(),
            0.3f,                   // blend time
            VTBlend_Cubic,
            0.f,
            false
        );
        FirstPersonBandageCamera->SetActive(true);
        if (OriginalCamera)
            OriginalCamera->SetActive(false);
    }
    // Show arm mesh
    if (FirstPersonArmMesh)
        FirstPersonArmMesh->SetVisibility(true);

    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
		Owner->GetMesh()->SetVisibility(false); // hide full body to prevent clipping with arm mesh
	}
    // Lock movement — set input mode to UI+Game so mouse is free but game still runs
    if (OwnerController)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        InputMode.SetHideCursorDuringCapture(false);
        OwnerController->SetInputMode(InputMode);
        OwnerController->SetShowMouseCursor(true);
    }
    // Spawn HUD
    if (BandageHUDClass && OwnerController)
    {
        BandageHUDInstance = CreateWidget<UUserWidget>(OwnerController, BandageHUDClass);
		if (BandageHUDInstance) BandageHUDInstance->AddToViewport();
    }
    SpawnWoundDecal();
    OnBandagingStarted.Broadcast();
}
void UBandageComponent::ExitBandagingMode(bool bCompleted)
{
    bIsBandaging = false;
    float HealingApplied = 0.f;
    if (bCompleted)
    {
        HealingApplied = CalculateHealing();
        // Apply healing to the character
        if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
        {
            if (UHealth* Health = Owner->FindComponentByClass<UHealth>())
            {
                Health->Heal(HealingApplied);
            }
        }
        // Clear wound if fully treated
        if (CurrentWound.CoveragePercent >= 1.f)
        {
            CurrentWound = FWoundData{};
        }
        else
        {
            // Partially treated — wound persists but less severe
            CurrentWound.CoveragePercent = 0.f;
            CurrentWound.WrapsRequired = FMath::Max(1,
                CurrentWound.WrapsRequired - WrapsApplied);
        }
    }
    // Restore camera
    if (FirstPersonBandageCamera)
        FirstPersonBandageCamera->SetActive(false);
    if (OriginalCamera)
        OriginalCamera->SetActive(true);
    if (FirstPersonArmMesh)
        FirstPersonArmMesh->SetVisibility(false);

    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
        Owner->GetMesh()->SetVisibility(true); // show full body again
    }
    // Restore input
    if (OwnerController)
    {
        FInputModeGameOnly InputMode;
        OwnerController->SetInputMode(InputMode);
        OwnerController->SetShowMouseCursor(false);
    }
    // Remove HUD
    if (BandageHUDInstance)
    {
        BandageHUDInstance->RemoveFromParent();
        BandageHUDInstance = nullptr;
    }
    CleanupDecals();
    OnBandagingEnded.Broadcast(HealingApplied);
}

// Private — Decals
void UBandageComponent::SpawnWoundDecal()
{
    if (!WoundDecalMaterial || !FirstPersonArmMesh) return;
    // Attach wound decal to the wrist/forearm bone
    // Adjust bone name and offset to match your arm mesh
    FVector DecalSize(4.f, 6.f, 6.f);
    WoundDecal = UGameplayStatics::SpawnDecalAttached(
        WoundDecalMaterial,
        DecalSize,
        FirstPersonArmMesh,
        FName("lowerarm_r"),     // <-- change to your bone name
        FVector(0.f, 0.f, 0.f),
        FRotator(-90.f, 0.f, 0.f),
        EAttachLocation::SnapToTarget,
        10.f                     // lifespan 0 = forever
    );
}
void UBandageComponent::SpawnWrapDecal(int32 WrapIndex)
{
    if (!WrapDecalMaterial || !FirstPersonArmMesh) return;
    // Offset each wrap along the forearm so they stack visually
    float WrapOffset = WrapIndex * 1.8f; // cm apart along the bone axis
    FVector Location(WrapOffset, 0.f, 0.f);
    FVector DecalSize(3.f, 8.f, 8.f);
    UDecalComponent* WrapDecal = UGameplayStatics::SpawnDecalAttached(
        WrapDecalMaterial,
        DecalSize,
        FirstPersonArmMesh,
        FName("lowerarm_r"),
        Location,
        FRotator(-90.f, 0.f, 0.f),
        EAttachLocation::SnapToTarget,
        0.f
    );
    if (WrapDecal)
        WrapDecals.Add(WrapDecal);
}
void UBandageComponent::CleanupDecals()
{
    if (WoundDecal)
    {
        WoundDecal->DestroyComponent();
        WoundDecal = nullptr;
    }
    for (UDecalComponent* Decal : WrapDecals)
    {
        if (Decal) Decal->DestroyComponent();
    }
    WrapDecals.Empty();
}

// Private — Helpers
float UBandageComponent::CalculateHealing() const
{
    // Coverage drives healing linearly — partial wrapping = partial healing
    return CurrentWound.MaxHealAmount * CurrentWound.CoveragePercent;
}
void UBandageComponent::ConfigureWoundFromSeverity(EWoundSeverity Severity)
{
    CurrentWound.Severity = Severity;
    switch (Severity)
    {
    case EWoundSeverity::Minor:
        CurrentWound.WrapsRequired = MinorWrapsRequired;
        CurrentWound.MaxHealAmount = MinorHealAmount;
        break;
    case EWoundSeverity::Moderate:
        CurrentWound.WrapsRequired = ModerateWrapsRequired;
        CurrentWound.MaxHealAmount = ModerateHealAmount;
        break;
    case EWoundSeverity::Severe:
        CurrentWound.WrapsRequired = SevereWrapsRequired;
        CurrentWound.MaxHealAmount = SevereHealAmount;
        break;
    }
}