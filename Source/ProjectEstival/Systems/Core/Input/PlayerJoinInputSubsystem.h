#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputCoreTypes.h"
#include "CoreMinimal.h"

#include "PlayerJoinInputSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELocalDeviceJoinPolicy : uint8
{
	RespectPlatformUser,
	AttachToFirstLocalPlayer
};

UCLASS()
class PROJECTESTIVAL_API UPlayerJoinInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void SetInputEnabled(bool bEnabled) { bInputEnabled = bEnabled; }
	bool IsInputEnabled() const { return bInputEnabled; }
	UFUNCTION(BlueprintCallable, Category = "Players|Input")
	void SetDeviceJoinPolicy(ELocalDeviceJoinPolicy Policy) { DeviceJoinPolicy = Policy; }
	bool JoinDevice(FInputDeviceId InputDeviceId) const;
	
private:
	friend class FPlayerJoinInputProcessor;
	TSharedPtr<class FPlayerJoinInputProcessor> InputProcessor;
	bool bInputEnabled = false;
	bool HandleKeyDown(const FKeyEvent& Event) const;
	int32 MaxLocalPlayers = 4;
	ELocalDeviceJoinPolicy DeviceJoinPolicy = ELocalDeviceJoinPolicy::RespectPlatformUser;
	FDelegateHandle ConnectionChangeHandle;
	void HandleDeviceConnectionChange(EInputDeviceConnectionState ConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId) const;
};
