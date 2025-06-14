#include "Enemy.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerDetectorSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectorSphere"));
	PlayerDetectorSphere->SetupAttachment(RootComponent);

	HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
	HPText->SetupAttachment(RootComponent);

	AttackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollisionBox"));
	AttackCollisionBox->SetupAttachment(RootComponent);

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerDetectorSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::DetectorOverlapBegin);
	PlayerDetectorSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::DetectorOverlapEnd);

	UpdateHP(HitPoints);

	OnAttackOverrideEndDelegate.BindUObject(this, &AEnemy::OnAttackOverrideAnimEnd);

	AttackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackBoxOverlapBegin);
	EnableAttackCollisionBox(false);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive && FollowTarget && !IsStunned) // When FollowTarget is NULL stop following, if enemy is NOT alive stop following, if enemy is Stunned stop following
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
			if (FollowTarget->IsAlive)
			{
				Attack();
			}
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
		CanAttack = false;

		GetAnimInstance()->JumpToNode(FName("JumpDie"), FName("CrabbyStateMachine"));

		EnableAttackCollisionBox(false);
	}
	else
	{
		Stun(StunDuration);
		GetAnimInstance()->JumpToNode(FName("JumpTakeHit"), FName("CrabbyStateMachine"));
	}
}

void AEnemy::Stun(float Duration)
{
	IsStunned = true;

	bool IsTimerAlreadyActive = GetWorldTimerManager().IsTimerActive(StunTimer);
	if (IsTimerAlreadyActive) // If true, enemy is already stun so we will stun it again from the beginning.
	{
		GetWorldTimerManager().ClearTimer(StunTimer);
	}

	GetWorldTimerManager().SetTimer(StunTimer, this, &AEnemy::OnStunTimerTimeout, 1.0f, false, Duration);

	GetAnimInstance()->StopAllAnimationOverrides(); // To stop the enemy while the enemy is attacking.
	EnableAttackCollisionBox(false);
}

void AEnemy::OnStunTimerTimeout()
{
	IsStunned = false;
}

void AEnemy::Attack()
{
	if (IsAlive && CanAttack && !IsStunned)
	{
		CanAttack = false;
		CanMove = false;

		GetAnimInstance()->PlayAnimationOverride(AttackAnimSequence, FName("DefaultSlot"), 1.0f, 0.0f,
			OnAttackOverrideEndDelegate);

		GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &AEnemy::OnAttackCooldownTimerTimeout, 1.0f,
			false, AttackCooldown);
	}
}

void AEnemy::OnAttackOverrideAnimEnd(bool Completed)
{
	if (IsAlive)
	{
		CanMove = true;
	}
}

void AEnemy::OnAttackCooldownTimerTimeout()
{
	if (IsAlive)
	{
		CanAttack = true;
	}
}

void AEnemy::AttackBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		Player->TakeDamage(AttackDamage, AttackStunDuration);
	}
}

void AEnemy::EnableAttackCollisionBox(bool Enabled)
{
	if (Enabled)
	{
		AttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttackCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
			ECollisionResponse::ECR_Overlap); // Enables collision.
	}
	else
	{
		AttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttackCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
			ECollisionResponse::ECR_Ignore); // Disables collision.
	}
}