#pragma once

#define VALIDATE_SETTING(Var) \
	MacrosLibrary::ValidateSetting(Var, TEXT(#Var))

#define DECLARE_GETTER_SETTER(Type, Name) \
	public: \
	Type Get##Name() const { return Name; } \
	void Set##Name(Type Var) {Name = Var; } \

	
class MacrosLibrary
{
public:
	
	static FName ValidateSetting(const FName SettingValue, const FString& SettingName);
	
};

class FScopedTimer
{
public:
	FScopedTimer(const TCHAR* InName) : Name(InName), StartTime(FPlatformTime::Seconds()){}
	~FScopedTimer()
	{
		const double Elapsed = FPlatformTime::Seconds() - StartTime;
	}
	
private:
	const TCHAR* Name;
	double StartTime;
};