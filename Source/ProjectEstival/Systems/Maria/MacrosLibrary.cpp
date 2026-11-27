#include "MacrosLibrary.h"

#include "EstivalLogger.h"

FName MacrosLibrary::ValidateSetting(const FName SettingValue, const FString& SettingName)
{
	if (SettingValue == NAME_None)
	{
		//EERROR(ECat::Gameplay, 5.f, "Setting %s has NAME_None Value", *SettingName);
	}
	return SettingValue;
}
