// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "BandageComponent.h"
#include "Health.h"
#include "MGP_2526Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class AMGP_2526Character : public ACharacter
{
    GENERATED_BODY()


    //Camera boom positioning the camera behind the character
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    //Follow camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    //First-person arm mesh shown only during bandaging
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* BandageArmMesh;

    //Camera that looks down at the arm during bandaging
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* BandageCamera;

    //The bandage component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBandageComponent* BandageComponent;

protected:


    //Health component to track HP and death
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivate = "true"))
    UHealth* HealthComponent;

    //Jump Input Action
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    //Move Input Action
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    //Look Input Action
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    // Mouse Look Input Action
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MouseLookAction;

    //Heal key
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* HealAction;

    //Wrap click
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* WrapAction;

public:

    AMGP_2526Character();

protected:

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    //Input handlers for bandaging
    void OnHealPressed();
    void OnWrapClicked();

public:

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoLook(float Yaw, float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

    //Expose for Blueprint use if needed
    UFUNCTION(BlueprintCallable, Category = "Health")
    void InflictWound(EWoundSeverity Severity);

public:

    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE UBandageComponent* GetBandageComponent() const { return BandageComponent; }
};
