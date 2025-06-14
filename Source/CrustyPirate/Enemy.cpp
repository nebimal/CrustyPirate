#include "Enemy.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerDetectorSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectorSphere"));
	PlayerDetectorSphere->SetupAttachment(RootComponent);

	HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
	HPText->SetupAttachment(RootComponent);

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerDetectorSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::DetectorOverlapBegin);
	PlayerDetectorSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::DetectorOverlapEnd);

	UpdateHP(HitPoints);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive && FollowTarget) // When FollowTarget is NULL, it stops following, or if enemy is NOT alive, also stop following.
	{
		// The Tick function is called for every frame, so if both the enemy is alive and the FollowTarget is player, we will move the enemies towards the player.
		float MoveDirection = (FollowTarget->GetActorLocation().X - GetActorLocation().X) > 0.0f ? 1.0f : -1.0f;// If positive, it's going to move right, and we're setting it to 1, if negative, it's going to move left, and we're setting it to -1;
		UpdateDirection(MoveDirection);
		if (ShouldMoveToTarget())
		{
			if (CanMove)
			{
				FVector WorldDirection = FVector(1.0f, 0.0f, 0.0f);
				AddMovementInput(WorldDirection, MoveDirection); // If MoveDirection is 1, player is on the right, enemy is going to move towards the right, if it's -1, player is on the left, enemy is going to move towards the left.
			}
		}
		else // Means we're already close enough to the player, so the enemy will attack the player.
		{
			// Attack
		}
	}
}

void AEnemy::UpdateDirection(float MoveDirection)
{
	FRotator CurrentRotation = GetActorRotation();
	if (MoveDirection < 0.0f)
	{
		if (CurrentRotation.Yaw != 180.0f) // Checks to see if player is already looking left
		{
			SetActorRotation(FRotator(CurrentRotation.Pitch, 180.0f, CurrentRotation.Roll));
			// Changes the rotation of the flipbook to the left.
		}
	}
	else if (MoveDirection > 0.0f)
	{
		if (CurrentRotation.Yaw != 0.0f) // Checks to see if player is already looking right
		{
			SetActorRotation(FRotator(CurrentRotation.Pitch, 0.0f, CurrentRotation.Roll));
			// Changes the rotation of the flipbook to the right.
		}
	}
}

bool AEnemy::ShouldMoveToTarget()
{
	bool Result = false;

	if (FollowTarget)
	{
		float DistToTarget = abs(FollowTarget->GetActorLocation().X - GetActorLocation().X); // Gets the positive distance of the enemy and the player.
		Result = DistToTarget > StopDistanceToTarget; // true if DistToTarget is greater than StopDistanceToTarget, false if its less than, meaning enemy is close to player so we're gonna stop following.
	}

	return Result;
}

void AEnemy::DetectorOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// OtherActor gets the actor we're overlapping with.
	// See if OtherActor is the player
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		FollowTarget = Player;
	}
}

void AEnemy::DetectorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		FollowTarget = NULL; // Player left the sphere so we're setting FollowTarget to NULL.
	}
}

void AEnemy::UpdateHP(int NewHP)
{
	HitPoints = NewHP;

	FString Str = FString::Printf(TEXT("HP: %d"), HitPoints);
	HPText->SetText(FText::FromString(Str)); // We need to cast from string to FText since SetText expects a FText class.
}

void AEnemy::TakeDamage(int DamageAmount, float StunDuration)
{
	if (!IsAlive)
	{
		return;
	}
	
	UpdateHP(HitPoints - DamageAmount);

	if (HitPoints <= 0)
	{
		IsAlive = false;
		UpdateHP(0);
		HPText->SetHiddenInGame(true); // Hides the HP Text

		IsAlive = false;
		CanMove = false;

		GetAnimInstance()->JumpToNode(FName("JumpDie"), FName("CrabbyStateMachine"));
	}
	else
	{
		GetAnimInstance()->JumpToNode(FName("JumpTakeHit"), FName("CrabbyStateMachine"));
	}
}