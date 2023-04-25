#include "LocalConditions.h"

using ETFCond = int;

template <typename tIntType>
class CConditionVariables
{
public:
	CConditionVariables(tIntType& nPlayerCond, tIntType& nPlayerCondEx, tIntType& nPlayerCondEx2, tIntType& nPlayerCondEx3, ETFCond eCond)
	{
		if (eCond >= 96)
		{
			if (eCond < 96 + 32)
			{
				m_pnCondVar = &nPlayerCondEx3;
				m_nCondBit = eCond - 96;
			}
		}
		else if (eCond >= 64)
		{
			if (eCond < (64 + 32))
			{
				m_pnCondVar = &nPlayerCondEx2;
				m_nCondBit = eCond - 64;
			}
		}
		else if (eCond >= 32)
		{
			if (eCond < (32 + 32))
			{
				m_pnCondVar = &nPlayerCondEx;
				m_nCondBit = eCond - 32;
			}
		}
		else
		{
			m_pnCondVar = &nPlayerCond;
			m_nCondBit = eCond;
		}
	}

	tIntType& CondVar() const
	{
		return *m_pnCondVar;
	}

	int CondBit() const
	{
		return 1 << m_nCondBit;
	}

private:
	tIntType* m_pnCondVar;
	int m_nCondBit;
};

class CTFCondition;


class CTFConditionsList
{
public:
	CTFConditionsList();

	bool InCondition(ETFCond type) const;

	CUtlVector<CTFCondition*> _conditions;

	int _condition_bits;
	int _old_condition_bits;
};

bool CTFConditionsList::InCondition(ETFCond type) const
{
	return ((_condition_bits & (1 << type)) != 0);
}

class CTFConditions
{
public:
	CTFConditions(ETFCond type, float duration, CBaseEntity* outer, CBaseEntity* provider = nullptr);
	virtual ~CTFConditions();

	virtual void Add(float duration);

	virtual void OnAdded() = 0;
	virtual void OnRemoved() = 0;
	virtual void OnThink() = 0;
	virtual void OnServerThink() = 0;

	// Condition Traits
	virtual bool IsHealable() { return false; }
	virtual bool UsesMinDuration() { return false; }

	ETFCond GetType() { return _type; }
	float GetMaxDuration() { return _max_duration; }
	void SetMaxDuration(float val) { _max_duration = val; }
	float GetMinDuration() { return _min_duration; }
	void SetMinDuration(float val) { if (UsesMinDuration()) { _min_duration = val; } }
	CBaseEntity* GetOuter() { return _outer; }
	void SetProvider(CBaseEntity* provider) { _provider = provider; }
	CBaseEntity* GetProvider() { return _provider; }

private:
	float _min_duration;
	float _max_duration;
	const ETFCond _type;
	CBaseEntity* _outer;
	CBaseEntity* _provider;
};

bool InCondition(CBaseEntity* pEntity, int eCond)
{
	if (eCond >= 0 && eCond < 122)
	{
		void* condList = pEntity->m_ConditionList();
		const auto& conditionList = *reinterpret_cast<CTFConditionsList*>(condList);

		if (conditionList._conditions.Count() > 0)
		{
			// Old condition system, only used for the first 32 conditions
			if (eCond < 32 && conditionList.InCondition(eCond))
			{
				return true;
			}

			CConditionVariables<const int> cPlayerCond(pEntity->GetCond(), pEntity->GetCondEx(), pEntity->GetCondEx2(), pEntity->GetCondEx3(), eCond);
			return (cPlayerCond.CondVar() & cPlayerCond.CondBit()) != 0;
		}
	}
	return false;
}

std::vector<std::wstring> LCESP::GetPlayerConditions(CBaseEntity* pEntity) const
{
	std::vector<std::wstring> szCond{};
	const int nCond = pEntity->GetCond();
	const int nCondEx = pEntity->GetCondEx();
	const int nCondEx2 = pEntity->GetCondEx2();
	const int nFlag = pEntity->GetFlags();
	if (Vars::Visuals::DrawOnScreenConditions.Value)
	{ 
		if (nCond & TFCond_Slowed)
		{
			szCond.emplace_back(L"AIMING");
		}

		if (nCond & TFCond_Zoomed)
		{
			szCond.emplace_back(L"ZOOMED");
		}

		if (nCond & TFCond_Disguising)
		{
			szCond.emplace_back(L"DISGUISING");
		}

		if (nCond & TFCond_Disguised)
		{
			szCond.emplace_back(L"DISGUISED");
		}

		if (nCond & TFCond_Cloaked)
		{
			szCond.emplace_back(L"STEALTHED");
		}

		if (nCond & TFCond_Ubercharged)
		{
			szCond.emplace_back(L"INVULNERABLE");
		}

		if (nCond & TFCond_TeleportedGlow)
		{
			szCond.emplace_back(L"TELEPORTED");
		}

		if (nCond & TFCond_Taunting)
		{
			szCond.emplace_back(L"TAUNTING");
		}

		if (nCond & TFCond_UberchargeFading)
		{
			szCond.emplace_back(L"INVULNERABLE WEARINGOFF");
		}

		if (nCond & TFCond_CloakFlicker)
		{
			szCond.emplace_back(L"STEALTHED BLINK");
		}

		if (nCond & TFCond_Teleporting)
		{
			szCond.emplace_back(L"SELECTED TO TELEPORT");
		}

		if (nCond & TFCond_Kritzkrieged)
		{
			szCond.emplace_back(L"CRITBOOSTED");
		}

		if (nCond & TFCond_TmpDamageBonus)
		{
			szCond.emplace_back(L"TMPDAMAGEBONUS");
		}

		if (nCond & TFCond_DeadRingered)
		{
			szCond.emplace_back(L"FEIGN DEATH");
		}

		if (nCond & TFCond_Bonked)
		{
			szCond.emplace_back(L"PHASE");
		}

		if (nCond & TFCond_Stunned)
		{
			szCond.emplace_back(L"STUNNED");
		}

		if (nCond & TFCond_Buffed)
		{
			szCond.emplace_back(L"OFFENSEBUFF");
		}

		if (nCond & TFCond_Charging)
		{
			szCond.emplace_back(L"SHIELD CHARGE");
		}

		if (nCond & TFCond_DemoBuff)
		{
			szCond.emplace_back(L"DEMO BUFF");
		}

		if (nCond & TFCond_CritCola)
		{
			szCond.emplace_back(L"ENERGY BUFF");
		}

		if (nCond & TFCond_InHealRadius)
		{
			szCond.emplace_back(L"RADIUSHEAL");
		}

		if (nCond & TFCond_Healing)
		{
			szCond.emplace_back(L"HEALTH BUFF");
		}

		if (nCond & TFCond_OnFire)
		{
			szCond.emplace_back(L"BURNING");
		}

		if (nCond & TFCond_Overhealed)
		{
			szCond.emplace_back(L"HEALTH OVERHEALED");
		}

		if (nCond & TFCond_Jarated)
		{
			szCond.emplace_back(L"URINE");
		}

		if (nCond & TFCond_Bleeding)
		{
			szCond.emplace_back(L"BLEEDING");
		}

		if (nCond & TFCond_DefenseBuffed)
		{
			szCond.emplace_back(L"DEFENSEBUFF");
		}

		if (nCond & TFCond_Milked)
		{
			szCond.emplace_back(L"MAD MILK");
		}

		if (nCond & TFCond_MegaHeal)
		{
			szCond.emplace_back(L"MEGAHEAL");
		}

		if (nCond & TFCond_RegenBuffed)
		{
			szCond.emplace_back(L"REGENONDAMAGEBUFF");
		}

		if (nCond & TFCond_MarkedForDeath)
		{
			szCond.emplace_back(L"MARKED FOR DEATH");
		}

		if (nCond & TFCond_NoHealingDamageBuff)
		{
			szCond.emplace_back(L"NOHEALINGDAMAGEBUFF");
		}

		if (nCondEx & TFCondEx_SpeedBuffAlly)
		{
			szCond.emplace_back(L"SPEED BOOST");
		}

		if (nCondEx & TFCondEx_HalloweenCritCandy)
		{
			szCond.emplace_back(L"CRITBOOSTED PUMPKIN");
		}

		if (nCondEx & TFCondEx_CritCanteen)
		{
			szCond.emplace_back(L"CRITBOOSTED USER BUFF");
		}

		if (nCondEx & TFCondEx_CritDemoCharge)
		{
			szCond.emplace_back(L"CRITBOOSTED DEMO CHARGE");
		}

		if (nCondEx & TFCondEx_CritHype)
		{
			szCond.emplace_back(L"SODAPOPPER HYPE");
		}

		if (nCondEx & TFCondEx_CritOnFirstBlood)
		{
			szCond.emplace_back(L"CRITBOOSTED FIRST BLOOD");
		}

		if (nCondEx & TFCondEx_CritOnWin)
		{
			szCond.emplace_back(L"CRITBOOSTED BONUS TIME");
		}

		if (nCondEx & TFCondEx_CritOnKill)
		{
			szCond.emplace_back(L"CRITBOOSTED ON KILL");
		}

		if (nCondEx & TFCondEx_RestrictToMelee)
		{
			szCond.emplace_back(L"CANNOT SWITCH FROM MELEE");
		}

		if (nCondEx & TFCondEx_DefenseBuffNoCritBlock)
		{
			szCond.emplace_back(L"DEFENSEBUFF NO CRIT BLOCK");
		}

		if (nCondEx & TFCondEx_PyroCrits)
		{
			szCond.emplace_back(L"CRITBOOSTED RAGE BUFF");
		}

		if (nCondEx & TFCondEx_FocusBuff)
		{
			szCond.emplace_back(L"SNIPERCHARGE RAGE BUFF");
		}

		if (nCondEx & TFCondEx_DisguisedRemoved)
		{
			szCond.emplace_back(L"DISGUISE WEARINGOFF");
		}

		if (nCondEx & TFCondEx_MarkedForDeathSilent)
		{
			szCond.emplace_back(L"MARKEDFORDEATH SILENT");
		}

		if (nCondEx & TFCondEx_DisguisedAsDispenser)
		{
			szCond.emplace_back(L"DISGUISED AS DISPENSER");
		}

		if (nCondEx & TFCondEx_UberchargedHidden)
		{
			szCond.emplace_back(L"INVULNERABLE HIDE UNLESS DAMAGED");
		}

		if (nCondEx & TFCondEx_UberchargedCanteen)
		{
			szCond.emplace_back(L"INVULNERABLE USER BUFF");
		}

		if (nCondEx & TFCondEx_HalloweenBombHead)
		{
			szCond.emplace_back(L"HALLOWEEN BOMB HEAD");
		}

		if (nCondEx & TFCondEx_HalloweenThriller)
		{
			szCond.emplace_back(L"HALLOWEEN THRILLER");
		}

	/*	if (nCondEx & TFCondEx_RadiusHealOnDamage)
		{
			szCond.emplace_back(L"RADIUSHEAL ON DAMAGE");
		}

		if (nCondEx & TFCondEx_CritBoostedCardEffect)
		{
			szCond.emplace_back(L"CRITBOOSTEDCARDEFFECT");
		}
 
		if (nCondEx & TFCondEx_InvulnerableCardEffect)
		{
			szCond.emplace_back(L"INVULNERABLE CARD EFFECT");
		}
	*/
		if (nCondEx & TFCondEx_BulletCharge)
		{
			szCond.emplace_back(L"MEDIGUN UBER BULLET RESIST");
		}

		if (nCondEx & TFCondEx_ExplosiveCharge)
		{
			szCond.emplace_back(L"MEDIGUN UBER BLAST RESIST");
		}

		if (nCondEx & TFCondEx_FireCharge)
		{
			szCond.emplace_back(L"MEDIGUN UBER FIRE RESIST");
		}

		if (nCondEx & TFCondEx_BulletResistance)
		{
			szCond.emplace_back(L"MEDIGUN SMALL BULLET RESIST");
		}

		if (nCondEx & TFCondEx_ExplosiveResistance)
		{
			szCond.emplace_back(L"MEDIGUN SMALL BLAST RESIST");
		}

		if (nCondEx & TFCondEx_FireResistance)
		{
			szCond.emplace_back(L"MEDIGUN SMALL FIRE RESIST");
		}

	//	if (nCondEx2 & TFCondEx2_Stealthed)
	//	{
	//		szCond.emplace_back(L"STEALTHED USER BUFF");
	//	}

		if (nCondEx2 & TFCondEx2_MedigunDebuff)
		{
			szCond.emplace_back(L"MEDIGUN DEBUFF");
		}

	//	if (nCondEx2 & TFCondEx2_StealthedUserBuffFade)
	//	{
	//		szCond.emplace_back(L"STEALTHED USER BUFF FADING");
	//	}

		if (nCondEx2 & TFCondEx2_BulletImmune)
		{
			szCond.emplace_back(L"BULLET IMMUNE");
		}

		if (nCondEx2 & TFCondEx2_BlastImmune)
		{
			szCond.emplace_back(L"BLAST IMMUNE");
		}

		if (nCondEx2 & TFCondEx2_FireImmune)
		{
			szCond.emplace_back(L"FIRE IMMUNE");
		}

	//	if (nCondEx2 & TFCondEx2_PreventDeath)
	//	{
	//		szCond.emplace_back(L"PREVENT DEATH");
	//	}

	//	if (nCondEx2 & TFCondEx2_MVMBotRadiowave)
	//	{
	//		szCond.emplace_back(L"MVM BOT STUN RADIOWAVE");
	//	}

		if (nCondEx2 & TFCondEx2_HalloweenSpeedBoost)
		{
			szCond.emplace_back(L"HALLOWEEN SPEED BOOST");
		}

		if (nCondEx2 & TFCondEx2_HalloweenQuickHeal)
		{
			szCond.emplace_back(L"HALLOWEEN QUICK HEAL");
		}

		if (nCondEx2 & TFCondEx2_HalloweenGiant)
		{
			szCond.emplace_back(L"HALLOWEEN GIANT");
		}

		if (nCondEx2 & TFCondEx2_HalloweenTiny)
		{
			szCond.emplace_back(L"HALLOWEEN TINY");
		}

		if (nCondEx2 & TFCondEx2_HalloweenInHell)
		{
			szCond.emplace_back(L"HALLOWEEN IN HELL");
		}

		if (nCondEx2 & TFCondEx2_HalloweenGhostMode)
		{
			szCond.emplace_back(L"HALLOWEEN GHOST MODE");
		}

	/*	if (nCondEx2 & TFCondEx2_MiniCritOnKill)
		{
			szCond.emplace_back(L"MINICRITBOOSTED ON KILL");
		}

		if (nCondEx2 & TFCondEx2_ObscuredSmoke)
		{
			szCond.emplace_back(L"OBSCURED SMOKE");
		}
	*/
		if (nCondEx2 & TFCondEx2_BlastJumping)
		{
			szCond.emplace_back(L"BLASTJUMPING");
		}

	/*	if (nCondEx2 & TFCondEx2_HalloweenKart)
		{
			szCond.emplace_back(L"HALLOWEEN KART");
		}

		if (nCondEx2 & TFCondEx2_HalloweenKartDash)
		{
			szCond.emplace_back(L"HALLOWEEN KART DASH");
		}

		if (nCondEx2 & TFCondEx2_BalloonHead)
		{
			szCond.emplace_back(L"BALLOON HEAD");
		}

		if (nCondEx2 & TFCondEx2_MeleeOnly)
		{
			szCond.emplace_back(L"MELEE ONLY");
		}

		if (nCondEx2 & TFCondEx2_SwimmingCurse)
		{
			szCond.emplace_back(L"SWIMMING CURSE");
		}

		if (nCondEx2 & TFCondEx2_FreezeInput)
		{
			szCond.emplace_back(L"FREEZE INPUT");
		}

		if (nCondEx2 & TFCondEx2_HalloweenKartCage)
		{
			szCond.emplace_back(L"HALLOWEEN KART CAGE");
		}

		if (nCondEx2 & TFCondEx2_DoNotUse0)
		{
			szCond.emplace_back(L"DONOTUSE 0");
		}
	*/

	/*	if (nCondEx3 & TFCondEx3_GrapplingHook)
		{
			szCond.emplace_back(L"GRAPPLINGHOOK");
		}

		if (nCondEx3 & TFCondEx3_GrapplingHookSafeFall)
		{
			szCond.emplace_back(L"GRAPPLINGHOOK SAFEFALL");
		}

		if (nCondEx3 & TFCondEx3_GrapplingHookLatched)
		{
			szCond.emplace_back(L"GRAPPLINGHOOK LATCHED");
		}

		if (nCondEx3 & TFCondEx3_GrapplingHookBleeding)
		{
			szCond.emplace_back(L"GRAPPLINGHOOK BLEEDING");
		}

		if (nCondEx3 & TFCondEx3_AfterburnImmune)
		{
			szCond.emplace_back(L"AFTERBURN IMMUNE");
		}

		if (nCondEx3 & TFCondEx3_CritboostedTempRune)
		{
			szCond.emplace_back(L"CRITBOOSTED RUNE TEMP");
		}

		if (nCondEx3 & TFCondEx3_PasstimeInterception)
		{
			szCond.emplace_back(L"PASSTIME INTERCEPTION");
		}

		if (nCondEx3 & TFCondEx3_SwimmingNoEffects)
		{
			szCond.emplace_back(L"SWIMMING NO EFFECTS");
		}

		if (nCondEx3 & TFCondEx3_Purgatory)
		{
			szCond.emplace_back(L"PURGATORY");
		}

		if (nCondEx3 & TFCondEx3_TeamGlows)
		{
			szCond.emplace_back(L"TEAM GLOWS");
		}

		if (nCondEx3 & TFCondEx3_KnockedIntoAir)
		{
			szCond.emplace_back(L"KNOCKED INTO AIR");
		}

		if (nCondEx3 & TFCondEx3_CompetitiveWinner)
		{
			szCond.emplace_back(L"COMPETITIVE WINNER");
		}

		if (nCondEx3 & TFCondEx3_CompetitiveLoser)
		{
			szCond.emplace_back(L"COMPETITIVE LOSER");
		}

		if (nCondEx3 & TFCondEx3_HealingDebuff)
		{
			szCond.emplace_back(L"HEALING DEBUFF");
		}

		if (nCondEx3 & TFCondEx3_PasstimePenaltyDebuff)
		{
			szCond.emplace_back(L"PASSTIME PENALTY DEBUFF");
		}

		if (nCondEx3 & TFCondEx3_GrappledToPlayer)
		{
			szCond.emplace_back(L"GRAPPLED TO PLAYER");
		}

		if (nCondEx3 & TFCondEx3_GrappledByPlayer)
		{
			szCond.emplace_back(L"GRAPPLED BY PLAYER");
		}

		if (nCondEx3 & TFCondEx3_ParachuteDeployed)
		{
			szCond.emplace_back(L"PARACHUTE DEPLOYED");
		}

		if (nCondEx3 & TFCondEx3_Gas)
		{
			szCond.emplace_back(L"GAS");
		}

		if (nCondEx3 & TFCondEx3_BurningPyro)
		{
			szCond.emplace_back(L"BURNING PYRO");
		}

		if (nCondEx3 & TFCondEx3_RocketPack)
		{
			szCond.emplace_back(L"ROCKETPACK");
		}

		if (nCondEx3 & TFCondEx3_LostFooting)
		{
			szCond.emplace_back(L"LOST FOOTING");
		}

		if (nCondEx3 & TFCondEx3_AirCurrent)
		{
			szCond.emplace_back(L"AIR CURRENT");
		}
	*/
	}
	
	return szCond;
}