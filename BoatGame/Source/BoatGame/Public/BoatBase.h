/*
Copyright (C) : 2022 Fatih YILMAZER
Contact: fatihyilm4zer@gmail.com
Github:  https://github.com/FatihYilmazer/Endless-Runner-Boat-Game
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "BoatBase.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class AProceduralPlatformGeneratorBase;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UAudioComponent;

UCLASS()
class BOATGAME_API ABoatBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABoatBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UFUNCTION()
		void DetachCurrentTurnLocation();
	UFUNCTION()
		UStaticMeshComponent* GetDriverMesh();
	UFUNCTION()
		void AddMaxSpeed(float AddValue);
	UFUNCTION(BlueprintImplementableEvent)
		void StartHUD();
	UFUNCTION(BlueprintImplementableEvent)
		void UpdatePointText(int32 PointCount);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateHUDOnCrash();
	UFUNCTION(BlueprintImplementableEvent)
		void ResetHUD();
	UFUNCTION(BlueprintImplementableEvent)
		void ShowTurnSignInHUD();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		AProceduralPlatformGeneratorBase* ProceduralPlatformGenerator;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool bIsTurnSignVisible;

	/*********************Input*************************************************************/

	UFUNCTION(BlueprintCallable)
		void Tap(bool bIsCallFromRightButton);
	UFUNCTION(BlueprintCallable)
		void MoveRight();
	UFUNCTION(BlueprintCallable)
		void MoveLeft();
	UFUNCTION(BlueprintCallable)
		void ResetTargetVelocityDirection();

private:

	/*********************Components********************************************************/

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SM_Boat;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SM_Driver;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UCameraComponent* Camera;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* BGSound;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* PointSound;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* CrashSound;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UFloatingPawnMovement* FloatingPawnMovement;

	/*********************General*************************************************************/

	UFUNCTION()
		void OnRestart();
	UFUNCTION()
		void SetMovement();
	UFUNCTION()
		void SetBoatRotation();
	UFUNCTION()
		void SetCameraMovement();
	UFUNCTION()
		void AttachTurnLocation(bool bIsCallFromRightButton);
	UFUNCTION()
		void  Detach();
	UFUNCTION()
		void ResetDriverAnim();
	UFUNCTION()
		void SetDriverCrashAnim();
	UFUNCTION()
		void OnCrash(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
			FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void OnPointOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UPROPERTY()
		int32 CollectedPointCount;
	UPROPERTY()
		UMaterialInterface* MI_DriverParent;
	UPROPERTY()
		UMaterialInstanceDynamic* MI_Driver;
	UPROPERTY()
		bool bIsTurning;
	UPROPERTY()
		bool bIsTurningCW;
	UPROPERTY()
		FTransform CurrentTurnTransform;
	UPROPERTY()
		float CamAlpha;
	UPROPERTY()
		FVector LastForwardVelocityDirection;
	UPROPERTY()
		FVector TargetVelocityDirection;
	UPROPERTY()
		FRotator TargetRotation;
	UPROPERTY()
		float BoatInterpSpeed;
	UPROPERTY()
		bool bIsInCrashState;
	UPROPERTY()
		FTimerHandle DetachTimerHandle;
};