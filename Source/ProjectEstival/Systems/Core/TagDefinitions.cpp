#include "TagDefinitions.h"

namespace TagDefinitions
{
	// Stats
	UE_DEFINE_GAMEPLAY_TAG(Stats_Damage, "Stats.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Stats_Resources_Health, "Stats.Resources.Health");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stats_MoveSpeed, "Stats.MoveSpeed", "Walking speed in cm/s, written to the character's Max Walk Speed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stats_AttackSpeed, "Stats.AttackSpeed", "Ability speed multiplier. Set Base to 1 for normal speed, 2 is twice as fast, never below 0.1.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stats_CooldownReduction, "Stats.CooldownReduction", "Percent taken off every cooldown, 0 to 100. Set Base to 0");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stats_DamageReduction, "Stats.DamageReduction", "Percent of incoming damage ignored, 0 to 100. Set Base to 0");
	
	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Invincible, "Status.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stunned, "Status.Stunned");

	// Ability events
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityEvent_Hit, "AbilityEvent.Hit", "The hit frame of an ability animation");
}