/*
Copyright (C) : 2022 Fatih YILMAZER
Contact: fatihyilm4zer@gmail.com
Github:  https://github.com/FatihYilmazer/Endless-Runner-Boat-Game
*/

#include "ProceduralPlatformGeneratorBase.h"
#include "BoatBase.h"
#include "Components/SceneComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

//Trace Channels
#define TC_TILE					ECC_GameTraceChannel2

//Utility

#define getlasttileinfo()	 (TileInfo[Iterator-1])
#define getrandomcrashanim() (AnimInfo[DANCE_ANIM_COUNT + FMath::RandHelper(CRASH_ANIM_COUNT)])
#define getrandomdanceanim() (AnimInfo[FMath::RandHelper(DANCE_ANIM_COUNT)])

//CONFIG

#define CAST_SHADOW										false

/*********************Tile Generation*************************************************************/

#define TILE2_SIZE									   2498.0f 
#define TILE2RL_SIZE									998.0f 
#define DEFAULT_STARTING_LOCATION_X			   TILE2_SIZE * -1
#define DEFAULT_BASE_TILE_COUNT							     4
#define DEFAULT_GENERATED_TILE_COUNT					    15
#define DEFAULT_LVL_SWITCHING_TILE_COUNT					10
#define IS_NEXT_TILE_FULL_LINE_TRACE_MINUS_Z_OFFSET	    350.0f

/*********************Points and Static Obstacles*************************************************************/

#define TILE2_PLACE_X_OFFSET						   1050.0f
#define TILE2_PLACE_Y_OFFSET							300.0f
#define STATIC_OBSTACLE_COUNT								 3
#define BOAT_OBSTACLE_COLOR_COUNT						     6
#define MAX_STATIC_OBSTACLE_VARIATION_COUNT				    11
#define POINT_EDGE_LIMIT									 2
#define POINT_GAP										200.0f

/*********************People*************************************************************/

#define HUMAN_TYPE_COUNT								     7
#define POLYGON_TOWN_TEXTURE_VARIATION_COUNT				 4
#define PEOPLE_SCALE									 0.85f

const FAnimInfo AnimInfo[12] = { 
									FAnimInfo(1.0f, 0.857143f), FAnimInfo(31.0f, 0.530973f), FAnimInfo(61.0f, 0.344828f),  
									FAnimInfo(91.0f, 1.0f), FAnimInfo(121.0f, 0.521739f), FAnimInfo(151.0f, 0.413793f), 
									FAnimInfo(181.0f, 0.294118f), FAnimInfo(211.0f, 0.344828f), FAnimInfo(241.0f, 0.869565f), 
									FAnimInfo(271.0f, 0.508475f) 
							   };

const FVector CanoeLocalSitLocations[2] = { {90.0f, 0.0f, 0.0f}, {-120.0f, 0.0f, 0.0f} };
const FVector PedalboatLocalSitLocations[2] = { {-40.0f, -50.0f, 0.0f}, {-40.0f, 50.0f, 0.0f} };

#define DANCE_ANIM_COUNT							         4
#define CRASH_ANIM_COUNT									 6

/*********************Environment*************************************************************/

#define POLYGON_CITY_TEXTURE_VARIATION_COUNT			     3
#define BILLBOARD_TEXTURE_VARIATION_COUNT					 8
#define NORMAL_BUILDING_COUNT								 9
#define SPECIAL_BUILDING_COUNT								 6
#define FILLER_BUILDING_COUNT								 2
#define SIGN_COUNT											15
#define PROP_COUNT											17
#define PROP_OFFSET										750.0f

// Sets default values
AProceduralPlatformGeneratorBase::AProceduralPlatformGeneratorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/*********************Components********************************************************/

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Static);
	Root->PrimaryComponentTick.bStartWithTickEnabled = false;

	HISM_Tile2 = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM_Tile2"));
	HISM_Tile2->SetupAttachment(Root);
	HISM_Tile2->SetMobility(EComponentMobility::Static);
	HISM_Tile2->PrimaryComponentTick.bStartWithTickEnabled = false;
	HISM_Tile2->SetGenerateOverlapEvents(false);
	HISM_Tile2->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	HISM_Tile2->SetCollisionProfileName(FName("Tile"), false);
	HISM_Tile2->CastShadow = CAST_SHADOW;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetTile2(TEXT("/Game/Assets/Canal/SM_Canal2.SM_Canal2"));
	if (FoundMeshAssetTile2.Succeeded())
	{
		HISM_Tile2->SetStaticMesh(FoundMeshAssetTile2.Object);
	}

	HISM_Tile2RL = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM_Tile2RL"));
	HISM_Tile2RL->SetupAttachment(Root);
	HISM_Tile2RL->SetMobility(EComponentMobility::Static);
	HISM_Tile2RL->PrimaryComponentTick.bStartWithTickEnabled = false;
	HISM_Tile2RL->SetGenerateOverlapEvents(false);
	HISM_Tile2RL->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	HISM_Tile2RL->SetCollisionProfileName(FName("Tile"), false);
	HISM_Tile2RL->CastShadow = CAST_SHADOW;
	HISM_Tile2RL->NumCustomDataFloats = 1;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetTile2RL(TEXT("/Game/Assets/Canal/SM_Canal2RL.SM_Canal2RL"));
	if (FoundMeshAssetTile2RL.Succeeded())
	{
		HISM_Tile2RL->SetStaticMesh(FoundMeshAssetTile2RL.Object);
	}

	HISM_Points = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM_Points"));
	HISM_Points->SetupAttachment(Root);
	HISM_Points->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f), false, nullptr, ETeleportType::TeleportPhysics);
	HISM_Points->SetMobility(EComponentMobility::Static);
	HISM_Points->PrimaryComponentTick.bStartWithTickEnabled = false;
	HISM_Points->SetGenerateOverlapEvents(true);
	HISM_Points->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	HISM_Points->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HISM_Points->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	HISM_Points->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HISM_Points->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	HISM_Points->bMultiBodyOverlap = true;
	HISM_Points->CastShadow = CAST_SHADOW;
	HISM_Points->NumCustomDataFloats = 1;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetPoints(TEXT("/Game/Assets/Points/SM_Fish.SM_Fish"));
	if (FoundMeshAssetPoints.Succeeded())
	{
		HISM_Points->SetStaticMesh(FoundMeshAssetPoints.Object);
	}

	CB_Tile2RL = CreateDefaultSubobject<UBoxComponent>(TEXT("CB_Tile2RL"));
	CB_Tile2RL->SetupAttachment(Root);
	CB_Tile2RL->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f), false, nullptr, ETeleportType::TeleportPhysics);
	CB_Tile2RL->SetMobility(EComponentMobility::Movable);
	CB_Tile2RL->InitBoxExtent(FVector(10.0f, 500.0f, 150.0f));
	CB_Tile2RL->PrimaryComponentTick.bStartWithTickEnabled = false;
	CB_Tile2RL->SetGenerateOverlapEvents(true);
	CB_Tile2RL->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	CB_Tile2RL->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CB_Tile2RL->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CB_Tile2RL->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CB_Tile2RL->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	SM_SkySphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_SkySphere"));
	SM_SkySphere->SetupAttachment(Root);
	SM_SkySphere->SetMobility(EComponentMobility::Movable);
	SM_SkySphere->PrimaryComponentTick.bStartWithTickEnabled = false;
	SM_SkySphere->SetGenerateOverlapEvents(false);
	SM_SkySphere->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SM_SkySphere->SetCollisionProfileName(FName("NoCollision"), false);
	SM_SkySphere->CastShadow = CAST_SHADOW;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetSkySphere(TEXT("/Game/Assets/Environment/SM_SkySphere.SM_SkySphere"));
	if (FoundMeshAssetSkySphere.Succeeded())
	{
		SM_SkySphere->SetStaticMesh(FoundMeshAssetSkySphere.Object);
	}

	SM_Env_Skyline = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Env_Skyline"));
	SM_Env_Skyline->SetupAttachment(Root);
	SM_Env_Skyline->SetRelativeScale3D(FVector(2.75f, 2.75f, 2.75f));
	SM_Env_Skyline->SetMobility(EComponentMobility::Movable);
	SM_Env_Skyline->PrimaryComponentTick.bStartWithTickEnabled = false;
	SM_Env_Skyline->SetGenerateOverlapEvents(false);
	SM_Env_Skyline->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SM_Env_Skyline->SetCollisionProfileName(FName("NoCollision"), false);
	SM_Env_Skyline->CastShadow = CAST_SHADOW;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMeshAssetSkyline(TEXT("/Game/Assets/Environment/SM_Env_Skyline.SM_Env_Skyline"));
	if (FoundMeshAssetSkyline.Succeeded())
	{
		SM_Env_Skyline->SetStaticMesh(FoundMeshAssetSkyline.Object);
	}
}

// Called when the game starts or when spawned
void AProceduralPlatformGeneratorBase::BeginPlay()
{
	Super::BeginPlay();

	Obstacles[0]->OnComponentHit.AddDynamic(this, &AProceduralPlatformGeneratorBase::CanoeOnHit);
	Obstacles[1]->OnComponentHit.AddDynamic(this, &AProceduralPlatformGeneratorBase::Pedalboat1OnHit);
	Obstacles[2]->OnComponentHit.AddDynamic(this, &AProceduralPlatformGeneratorBase::Pedalboat2OnHit);
	CB_Tile2RL->OnComponentBeginOverlap.AddDynamic(this, &AProceduralPlatformGeneratorBase::On2RLCollisionBoxOverlap);

	OnNewGame();
}

// Called every frame
void AProceduralPlatformGeneratorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralPlatformGeneratorBase::SetupCustomData()
{
	for (int32 i = 0; i < Obstacles.Num(); i++)
	{
		Obstacles[i]->NumCustomDataFloats = 1;
	}

	for (int32 i = 0; i < People.Num(); i++)
	{
		People[i]->NumCustomDataFloats = 3;
	}

	for (int32 i = 4; i < 16; i++)
	{
		EnvironmentItems[i]->NumCustomDataFloats = 1;
	}
	EnvironmentItems[8]->NumCustomDataFloats = 2;
	EnvironmentItems[32]->NumCustomDataFloats = 2;
}

int32 AProceduralPlatformGeneratorBase::GetCurrentObstacleDifficulty()
{
	return CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT;
}

void AProceduralPlatformGeneratorBase::OnNewGame()
{
	ObstaclesIndexes.SetNum(STATIC_OBSTACLE_COUNT);
	PeopleIndexes.SetNum(HUMAN_TYPE_COUNT);
	EnvironmentItemsIndexes.SetNum(NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT + FILLER_BUILDING_COUNT + SIGN_COUNT + PROP_COUNT);

	CurrentLVL = 0;
	PointCountPerTurn = 5;
	AvailableTurnTransform.SetLocation(FVector(500.0f,500.0f,0.0f));
	bShouldGenerateNewPlatform = false;
	bShouldGenerateNewLVLSwitch = false;

	InitializeTileGeneration();
	GenerateTiles();
	SetUpBackground();
	GeneratePlatformPointsAndStaticObstacles();
	GeneratePeople();
	InitializeEnvironmentGeneration();
	GeneratePlatformEnvironment();

	Iterator = -1;
	CurrentTile2RLIndex = -1;
	UpdateTile2RLCollisionBoxLocation();
	bShouldGenerateNewLVLSwitch = true;
}

void AProceduralPlatformGeneratorBase::OnRestart()
{
	HISM_Tile2->ClearInstances();
	HISM_Tile2RL->ClearInstances();
	HISM_Points->ClearInstances();

	for (int32 i = 0; i < Obstacles.Num(); i++)
	{
		Obstacles[i]->ClearInstances();
	}

	for (int32 i = 0; i < People.Num(); i++)
	{
		People[i]->ClearInstances();
	}

	for (int32 i = 0; i < EnvironmentItems.Num(); i++)
	{
		EnvironmentItems[i]->ClearInstances();
	}

	PointsIndexes.Empty();
	for (int32 i = 0; i < ObstaclesIndexes.Num(); i++)
	{
		ObstaclesIndexes[i].Indexes.Empty();
	}
	for (int32 i = 0; i < PeopleIndexes.Num(); i++)
	{
		PeopleIndexes[i].Indexes.Empty();
	}
	for (int32 i = 0; i < EnvironmentItemsIndexes.Num(); i++)
	{
		EnvironmentItemsIndexes[i].Indexes.Empty();
	}

	//CurrentLVL = 0;
	PointCountPerTurn = 5;
	bIsObstacleBoatGotHit = false;
	AvailableTurnTransform.SetLocation(FVector(500.0f, 500.0f, 0.0f));
	bShouldGenerateNewPlatform = false;
	bShouldGenerateNewLVLSwitch = false;

	InitializeTileGeneration();
	GenerateTiles();
	SetUpBackground();
	GeneratePlatformPointsAndStaticObstacles();
	GeneratePeople();
	InitializeEnvironmentGeneration();
	GeneratePlatformEnvironment();

	Iterator = -1;
	CurrentTile2RLIndex = -1;
	UpdateTile2RLCollisionBoxLocation();
	bShouldGenerateNewLVLSwitch = true;
}

UHierarchicalInstancedStaticMeshComponent * AProceduralPlatformGeneratorBase::GetHISMPoints()
{
	return HISM_Points;
}

void AProceduralPlatformGeneratorBase::RemovePoint(int32 InstanceIndex)
{
	HISM_Points->RemoveInstance(InstanceIndex);

	for (int32 i = 0; i < PointsIndexes.Num(); i++)
	{
		if (PointsIndexes[i] == HISM_Points->GetInstanceCount())
		{
			PointsIndexes[i] = InstanceIndex;
			return;
		}
	}
}

/*********************General*************************************************************/

void AProceduralPlatformGeneratorBase::GenerateProceduralPlatform()
{
	ClearOldItems();
	GenerateTiles();
	SetUpBackground();
	GeneratePlatformPointsAndStaticObstacles();
	GeneratePeople();
	GeneratePlatformEnvironment();
}

void AProceduralPlatformGeneratorBase::GenerateLVLSwitch()
{
	ClearOldItems();
	GenerateLVLSwitchPointsAndStaticObstacles();
	GeneratePeople();
	GenerateLVLSwitchEnvironment();
}

void AProceduralPlatformGeneratorBase::ClearOldItems()
{

	TArray<int32> TempEnvironmentItemIndexes;

	for (int32 i = 0; i < HISM_Points->GetInstanceCount(); i++)
	{
		if (!PointsIndexes.Contains(i))
		{
			TempEnvironmentItemIndexes.Add(i);
		}
	}
	HISM_Points->RemoveInstances(TempEnvironmentItemIndexes);
	TempEnvironmentItemIndexes.Empty();
	PointsIndexes.Empty();

	for (int32 i = 0; i < ObstaclesIndexes.Num(); i++)
	{
		for (int32 j = 0; j < Obstacles[i]->GetInstanceCount(); j++)
		{
			if (!ObstaclesIndexes[i].Indexes.Contains(j))
			{
				TempEnvironmentItemIndexes.Add(j);
			}
		}
		Obstacles[i]->RemoveInstances(TempEnvironmentItemIndexes);
		TempEnvironmentItemIndexes.Empty();
		ObstaclesIndexes[i].Indexes.Empty();
	}

	for (int32 i = 0; i < PeopleIndexes.Num(); i++)
	{
		for (int32 j = 0; j < People[i]->GetInstanceCount(); j++)
		{
			if (!PeopleIndexes[i].Indexes.Contains(j))
			{
				TempEnvironmentItemIndexes.Add(j);
			}
		}
		People[i]->RemoveInstances(TempEnvironmentItemIndexes);
		TempEnvironmentItemIndexes.Empty();
		PeopleIndexes[i].Indexes.Empty();
	}

	for (int32 i = 0; i < EnvironmentItemsIndexes.Num(); i++)
	{
		for (int32 j = 0; j < EnvironmentItems[i]->GetInstanceCount(); j++)
		{
			if (!EnvironmentItemsIndexes[i].Indexes.Contains(j))
			{
				TempEnvironmentItemIndexes.Add(j);
			}
		}
		EnvironmentItems[i]->RemoveInstances(TempEnvironmentItemIndexes);
		TempEnvironmentItemIndexes.Empty();
		EnvironmentItemsIndexes[i].Indexes.Empty();
	}
}

void AProceduralPlatformGeneratorBase::On2RLCollisionBoxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	GetWorldTimerManager().ClearTimer(CBTile2RLTimerHandle);
	GetWorldTimerManager().SetTimer(CBTile2RLTimerHandle, this, &AProceduralPlatformGeneratorBase::UpdateTile2RLCollisionBoxLocation, 0.1f, false);
}

void AProceduralPlatformGeneratorBase::UpdateTile2RLCollisionBoxLocation()
{
	if (bShouldGenerateNewPlatform)
	{
		bShouldGenerateNewPlatform = false;
		IncrementCurrentLVL();
		GenerateProceduralPlatform();
		Iterator = -1;
		CurrentTile2RLIndex = -1;
		UpdateTile2RLCollisionBoxLocation();
		bShouldGenerateNewLVLSwitch = true;
		return;
	}

	if (bShouldGenerateNewLVLSwitch)
	{
		bShouldGenerateNewLVLSwitch = false;
		GenerateLVLSwitch();
		UpdateTile2RLCollisionBoxLocation();
		return;
	}

	CurrentTile2RLIndex++;
	if (FindNext2RLTileIndexInTileInfo()) // If last turn.
	{
		bShouldGenerateNewPlatform = true;
		FTransform TempT;
		HISM_Tile2->GetInstanceTransform(HISM_Tile2->GetInstanceCount() - LVLSwitchingTileCount, TempT, false);
		CB_Tile2RL->SetRelativeTransform(TempT, false, nullptr, ETeleportType::TeleportPhysics);
		PawnBoat->DetachCurrentTurnLocation();
		return;
	}

	if (TileInfo[Iterator+1].TileType == ETileType::Road2RL)
	{
		UpdateTile2RLCollisionBoxLocation();
		return;
	}

	FTransform TempT;
	HISM_Tile2RL->GetInstanceTransform(CurrentTile2RLIndex, TempT, false);
	FVector OffsetDirection = TileInfo[Iterator].FirstConnectionType == ETile2RLConnectionType::Back ?
		TempT.TransformVectorNoScale(FVector::RightVector) : TempT.TransformVectorNoScale(FVector::BackwardVector);
	float YawOffset = TileInfo[Iterator].FirstConnectionType == ETile2RLConnectionType::Back ? 90.0f : 180.0f;

	CB_Tile2RL->SetRelativeTransform(FTransform(
		{
			0.0f,
			TempT.GetRotation().Rotator().Yaw + YawOffset,
			0.0f
		},
		TempT.GetLocation() + (OffsetDirection * (TILE2RL_SIZE / 2.0f)),
		FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	SetAvailableTurnTransform();
	PawnBoat->DetachCurrentTurnLocation();
}

bool AProceduralPlatformGeneratorBase::FindNext2RLTileIndexInTileInfo()
{
	for (int32 i = Iterator + 1; i < TileInfo.Num(); i++)
	{
		if (TileInfo[i].TileType == ETileType::Road2RL)
		{
			Iterator = i;
			return false;
		}
	}
	return true;
}

void AProceduralPlatformGeneratorBase::SetAvailableTurnTransform()
{
	FTransform Tile2RLInstanceTransform;
	int32 TempTile2RLIndex;
	FVector BackVector;
	FVector RightVector;
	FVector Offset;

	if (TileInfo[Iterator-1].TileType == ETileType::Road2RL)
	{
		TempTile2RLIndex = CurrentTile2RLIndex - 1;
		HISM_Tile2RL->GetInstanceTransform(TempTile2RLIndex, Tile2RLInstanceTransform, true);
		BackVector = Tile2RLInstanceTransform.TransformVectorNoScale(FVector::BackwardVector);
		RightVector = Tile2RLInstanceTransform.TransformVectorNoScale(FVector::RightVector);

		if (TileInfo[Iterator].FirstConnectionType == ETile2RLConnectionType::Back) 
		{
			//Back Offset
			Offset = Tile2RLInstanceTransform.GetLocation() + BackVector * ((TILE2RL_SIZE / 2.0f) + 30.0f) + RightVector * (TILE2RL_SIZE / 2.0f);
		}
		else 
		{
			//Right Offset
			Offset = Tile2RLInstanceTransform.GetLocation() + RightVector * ((TILE2RL_SIZE / 2.0f) + 30.0f) + BackVector * (TILE2RL_SIZE / 2.0f);
		}
		 
		AvailableTurnTransform.SetRotation(TileInfo[Iterator].FirstConnectionType == ETile2RLConnectionType::Back ? 
			FRotationMatrix::MakeFromX(BackVector).ToQuat() : FRotationMatrix::MakeFromX(RightVector).ToQuat());
		AvailableTurnTransform.SetLocation(Offset);
	}
	else
	{
		TempTile2RLIndex = CurrentTile2RLIndex;
		HISM_Tile2RL->GetInstanceTransform(TempTile2RLIndex, Tile2RLInstanceTransform, true);
		BackVector = Tile2RLInstanceTransform.TransformVectorNoScale(FVector::BackwardVector);
		RightVector = Tile2RLInstanceTransform.TransformVectorNoScale(FVector::RightVector);
		//Diagonal Offset
		Offset = Tile2RLInstanceTransform.GetLocation() + (BackVector + RightVector) * ( (TILE2RL_SIZE / 2.0f) * 1.414214f);

		AvailableTurnTransform.SetRotation(TileInfo[Iterator].FirstConnectionType == ETile2RLConnectionType::Back ?
			FRotationMatrix::MakeFromX(RightVector).ToQuat() : FRotationMatrix::MakeFromX(BackVector).ToQuat());
		AvailableTurnTransform.SetLocation(Offset);
	}
}

void AProceduralPlatformGeneratorBase::IncrementCurrentLVL()
{
	CurrentLVL++;

	if (CurrentLVL<4)
	{
		PawnBoat->AddMaxSpeed(25.0f);
	}
}

/*********************Tile Generation*************************************************************/

void AProceduralPlatformGeneratorBase::InitializeTileGeneration()
{
	GenerationStartingLocation = FVector(DEFAULT_STARTING_LOCATION_X, 0.0f, 0.0f);
	GenerationDirection = FVector::ForwardVector;
	BaseTileCount = DEFAULT_BASE_TILE_COUNT;
	TileInfo.SetNum(BaseTileCount);

	for (int32 i = 0; i < BaseTileCount; i++)
	{
		HISM_Tile2->AddInstance(FTransform(FRotationMatrix::MakeFromX(GenerationDirection).Rotator(),
			GenerationStartingLocation + (GenerationDirection*i*TILE2_SIZE), FVector::OneVector));
		TileInfo[i].TileType = ETileType::Road2;
	}
	LVLSwitchingTileCount = BaseTileCount;
}

void AProceduralPlatformGeneratorBase::GenerateTiles()
{
	//Remove old tiles and set up Tile Info
	ClearOldRoadTiles();
	Iterator = 0;
	for (int32 i = TileInfo.Num() - LVLSwitchingTileCount; i < TileInfo.Num(); i++)
	{
		TileInfo[Iterator] = TileInfo[i];
		Iterator++;
	}

	OldLVLSwitchingTileCount = LVLSwitchingTileCount;
	UpdateTileGenerationParameters();
	TileInfo.SetNum(OldLVLSwitchingTileCount + GeneratedTileCount + LVLSwitchingTileCount);
	Iterator = OldLVLSwitchingTileCount;
	Tile2Streak = 0;
	Tile2RLStreak = 0;
	AttachTile2To2RL();
	
	while(Iterator < TileInfo.Num()) 
	{
		if (IsNextTileFull())
		{
			ArrangeGenerationOnNextTileFull();
		}
		else
		{
			if(Iterator < OldLVLSwitchingTileCount + GeneratedTileCount)
			{
				LVL1();
			}
			else
			{
				if (getlasttileinfo().TileType == ETileType::Road2)
				{
					AttachTile2To2();
				}
				else
				{
					AttachTile2RLTo2();
				}
			}
		}
	}
}

void AProceduralPlatformGeneratorBase::ClearOldRoadTiles()
{
	Iterator = 0;
	TArray<FTransform> OldLVLSwitchingTileTransforms;
	OldLVLSwitchingTileTransforms.SetNum(LVLSwitchingTileCount);
	for (int32 i = HISM_Tile2->GetInstanceCount() - LVLSwitchingTileCount; i < HISM_Tile2->GetInstanceCount(); i++)
	{
		HISM_Tile2->GetInstanceTransform(i, OldLVLSwitchingTileTransforms[Iterator], false);
		Iterator++;
	}
	
	HISM_Tile2->ClearInstances();
	HISM_Tile2RL->ClearInstances();

	//Reordering IHSM array to use GetHISMLastInstanceTransform
	HISM_Tile2->AddInstances(OldLVLSwitchingTileTransforms, false);
}

void AProceduralPlatformGeneratorBase::UpdateTileGenerationParameters()
{
	GeneratedTileCount = DEFAULT_GENERATED_TILE_COUNT;
	LVLSwitchingTileCount = DEFAULT_LVL_SWITCHING_TILE_COUNT;
}

void AProceduralPlatformGeneratorBase::GetHISMLastInstanceTransform(UHierarchicalInstancedStaticMeshComponent * HISM, FTransform & LastInstanceTransform) const
{
	HISM->GetInstanceTransform(HISM->GetInstanceCount() - 1, LastInstanceTransform, false);
}

void AProceduralPlatformGeneratorBase::RemoveLastTile()
{
	UHierarchicalInstancedStaticMeshComponent* LastTileTypeRef = getlasttileinfo().TileType == ETileType::Road2 ? HISM_Tile2 : HISM_Tile2RL;
	LastTileTypeRef->RemoveInstance(LastTileTypeRef->GetInstanceCount() - 1);
	Iterator--;
}

void AProceduralPlatformGeneratorBase::AttachTile2To2()
{
	FTransform LastTile2InstanceTransform;
	GetHISMLastInstanceTransform(HISM_Tile2, LastTile2InstanceTransform);
	HISM_Tile2->AddInstance( FTransform (LastTile2InstanceTransform.GetRotation().Rotator(),
		LastTile2InstanceTransform.GetLocation() + (LastTile2InstanceTransform.TransformVectorNoScale(FVector::ForwardVector)*TILE2_SIZE), FVector::OneVector) );
	TileInfo[Iterator].TileType = ETileType::Road2;
	Tile2Streak++;
	Iterator++;
}

void AProceduralPlatformGeneratorBase::AttachTile2To2RL(const bool bIsSpecifiedConnection, const ETile2RLConnectionType SpecifiedConnectionType)
{
	bool bIsConnectionFromBack;

	if (bIsSpecifiedConnection)
	{
		bIsConnectionFromBack = SpecifiedConnectionType == ETile2RLConnectionType::Back;

	}
	else
	{
		bIsConnectionFromBack = FMath::RandBool();
	}

	FTransform LastTile2InstanceTransform;
	GetHISMLastInstanceTransform(HISM_Tile2, LastTile2InstanceTransform);

	//Attached From Back
	if (bIsConnectionFromBack)
	{
		HISM_Tile2RL->AddInstance(FTransform(LastTile2InstanceTransform.GetRotation().Rotator(),
			LastTile2InstanceTransform.GetLocation() + (LastTile2InstanceTransform.TransformVectorNoScale(FVector::ForwardVector) * ((TILE2_SIZE + TILE2RL_SIZE) / 2.0f)),
			FVector::OneVector));
		
		TileInfo[Iterator].FirstConnectionType = ETile2RLConnectionType::Back;
	}
	//Attached From Right
	else
	{
		int32 Index = HISM_Tile2RL->AddInstance(FTransform(
			{
				0.0f,
				LastTile2InstanceTransform.GetRotation().Rotator().Yaw + 90.0f,
				0.0f
			},
			LastTile2InstanceTransform.GetLocation() + (LastTile2InstanceTransform.TransformVectorNoScale(FVector::ForwardVector) * ((TILE2_SIZE + TILE2RL_SIZE) / 2.0f)),
			FVector::OneVector));

		TileInfo[Iterator].FirstConnectionType = ETile2RLConnectionType::Right;
		HISM_Tile2RL->SetCustomDataValue(Index, 0, 1.0f, false);
	}

	TileInfo[Iterator].TileType = ETileType::Road2RL;
	TileInfo[Iterator].bIsOtherDirectionDeadEnd = false;
	Tile2Streak = 0;
	Tile2RLStreak = 1;
	Iterator++;
}

void AProceduralPlatformGeneratorBase::AttachTile2RLTo2()
{
	FTransform LastTile2RLInstanceTransform;
	GetHISMLastInstanceTransform(HISM_Tile2RL, LastTile2RLInstanceTransform);

	FVector OffsetDirection = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ?
		LastTile2RLInstanceTransform.TransformVectorNoScale(FVector::RightVector) : LastTile2RLInstanceTransform.TransformVectorNoScale(FVector::BackwardVector);
	float YawOffset = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ? 90.0f : 180.0f;

	HISM_Tile2->AddInstance(FTransform(
		{
			0.0f,
			LastTile2RLInstanceTransform.GetRotation().Rotator().Yaw + YawOffset,
			0.0f
		},
		LastTile2RLInstanceTransform.GetLocation() + (OffsetDirection * ((TILE2_SIZE + TILE2RL_SIZE) / 2.0f)),
		FVector::OneVector));

	TileInfo[Iterator].TileType = ETileType::Road2;
	Tile2Streak = 1;
	Tile2RLStreak = 0;
	Iterator++;
}

void AProceduralPlatformGeneratorBase::AttachTile2RLTo2RL()
{
	FTransform LastTile2RLInstanceTransform;
	GetHISMLastInstanceTransform(HISM_Tile2RL, LastTile2RLInstanceTransform);

	FVector OffsetDirection = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ?
		LastTile2RLInstanceTransform.TransformVectorNoScale(FVector::RightVector) : LastTile2RLInstanceTransform.TransformVectorNoScale(FVector::BackwardVector);
	float YawOffset = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ? 90.0f : -90.0f;

	int32 Index = HISM_Tile2RL->AddInstance(FTransform(
		{
			0.0f,
			LastTile2RLInstanceTransform.GetRotation().Rotator().Yaw + YawOffset,
			0.0f
		},
		LastTile2RLInstanceTransform.GetLocation() + (OffsetDirection * TILE2RL_SIZE),
		FVector::OneVector));

	if (getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Right)
	{
		HISM_Tile2RL->SetCustomDataValue(Index, 0, 1.0f, false);
	}

	TileInfo[Iterator].TileType = ETileType::Road2RL;
	TileInfo[Iterator].FirstConnectionType = getlasttileinfo().FirstConnectionType;
	TileInfo[Iterator].bIsOtherDirectionDeadEnd = true;
	Tile2RLStreak = 2;
	Iterator++;
}

bool AProceduralPlatformGeneratorBase::IsNextTileFull()
{
	UWorld* World = GetWorld();
	check(World);
	FCollisionQueryParams Params;
	FHitResult HitResult;

	FTransform LastTileInstanceTransform;
	FVector OffsetDirection;
	FVector TraceStart;
	FVector TraceEnd;

	if (getlasttileinfo().TileType == ETileType::Road2)
	{
		GetHISMLastInstanceTransform(HISM_Tile2, LastTileInstanceTransform);
		OffsetDirection = LastTileInstanceTransform.TransformVectorNoScale(FVector::ForwardVector);
	}
	else
	{
		GetHISMLastInstanceTransform(HISM_Tile2RL, LastTileInstanceTransform);
		OffsetDirection = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ?
			LastTileInstanceTransform.TransformVectorNoScale(FVector::RightVector) : LastTileInstanceTransform.TransformVectorNoScale(FVector::BackwardVector);
	}

	TraceStart = LastTileInstanceTransform.GetLocation() + (OffsetDirection * ((TILE2_SIZE + TILE2RL_SIZE) / 2.0f));
	TraceEnd = TraceStart;
	TraceStart.Z += 10.0f;
	TraceEnd.Z -= IS_NEXT_TILE_FULL_LINE_TRACE_MINUS_Z_OFFSET;

	World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TC_TILE, Params);

	return HitResult.bBlockingHit;
}

void AProceduralPlatformGeneratorBase::ArrangeGenerationOnNextTileFull()
{
	while (getlasttileinfo().TileType == ETileType::Road2 || getlasttileinfo().bIsOtherDirectionDeadEnd)
	{
		RemoveLastTile();
	}
	FlipLast2RLTile();
}

void AProceduralPlatformGeneratorBase::FlipLast2RLTile()
{
	FTransform LastTile2RLInstanceTransform;
	GetHISMLastInstanceTransform(HISM_Tile2RL, LastTile2RLInstanceTransform);
	int32 FlipMultiplier = getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back ? 1 : -1; // 1 CW		-1 CCW
	HISM_Tile2RL->UpdateInstanceTransform(HISM_Tile2RL->GetInstanceCount() - 1, FTransform(
		{
			0.0f,
			LastTile2RLInstanceTransform.GetRotation().Rotator().Yaw + FlipMultiplier * 90.0f,
			0.0f
		},
		LastTile2RLInstanceTransform.GetLocation(), FVector::OneVector), false, true, true);
	if (getlasttileinfo().FirstConnectionType == ETile2RLConnectionType::Back)
	{
		getlasttileinfo().FirstConnectionType = ETile2RLConnectionType::Right;
		HISM_Tile2RL->SetCustomDataValue(HISM_Tile2RL->GetInstanceCount() - 1, 0, 1.0f, false);
	}
	else
	{
		getlasttileinfo().FirstConnectionType = ETile2RLConnectionType::Back;
		HISM_Tile2RL->SetCustomDataValue(HISM_Tile2RL->GetInstanceCount() - 1, 0, 0.0f, false);
	}
	getlasttileinfo().bIsOtherDirectionDeadEnd = true;
	Tile2RLStreak = 1;
}

//LVLs

void AProceduralPlatformGeneratorBase::LVL1()
{
	if (getlasttileinfo().TileType == ETileType::Road2)
	{
		if (Tile2Streak < 7)
		{
			AttachTile2To2();
		}
		else
		{
			AttachTile2To2RL();
		}
	}
	else
	{
		AttachTile2RLTo2();
	}
}

void AProceduralPlatformGeneratorBase::LVL2()
{
}

void AProceduralPlatformGeneratorBase::LVL3()
{
}

/*********************Points and Static Obstacles*************************************************************/

void AProceduralPlatformGeneratorBase::GeneratePlatformPointsAndStaticObstacles()
{
	int32 TempTile2Index = OldLVLSwitchingTileCount;
	int32 TempTile2RLIndex = 0;
	TArray<FTransform> PointTransforms;
	PointTransforms.SetNum(HISM_Tile2RL->GetInstanceCount() * PointCountPerTurn);
	FTransform TempTransform;

	for (int32 i = OldLVLSwitchingTileCount; i < TileInfo.Num() - LVLSwitchingTileCount - 1; i++)
	{
		//Point generation for Tile2RL.
		if (TileInfo[i].TileType == ETileType::Road2RL)
		{
			float StartingAngle = TileInfo[i].FirstConnectionType == ETile2RLConnectionType::Back ? (3.0f / 2.0f) * PI : 0.0f;
			float AddedAnglePerCycle = ((PI / 2.0f) / (PointCountPerTurn - 1)) *
				(TileInfo[i].FirstConnectionType == ETile2RLConnectionType::Back ? 1 : -1); // 1 CW -1 CCW 

			HISM_Tile2RL->GetInstanceTransform(TempTile2RLIndex, TempTransform, false);
			FVector PointGenerationOrigin = TempTransform.GetLocation() +
				TempTransform.TransformVectorNoScale(FVector::BackwardVector)*(TILE2RL_SIZE / 2.0f) + TempTransform.TransformVectorNoScale(FVector::RightVector)*(TILE2RL_SIZE / 2.0f);

			//Change a Tile2RL transform to a point generation origin transform.
			TempTransform.SetLocation(PointGenerationOrigin);

			//Set Tile2RL Points locations.
			for (int32 j = 0; j < PointCountPerTurn; j++)
			{
				float CurrentAngle = StartingAngle + j * AddedAnglePerCycle;
				FVector PointLocation = TempTransform.TransformPosition(FVector(FMath::Cos(CurrentAngle)*(TILE2RL_SIZE / 2.0f), FMath::Sin(CurrentAngle)*(TILE2RL_SIZE / 2.0f), 0.0f));
				PointTransforms[TempTile2RLIndex * PointCountPerTurn + j].SetLocation(PointLocation);
			}

			TempTile2RLIndex++;

			bIsFirstObstacleTileAfterTurn = true;
			bIsXOrientationMid = false;
			LastObstacleOrientation = EObstacleOrientation::LeftRight;
		}

		//Point and obstacle generation for Tile2.
		else
		{
			HISM_Tile2->GetInstanceTransform(TempTile2Index, TempTransform, false);

			if (TileInfo[i - 2].TileType == ETileType::Road2RL || TileInfo[i - 1].TileType == ETileType::Road2RL || TileInfo[i + 1].TileType == ETileType::Road2RL || TileInfo[i + 2].TileType == ETileType::Road2RL)
			{
				SetPointTransformsWithoutPlacingObstacle(TempTransform, PointTransforms);
				TempTile2Index++;
				continue;
			}

			FVector CurrentXOffset;
			
			if (bIsXOrientationMid)
			{
				CurrentXOffset = FVector::ZeroVector;
				SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
				PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
				bIsXOrientationMid = false;
			}
			else
			{
				CurrentXOffset = TempTransform.TransformVectorNoScale(FVector::ForwardVector)*TILE2_PLACE_X_OFFSET * -1;
				SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
				PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
				CurrentXOffset = TempTransform.TransformVectorNoScale(FVector::ForwardVector)*TILE2_PLACE_X_OFFSET;
				SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
				PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
				bIsXOrientationMid = true;
			}
			
			TempTile2Index++;
		}
	}
	
	PointsIndexes = HISM_Points->AddInstances(PointTransforms, true);
	for (int32 i : PointsIndexes)
	{
		HISM_Points->SetCustomDataValue(i, 0, float(i), false);
	}
}

void AProceduralPlatformGeneratorBase::GenerateLVLSwitchPointsAndStaticObstacles()
{
	int32 TempTile2Index = HISM_Tile2->GetInstanceCount() - LVLSwitchingTileCount;
	TArray<FTransform> PointTransforms;
	FTransform TempTransform;

	for (int32 i = TileInfo.Num() - LVLSwitchingTileCount; i < TileInfo.Num() - 3; i++)
	{
		HISM_Tile2->GetInstanceTransform(TempTile2Index, TempTransform, false);

		if (TileInfo[i - 2].TileType == ETileType::Road2RL || TileInfo[i - 1].TileType == ETileType::Road2RL || TileInfo[i + 1].TileType == ETileType::Road2RL || TileInfo[i + 2].TileType == ETileType::Road2RL)
		{
			bIsFirstObstacleTileAfterTurn = true;
			bIsXOrientationMid = false;
			LastObstacleOrientation = EObstacleOrientation::LeftRight;
			SetPointTransformsWithoutPlacingObstacle(TempTransform, PointTransforms);
			TempTile2Index++;
			continue;
		}

		FVector CurrentXOffset;

		if (bIsXOrientationMid)
		{
			CurrentXOffset = FVector::ZeroVector;
			SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
			PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
			bIsXOrientationMid = false;
		}
		else
		{
			CurrentXOffset = TempTransform.TransformVectorNoScale(FVector::ForwardVector)*TILE2_PLACE_X_OFFSET * -1;
			SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
			PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
			CurrentXOffset = TempTransform.TransformVectorNoScale(FVector::ForwardVector)*TILE2_PLACE_X_OFFSET;
			SetStaticObstacleOrientation(CurrentLVL % MAX_STATIC_OBSTACLE_VARIATION_COUNT);
			PlaceObstacleAndSetPointTransforms(TempTransform, CurrentXOffset, PointTransforms);
			bIsXOrientationMid = true;
		}
		TempTile2Index++;
	}

	PointsIndexes = HISM_Points->AddInstances(PointTransforms, true);
	for (int32 i : PointsIndexes)
	{
		HISM_Points->SetCustomDataValue(i, 0, float(i), false);
	}
}

void AProceduralPlatformGeneratorBase::SetStaticObstacleOrientation(int32 ObstacleHardness)
{
	switch (ObstacleHardness)
	{

	case 0:

		EasyLVL1();
		break;

	case 1:

		EasyLVL2();
		break;

	case 2:
	
		if (FMath::RandBool())
		{
			EasyLVL1();
		}
		else
		{
			HardLVL1();
		}
		break;

	case 3:

		if (FMath::RandBool())
		{
			EasyLVL2();
		}
		else
		{
			HardLVL2();
		}
		break;

	case 4:

		HardLVL1();
		break;

	case 5:

		HardLVL2();
		break;

	case 6: //PingPongPattern

		if (bIsFirstObstacleTileAfterTurn)
		{
			CurrentObstacleGenerationState = FMath::RandRange(0, 1);
			bIsFirstObstacleTileAfterTurn = false;
		}
		if (CurrentObstacleGenerationState)
		{
			PingPongPatternLeft();
		}
		else
		{
			PingPongPatternRight();
		}
		break;

	case 7: //ZigZagPattern

		ZigZagPattern();
		break;

	case 8: //MixPattern1

		if (bIsFirstObstacleTileAfterTurn)
		{
			CurrentObstacleGenerationState = FMath::RandBool() ? FMath::RandRange(0, 1) : 2;
			bIsFirstObstacleTileAfterTurn = false;
		}
		switch (CurrentObstacleGenerationState)
		{

		case 0:

			PingPongPatternLeft();
			break;

		case 1:

			PingPongPatternRight();
			break;

		case 2:

			ZigZagPattern();
			break;

		}
		break;

	case 9: //MixPattern2

		if (bIsFirstObstacleTileAfterTurn)
		{
			if (FMath::RandBool())
			{
				CurrentObstacleGenerationState = FMath::RandBool() ? FMath::RandRange(0, 1) : 2;
			}
			else
			{
				CurrentObstacleGenerationState = 3;
			}
			bIsFirstObstacleTileAfterTurn = false;
		}
		switch (CurrentObstacleGenerationState)
		{

		case 0:

			PingPongPatternLeft();
			break;

		case 1:

			PingPongPatternRight();
			break;

		case 2:

			ZigZagPattern();
			break;

		case 3:

			HardLVL3();
			break;

		}
		break;

	case 10:

		HardLVL3();
		break;
	}
}

void AProceduralPlatformGeneratorBase::PlaceObstacleAndSetPointTransforms(FTransform & Tile2Transform, FVector & XOffset, TArray<FTransform>& PointTransforms)
{
	FRotator Rotation = Tile2Transform.GetRotation().Rotator();
	FVector LeftOffset = Tile2Transform.TransformVectorNoScale(FVector::LeftVector)*TILE2_PLACE_Y_OFFSET;
	FVector LeftLocation = Tile2Transform.GetLocation() + XOffset + LeftOffset;
	FVector MidLocation = Tile2Transform.GetLocation() + XOffset;
	FVector RightLocation = Tile2Transform.GetLocation() + XOffset + LeftOffset * -1;
	FVector PointLocation;

	switch (LastObstacleOrientation)
	{

	case EObstacleOrientation::Left:

		AddRandomStaticObstacle(FTransform(Rotation, LeftLocation, FVector::OneVector));
		PointLocation = FMath::RandBool() ? MidLocation : RightLocation;
		break;

	case EObstacleOrientation::Mid:

		AddRandomStaticObstacle(FTransform(Rotation, MidLocation, FVector::OneVector));
		PointLocation = FMath::RandBool() ? LeftLocation : RightLocation;
		break;

	case EObstacleOrientation::Right:

		AddRandomStaticObstacle(FTransform(Rotation, RightLocation, FVector::OneVector));
		PointLocation = FMath::RandBool() ? LeftLocation : MidLocation;
		break;

	case EObstacleOrientation::LeftMid:

		AddRandomStaticObstacle(FTransform(Rotation, LeftLocation, FVector::OneVector));
		AddRandomStaticObstacle(FTransform(Rotation, MidLocation, FVector::OneVector));
		PointLocation = RightLocation;
		break;

	case EObstacleOrientation::MidRight:

		AddRandomStaticObstacle(FTransform(Rotation, MidLocation, FVector::OneVector));
		AddRandomStaticObstacle(FTransform(Rotation, RightLocation, FVector::OneVector));
		PointLocation = LeftLocation;
		break;

	case EObstacleOrientation::LeftRight:

		AddRandomStaticObstacle(FTransform(Rotation, LeftLocation, FVector::OneVector));
		AddRandomStaticObstacle(FTransform(Rotation, RightLocation, FVector::OneVector));
		PointLocation = MidLocation;
		break;

	}

	AddPoint(PointLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
}

void AProceduralPlatformGeneratorBase::SetPointTransformsWithoutPlacingObstacle(FTransform & Tile2Transform, TArray<FTransform>& PointTransforms)
{
	FVector LeftOffset = Tile2Transform.TransformVectorNoScale(FVector::LeftVector)*TILE2_PLACE_Y_OFFSET;
	FVector LeftLocation = Tile2Transform.GetLocation() + LeftOffset;
	FVector MidLocation = Tile2Transform.GetLocation();
	FVector RightLocation = Tile2Transform.GetLocation() + LeftOffset * -1;

	switch (FMath::RandHelper(6))
	{

	case 0: 
		
		AddPoint(FMath::RandBool() ? LeftLocation : MidLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	case 1:	

		AddPoint(FMath::RandBool() ? MidLocation : RightLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	case 2: 

		AddPoint(FMath::RandBool() ? LeftLocation : RightLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	case 3: 

		AddPoint(LeftLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		AddPoint(MidLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	case 4: 

		AddPoint(MidLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		AddPoint(RightLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	case 5: 

		AddPoint(LeftLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		AddPoint(RightLocation, Tile2Transform.TransformVectorNoScale(FVector::ForwardVector), PointTransforms);
		break;

	}
}

void AProceduralPlatformGeneratorBase::AddRandomStaticObstacle(const FTransform & InstanceTransform)
{
	int32 RandomObstacleType = FMath::RandHelper(STATIC_OBSTACLE_COUNT);
	int32 AddedInstanceIndex = Obstacles[RandomObstacleType]->AddInstance(InstanceTransform);
	Obstacles[RandomObstacleType]->SetCustomDataValue(AddedInstanceIndex, 0, float(FMath::RandHelper(BOAT_OBSTACLE_COLOR_COUNT)), true);
	ObstaclesIndexes[RandomObstacleType].Indexes.Add(AddedInstanceIndex);
}

void AProceduralPlatformGeneratorBase::EasyLVL1()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftMid || LastObstacleOrientation == EObstacleOrientation::MidRight)
	{
		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::Left : EObstacleOrientation ::Right;
	}
	else
	{
		switch (FMath::RandHelper(3))
		{

		case 0:

			LastObstacleOrientation = EObstacleOrientation::Left;
			break;

		case 1:

			LastObstacleOrientation = EObstacleOrientation::Mid;
			break;

		case 2:

			LastObstacleOrientation = EObstacleOrientation::Right;
			break;

		}
	}
}

void AProceduralPlatformGeneratorBase::EasyLVL2()
{
	switch (LastObstacleOrientation)
	{

	case EObstacleOrientation::Left:

		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::Mid : EObstacleOrientation::Right;
		break;

	case EObstacleOrientation::Mid:

		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::Left : EObstacleOrientation::Right;
		break;

	case EObstacleOrientation::Right:

		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::Left : EObstacleOrientation::Mid;
		break;

	case EObstacleOrientation::LeftMid:

		LastObstacleOrientation = EObstacleOrientation::Right;
		break;

	case EObstacleOrientation::MidRight:

		LastObstacleOrientation = EObstacleOrientation::Left;
		break;

	case EObstacleOrientation::LeftRight:

		LastObstacleOrientation = EObstacleOrientation::Mid;
		break;

	}
}

void AProceduralPlatformGeneratorBase::HardLVL1()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftMid)
	{
		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::LeftMid : EObstacleOrientation::LeftRight;
	}
	else if (LastObstacleOrientation == EObstacleOrientation::MidRight)
	{
		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::MidRight : EObstacleOrientation::LeftRight;
	}
	else
	{
		switch (FMath::RandHelper(3))
		{

		case 0:

			LastObstacleOrientation = EObstacleOrientation::LeftMid;
			break;

		case 1:

			LastObstacleOrientation = EObstacleOrientation::MidRight;
			break;

		case 2:

			LastObstacleOrientation = EObstacleOrientation::LeftRight;
			break;

		}
	}
}

void AProceduralPlatformGeneratorBase::HardLVL2()
{
	if (LastObstacleOrientation == EObstacleOrientation::Mid || LastObstacleOrientation == EObstacleOrientation::LeftMid || LastObstacleOrientation == EObstacleOrientation::MidRight)
	{
		LastObstacleOrientation = EObstacleOrientation::LeftRight;
	}
	else if (LastObstacleOrientation == EObstacleOrientation::Left)
	{
		LastObstacleOrientation = EObstacleOrientation::MidRight;
	}
	else if (LastObstacleOrientation == EObstacleOrientation::Right)
	{
		LastObstacleOrientation = EObstacleOrientation::LeftMid;
	}
	else
	{
		switch (FMath::RandHelper(3))
		{

		case 0:

			LastObstacleOrientation = EObstacleOrientation::LeftMid;
			break;

		case 1:

			LastObstacleOrientation = EObstacleOrientation::MidRight;
			break;

		case 2:

			LastObstacleOrientation = EObstacleOrientation::LeftRight;
			break;

		}
	}
}

void AProceduralPlatformGeneratorBase::HardLVL3()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftRight)
	{
		LastObstacleOrientation = FMath::RandBool() ? EObstacleOrientation::LeftMid : EObstacleOrientation::MidRight;
	}
	else
	{
		LastObstacleOrientation = EObstacleOrientation::LeftRight;
	}
}

void AProceduralPlatformGeneratorBase::ZigZagPattern()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftRight)
	{
		if (bZigZagPatternInfo)
		{
			LastObstacleOrientation = EObstacleOrientation::LeftMid;
			bZigZagPatternInfo = false;
		}
		LastObstacleOrientation = EObstacleOrientation::MidRight;
		bZigZagPatternInfo = true;
	}
	else
	{
		LastObstacleOrientation = EObstacleOrientation::LeftRight;
	}
}

void AProceduralPlatformGeneratorBase::PingPongPatternLeft()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftRight)
	{
		LastObstacleOrientation = EObstacleOrientation::LeftMid;
	}
	else
	{
		LastObstacleOrientation = EObstacleOrientation::LeftRight;
	}
}

void AProceduralPlatformGeneratorBase::PingPongPatternRight()
{
	if (LastObstacleOrientation == EObstacleOrientation::LeftRight)
	{
		LastObstacleOrientation = EObstacleOrientation::MidRight;
	}
	else
	{
		LastObstacleOrientation = EObstacleOrientation::LeftRight;
	}
}

void AProceduralPlatformGeneratorBase::AddPoint(FVector & Location, FVector ForwardVector, TArray<FTransform>& PointTransforms)
{
	PointTransforms.Reserve(PointTransforms.Num() + (POINT_EDGE_LIMIT * 2 + 1));
	for (int i = POINT_EDGE_LIMIT * -1; i < POINT_EDGE_LIMIT + 1 ; i++)
	{
		PointTransforms.Add(FTransform({0.f, 0.f, 0.f}, Location + i * ForwardVector * POINT_GAP, FVector::OneVector));
	}
}

void AProceduralPlatformGeneratorBase::CanoeOnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
	TArray<FVector> LocalSitLocations = {CanoeLocalSitLocations[0], CanoeLocalSitLocations[1]};
	OnStaticObstacleBoatHit(Obstacles[0], Hit.Item, LocalSitLocations);
}

void AProceduralPlatformGeneratorBase::Pedalboat1OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
	TArray<FVector> LocalSitLocations = { PedalboatLocalSitLocations[0], PedalboatLocalSitLocations[1] };
	OnStaticObstacleBoatHit(Obstacles[1], Hit.Item, LocalSitLocations);
}

void AProceduralPlatformGeneratorBase::Pedalboat2OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
	TArray<FVector> LocalSitLocations = { PedalboatLocalSitLocations[0], PedalboatLocalSitLocations[1] };
	OnStaticObstacleBoatHit(Obstacles[2], Hit.Item, LocalSitLocations);
}

/*********************People*************************************************************/

void AProceduralPlatformGeneratorBase::GeneratePeople()
{
	TArray<FVector> LocalSitLocations = {CanoeLocalSitLocations[0], CanoeLocalSitLocations[1]};
	GenerateObstacleBoatPeople(0, LocalSitLocations);
	LocalSitLocations = { PedalboatLocalSitLocations[0], PedalboatLocalSitLocations[1] };
	GenerateObstacleBoatPeople(1, LocalSitLocations);
	GenerateObstacleBoatPeople(2, LocalSitLocations);
}

void AProceduralPlatformGeneratorBase::GenerateObstacleBoatPeople(int32 ObstacleBoaType, const TArray<FVector>& LocalSitLocations)
{
	for (int32 i = 0; i < ObstaclesIndexes[ObstacleBoaType].Indexes.Num(); i++)
	{
		for (int32 j = 0;  j < LocalSitLocations.Num(); j++)
		{
			int32 HumanType = FMath::RandHelper(HUMAN_TYPE_COUNT);
			FTransform ObstacleBoatTransform;
			Obstacles[ObstacleBoaType]->GetInstanceTransform(ObstaclesIndexes[ObstacleBoaType].Indexes[i], ObstacleBoatTransform, false);
			int32 InstanceIndex = People[HumanType]->AddInstance(FTransform(
				{
					0.0f, 
					ObstacleBoatTransform.GetRotation().Rotator().Yaw + 90.0f, 
					0.0f
				}, 
				ObstacleBoatTransform.TransformPosition(LocalSitLocations[j]), 
				FVector(PEOPLE_SCALE, PEOPLE_SCALE, PEOPLE_SCALE)));

			FAnimInfo RandomDanceAnim = getrandomdanceanim();
			People[HumanType]->SetCustomDataValue(InstanceIndex, 0, RandomDanceAnim.AnimNumber, false);
			People[HumanType]->SetCustomDataValue(InstanceIndex, 1, RandomDanceAnim.AnimSpeed, false);
			People[HumanType]->SetCustomDataValue(InstanceIndex, 2, float(FMath::RandHelper(POLYGON_TOWN_TEXTURE_VARIATION_COUNT)), false);
			PeopleIndexes[HumanType].Indexes.Add(InstanceIndex);
		}
	}
}

void AProceduralPlatformGeneratorBase::OnStaticObstacleBoatHit(UHierarchicalInstancedStaticMeshComponent * ObstacleBoatRef, const int32 HitItem, const TArray<FVector>& LocalSitLocations)
{
	if (bIsObstacleBoatGotHit)
	{
		return;
	}

	bIsObstacleBoatGotHit = true;

	SetPeopleCollisionProfile(FName("People"));

	FTransform BoatT;
	ObstacleBoatRef->GetInstanceTransform(HitItem, BoatT, false);
	
	for (int32 i = 0 ; i < LocalSitLocations.Num(); i++)
	{
		ChangeHumanAnimOnCrash(BoatT, LocalSitLocations[i]);
	}

	SetPeopleCollisionProfile(FName("NoCollision"));
}

void AProceduralPlatformGeneratorBase::SetPeopleCollisionProfile(FName CollisionProfileName)
{
	for (int32 i = 0; i < People.Num(); i++)
	{
		People[i]->SetCollisionProfileName(CollisionProfileName, false);
	}
}

void AProceduralPlatformGeneratorBase::ChangeHumanAnimOnCrash(FTransform & BoatTransform, FVector LocalSitLocation)
{
	UWorld* World = GetWorld();
	check(World);
	FCollisionQueryParams Params;
	FHitResult HitResult;

	FVector TraceEnd = BoatTransform.TransformPosition(LocalSitLocation);
	FVector TraceStart = TraceEnd;
	TraceStart.Z += 50.0f;

	World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, Params);

	UHierarchicalInstancedStaticMeshComponent* HumanRef = Cast<UHierarchicalInstancedStaticMeshComponent>(HitResult.GetComponent());

	if (HumanRef)
	{
		FTransform TempTransform;
		HumanRef->GetInstanceTransform(HitResult.Item, TempTransform, false);
		TempTransform.SetRotation(FRotator(
			{
				0.0f,
				FRotationMatrix::MakeFromX(PawnBoat->GetDriverMesh()->GetComponentLocation() - TempTransform.GetLocation()).Rotator().Yaw - 90.0f,
				0.0f
			}).Quaternion());

		HumanRef->UpdateInstanceTransform(HitResult.Item, TempTransform, false, false, true);
		FAnimInfo RandomCrashAnim = getrandomcrashanim();
		HumanRef->SetCustomDataValue(HitResult.Item, 0, RandomCrashAnim.AnimNumber, false);
		HumanRef->SetCustomDataValue(HitResult.Item, 1, RandomCrashAnim.AnimSpeed, true);
	}
}

/*********************Environment*************************************************************/

void AProceduralPlatformGeneratorBase::InitializeEnvironmentGeneration()
{
	int32 Tile2Index = 1;

	for (int32 i = 1; i < BaseTileCount - 1; i++)
	{
		AddEnvironmentToTile2(Tile2Index, true, false);
		Tile2Index++;
	}
}

void AProceduralPlatformGeneratorBase::GeneratePlatformEnvironment()
{
	int32 Tile2Index = OldLVLSwitchingTileCount - 1;
	int32 Tile2RLIndex = 0;

	for (int32 i = Tile2Index; i < TileInfo.Num() - LVLSwitchingTileCount; i++)
	{
		if (TileInfo[i].TileType == ETileType::Road2)
		{
			if (TileInfo[i-1].TileType == ETileType::Road2RL)
			{
				AddEnvironmentToTile2(Tile2Index, false, TileInfo[i - 1].FirstConnectionType == ETile2RLConnectionType::Right);
			}
			else if (TileInfo[i + 1].TileType == ETileType::Road2RL)
			{
				AddEnvironmentToTile2(Tile2Index, false, TileInfo[i + 1].FirstConnectionType == ETile2RLConnectionType::Right);
			}
			else
			{
				AddEnvironmentToTile2(Tile2Index, true, false);
			}

			Tile2Index++;
		}
		else
		{
			if (i == TileInfo.Num() - LVLSwitchingTileCount - 1) // if i equal last index 
			{
				AddEnvironmentToTile2RLArc(Tile2RLIndex);
			}
			else
			{
				AddEnvironmentToTile2RL(Tile2Index, i, Tile2RLIndex);
			}

			Tile2RLIndex++;
		}
	}
}

void AProceduralPlatformGeneratorBase::GenerateLVLSwitchEnvironment()
{
	int32 Tile2Index;
	int32 PlatformLastTileInfoIndex = TileInfo.Num() - LVLSwitchingTileCount - 1;

	if (TileInfo[PlatformLastTileInfoIndex].TileType == ETileType::Road2) //Platform last tile info
	{
		Tile2Index = HISM_Tile2->GetInstanceCount() - LVLSwitchingTileCount; //First LVL switch tile index
		AddEnvironmentToTile2(Tile2Index, true, false);
	}
	else
	{
		Tile2Index = HISM_Tile2->GetInstanceCount() - LVLSwitchingTileCount - 1; //Platform last tile2 index
		AddEnvironmentToTile2(Tile2Index, false, TileInfo[PlatformLastTileInfoIndex].FirstConnectionType == ETile2RLConnectionType::Back);
		Tile2Index++;
		AddEnvironmentToTile2(Tile2Index, false, TileInfo[PlatformLastTileInfoIndex].FirstConnectionType == ETile2RLConnectionType::Right);
	}

	Tile2Index++;

	for (int32 i = TileInfo.Num() - LVLSwitchingTileCount + 1; i < TileInfo.Num() - 1; i++)
	{
		AddEnvironmentToTile2(Tile2Index, true, false);
		Tile2Index++;
	}
}

void AProceduralPlatformGeneratorBase::AddEnvironmentToTile2(int32 Tile2Index, bool bIsBothSide, bool bIsRight)
{
	FTransform Tile2Transform;
	HISM_Tile2->GetInstanceTransform(Tile2Index, Tile2Transform);

	if (!bIsBothSide)
	{
		if (bIsRight)
		{
			AddEnvironment(Tile2Transform, false);
		}
		else
		{
			AddEnvironment(FTransform(
				{
					0.0f,
					Tile2Transform.GetRotation().Rotator().Yaw + 180.0f,
					0.0f
				},
				Tile2Transform.GetLocation(), FVector::OneVector), false);
		}
		return;
	}

	AddEnvironment(Tile2Transform, false);
	AddEnvironment(FTransform(
		{
			0.0f,
			Tile2Transform.GetRotation().Rotator().Yaw + 180.0f,
			0.0f
		},
		Tile2Transform.GetLocation(), FVector::OneVector), false);
}

void AProceduralPlatformGeneratorBase::AddEnvironmentToTile2RL(int32 Tile2Index, int32 TileInfoIndex, int32 Tile2RLIndex)
{
	AddEnvironmentToTile2RLArc(Tile2RLIndex);

	FTransform Tile2Transform;
	HISM_Tile2->GetInstanceTransform(Tile2Index - 1, Tile2Transform);
	float YawOffset = TileInfo[TileInfoIndex].FirstConnectionType == ETile2RLConnectionType::Back ? 0.0f : 180.0f;

	AddEnvironment(FTransform(
		{
			0.0f,
			Tile2Transform.GetRotation().Rotator().Yaw + YawOffset,
			0.0f
		},
		Tile2Transform.GetLocation(), FVector::OneVector), true);
}

void AProceduralPlatformGeneratorBase::AddEnvironment(FTransform InstanceTransform, bool bIsCorner)
{
	int32 RandomBuildingType;
	int32 AddedBuildingIndex;
	int32 RandomSignType = 0;
	int32 AddedSignIndex;
	int32 RandomPropType1 = 0;
	int32 RandomPropType2 = 0;
	int32 AddedPropIndex1;
	int32 AddedPropIndex2;

	if (FMath::RandHelper(10) == 0)
	{
		RandomBuildingType = NORMAL_BUILDING_COUNT + FMath::RandHelper(SPECIAL_BUILDING_COUNT);
		AddedBuildingIndex = EnvironmentItems[RandomBuildingType]->AddInstance(InstanceTransform);
		EnvironmentItems[RandomBuildingType]->SetCustomDataValue(AddedBuildingIndex, 0, float(FMath::RandHelper(POLYGON_CITY_TEXTURE_VARIATION_COUNT)));
	}
	else
	{
		RandomBuildingType = FMath::RandHelper(bIsCorner ? 5 : NORMAL_BUILDING_COUNT);
		AddedBuildingIndex = EnvironmentItems[RandomBuildingType]->AddInstance(InstanceTransform);
		if (RandomBuildingType > 3)
		{
			EnvironmentItems[RandomBuildingType]->SetCustomDataValue(AddedBuildingIndex, 0, float(FMath::RandHelper(POLYGON_CITY_TEXTURE_VARIATION_COUNT)));
		}
		if (RandomBuildingType == 8)
		{
			EnvironmentItems[RandomBuildingType]->SetCustomDataValue(AddedBuildingIndex, 1, float(FMath::RandHelper(BILLBOARD_TEXTURE_VARIATION_COUNT)));
		}
		if (RandomBuildingType == 1 || RandomBuildingType == 7 || RandomBuildingType == 8)
		{
			if (FMath::RandBool())
			{
				RandomSignType = NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT + FILLER_BUILDING_COUNT + FMath::RandHelper(SIGN_COUNT);
				AddedSignIndex = AddSign(RandomBuildingType, AddedBuildingIndex, RandomSignType);
			}
		}
	}

	if (FMath::RandBool())
	{
		RandomPropType1 = NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT + FILLER_BUILDING_COUNT + SIGN_COUNT + FMath::RandHelper(PROP_COUNT);
		AddedPropIndex1 = AddProp(RandomBuildingType, AddedBuildingIndex, RandomPropType1, PROP_OFFSET * -1);
	}
	if (FMath::RandBool())
	{
		RandomPropType2 = NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT + FILLER_BUILDING_COUNT + SIGN_COUNT + FMath::RandHelper(PROP_COUNT);
		AddedPropIndex2 = AddProp(RandomBuildingType, AddedBuildingIndex, RandomPropType2, PROP_OFFSET);
	}

	EnvironmentItemsIndexes[RandomBuildingType].Indexes.Add(AddedBuildingIndex);

	if (RandomSignType != 0)
	{
		EnvironmentItemsIndexes[RandomSignType].Indexes.Add(AddedSignIndex);
	}
	if (RandomPropType1 != 0)
	{
		EnvironmentItemsIndexes[RandomPropType1].Indexes.Add(AddedPropIndex1);
	}
	if (RandomPropType2 != 0)
	{
		EnvironmentItemsIndexes[RandomPropType2].Indexes.Add(AddedPropIndex2);
	}
}

void AProceduralPlatformGeneratorBase::AddEnvironmentToTile2RLArc(int32 InstanceIndex)
{
	FTransform Tile2RLTransform;
	HISM_Tile2RL->GetInstanceTransform(InstanceIndex, Tile2RLTransform);
	FVector Offset = Tile2RLTransform.TransformVectorNoScale(FVector::ForwardVector) * (TILE2RL_SIZE / 2) + Tile2RLTransform.TransformVectorNoScale(FVector::LeftVector) * (TILE2RL_SIZE / 2);

	AddEnvironment(FTransform(
		{
			0.0f,
			Tile2RLTransform.GetRotation().Rotator().Yaw - 135.0f,
			0.0f
		},
		Tile2RLTransform.GetLocation() + Offset, FVector::OneVector), false);

	int32 RandomBuildingType;
	int32 AddedBuildingIndex;

	RandomBuildingType = FMath::RandBool() ? NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT : NORMAL_BUILDING_COUNT + SPECIAL_BUILDING_COUNT + 1;
	AddedBuildingIndex = EnvironmentItems[RandomBuildingType]->AddInstance(Tile2RLTransform);
	EnvironmentItemsIndexes[RandomBuildingType].Indexes.Add(AddedBuildingIndex);
}

int32 AProceduralPlatformGeneratorBase::AddSign(int32 BuildingType, int32 BuildingIndex, int32 SignType)
{
	int32 SignIndex;
	FTransform BuildingT;
	EnvironmentItems[BuildingType]->GetInstanceTransform(BuildingIndex, BuildingT);
	SignIndex = EnvironmentItems[SignType]->AddInstance(FTransform(BuildingT.GetRotation().Rotator(),
		BuildingT.TransformPosition(EnvironmentItems[BuildingType]->GetSocketLocation(FName("Sign"))), FVector::OneVector));
	return SignIndex;
}

int32 AProceduralPlatformGeneratorBase::AddProp(int32 BuildingType, int32 BuildingIndex, int32 PropType, float Offset)
{
	int32 PropIndex;
	FTransform BuildingTransform;
	EnvironmentItems[BuildingType]->GetInstanceTransform(BuildingIndex, BuildingTransform);
	PropIndex = EnvironmentItems[PropType]->AddInstance(FTransform(BuildingTransform.GetRotation().Rotator(),
		BuildingTransform.GetLocation() + (BuildingTransform.TransformVectorNoScale(FVector::ForwardVector)*Offset), FVector::OneVector));
	if (PropType == 32)
	{
		EnvironmentItems[PropType]->SetCustomDataValue(PropIndex, 1, float(FMath::RandHelper(BILLBOARD_TEXTURE_VARIATION_COUNT)));
	}
	return PropIndex;
}

void AProceduralPlatformGeneratorBase::SetUpBackground()
{
	FTransform LastInstanceTransform;
	HISM_Tile2RL->GetInstanceTransform(0, LastInstanceTransform, false);
	SM_SkySphere->SetRelativeLocation(LastInstanceTransform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	SM_Env_Skyline->SetRelativeLocation(LastInstanceTransform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
}
