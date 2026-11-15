// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DpsTrap.generated.h"


class UBoxComponent;
class UStaticMeshComponent;


UENUM(BlueprintType)
enum class ETrapState : uint8
{
	Idle,
	Telegraph,
	Active,
	Cooldown
};


UCLASS()
class PROJECTESTIVAL_API ADpsTrap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADpsTrap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Trap Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerArea;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VisualMesh;
	
	//Trap Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TelegraphDelay = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float Active = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float Cooldown = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DpsRate = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DamagePerRate = 10.0f;
	
	
private:
	ETrapState CurrentState;
	
	FTimerHandle StateTimerHandle;
	FTimerHandle DpsTimerHandle;
	
	
	UPROPERTY(Transient)
	TArray<AActor*> ActorsInArea;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void SetState(ETrapState NewState);
	void ActivateTrap();
	void EndTrapActive();
	void ResetToIdle();
	void ApplyDpsTick();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
