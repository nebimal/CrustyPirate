#include "PlayerCharacter.h"

#include "Enemy.h"

#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); 

	AttackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollisionBox"));
	AttackCollisionBox->SetupAttachment(RootComponent);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	OnAttackOverrideEndDelegate.BindUObject(this, &APlayerCharacter::OnAttackOverrideAnimEnd);

	AttackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::AttackBoxOverlapBegin); // When there's an overlap with the collision box, the 'AttackBoxOverlapBegin' function will be called.
	EnableAttackCollisionBox(false); // Before we start the game, we make sure the collision box is disabled.

	MyGameInstance = Cast<UCrustyPirateGameInstance>(GetGameInstance());
	if (MyGameInstance)
	{
		HitPoints = MyGameInstance->PlayerHP; // Uses the HP that the CrustyPirateGameInstance has.
	}										  


	if (PlayerHUDClass)
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PlayerHUDClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToPlayerScreen(); // Adds this to the game

			PlayerHUDWidget->SetHP(HitPoints);
			PlayerHUDWidget->SetDiamonds(50); // Implement diamonds and levels.
			PlayerHUDWidget->SetLevel(1);
		}
	}
}
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) 
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move); // If we press on the 'a' or 'd' key, the Move function will be called.
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::JumpStarted); // JumpStarted gets called when we first press the space bar, only once.
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::JumpEnded);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack); // Calls the Attack function when the left mouse button is pressed
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	float MoveActionValue = Value.Get<float>();
	// 1 if 'd' is pressed
	// -1 if 'a' is pressed.

	if (IsAlive && CanMove && !IsStunned)
	{
		FVector Direction = FVector(1.0f, 0.0f, 0.0f);
		AddMovementInput(Direction, MoveActionValue);
		UpdateDirection(MoveActionValue);
	}
}

void APlayerCharacter::UpdateDirection(float MoveDirection)
{
	FRotator CurrentRotation = Controller->GetControlRotation(); // Looks at current rotation of player
	if (MoveDirection < 0.0f) //  If 'a' is pressed, player moving towards left, this if is called since a = -1.
	{
		if (CurrentRotation.Yaw != 180.0f) // Checks to see if player is already looking left
		{
			Controller->SetControlRotation(FRotator(CurrentRotation.Pitch, 180.0f, CurrentRotation.Roll));
			// Changes the rotation of the flipbook to the left.
		}
	}
	else if (MoveDirection > 0.0f) // If 'd' is pressed, player moving towards right,, this else if is called since d = 1.
	{
		if (CurrentRotation.Yaw != 0.0f) // Checks to see if player is already looking right
		{
			Controller->SetControlRotation(FRotator(CurrentRotation.Pitch, 0.0f, CurrentRotation.Roll));
			// Changes the rotation of the flipbook to the right.
		}
	}
}

void APlayerCharacter::JumpStarted(const FInputActionValue& Value)
{
	if (IsAlive && CanMove && !IsStunned)
	{
		Jump();
	}
}

void APlayerCharacter::JumpEnded(const FInputActionValue& Value)
{
	StopJumping();
}

void APlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (IsAlive && CanAttack && !IsStunned)
	{
		CanAttack = false;
		CanMove = false; // When we attack, we set 'CanMove' and 'CanAttack' to false so player can't move nor attack while we're in the attack animation.
	
		//EnableAttackCollisionBox(true); // Enables the collision box

		GetAnimInstance()->PlayAnimationOverride(AttackAnimSequence, FName("DefaultSlot"), 1.0f, 0.0f,
			OnAttackOverrideEndDelegate); // Play's the 'AttackAnimSequence' in the Override slot, and when the animation is over, it call's the 'OnAttackOverrideAnimEnd' function.
	}
}

void APlayerCharacter::OnAttackOverrideAnimEnd(bool Completed)
{ 
	CanAttack = true; // Player can move and attack again after the attack animation is finished.
	CanMove = true;

	//EnableAttackCollisionBox(false);
}

void APlayerCharacter::AttackBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);

	if (Enemy)
	{
		Enemy->TakeDamage(AttackDamage, AttackStunDuration);
	}
}

void APlayerCharacter::EnableAttackCollisionBox(bool Enabled)
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

void APlayerCharacter::TakeDamage(int DamageAmount, float StunDuration)
{
	if (!IsAlive)
	{
		return;
	}

	Stun(StunDuration);
	UpdateHP(HitPoints - DamageAmount);

	if (HitPoints <= 0)
	{
		// Player is dead
		UpdateHP(0);

		IsAlive = false;
		CanMove = false;
		CanAttack = false;

		GetAnimInstance()->JumpToNode(FName("JumpDie"), FName("CaptainStateMachine"));
		EnableAttackCollisionBox(false);
	}
	else
	{
		// Player is alive
		GetAnimInstance()->JumpToNode(FName("JumpTakeHit"), FName("CaptainStateMachine"));
	}
}

void APlayerCharacter::UpdateHP(int NewHP)
{
	HitPoints = NewHP;
	MyGameInstance->SetPlayerHP(HitPoints); // We need this because if we switch levels, the gamemode will end, restarting the scores, but the GameInstance only ends when the game is finished so we store the values there.
	PlayerHUDWidget->SetHP(HitPoints);      // Changes the player HP on the HUD.
}

void APlayerCharacter::Stun(float Duration)
{
	IsStunned = true;

	bool IsTimerAlreadyActive = GetWorldTimerManager().IsTimerActive(StunTimer);
	if (IsTimerAlreadyActive) // If true, enemy is already stun so we will stun it again from the beginning.
	{
		GetWorldTimerManager().ClearTimer(StunTimer);
	}

	GetWorldTimerManager().SetTimer(StunTimer, this, &APlayerCharacter::OnStunTimerTimeout, 1.0f, false, Duration);

	GetAnimInstance()->StopAllAnimationOverrides(); // To stop the enemy while the enemy is attacking.
	EnableAttackCollisionBox(false);
}

void APlayerCharacter::OnStunTimerTimeout()
{
	IsStunned = false;
}


void APlayerCharacter::CollectItem(CollectableType ItemType)
{
	UGameplayStatics::PlaySound2D(GetWorld(), ItemPickupSound); // Plays the sound

	switch (ItemType) // We're using the enum in CollectableItem to see which item was pickedup/Overlapped with.
	{
		case CollectableType::HealthPotion:
		{

		}break;

		case CollectableType::Diamond:
		{

		}break;

		case CollectableType::DoubleJumpUpgrade:
		{

		}break;

		default: 
		{ // Empty
		}break;

	}
}