// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGP_2526Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MGP_2526.h"

AMGP_2526Character::AMGP_2526Character()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealth>(TEXT("HealthComponent"));
    //Bandage arm mesh
    //Positioned so it's visible from the bandage camera angle.
    //You'll assign the actual skeletal mesh asset in your Blueprint child.
    BandageArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BandageArmMesh"));
    BandageArmMesh->SetupAttachment(RootComponent);
    //Hide by default — BandageComponent will toggle visibility
    BandageArmMesh->SetVisibility(false);
    //Only render in first-person (won't appear in third-person shadow/world)
    BandageArmMesh->SetOnlyOwnerSee(true);

    //Bandage camera
    //Attach directly to the root so we can position it freely in Blueprint.
    //Aimed slightly above and angled down toward the forearm.
    BandageCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BandageCamera"));
    BandageCamera->SetupAttachment(RootComponent);
    //Offset forward and down to look at the outstretched arm
    BandageCamera->SetRelativeLocation(FVector(30.f, 40.f, 150.f));
    BandageCamera->SetRelativeRotation(FRotator(-40.f, 0.f, 0.f));
    BandageCamera->bAutoActivate = false; // BandageComponent activates this

    //Bandage component
    BandageComponent = CreateDefaultSubobject<UBandageComponent>(TEXT("BandageComponent"));
    //Wire up the two sub-components it needs
    BandageComponent->FirstPersonArmMesh = BandageArmMesh;
    BandageComponent->FirstPersonBandageCamera = BandageCamera;
    //in your Blueprint child class via the Details panel
}

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Move);
        EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);

        //Bandage input bindings
        if (HealAction)
        {
            EnhancedInputComponent->BindAction(HealAction, ETriggerEvent::Started, this,
                &AMGP_2526Character::OnHealPressed);
        }
        if (WrapAction)
        {
            EnhancedInputComponent->BindAction(WrapAction, ETriggerEvent::Started, this,
                &AMGP_2526Character::OnWrapClicked);
        }
        if (DamageAction)
        {
            EnhancedInputComponent->BindAction(DamageAction, ETriggerEvent::Started, this,
				&AMGP_2526Character::OnDamagePressed);
		}
    }
    else
    {
        UE_LOG(LogMGP_2526, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
    }
}

void AMGP_2526Character::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);
}

void AMGP_2526Character::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
    if (GetController() != nullptr)
    {
        const FRotator Rotation = GetController()->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(ForwardDirection, Forward);
        AddMovementInput(RightDirection, Right);
    }
}

void AMGP_2526Character::DoLook(float Yaw, float Pitch)
{
    if (GetController() != nullptr)
    {
        AddControllerYawInput(Yaw);
        AddControllerPitchInput(Pitch);
    }
}

void AMGP_2526Character::DoJumpStart()
{
    Jump();
}

void AMGP_2526Character::DoJumpEnd()
{
    StopJumping();
}

void AMGP_2526Character::BeginPlay()
{
    Super::BeginPlay();


    if (HealthComponent)
    {
		HealthComponent->OnHealthChanged.AddDynamic(this, &AMGP_2526Character::OnHealthChanged);
    }
}

void AMGP_2526Character::OnDamagePressed()
{
    if (!HealthComponent) return;
        HealthComponent->TakeDamage(25.f);
}

//Bandage input handlers
void AMGP_2526Character::OnHealPressed()
{
    if (!BandageComponent) return;

    if (BandageComponent->IsBandaging())
        BandageComponent->CancelBandaging();
    else
        BandageComponent->TryStartBandaging();
}

void AMGP_2526Character::OnHealthChanged(float NewHealth, float Delta)
{
    // Only inflict wounds when taking damage, not when healing
    if (Delta >= 0.f) return;

    // Don't inflict a new wound if already wounded
    if (BandageComponent->GetWoundData().bIsWounded) return;

    float HealthPercent = NewHealth / HealthComponent->GetMaxHealth();

    if (HealthPercent <= 0.25f)
    {
        // Below 25% — severe wound
        BandageComponent->InflictWound(EWoundSeverity::Severe);
    }
    else if (HealthPercent <= 0.50f)
    {
        // Below 50% — moderate wound
        BandageComponent->InflictWound(EWoundSeverity::Moderate);
    }
    else if (HealthPercent <= 0.75f)
    {
        // Below 75% — minor wound
        BandageComponent->InflictWound(EWoundSeverity::Minor);
    }
}

void AMGP_2526Character::OnWrapClicked()
{
    if (!BandageComponent) return;
    BandageComponent->ApplyWrap();
}

void AMGP_2526Character::InflictWound(EWoundSeverity Severity)
{
    if (!BandageComponent) return;
    BandageComponent->InflictWound(Severity);
}

