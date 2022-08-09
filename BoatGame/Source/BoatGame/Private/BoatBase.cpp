/*
Copyright (C) : 2022 Fatih YILMAZER
Contact: fatihyilm4zer@gmail.com
Github:  https://github.com/FatihYilmazer/Endless-Runner-Boat-Game
*/

#include "BoatBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "ProceduralPlatformGeneratorBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/AudioComponent.h"

//CONFIG

#define BOAT_MAX_YAW					  45.0f
#define TURN_DISTANCE_THRESHOLD			1000.0f
#define SPRING_ARM_PITCH				 -17.5f
#define SPRING_ARM_LENGTH				 850.0f
#define CAST_SHADOW						  false

const FAnimInfo AnimInfo[4] = { FAnimInfo(1.0f, 0.967742f), FAnimInfo(31.0f, 0.652174f), FAnimInfo(61.0f, 0.576923f), FAnimInfo(91.0f, 0.560748f)};

// Sets default values
ABoatBase::ABoatBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SM_Boat = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Boat"));
	RootComponent = SM_Boat;
	SM_Boat->SetMobility(EComponentMobility::Movable);
	SM_Boat->PrimaryComponentTick.bStartWithTickEnabled = false;
	SM_Boat->SetGenerateOverlapEvents(true);
	SM_Boat->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SM_Boat->SetCollisionProfileName(FName("Boat"), false);
	SM_Boat->CastShadow = CAST_SHADOW;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetBoat(TEXT("/Game/Assets/Boat/SM_Motorboat.SM_Motorboat"));
	if (FoundMeshAssetBoat.Succeeded())
	{
		SM_Boat->SetStaticMesh(FoundMeshAssetBoat.Object);
	}

	SM_Driver = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Driver"));
	SM_Driver->SetupAttachment(RootComponent);
	SM_Driver->SetRelativeTransform(FTransform({0.0f, -90.0f, 0.0f}, FVector(-90.0f, 25.0f, 0.0f), FVector(0.85f, 0.85f, 0.85f)));
	SM_Driver->SetMobility(EComponentMobility::Movable);
	SM_Driver->PrimaryComponentTick.bStartWithTickEnabled = false;
	SM_Driver->SetGenerateOverlapEvents(false);
	SM_Driver->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SM_Driver->SetCollisionProfileName(FName("NoCollision"), false);
	SM_Driver->CastShadow = CAST_SHADOW;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetDriver(TEXT("/Game/Assets/Driver/SM_Driver.SM_Driver"));
	if (FoundMeshAssetDriver.Succeeded())
	{
		SM_Driver->SetStaticMesh(FoundMeshAssetDriver.Object);
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeTransform(FTransform({ SPRING_ARM_PITCH, 0.0f, 0.0f }, FVector(325.0f, 0.0f, 10.0f), FVector::OneVector));
	SpringArm->TargetArmLength = SPRING_ARM_LENGTH;
	SpringArm->bDoCollisionTest = false;
	SpringArm->PrimaryComponentTick.bStartWithTickEnabled = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	BGSound = CreateDefaultSubobject<UAudioComponent>(TEXT("BGSound"));
	BGSound->SetupAttachment(RootComponent);
	BGSound->VolumeMultiplier = 8.0f;
	BGSound->PrimaryComponentTick.bStartWithTickEnabled = false;
	BGSound->bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> FoundMeshAssetBGSound(TEXT("/Game/Assets/Sound/BGMusic_Cue_Audio.BGMusic_Cue_Audio"));
	if (FoundMeshAssetBGSound.Succeeded())
	{
		BGSound->Sound = FoundMeshAssetBGSound.Object;
	}

	PointSound = CreateDefaultSubobject<UAudioComponent>(TEXT("PointSound"));
	PointSound->SetupAttachment(RootComponent);
	PointSound->VolumeMultiplier = 0.75f;
	PointSound->PrimaryComponentTick.bStartWithTickEnabled = false;
	PointSound->bAutoActivate = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> FoundMeshAssetPointSound(TEXT("/Game/Assets/Sound/Coins_10_wav_Cue.Coins_10_wav_Cue"));
	if (FoundMeshAssetPointSound.Succeeded())
	{
		PointSound->Sound = FoundMeshAssetPointSound.Object;
	}

	CrashSound = CreateDefaultSubobject<UAudioComponent>(TEXT("CrashSound"));
	CrashSound->SetupAttachment(RootComponent);
	CrashSound->VolumeMultiplier = 4.0f;
	CrashSound->PrimaryComponentTick.bStartWithTickEnabled = false;
	CrashSound->bAutoActivate = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> FoundMeshAssetCrashSound(TEXT("/Game/Assets/Sound/Error_Buzz_05_wav_Cue.Error_Buzz_05_wav_Cue"));
	if (FoundMeshAssetCrashSound.Succeeded())
	{
		CrashSound->Sound = FoundMeshAssetCrashSound.Object;
	}

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->MaxSpeed = 2075.0f;
	FloatingPawnMovement->Acceleration = 2150.0f;
	FloatingPawnMovement->Deceleration = 80000.0f;
	FloatingPawnMovement->TurningBoost = 0.6f;
	FloatingPawnMovement->SetPlaneConstraintNormal(FVector::UpVector);
	FloatingPawnMovement->bConstrainToPlane = true;
	FloatingPawnMovement->bSnapToPlaneAtStart = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FoundMaterialAsset(TEXT("/Game/Assets/Driver/M_Driver.M_Driver"));
	if (FoundMaterialAsset.Succeeded())
	{
		MI_DriverParent = FoundMaterialAsset.Object;
	}

	CollectedPointCount = 0;
	bIsTurning = false;
	CamAlpha = 0.0f;
	BoatInterpSpeed = 8.0f;
	LastForwardVelocityDirection = FVector::ForwardVector;
	TargetVelocityDirection = FVector::ForwardVector;
	bIsInCrashState = false;
}

// Called when the game starts or when spawned
void ABoatBase::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);
	
	SM_Boat->OnComponentHit.AddDynamic(this, &ABoatBase::OnCrash);
	SM_Boat->OnComponentBeginOverlap.AddDynamic(this, &ABoatBase::OnPointOverlap);

	MI_Driver = UMaterialInstanceDynamic::Create(MI_DriverParent, this);
	SM_Driver->SetMaterial(0, MI_Driver);
	ResetDriverAnim();
}

// Called every frame
void ABoatBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetMovement();
	SetBoatRotation();
	SetCameraMovement();

	if (!bIsTurnSignVisible && FVector2D::DistSquared(FVector2D(ProceduralPlatformGenerator->AvailableTurnTransform.GetLocation()), FVector2D(GetActorLocation()))
		<= TURN_DISTANCE_THRESHOLD * TURN_DISTANCE_THRESHOLD)
	{
		bIsTurnSignVisible = true;
		ShowTurnSignInHUD();
	}
}

// Called to bind functionality to input
void ABoatBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABoatBase::DetachCurrentTurnLocation()
{
	if (bIsTurning)
	{
		GetWorldTimerManager().SetTimer(DetachTimerHandle, this, &ABoatBase::Detach, 0.1f, false);
	}
}

UStaticMeshComponent * ABoatBase::GetDriverMesh()
{
	return SM_Driver;
}

void ABoatBase::AddMaxSpeed(float AddValue)
{
	FloatingPawnMovement->MaxSpeed += AddValue;
}

/*********************Input*************************************************************/

void ABoatBase::Tap(bool bIsCallFromRightButton)
{
	if (IsActorTickEnabled())
	{
		AttachTurnLocation(bIsCallFromRightButton);
		return;
	}
	if (bIsInCrashState)
	{
		OnRestart();
		UpdatePointText(CollectedPointCount);
		return;
	}
	SetActorTickEnabled(true);
	StartHUD();
}

void ABoatBase::MoveRight()
{
	TargetVelocityDirection = GetActorForwardVector() + GetActorRightVector();
}

void ABoatBase::MoveLeft()
{
	TargetVelocityDirection = GetActorForwardVector() + GetActorRightVector() * -1;
}

void ABoatBase::ResetTargetVelocityDirection()
{
	TargetVelocityDirection = LastForwardVelocityDirection;
}

/*********************General*************************************************************/

void ABoatBase::OnRestart()
{
	SetActorTickEnabled(false);
	SetActorLocationAndRotation(FVector::ZeroVector, {0.f, 0.f, 0.f}, false, nullptr, ETeleportType::None);
	SpringArm->SetRelativeTransform(FTransform({ SPRING_ARM_PITCH, 0.0f, 0.0f }, FVector(325.0f, 0.0f, 10.0f), FVector::OneVector));
	Camera->SetRelativeLocation({ 0.0f, 0.0f, 0.0f });
	ProceduralPlatformGenerator->OnRestart();
	CollectedPointCount = 0;
	bIsTurning = false;
	CamAlpha = 0.0f;
	LastForwardVelocityDirection = FVector::ForwardVector;
	TargetVelocityDirection = FVector::ForwardVector;
	bIsInCrashState = false;
	ResetDriverAnim();
	bIsTurnSignVisible = false;
	ResetHUD();
}

void ABoatBase::SetMovement()
{
	if (bIsTurning)
	{
		FVector2D Temp = FVector2D((GetActorLocation() - CurrentTurnTransform.GetLocation()).GetSafeNormal(0.0001f));
		//Rotates around axis (0,0,1)
		Temp = Temp.GetRotated(bIsTurningCW ? 90.0f : -90.0f);
		TargetVelocityDirection.X = Temp.X;
		TargetVelocityDirection.Y = Temp.Y;
	}
	FloatingPawnMovement->AddInputVector(TargetVelocityDirection);
}

void ABoatBase::SetBoatRotation()
{
	if (bIsTurning)
	{
		float RotationOffset = bIsTurningCW ? BOAT_MAX_YAW : BOAT_MAX_YAW * -1;
		TargetRotation = { 0.0f, FRotationMatrix::MakeFromX(FloatingPawnMovement->Velocity).Rotator().Yaw + RotationOffset, 0.0f };
		BoatInterpSpeed = 200.0f;
	}
	else
	{
		TargetRotation = FRotationMatrix::MakeFromX(FloatingPawnMovement->Velocity).Rotator();
		BoatInterpSpeed = 8.0f;
	}
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), 0.5f + CamAlpha * (BoatInterpSpeed - 0.5f)));
}

void ABoatBase::SetCameraMovement()
{
	FRotator TargetRot = { SPRING_ARM_PITCH, FRotationMatrix::MakeFromX(LastForwardVelocityDirection).Rotator().Yaw, 0.0f };
	SpringArm->SetWorldRotation(FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, UGameplayStatics::GetWorldDeltaSeconds(this), 5.0f));

	if (CamAlpha < 1.0f)
	{
		float Target;

		if (bIsTurning)
		{
			CamAlpha += UGameplayStatics::GetWorldDeltaSeconds(this) / 0.75f;
			Target = -400.0f;
		}
		else
		{
			CamAlpha += UGameplayStatics::GetWorldDeltaSeconds(this) / 3.5f;
			Target = 0.0f;
		}

		Camera->SetRelativeLocation(
			{ UKismetMathLibrary::Ease(Camera->GetRelativeLocation().X, Target, CamAlpha, EEasingFunc::EaseIn, 2.0f, -1), 0.0f, 0.0f },
			false, nullptr, ETeleportType::None);
	}
}

void ABoatBase::AttachTurnLocation(bool bIsCallFromRightButton)
{
	if (bIsTurning)
	{
		return;
	}
	if (FVector2D::DistSquared(FVector2D(ProceduralPlatformGenerator->AvailableTurnTransform.GetLocation()), FVector2D(GetActorLocation()))
		<= TURN_DISTANCE_THRESHOLD * TURN_DISTANCE_THRESHOLD)
	{
		bIsTurningCW = FVector::DotProduct(GetActorRightVector(), (GetActorLocation() - ProceduralPlatformGenerator->AvailableTurnTransform.GetLocation()).GetSafeNormal(0.0001f)) <= 0.0f;
		if ((bIsTurningCW && bIsCallFromRightButton) || (!bIsTurningCW && !bIsCallFromRightButton))
		{
			CurrentTurnTransform = ProceduralPlatformGenerator->AvailableTurnTransform;
			bIsTurning = true;
			FloatingPawnMovement->TurningBoost = 110.0f;
			LastForwardVelocityDirection = CurrentTurnTransform.TransformVectorNoScale(FVector::ForwardVector);
			CamAlpha = 0.0f;
		}
	}
}

void ABoatBase::Detach()
{
	bIsTurning = false;
	FloatingPawnMovement->TurningBoost = 0.6f;
	TargetVelocityDirection = LastForwardVelocityDirection;
	CamAlpha = 0.0f;
	GetWorldTimerManager().ClearTimer(DetachTimerHandle);
}

void ABoatBase::ResetDriverAnim()
{
	MI_Driver->SetScalarParameterValue(FName("AnimNumber"), AnimInfo[0].AnimNumber);
	MI_Driver->SetScalarParameterValue(FName("AnimSpeed"), AnimInfo[0].AnimSpeed);
}

void ABoatBase::SetDriverCrashAnim()
{
	int32 RandomCrashAnimIndex = FMath::RandHelper(3) + 1;
	MI_Driver->SetScalarParameterValue(FName("AnimNumber"), AnimInfo[RandomCrashAnimIndex].AnimNumber);
	MI_Driver->SetScalarParameterValue(FName("AnimSpeed"), AnimInfo[RandomCrashAnimIndex].AnimSpeed);
}

void ABoatBase::OnCrash(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
	bIsInCrashState = true;
	SetActorTickEnabled(false);
	SetDriverCrashAnim();
	CrashSound->Play(0.0f);
	UpdateHUDOnCrash();
}

void ABoatBase::OnPointOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (UHierarchicalInstancedStaticMeshComponent* Ref = Cast<UHierarchicalInstancedStaticMeshComponent>(OtherComp))
	{
		if (Ref != ProceduralPlatformGenerator->GetHISMPoints())
		{
			return;
		}
		ProceduralPlatformGenerator->RemovePoint(OtherBodyIndex);
		PointSound->Play(0.0f);
		CollectedPointCount++;
		UpdatePointText(CollectedPointCount);
	}
}
