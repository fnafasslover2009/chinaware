#include "CritHack.h"
#define MASK_SIGNED 0x7FFFFFFF

// i hate crithack

/* Returns whether random crits are enabled on the server */
bool CCritHack::AreRandomCritsEnabled()
{
	if (static auto tf_weapon_criticals = g_ConVars.FindVar("tf_weapon_criticals"); tf_weapon_criticals)
	{
		return tf_weapon_criticals->GetBool();
	}
	return true;
}

/* Returns whether the crithack should run */
bool CCritHack::IsEnabled()
{
	if (!Vars::CritHack::Active.Value) { return false; }
	if (!AreRandomCritsEnabled()) { return false; }
	if (!I::EngineClient->IsInGame()) { return false; }

	return true;
}

bool CCritHack::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false;

		if (pWeapon->GetClip1() > 0)
		{
			bLoading = true;
		}

		if (!(pCmd->buttons & IN_ATTACK) && bLoading)
		{
			bLoading = false;
			return true;
		}
	}

	else
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
		{
			static bool bCharging = false;

			if (pWeapon->GetChargeBeginTime() > 0.0f)
			{
				bCharging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && bCharging)
			{
				bCharging = false;
				return true;
			}
		}

		//pssst..
		//Dragon's Fury has a gauge (seen on the weapon model) maybe it would help for pSilent hmm..
		/*
		if (pWeapon->GetWeaponID() == 109) {
		}*/

		else
		{
			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
			{
				return true;
			}
		}
	}

	return false;
}

bool CCritHack::NoRandomCrits(CBaseCombatWeapon* pWeapon)
{
	float CritChance = Utils::ATTRIB_HOOK_FLOAT(1, "mult_crit_chance", pWeapon);
	if (CritChance == 0)
	{
		return true;
	}
	else 
	return false;
	//list of weapons that cant random crit, but dont have the attribute for it
	switch (pWeapon->GetWeaponID())
	{
		//scout
		case TF_WEAPON_JAR_MILK:
		//soldier
		case TF_WEAPON_BUFF_ITEM:
		//pyro
		case TF_WEAPON_JAR_GAS:
		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_ROCKETPACK:
		//demo
		case TF_WEAPON_PARACHUTE: //also for soldier
		//heavy
		case TF_WEAPON_LUNCHBOX:
		//engineer
		case TF_WEAPON_PDA_ENGINEER_BUILD:
		case TF_WEAPON_PDA_ENGINEER_DESTROY:
		case TF_WEAPON_LASER_POINTER:
		//medic
		case TF_WEAPON_MEDIGUN:
		//sniper
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_JAR:
		//spy
		case TF_WEAPON_KNIFE:
		case TF_WEAPON_PDA_SPY_BUILD:
		case TF_WEAPON_PDA_SPY:
			return true;
			break;
		default: return false; break;
	}
}

bool CCritHack::ShouldCrit()
{
	static KeyHelper critKey{ &Vars::CritHack::CritKey.Value };
	if (critKey.Down()) { return true; }
	if (G::CurWeaponType == EWeaponType::MELEE && Vars::CritHack::AlwaysMelee.Value) { return true; }

	return false;
}

int CCritHack::LastGoodCritTick(const CUserCmd* pCmd)
{
	int retVal = -1;
	bool popBack = false;

	for (auto it = CritTicks.rbegin(); it != CritTicks.rend(); ++it)
	{
		if (*it >= pCmd->command_number)
		{
			retVal = *it;
		}
		else
		{
			popBack = true;
		}
	}

	if (popBack)
	{
		CritTicks.pop_back();
	}

	if (auto netchan = I::EngineClient->GetNetChannelInfo())
	{
		const int lastOutSeqNr = netchan->m_nOutSequenceNr;
		const int newOutSeqNr = pCmd->command_number - 1;
		if (newOutSeqNr > lastOutSeqNr)
		{
			netchan->m_nOutSequenceNr = newOutSeqNr;
		}
	}

	return retVal;
}

void CCritHack::ScanForCrits(const CUserCmd* pCmd, int loops)
{
	static int previousWeapon = 0;
	static int previousCrit = 0;
	static int startingNum = pCmd->command_number;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal) { return; }

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon) { return; }

	if (G::IsAttacking || IsAttacking(pCmd, pWeapon) /*|| pCmd->buttons & IN_ATTACK*/)
	{
		return;
	}

	const bool bRescanRequired = previousWeapon != pWeapon->GetIndex();
	if (bRescanRequired)
	{
		startingNum = pCmd->command_number;
		previousWeapon = pWeapon->GetIndex();
		CritTicks.clear();
	}

	if (CritTicks.size() >= 256)
	{
		return;
	}

	//float CritBucketBP = *reinterpret_cast<float*>(pWeapon + 0xA54);
	ProtectData = true; //	stop shit that interferes with our crit bucket because it will BREAK it
	const int seedBackup = MD5_PseudoRandom(pCmd->command_number) & MASK_SIGNED;
	for (int i = 0; i < loops; i++)
	{
		const int cmdNum = startingNum + i;
		const bool result = (Utils::RandInt(0, 9999) == 0);
		*I::RandomSeed = MD5_PseudoRandom(cmdNum) & MASK_SIGNED;
		if (pWeapon->WillCrit() || result)
		{
			// Check if the command number is already in CritTicks
			bool isNewCommand = true;
			for (const auto& critTick : CritTicks)
			{
				if (critTick == cmdNum)
				{
					isNewCommand = false;
					break;
				}
			}

			if (isNewCommand)
			{
				CritTicks.push_back(cmdNum); //	store our wish command number for later reference
			}
		}
	}
	startingNum += loops;
	ProtectData = false; //	we no longer need to be protecting important crit data

	//*reinterpret_cast<float*>(pWeapon + 0xA54) = CritBucketBP;
	*reinterpret_cast<int*>(pWeapon + 0xA5C) = 0; //	dont comment this out, makes sure our crit mult stays as low as possible
	//	crit mult can reach a maximum value of 3!! which means we expend 3 crits WORTH from our bucket
	//	by forcing crit mult to be its minimum value of 1, we can crit more without directly fucking our bucket
	//	yes ProtectData stops this value from changing artificially, but it still changes when you fire and this is worth it imo.
	*I::RandomSeed = seedBackup;
}

void CCritHack::Run(CUserCmd* pCmd)
{
	if (!IsEnabled()) { return; }

	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pWeapon || !pWeapon->CanFireCriticalShot(false) || pWeapon->IsCritBoosted()) { return; }

	CGameEvent* pEvent = nullptr;
	const FNV1A_t uNameHash = 0;

	ScanForCrits(pCmd, 50);

	const int closestGoodTick = LastGoodCritTick(pCmd); //	retrieve our wish
	if (IsAttacking(pCmd, pWeapon)) //	is it valid & should we even use it
	{
		if (ShouldCrit() && (pWeapon->GetCritTokenBucket() >= 100) && CritBanned(pEvent, uNameHash).critbanned == false)
		{
			if (closestGoodTick < 0) { return; }
			pCmd->command_number = closestGoodTick; //	set our cmdnumber to our wish
			pCmd->random_seed = MD5_PseudoRandom(closestGoodTick) & MASK_SIGNED;//	trash poopy whatever who cares
			if (G::CurWeaponType != EWeaponType::MELEE && CritBanned(pEvent, uNameHash).critbanned == false)
			{
				I::EngineClient->GetNetChannelInfo()->m_nOutSequenceNr = closestGoodTick - 1;
			}
		}
		else if (Vars::CritHack::AvoidRandom.Value) //	we don't want to crit
		{
			for (int tries = 1; tries < 25; tries++)
			{
				if (std::find(CritTicks.begin(), CritTicks.end(), pCmd->command_number + tries) != CritTicks.end())
				{
					continue; //	what a useless attempt
				}
				pCmd->command_number += tries;
				pCmd->random_seed = MD5_PseudoRandom(pCmd->command_number) & MASK_SIGNED;
				break; //	we found a seed that we can use to avoid a crit and have skipped to it, woohoo
			}
		}
	}
}

struct observedcrits
{
	bool critbanned;
	int damagedone;
};

CCritHack::observedcrits CCritHack::CritBanned(CGameEvent* pEvent, const FNV1A_t uNameHash) // this is gay nd not work
{
	const auto pWeapon = g_EntityCache.GetWeapon();
	CTFPlayerResource* pPlayerResource = g_EntityCache.GetPR();

	if (!I::EngineClient->IsConnected() || !I::EngineClient->IsInGame()) { return { true, 0 }; } // set these to true first

	if (const auto pLocal = g_EntityCache.GetLocal())
	{
		if (uNameHash == FNV1A::HashConst("player_hurt"))
		{
			if (const auto pEntity = I::ClientEntityList->GetClientEntity( I::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) ) ) ) // lol
			{
				const auto Attacked = I::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );
				const auto Attacker = I::EngineClient->GetPlayerForUserID( pEvent->GetInt( "attacker" ) );
				const auto Health = pEvent->GetInt("health");
				      auto Damage = pEvent->GetInt("damageamount");
				const auto Crit = pEvent->GetBool("crit");

				static int cacheddamage = 0;
				static int critdamage = 0;
				static int meleedamage = 0;
				static int rounddamage = 0;
				static int rangeddamage = 0;

				if (pEntity == pLocal) { return { true, 0 }; }

				if (Attacker == pLocal->GetIndex() && Attacked != Attacker)
				{
					DVariant dawg;
					CClientEntityList blawg{};
					const auto entity = blawg.GetClientEntity(Attacked);
					if (entity == nullptr)
						return { true, 0 };

					rounddamage = static_cast<float>(pPlayerResource->GetDamage(pLocal->GetIndex()));

					if (Crit && G::CurWeaponType != EWeaponType::MELEE)
						critdamage += Damage;
					else if (Crit && G::CurWeaponType == EWeaponType::MELEE)
						meleedamage += Damage;
					
					float sentchance = dawg.m_Float;

					if (sentchance)
					{
						rangeddamage = rounddamage - meleedamage;
						if (rangeddamage != 0 && 2.0f * sentchance + 1 != 0.0f)
						{
							cacheddamage = rangeddamage - rounddamage;
							critdamage = (3.0f * rangeddamage * sentchance) / (2.0f * sentchance + 1);
						}
					}
			
					// what we need to be at or lower to get a crit
					float crit_mult = static_cast<float>(pWeapon->GetCritMult());
					float chance = 0.02f;

					if (G::CurWeaponType == EWeaponType::MELEE)
					{
						chance = 0.15f;
					}

					float flMultCritChance = Utils::ATTRIB_HOOK_FLOAT(crit_mult * chance, "mult_crit_chance", pWeapon, 0, 1);

					// automatic weapons
					if (pWeapon->IsRapidFire())
					{
						float flTotalCritChance = std::clamp(0.02f * crit_mult, 0.01f, 0.99f);
						float flCritDuration = 2.0f;
						float flNonCritDuration = (flCritDuration / flTotalCritChance) - flCritDuration;
						float flStartCritChance = 1 / flNonCritDuration;
						flMultCritChance = Utils::ATTRIB_HOOK_FLOAT(flStartCritChance, "mult_crit_chance", pWeapon, 0, 1);
					}
					
					float normalizeddamage = (float)critdamage / 3.0f;
					float niggachance = normalizeddamage / (normalizeddamage + (float)((cacheddamage - rounddamage) - critdamage));
					float NeededChance = flMultCritChance + 0.1f;

					if (niggachance >= NeededChance || pWeapon->GetObservedCritChance() >= NeededChance)	
					{ 
						CritTicks.clear();
						return  { true, Damage };
					}
				}
			}
		}
	}
	return { false, Damage }; 
}

int CCritHack::potentialcrits(float damage)
{

}

void CCritHack::Draw()
{
	if (!Vars::CritHack::Indicators.Value) { return; }
	if (!IsEnabled() || !G::CurrentUserCmd) { return; }

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive()) { return; }

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon) { return; }

	const int x = Vars::CritHack::IndicatorPos.c;
	int currentY = Vars::CritHack::IndicatorPos.y;

	const float bucket = *reinterpret_cast<float*>(pWeapon + 0xA54);
	const int seedRequests = *reinterpret_cast<int*>(pWeapon + 0xA5C);

	CGameEvent* pEvent = nullptr;
	const FNV1A_t uNameHash = 0;

	int longestW = 40;

	if (Vars::Debug::DebugInfo.Value)
	{ 
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 255, 255, 255, 255, }, ALIGN_CENTERHORIZONTAL, tfm::format("%x", reinterpret_cast<float*>(pWeapon + 0xA54)).c_str());
	}
	if (ShouldCrit() && NoRandomCrits(pWeapon) == false) // Are we currently forcing crits?
	{
		if (CritTicks.size() > 0)
		{
			g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 0, 255, 0, 255 }, ALIGN_CENTERHORIZONTAL, "Forcing Crits");
		}
	}
	if (NoRandomCrits(pWeapon) == true) //Can this weapon do random crits?
	{
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 255, 95, 95, 255 }, ALIGN_CENTERHORIZONTAL, L"No Random Crits");
	}
	if ((CritTicks.size() == 0 && NoRandomCrits(pWeapon) == false) || CritBanned(pEvent, uNameHash).critbanned == true) //Crit banned check
	{
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 255, 0, 0, 255 }, ALIGN_CENTERHORIZONTAL, L"Crit Banned");
	}
	static auto tf_weapon_criticals_bucket_cap = g_ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	const float bucketCap = tf_weapon_criticals_bucket_cap->GetFloat();
	const std::wstring bucketstr = L"Crit Bucket: " + std::to_wstring(static_cast<int>(bucket)) + L" / " + std::to_wstring(static_cast<int>(bucketCap));
	if (NoRandomCrits(pWeapon) == false)
	{
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 181, 181, 181, 255 }, ALIGN_CENTERHORIZONTAL, bucketstr.c_str());
	}
	int w, h;
	I::VGuiSurface->GetTextSize(g_Draw.m_vecFonts.at(FONT_INDICATORS).dwFont, bucketstr.c_str(), w, h);

	if (w > longestW)
	{
		longestW = w;
	}
	if (Vars::Debug::DebugInfo.Value)
	{
		const std::wstring seedText = L"m_nCritSeedRequests: " + std::to_wstring(seedRequests);
		const std::wstring FoundCrits = L"Found Crit Ticks: " + std::to_wstring(CritTicks.size());
		const std::wstring commandNumber = L"cmdNumber: " + std::to_wstring(G::CurrentUserCmd->command_number);
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 181, 181, 181, 255 }, ALIGN_CENTERHORIZONTAL, seedText.c_str());
		I::VGuiSurface->GetTextSize(g_Draw.m_vecFonts.at(FONT_INDICATORS).dwFont, seedText.c_str(), w, h);
		if (w > longestW)
		{
			longestW = w;
		}
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 181, 181, 181, 255 }, ALIGN_CENTERHORIZONTAL, FoundCrits.c_str());
		I::VGuiSurface->GetTextSize(g_Draw.m_vecFonts.at(FONT_INDICATORS).dwFont, FoundCrits.c_str(), w, h);
		if (w > longestW)
		{
			longestW = w;
		}
		g_Draw.String(FONT_INDICATORS, x, currentY += 15, { 181, 181, 181, 255 }, ALIGN_CENTERHORIZONTAL, commandNumber.c_str());
		I::VGuiSurface->GetTextSize(g_Draw.m_vecFonts.at(FONT_INDICATORS).dwFont, commandNumber.c_str(), w, h);
		if (w > longestW)
		{
			longestW = w;
		}
	}
	IndicatorW = longestW * 2;
	IndicatorH = currentY;
}