/*
Copyright (C) : 2022 Fatih YILMAZER 
Contact: fatihyilm4zer@gmail.com
Github:  https://github.com/FatihYilmazer/Endless-Runner-Boat-Game
*/ 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ProceduralPlatformGeneratorBase.generated.h"

/*
Generation Rules

1. This actor world location should be (0,0,0). Use GenerationStartingLocation and GenerationDirection instead of this actor location.

2. Cannot place Tile2RL type tiles 3 times in a row.

3. BaseTiles and LVLSwitchingTiles types always have to be Tile2.

4. BaseTileCount and LVLSwitchingTileCount tile count always have to be more than 3.

*/

class USceneComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UBoxComponent;
class UStaticMeshComponent;
class ABoatBase;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Road2		UMETA(DisplayName = "Road2"),
	Road2RL		UMETA(DisplayName = "Road2RL")
};

UENUM(BlueprintType)
enum class ETile2RLConnectionType : uint8
{
	Back		UMETA(DisplayName = "Back"),
	Right		UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EObstacleOrientation : uint8
{
	Left		UMETA(DisplayName = "Left"),
	Mid			UMETA(DisplayName = "Mid"),
	Right		UMETA(DisplayName = "Right"),
	LeftMid		UMETA(DisplayName = "LeftMid"),
	MidRight	UMETA(DisplayName = "MidRight"),
	LeftRight	UMETA(DisplayName = "LeftRight")
};

USTRUCT(BlueprintType)
struct FTileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		ETileType TileType;

	//Just meaningful for Tile2RL
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		ETile2RLConnectionType FirstConnectionType;

	//Just meaningful for Tile2RL
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsOtherDirectionDeadEnd = false;


	FTileInfo()
	{
		TileType = ETileType::Road2;
		FirstConnectionType = ETile2RLConnectionType::Back;
		bIsOtherDirectionDeadEnd = false;
	}

	FTileInfo(ETileType NewTileType, ETile2RLConnectionType NewTile2RLConnectionType, bool bNewIsOtherDirectionDeadEnd)
	{
		TileType = NewTileType;
		FirstConnectionType = NewTile2RLConnectionType;
		bIsOtherDirectionDeadEnd = bNewIsOtherDirectionDeadEnd;
	}

};

USTRUCT(BlueprintType)
struct FAnimInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float AnimNumber;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float AnimSpeed;

	FAnimInfo()
	{
		AnimNumber = 0.0f;
		AnimSpeed = 0.0f;
	}

	FAnimInfo(float NewAnimNumber, float NewAnimSpeed)
	{
		AnimNumber = NewAnimNumber;
		AnimSpeed = NewAnimSpeed;
	}

};

USTRUCT(BlueprintType)
struct FHISMIndexes
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<int32> Indexes;
};


UCLASS()
class BOATGAME_API AProceduralPlatformGeneratorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralPlatformGeneratorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION(BlueprintCallable)
		void SetupCustomData();
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentObstacleDifficulty();
	UFUNCTION()
		void OnNewGame();
	UFUNCTION()
		void OnRestart();
	UFUNCTION()
		UHierarchicalInstancedStaticMeshComponent* GetHISMPoints();
	UFUNCTION()
		void RemovePoint(int32 InstanceIndex);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		ABoatBase* PawnBoat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FTransform AvailableTurnTransform;

private:

	/*********************Components********************************************************/

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UHierarchicalInstancedStaticMeshComponent* HISM_Tile2;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UHierarchicalInstancedStaticMeshComponent* HISM_Tile2RL;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UHierarchicalInstancedStaticMeshComponent* HISM_Points;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UBoxComponent* CB_Tile2RL;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SM_SkySphere;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SM_Env_Skyline;

	/*********************General*************************************************************/

	UFUNCTION()
		void GenerateProceduralPlatform();
	UFUNCTION()
		void GenerateLVLSwitch();
	UFUNCTION()
		void ClearOldItems();
	UFUNCTION()
		void On2RLCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
		void UpdateTile2RLCollisionBoxLocation();
	UFUNCTION()
		bool FindNext2RLTileIndexInTileInfo();
	UFUNCTION()
		void SetAvailableTurnTransform();
	UFUNCTION()
		void IncrementCurrentLVL();

	UPROPERTY()
		int32 CurrentLVL;
	UPROPERTY()
		int32 Iterator;
	UPROPERTY()
		bool bShouldGenerateNewPlatform;
	UPROPERTY()
		bool bShouldGenerateNewLVLSwitch;
	UPROPERTY()
		TArray<int32> PointsIndexes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UHierarchicalInstancedStaticMeshComponent*> Obstacles;
	UPROPERTY()
		TArray<FHISMIndexes> ObstaclesIndexes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UHierarchicalInstancedStaticMeshComponent*> People;
	UPROPERTY()
		TArray<FHISMIndexes> PeopleIndexes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<UHierarchicalInstancedStaticMeshComponent*> EnvironmentItems;
	UPROPERTY()
		TArray<FHISMIndexes> EnvironmentItemsIndexes;
	UPROPERTY()
		FTimerHandle CBTile2RLTimerHandle;


	/*********************Tile Generation*************************************************************/

	UFUNCTION()
		void InitializeTileGeneration();
	UFUNCTION()
		void GenerateTiles();
	UFUNCTION()
		void ClearOldRoadTiles();
	UFUNCTION()
		void UpdateTileGenerationParameters();
	UFUNCTION()
		void GetHISMLastInstanceTransform(UHierarchicalInstancedStaticMeshComponent* HISM, FTransform &LastInstanceTransform) const;
	UFUNCTION()
		void RemoveLastTile();
	UFUNCTION()
		void AttachTile2To2();
	UFUNCTION()
		void AttachTile2To2RL(const bool bIsSpecifiedConnection = false, const ETile2RLConnectionType SpecifiedConnectionType = ETile2RLConnectionType::Back);
	UFUNCTION()
		void AttachTile2RLTo2();
	UFUNCTION()
		void AttachTile2RLTo2RL();
	UFUNCTION()
		bool IsNextTileFull();
	UFUNCTION()
		void ArrangeGenerationOnNextTileFull();
	UFUNCTION()
		void FlipLast2RLTile();

	//LVLs

	UFUNCTION()
		void LVL1();
	UFUNCTION()
		void LVL2();
	UFUNCTION()
		void LVL3();

	UPROPERTY()
		TArray<FTileInfo> TileInfo;
	UPROPERTY()
		FVector GenerationStartingLocation;
	UPROPERTY()
		FVector GenerationDirection;
	UPROPERTY()
		int32 BaseTileCount;
	UPROPERTY()
		int32 OldLVLSwitchingTileCount;
	UPROPERTY()
		int32 GeneratedTileCount;
	UPROPERTY()
		int32 LVLSwitchingTileCount;
	UPROPERTY()
		int32 Tile2Streak;
	UPROPERTY()
		int32 Tile2RLStreak;
	UPROPERTY()
		int32 CurrentTile2RLIndex;

	/*********************Points and Static Obstacles*************************************************************/

	UFUNCTION()
		void GeneratePlatformPointsAndStaticObstacles();
	UFUNCTION()
		void GenerateLVLSwitchPointsAndStaticObstacles();
	UFUNCTION()
		void SetStaticObstacleOrientation(int32 ObstacleHardness);
	UFUNCTION()
		void PlaceObstacleAndSetPointTransforms(FTransform &Tile2Transform, FVector &XOffset, TArray<FTransform> &PointTransforms);
	UFUNCTION()
		void SetPointTransformsWithoutPlacingObstacle(FTransform &Tile2Transform, TArray<FTransform> &PointTransforms);
	UFUNCTION()
		void AddRandomStaticObstacle(const FTransform &InstanceTransform);
	UFUNCTION()
		void EasyLVL1();
	UFUNCTION()
		void EasyLVL2();
	UFUNCTION()
		void HardLVL1();
	UFUNCTION()
		void HardLVL2();
	UFUNCTION()
		void HardLVL3();
	UFUNCTION()
		void ZigZagPattern();
	UFUNCTION()
		void PingPongPatternLeft();
	UFUNCTION()
		void PingPongPatternRight();
	UFUNCTION()
		void AddPoint(FVector &Location, FVector ForwardVector, TArray<FTransform> &PointTransforms);
	UFUNCTION()
		void CanoeOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
			FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void Pedalboat1OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
			FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void Pedalboat2OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
			FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
		int32 PointCountPerTurn;
	UPROPERTY()
		bool bIsFirstObstacleTileAfterTurn;
	UPROPERTY()
		bool bIsXOrientationMid;
	UPROPERTY()
		EObstacleOrientation LastObstacleOrientation;
	UPROPERTY()
		bool bZigZagPatternInfo;
	UPROPERTY()
		int32 CurrentObstacleGenerationState;
	UPROPERTY()
		bool bIsObstacleBoatGotHit;

	/*********************People*************************************************************/
	
	UFUNCTION()
		void GeneratePeople();
	UFUNCTION()
		void GenerateObstacleBoatPeople(int32 ObstacleBoaType, const TArray<FVector> &LocalSitLocations);
	UFUNCTION()
		void OnStaticObstacleBoatHit(UHierarchicalInstancedStaticMeshComponent* ObstacleBoatRef, const int32 HitItem, const TArray<FVector> &LocalSitLocations);
	UFUNCTION()
		void SetPeopleCollisionProfile(FName CollisionProfileName);
	UFUNCTION()
		void ChangeHumanAnimOnCrash(FTransform &BoatTransform, FVector LocalSitLocation);

	/*********************Environment*************************************************************/

	UFUNCTION()
		void InitializeEnvironmentGeneration();
	UFUNCTION()
		void GeneratePlatformEnvironment();
	UFUNCTION()
		void GenerateLVLSwitchEnvironment();
	UFUNCTION()
		void AddEnvironmentToTile2(int32 Tile2Index, bool bIsBothSide = true, bool bIsRight = false);
	UFUNCTION()
		void AddEnvironmentToTile2RL(int32 Tile2Index, int32 TileInfoIndex, int32 Tile2RLIndex);
	UFUNCTION()
		void AddEnvironment(FTransform InstanceTransform, bool bIsCorner);
	UFUNCTION()
		void AddEnvironmentToTile2RLArc(int32 InstanceIndex);
	UFUNCTION()
		int32 AddSign(int32 BuildingType, int32 BuildingIndex, int32 SignType);
	UFUNCTION()
		int32 AddProp(int32 BuildingType, int32 BuildingIndex, int32 PropType, float Offset);
	UFUNCTION()
		void SetUpBackground();
};