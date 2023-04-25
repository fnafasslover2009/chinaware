#include "AutoDetonate.h"

#include "../../Vars.h"

//credits to KGB
class CEntitySphereQuery
{
public:
	CEntitySphereQuery(const Vec3& center, const float radius, const int flagMask = 0,
					   const int partitionMask = PARTITION_CLIENT_NON_STATIC_EDICTS)
	{
		static DWORD dwAddress = g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC 14 D9 45 0C");
		reinterpret_cast<void(__thiscall*)(void*, const Vec3&, float, int, int)>(dwAddress)(
			this, center, radius, flagMask, partitionMask);
	}

	CBaseEntity* GetCurrentEntity()
	{
		return (m_nListIndex < m_nListCount) ? m_pList[m_nListIndex] : nullptr;
	}

	void NextEntity()
	{
		m_nListIndex++;
	}

private:
	int m_nListIndex, m_nListCount;
	CBaseEntity* m_pList[MAX_SPHERE_QUERY];
};

bool CAutoDetonate::CheckDetonation(CBaseEntity* pLocal, const std::vector<CBaseEntity*>& entityGroup, float radius, CUserCmd* pCmd)
{
	for (const auto& pExplosive : entityGroup)
	{
		if (pExplosive->GetPipebombType() == TYPE_STICKY)
		{
			if (!pExplosive->GetPipebombPulsed()) { continue; }
		}

		CBaseEntity* pTarget;

		// Iterate through entities in sphere radius
		for (CEntitySphereQuery sphere(pExplosive->GetWorldSpaceCenter(), radius);
			 (pTarget = sphere.GetCurrentEntity()) != nullptr;
			 sphere.NextEntity())
		{
			if (!pTarget || pTarget == pLocal || !pTarget->IsAlive() || pTarget->GetTeamNum() == pLocal->
				GetTeamNum())
			{
				continue;
			}

			const Vec3 vOrigin = pExplosive->GetWorldSpaceCenter();
			Vec3 vPos{};
			pTarget->GetCollision()->CalcNearestPoint(vOrigin, &vPos);
			if ((vOrigin - vPos).Length() > radius) 
			{ 
				continue; 
			}

			const bool isPlayer = Vars::Triggerbot::Detonate::DetonateTargets.Value & (PLAYER) && pTarget->IsPlayer();
			const bool isSentry = Vars::Triggerbot::Detonate::DetonateTargets.Value & (SENTRY) && pTarget->GetClassID() == ETFClassID::CObjectSentrygun;
			const bool isDispenser = Vars::Triggerbot::Detonate::DetonateTargets.Value & (DISPENSER) && pTarget->GetClassID() == ETFClassID::CObjectDispenser;
			const bool isTeleporter = Vars::Triggerbot::Detonate::DetonateTargets.Value & (TELEPORTER) && pTarget->GetClassID() == ETFClassID::CObjectTeleporter;
			const bool isNPC = Vars::Triggerbot::Detonate::DetonateTargets.Value & (NPC) && pTarget->IsNPC();
			const bool isBomb = Vars::Triggerbot::Detonate::DetonateTargets.Value & (BOMB) && pTarget->IsBomb();
			const bool isSticky = Vars::Triggerbot::Detonate::DetonateTargets.Value & (STICKY) && (G::CurItemDefIndex == Demoman_s_TheQuickiebombLauncher || G::CurItemDefIndex == Demoman_s_TheScottishResistance);


			if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || pTarget->GetPipebombType() == TYPE_STICKY && isSticky)
			{
				if (isPlayer && F::AutoGlobal.ShouldIgnore(pTarget))
				{
					continue;
				}

				CGameTrace trace = {};
				CTraceFilterWorldAndPropsOnly traceFilter = {};
				Utils::Trace(pExplosive->GetWorldSpaceCenter(), pTarget->GetWorldSpaceCenter(), MASK_SOLID, &traceFilter, &trace);

				if (trace.flFraction >= 0.99f || trace.entity == pTarget)
				{
					if (G::CurItemDefIndex == Demoman_s_TheScottishResistance)
					{	//	super fucking ghetto holy shit
						Vec3 vAngleTo = Math::CalcAngle(pLocal->GetWorldSpaceCenter(), pExplosive->GetWorldSpaceCenter());
						Utils::FixMovement(pCmd, vAngleTo);
						pCmd->viewangles = vAngleTo;
						G::SilentTime = true;
					}
					return true;
				}
			}
		}
	}

	return false;
}

void CAutoDetonate::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Triggerbot::Detonate::Active.Value) { return; }

	float flStickyRadius = Utils::ATTRIB_HOOK_FLOAT(148.0f, "mult_explosion_radius", pWeapon);
	for (const auto& pExplosive : g_EntityCache.GetGroup(EGroupType::LOCAL_STICKIES))
	{
		float flRadiusMod = 1.0f;
		if (pExplosive->GetPipebombType() == TYPE_STICKY)
		{
			if (pExplosive->GetTouched() == false)
			{
				float flArmTime = I::Cvar->FindVar("tf_grenadelauncher_livetime")->GetFloat(); //0.8
				float RampTime = I::Cvar->FindVar("tf_sticky_radius_ramp_time")->GetFloat(); //2.0
				float AirDetRadius = I::Cvar->FindVar("tf_sticky_airdet_radius")->GetFloat(); //0.85
				flRadiusMod *= Math::RemapValClamped(I::GlobalVars->curtime - pExplosive->GetCreationTime(), flArmTime, flArmTime + RampTime, AirDetRadius, 1.0f);
			}
			flStickyRadius *= flRadiusMod;
		}
	}

	bool shouldDetonate = false;

	// Check sticky detonation
	if (Vars::Triggerbot::Detonate::Stickies.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_STICKIES), flStickyRadius * Vars::Triggerbot::Detonate::RadiusScale.Value, pCmd))
	{
		shouldDetonate = true;
	}

	// Check flare detonation
	if (Vars::Triggerbot::Detonate::Flares.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_FLARES), 110.0f * Vars::Triggerbot::Detonate::RadiusScale.Value, pCmd))
	{
		shouldDetonate = true;
	}

	if (!shouldDetonate) { return; }
	pCmd->buttons |= IN_ATTACK2;
}
