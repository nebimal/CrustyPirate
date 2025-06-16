#include "CollectableItem.h"

#include "PlayerCharacter.h"

ACollectableItem::ACollectableItem()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);

	ItemFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("ItemFlipbook"));
	ItemFlipbook->SetupAttachment(RootComponent);

}

void ACollectableItem::BeginPlay()
{
	Super::BeginPlay();
	
	CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &ACollectableItem::OverlapBegin); // Call this function when there's an overlap with the capsule.
}

void ACollectableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACollectableItem::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor); 

	if (Player && Player->IsAlive) // Ensures the actor we overlapped with is a player and player is alive.
	{
		Player->CollectItem(Type);
		Destroy(); // Item is collected so we dont want it to be in the game anymore.
	}
}
