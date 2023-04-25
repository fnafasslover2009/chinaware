#include "GetWeaponName.h"
#pragma warning (disable : 26819) //fallthrough warning

const char* WeaponESP::GetWeaponName(int weaponID, int itemIndex, EWeaponType weaponType)
{
	if (itemIndex == 474) //conscientious objector
		return "MELEE"; //because this nigger shit keeps on showing wrong shit if i dont do this

	switch (weaponID)
	{
	case TF_WEAPON_BAT:
		switch (itemIndex)
		{
		case Scout_t_TheCandyCane:
			return "CANDY CANE";
		case Scout_t_TheBostonBasher:
		case Scout_t_ThreeRuneBlade:
			return "BOSTON BASHER";
		case Scout_t_SunonaStick:
			return "SUN ON A STICK";
		case Scout_t_TheFanOWar:
			return "FAN O WAR";
		case Scout_t_TheAtomizer:
			return "ATOMIZER";
		default: return "BAT";
		}
	case TF_WEAPON_BAT_FISH:
		return "HOLY MACKEREL";
	case TF_WEAPON_BAT_GIFTWRAP:
		return "WRAP ASSASSIN";
	case TF_WEAPON_BAT_WOOD:
		return "SANDMAN";
	case TF_WEAPON_BONESAW:
		switch (itemIndex)
		{
		case Medic_t_Amputator:
			return "AMPUTATOR";
		case Medic_t_TheUbersaw:
		case Medic_t_FestiveUbersaw:
			return "UBERSAW";
		case Medic_t_TheVitaSaw:
			return "VITA-SAW";
		case Medic_t_TheSolemnVow:
			return "SOLEMN VOW";
		default: return "BONESAW";
		}
	case TF_WEAPON_BOTTLE:
		return "BOTTLE";
	case TF_WEAPON_BREAKABLE_SIGN:
		return "NEON ANNIHILATOR";
	case TF_WEAPON_BUFF_ITEM:
		switch (itemIndex)
		{
		case Soldier_s_TheBuffBanner:
		case Soldier_s_FestiveBuffBanner:
			return "BUFF BANNER";
		case Soldier_s_TheBattalionsBackup:
			return "BATTALIONS BACKUP";
		case Soldier_s_TheConcheror:
			return "CONCHEROR";
		}
	case TF_WEAPON_BUILDER:
		switch (itemIndex)
		{
		case Engi_p_PDA:
			return "TOOLBOX";
		case Spy_s_Sapper:
		case Spy_s_SapperR:
			return "SAPPER";
		}
	case TF_WEAPON_CANNON:
		return "LOOSE CANNON";
	case TF_WEAPON_CHARGED_SMG:
		return "CLEANERS CARBINE";
	case TF_WEAPON_CLEAVER:
		return "CLEAVER";
	case TF_WEAPON_CLUB:
		switch (itemIndex)
		{
		case Sniper_t_TheTribalmansShiv:
			return "TRIBALMANS SHIV";
		case Sniper_t_TheBushwacka:
			return "BUSHWACKA";
		case Sniper_t_TheShahanshah:
			return "SHAHANSHAH";
		default: return "KUKRI";
		}
	case TF_WEAPON_COMPOUND_BOW:
		return "HUNTSMAN";
	case TF_WEAPON_CROSSBOW:
		return "CROSSBOW";
	case TF_WEAPON_DRG_POMSON:
		return "POMSON";
	case TF_WEAPON_FIREAXE:
		switch (itemIndex)
		{
		case Pyro_t_TheAxtinguisher:
		case Pyro_t_TheFestiveAxtinguisher:
		case Pyro_t_ThePostalPummeler:
			return "AXTINGUISHER";
		case Pyro_t_Homewrecker:
		case Pyro_t_TheMaul:
			return "HOMEWRECKER";
		case Pyro_t_TheBackScratcher:
			return "BACK SCRATCHER";
		case Pyro_t_ThePowerjack:
			return "POWERJACK";
		case Pyro_t_SharpenedVolcanoFragment:
			return "VOLCANO FRAGMENT";
		case Pyro_t_TheThirdDegree:
			return "THIRD DEGREE";
		default: return "FIRE AXE";
		}
	case TF_WEAPON_FISTS:
		switch (itemIndex)
		{
		case Heavy_t_TheKillingGlovesofBoxing:
			return "KGB";
		case Heavy_t_GlovesofRunningUrgently:
		case Heavy_t_GlovesofRunningUrgentlyMvM:
		case Heavy_t_FestiveGlovesofRunningUrgently:
		case Heavy_t_TheBreadBite:
			return "GRU";
		case Heavy_t_WarriorsSpirit:
			return "WARRIORS SPIRIT";
		case Heavy_t_FistsofSteel:
			return "FISTS OF STEEL";
		case Heavy_t_TheEvictionNotice:
			return "EVICTION NOTICE";
		case Heavy_t_TheHolidayPunch:
			return "HOLIDAY PUNCH";
		default: return "FISTS";
		}
	case TF_WEAPON_FLAMETHROWER:
		switch (itemIndex)
		{
		case Pyro_m_TheBackburner:
		case Pyro_m_FestiveBackburner:
			return "BACKBURNER";
		case Pyro_m_TheDegreaser:
			return "DEGREASER";
		case Pyro_m_ThePhlogistinator:
			return "PHLOGISTINATOR";
		case Pyro_m_NostromoNapalmer:
			return "NOSTROMO NAPALMER"; // i have this separated because it has like one special stat compared to the rest of the flamethrowers
		default: return "FLAME THROWER";
		}
	case TF_WEAPON_FLAME_BALL:
		return "DRAGONS FURY";
	case TF_WEAPON_FLAREGUN:
		switch (itemIndex)
		{
		case Pyro_s_TheDetonator:
			return "DETONATOR";
		case Pyro_s_TheScorchShot:
			return "SCORCH SHOT";
		default: return "FLARE GUN";
		}
	case TF_WEAPON_GRAPPLINGHOOK:
		return "GRAPPLING HOOK";
	case TF_WEAPON_GRENADELAUNCHER:
		switch (itemIndex)
		{
		case Demoman_m_TheLochnLoad:
			return "LOCH N LOAD";
		case Demoman_m_TheIronBomber:
			return "IRON BOMBER";
		default: return "GRENADE LAUNCHER";
		}
	case TF_WEAPON_HANDGUN_SCOUT_PRIMARY:
		return "SHORTSTOP";
	case TF_WEAPON_HANDGUN_SCOUT_SEC:
		switch (itemIndex)
		{
		case Scout_s_TheWinger:
			return "WINGER";
		case Scout_s_PrettyBoysPocketPistol:
			return "POCKET PISTOL";
		}
	case TF_WEAPON_JAR:
		return "JARATE";
	case TF_WEAPON_JAR_GAS:
		return "GAS PASSER";
	case TF_WEAPON_JAR_MILK:
		return "MAD MILK";
	case TF_WEAPON_KNIFE:
		switch (itemIndex)
		{
		case Spy_t_YourEternalReward:
		case Spy_t_TheWangaPrick:
			return "ETERNAL REWARD";
		case Spy_t_ConniversKunai:
			return "KUNAI";
		case Spy_t_TheBigEarner:
			return "BIG EARNER";
		case Spy_t_TheSpycicle:
			return "SPYCICLE";
		default: return "KNIFE";
		}
	case TF_WEAPON_LASER_POINTER:
		return "WRANGLER";
	case TF_WEAPON_LUNCHBOX:
		switch (itemIndex)
		{
		case Heavy_s_TheDalokohsBar:
		case Heavy_s_Fishcake:
			return "DALOKOHS BAR";
		case Heavy_s_TheBuffaloSteakSandvich:
			return "STEAK";
		case Heavy_s_SecondBanana:
			return "BANANA";
		case Scout_s_BonkAtomicPunch:
		case Scout_s_FestiveBonk:
			return "BONK";
		case Scout_s_CritaCola:
			return "CRIT COLA";
		default: return "SANDVICH";
		}
	case TF_WEAPON_MECHANICAL_ARM:
		return "SHORT CIRCUIT";
	case TF_WEAPON_MEDIGUN:
		switch (itemIndex)
		{
		case Medic_s_TheKritzkrieg:
			return "KRITZKRIEG";
		case Medic_s_TheQuickFix:
			return "QUICK FIX";
		case Medic_s_TheVaccinator:
			return "VACCINATOR";
		default: return "MEDIGUN";
		}
	case TF_WEAPON_MINIGUN:
		switch (itemIndex)
		{
		case Heavy_m_Natascha:
			return "NATASCHA";
		case Heavy_m_TheBrassBeast:
			return "BRASS BEAST";
		case Heavy_m_Tomislav:
			return "TOMISLAV";
		case Heavy_m_TheHuoLongHeater:
		case Heavy_m_TheHuoLongHeaterG:
			return "HUO-LONG HEATER";
		default: return "MINIGUN";
		}
	case TF_WEAPON_PARACHUTE: // i think you equip this during A pose on demoman?
		return "BASE JUMPER";
	case TF_WEAPON_PARTICLE_CANNON:
		return "COW MANGLER";
	case TF_WEAPON_PDA_ENGINEER_BUILD:
		return "BUILD PDA";
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
		return "DESTRUCTION PDA";
	case TF_WEAPON_PDA_SPY:
		return "DISGUISE KIT";
	case TF_WEAPON_PEP_BRAWLER_BLASTER:
		return "BABY FACE BLASTER";
	case TF_WEAPON_PIPEBOMBLAUNCHER:
		switch (itemIndex)
		{
		case Demoman_s_TheScottishResistance:
			return "SCOTTISH RESISTANCE";
		case Demoman_s_StickyJumper:
			return "STICKY JUMPER";
		case Demoman_s_TheQuickiebombLauncher:
			return "QUICKIE LAUNCHER";
		default: return "STICKY LAUNCHER";
		}
	case TF_WEAPON_PISTOL:
	case TF_WEAPON_PISTOL_SCOUT:
		return "PISTOL";
	case TF_WEAPON_RAYGUN:
		return "BISON";
	case TF_WEAPON_RAYGUN_REVENGE:
		return "MANMELTER";
	case TF_WEAPON_REVOLVER:
		switch (itemIndex)
		{
		case Spy_m_TheAmbassador:
		case Spy_m_FestiveAmbassador:
			return "AMBASSADOR";
		case Spy_m_LEtranger:
			return "LETRANGER";
		case Spy_m_TheEnforcer:
			return "ENFORCER";
		case Spy_m_TheDiamondback:
			return "DIAMONDBACK";
		default: return "REVOLVER";
		}
	case TF_WEAPON_ROCKETLAUNCHER:
		switch (itemIndex)
		{
		case Soldier_m_TheBlackBox:
		case Soldier_m_FestiveBlackBox:
			return "BLACK BOX";
		case Soldier_m_RocketJumper:
			return "ROCKET JUMPER";
		case Soldier_m_TheLibertyLauncher:
			return "LIBERTY LAUNCHER";
		case Soldier_m_TheOriginal:
			return "ORIGINAL";
		case Soldier_m_TheBeggarsBazooka:
			return "BEGGARS BAZOOKA";
		case Soldier_m_TheAirStrike:
			return "AIR STRIKE";
		default: return "ROCKET LAUNCHER";
		}
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		return "DIRECT HIT";
	case TF_WEAPON_ROCKETPACK:
		return "THERMAL THRUSTER";
	case TF_WEAPON_SCATTERGUN:
		switch (itemIndex)
		{
		case Scout_m_ForceANature:
		case Scout_m_FestiveForceANature:
			return "FORCE A NATURE";
		default: return "SCATTERGUN";
		}
	case TF_WEAPON_SENTRY_REVENGE:
		return "FRONTIER JUSTICE";
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		return "RESCUE RANGER";
	case TF_WEAPON_SHOTGUN_HWG:
	case TF_WEAPON_SHOTGUN_PRIMARY:
	case TF_WEAPON_SHOTGUN_PYRO:
	case TF_WEAPON_SHOTGUN_SOLDIER:
		switch (itemIndex)
		{
		case Engi_m_TheWidowmaker:
			return "WIDOWMAKER";
		default: return "SHOTGUN";
		}
	case TF_WEAPON_SHOVEL:
		switch (itemIndex)
		{
		case Soldier_t_TheEqualizer:
			return "EQUALIZER";
		case Soldier_t_ThePainTrain:
			return "PAIN TRAIN";
		case Soldier_t_TheHalfZatoichi:
			return "HALF ZATOICHI";
		case Soldier_t_TheMarketGardener:
			return "MARKET GARDENER";
		case Soldier_t_TheDisciplinaryAction:
			return "WHIP";
		case Soldier_t_TheEscapePlan:
			return "ESCAPE PLAN";
		default: return "SHOVEL";
		}
	case TF_WEAPON_SLAP:
		return "HOT HAND";
	case TF_WEAPON_SMG:
		return "SMG";
	case TF_WEAPON_SNIPERRIFLE:
		switch (itemIndex)
		{
		case Sniper_m_TheSydneySleeper:
			return "SYDNEY SLEEPER";
		case Sniper_m_TheMachina:
		case Sniper_m_ShootingStar:
			return "MACHINA";
		case Sniper_m_TheHitmansHeatmaker:
			return "HITMANS HEATMAKER";
		default: return "SNIPER RIFLE";
		}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		return "CLASSIC";
	case TF_WEAPON_SNIPERRIFLE_DECAP:
		return "BAZAAR BARGAIN";
	case TF_WEAPON_SODA_POPPER:
		return "SODA POPPER";
	case TF_WEAPON_SPELLBOOK:
		return "SPELLBOOK";
	case TF_WEAPON_STICKBOMB:
		return "CABER";
	case TF_WEAPON_SWORD:
		switch (itemIndex)
		{
		case Demoman_t_TheScotsmansSkullcutter:
			return "SKULLCUTTER";
		case Demoman_t_TheClaidheamhMor:
			return "CLAIDHEAMH MOR";
		case Demoman_t_ThePersianPersuader:
			return "PERSIAN PERSUADER";
		default: return "EYELANDER";
		}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
		switch (itemIndex)
		{
		case Medic_m_TheBlutsauger:
			return "BLUTSAUGER";
		case Medic_m_TheOverdose:
			return "OVERDOSE";
		default: return "SYRINGE GUN";
		}
	case TF_WEAPON_WRENCH:
		switch (itemIndex)
		{
		case Engi_t_TheSouthernHospitality:
			return "SOUTHERN HOSPITALITY";
		case Engi_t_TheJag:
			return "JAG";
		case Engi_t_TheEurekaEffect:
			return "EUREKA EFFECT";
		case Engi_t_TheGunslinger: //apparently this is part of wrench? weapon index website says tf_weapon_robot_arm...
			return "GUNSLINGER";
		default: return "WRENCH";
		}
	}

	switch (itemIndex) // in the case idk the weapon id :shrug:
	{ 
//	case Scout_s_BonkAtomicPunch: UNDER TF_WEAPON_LUNCHBOX
//	case Scout_s_FestiveBonk:
//		return "BONK";
//	case Scout_s_CritaCola:
//		return "CRIT COLA";
//	case Soldier_m_TheAirStrike: under TF_WEAPON_ROCKETLAUNCHER
//		return "AIR STRIKE";
	case Heavy_m_Deflector_mvm:
		return "DEFLECTOR";
	case Pyro_m_DragonsFury: //idek if i need this here
		return "DRAGONS FURY";
	case Misc_t_Saxxy:
		return "SAXXY";
	case Misc_t_FryingPan:
		return "FRYING PAN";
	case Misc_t_GoldFryingPan:
		return "GOLDEN FRYING PAN";
	}

	switch (weaponType) //last ditch effort to get a name
	{
	case EWeaponType::HITSCAN:
		return "HITSCAN WEAPON";
	case EWeaponType::PROJECTILE:
		return "PROJECTILE WEAPON";
	case EWeaponType::MELEE:
		return "MELEE";
	}

	return "UNKNOWN";
}