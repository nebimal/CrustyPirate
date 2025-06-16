#include "LevelExit.h"

#include "Kismet/GameplayStatics.h"

#include "PlayerCharacter.h"
#include "CrustyPirateGameInstance.h"

ALevelExit::ALevelExit()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);

	DoorFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoorFlipbook"));
	DoorFlipbook->SetupAttachment(RootComponent);

	DoorFlipbook->SetPlayRate(0.0f); // We set it to 0 because we want the door to remain close the whole time, until the player reaches the door, then it'll open.
	DoorFlipbook->SetLooping(false); // We want the animation only to play once.
}

void ALevelExit::BeginPlay()
{
	Super::BeginPlay();

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALevelExit::OverlapBegin);

	DoorFlipbook->SetPlaybackPosition(0.0f, false); // Sets the playback position to the first frame, to ensure the door is closed.
	
}

void ALevelExit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelExit::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

	if (Player && Player->IsAlive) // Ensures the actor we overlapped with is a player and player is alive.
	{
		if (IsActive)
		{
			Player->Deactivate(); // Deactivates the player so they can't move, hit, etc.

			IsActive = false;

			DoorFlipbook->SetPlayRate(1.0f);  // Sets the playrate to 1 since the collision between the player and the door happened, we want to see the door opening.
			DoorFlipbook->PlayFromStart();

			UGameplayStatics::PlaySound2D(GetWorld(), PlayerEnterSound);

			GetWorldTimerManager().SetTimer(WaitTimer, this, &ALevelExit::OnWaitTimerTimeout, 1.0f, false, WaitTimeInSeconds); // When the timer ends, call 'OnWaitTimerTimeout' function.
		}
	}
}

void ALevelExit::OnWaitTimerTimeout()
{
	UCrustyPirateGameInstance* MyGameInstance = Cast<UCrustyPirateGameInstance>(GetGameInstance());
	if (MyGameInstance)
	{
		MyGameInstance->ChangeLevel(LevelIndex);
	}

}

