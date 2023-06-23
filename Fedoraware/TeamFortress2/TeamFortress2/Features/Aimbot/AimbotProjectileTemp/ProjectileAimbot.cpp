#include "ProjectileAimbot.h"

/// 1.0 Scan targets for most viable target.
/// 1.1 Ensure target is visible unless the user has specified that they want to predict non visible entities
/// 1.2 Scan along path until target is visible in a set amount of points (Possibly user specified) before we attempt to aim at them
/// 
/// 2.0 Once we have a verified target, gather required info.
/// 2.1 Gather info about the target & the weapon we are using (if not cached)
/// 
/// 3.0 Create a vector of points that we could use to hit the target
/// 3.1 Validate these points against the selected hitboxes that the user wishes to target
/// 3.2 Check visibility of points prior to seeing if we could hit them with a hull trace
/// 
/// 4.0 Scan all (necessary) available points in the vector until we find a point which we are able to hit also ensure the time matches up for the simtime & travel time
/// 4.1 If the weapon is effected by gravity, instead of a hull trace, arc prediction will be used to ensure we can hit the point.
/// 4.2 Adjust aim as required to hit the target
/// 
/// 5.0 Aim at point
/// 5.1 Fire

/// CProjectileAimbot::FillWeaponInfo
/// This will run ONLY if G::CurItemDefIndex is not the same across two different instances (very minor optimisation imo)
/// It will cache all required data for the aimbot to run
void CProjectileAimbot::FillWeaponInfo() {
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	const int& iCurWeapon = G::CurItemDefIndex;	//	this reference looks nicer than writing out CurItemDefIndex every time.
	iWeapon.flInitialVelocity = GetWeaponVelocity();
	iWeapon.vInitialLocation = iTarget.vAngles + GetFireOffset();
	iWeapon.vMaxs = GetWeaponBounds(); iWeapon.vMins = iWeapon.vMaxs * -1.f;
}

float CProjectileAimbot::GetWeaponVelocity() {
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	const bool bRune = pLocal->IsPrecisionRune();

	switch (iWeapon.pWeapon->GetWeaponID()) {
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
	{
		return (bRune ? 2.5f : 1.f) * Utils::ATTRIB_HOOK_FLOAT(1100, "mult_projectile_speed", iWeapon.pWeapon, 0, 1);
	}
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_CANNON:
	{
		return (bRune ? 2.5f : 1.f) * Utils::ATTRIB_HOOK_FLOAT(1200, "mult_projectile_speed", iWeapon.pWeapon, 0, 1);
	}
	case TF_WEAPON_FLAMETHROWER:
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		return 1000.f;
	}
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	case TF_WEAPON_CROSSBOW:
	{
		return 2400.f;
	}
	case TF_WEAPON_RAYGUN:
	{
		return 1200.f;
	}
	case TF_WEAPON_FLAREGUN:
	{
		return 2000.f;
	}
	case TF_WEAPON_FLAME_BALL:
	case TF_WEAPON_RAYGUN_REVENGE:
	case TF_WEAPON_CLEAVER:
	{
		return 3000.f;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		if (iWeapon.pWeapon->GetChargeBeginTime() == 0.f) { return 1800.f; }
		return Math::RemapValClamped(TICKS_TO_TIME(pLocal->m_nTickBase()) - iWeapon.pWeapon->GetChargeBeginTime(), 0.0f, 1.f, 1800.f, 2600.f);
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		if (iWeapon.pWeapon->GetChargeBeginTime() == 0.f) { return 900.f; }
		return Math::RemapValClamped(TICKS_TO_TIME(pLocal->m_nTickBase()) - iWeapon.pWeapon->GetChargeBeginTime(), 0.0f, G::CurItemDefIndex == Demoman_s_TheQuickiebombLauncher ? 1.2f : 4.f, 900.f, 2400.f);
	}
	}

	return 0.f;
}

Vec3 CProjectileAimbot::GetWeaponBounds()
{
	return Vec3();
}

Vec3 CProjectileAimbot::GetFireOffset() {
	Vec3 vOffset{ 23.5f, 12.0f, -3.0f };
	switch (iWeapon.pWeapon->GetWeaponID()) {
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		vOffset = { 23.5f, 8.0f, -3.0f };
		break;
	}
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_STICKBOMB:
	case TF_WEAPON_STICKY_BALL_LAUNCHER:
	{
		vOffset = { 16.0f, 8.0f, -6.0f };
		break;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC: {
		vOffset = { 16.0f, 6.0f, -8.0f };
		break;
	}
	}

	return vOffset;
}

Vec3 CProjectileAimbot::GetFirePos() {
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	Vec3 vFiringPos = pLocal->GetShootPos();
	const Vec3 vOffset = GetFireOffset();

	Utils::GetProjectileFireSetup(pLocal, iTarget.vAngles, vOffset, &vFiringPos);

	//if (GetInterpolatedStartPosOffset() > 0.f) {
	//	const Vec3 vTempOffset = { vFiringPos.x, vFiringPos.y, vFiringPos.z - GetInterpolatedStartPosOffset() };
	//	CGameTrace trace = {};
	//	CTraceFilterHitscan filter = {};
	//	filter.pSkip = g_EntityCache.GetLocal();
	//	Utils::Trace(vOffset, vTempOffset, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);
	//	vOffset = trace.vEndPos;
	//}
	return vFiringPos;
}

float CProjectileAimbot::GetInterpolatedStartPosOffset()
{
	return G::LerpTime * iWeapon.flInitialVelocity;
}

__inline bool CProjectileAimbot::DoesTargetNeedPrediction() {
	return iTargetInfo.pEntity->GetVelocity().Length() > 0;	//	buildings can't move so this shouldn't predict buildings :troller:
}

__inline bool CProjectileAimbot::BeginTargetPrediction() {
	iTargetInfo.vBackupAbsOrigin = iTargetInfo.pEntity->GetAbsOrigin();
	return F::MoveSim.Initialize(iTargetInfo.pEntity);
}

__inline void CProjectileAimbot::RunPrediction() {
	F::MoveSim.RunTick(iTargetInfo.iMoveData, iTargetInfo.vAbsOrigin);
	iTargetInfo.pEntity->SetAbsOrigin(iTargetInfo.vAbsOrigin);
	iTarget.vShootPos = iTargetInfo.vAbsOrigin + iTarget.vShootOffset;
}

__inline void CProjectileAimbot::EndTargetPrediction() {
	F::MoveSim.Restore();
	iTargetInfo.pEntity->SetAbsOrigin(iTargetInfo.vBackupAbsOrigin);
}

void CProjectileAimbot::PredictProjectileTick() {}
bool CProjectileAimbot::CanTargetTeammates() {}
bool CProjectileAimbot::TimeMatch() {}
void CProjectileAimbot::RequiredStickyCharge() {}
bool CProjectileAimbot::ShouldTargetThisTick() {}
bool CProjectileAimbot::IsEntityVisible(CBaseEntity* pEntity) {}
bool CProjectileAimbot::IsEntityVisibleFromPoint(CBaseEntity* pEntity, const Vec3 vPoint){}
bool CProjectileAimbot::IsPointVisible(const Vec3 vPoint){}
bool CProjectileAimbot::IsPointVisibleFromPoint(const Vec3 vPoint, const Vec3 vFromPoint){}
bool CProjectileAimbot::HullVisibility(const Vec3 vPoint, const Vec3 vFromPoint, const Vec3 vHullSize){}

bool CProjectileAimbot::WillPointHitBone(const Vec3 vPoint, const int nBone, CBaseEntity* pEntity) {
	if (BoneFromPoint(vPoint, pEntity) != nBone) { return false; }
	if (!IsPointVisibleFromPoint(vPoint, pEntity->GetBonePos(nBone))) { return false; }
	return true;
}

bool CProjectileAimbot::EstimateProjectileAngle() {
	const float flGravity = g_ConVars.sv_gravity->GetFloat() * iWeapon.flGravity;
	const Vec3 vDelta = iTarget.vShootPos - iWeapon.vInitialLocation;
	const float flHyp = sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y);
	const float flDist = vDelta.z;
	const float flVel = iWeapon.flInitialVelocity;

	if (!flGravity)
	{
		const Vec3 vAngleTo = Math::CalcAngle(iWeapon.vInitialLocation, iTarget.vShootPos);
		iTarget.vAngles.x = -DEG2RAD(vAngleTo.x);
		iTarget.vAngles.y = DEG2RAD(vAngleTo.y);
	}
	else
	{	//	arch
		const float flRoot = pow(flVel, 4) - flGravity * (flGravity * pow(flHyp, 2) + 2.f * flDist * pow(flVel, 2));
		if (flRoot < 0.f)
		{
			return false;
		}
		iTarget.vAngles.x = atan((pow(flVel, 2) - sqrt(flRoot)) / (flGravity * flHyp));
		iTarget.vAngles.y = atan2(vDelta.y, vDelta.x);
	}
	iTarget.flTime = flHyp / (cos(iTarget.vAngles.x) * flVel);

	return true;
}

int CProjectileAimbot::BoneFromPoint(const Vec3 vPoint, CBaseEntity* pEntity) {
	int iClosestBone = -1;
	float flDistance = FLT_MAX;
	for (int iBone = 0; iBone <= 14; iBone++) {
		const Vec3 vBonePos = pEntity->GetBonePos(iBone);
		const float flDelta = vPoint.DistTo(vBonePos);
		if (flDelta < flDistance) {
			iClosestBone = iBone;
			flDistance = flDelta;
		}
	}
	return iClosestBone;
}

int CProjectileAimbot::GetTargetIndex() {
	vTargetRecords.clear();
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	const Vec3 vAngles = pLocal->GetEyeAngles();
	const Vec3 vEyePos = pLocal->GetEyePosition();

	std::vector<CBaseEntity*> vNonPlayers;
	const std::vector<CBaseEntity*> group1 = g_EntityCache.GetGroup(EGroupType::BUILDINGS_ENEMIES);
	const std::vector<CBaseEntity*> group2 = g_EntityCache.GetGroup(EGroupType::WORLD_NPC);
	vNonPlayers.insert(vNonPlayers.end(), group1.begin(), group1.end());
	vNonPlayers.insert(vNonPlayers.end(), group2.begin(), group2.end());


	for (CBaseEntity* CTFPlayer : g_EntityCache.GetGroup(CanTargetTeammates() ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES)) {
		//	Lets do an FoV check first.
		float flFoVTo = -1;
		for (int iBone = 0; iBone < 15; iBone++) {
			const Vec3 vBonePos = CTFPlayer->GetHitboxPos(iBone);
			const Vec3 vAngTo = Math::CalcAngle(vEyePos, vBonePos);
			const float flFoVToTemp = Math::CalcFov(vAngles, vAngTo);
			if (flFoVToTemp < flFoVTo && flFoVToTemp < Vars::Aimbot::Projectile::AimFOV.Value) {
				flFoVTo = flFoVToTemp;
			}
		}
		if (flFoVTo < 0) { continue; }

		if ((CTFPlayer->GetTeamNum() == pLocal->GetTeamNum())) {
			if (CTFPlayer->GetHealth() >= CTFPlayer->GetMaxHealth())
			{ continue; }
		}
		else if (F::AimbotGlobal.ShouldIgnore(CTFPlayer)) {
			continue;
		}

		//vTargetRecords.push_back(TTargetRecord(flFoVTo, F::AimbotGlobal.GetPriority(CTFPlayer->GetIndex()), CTFPlayer));

	}

	for (CBaseEntity* pEntity : vNonPlayers) {
		const Vec3 vBonePos = pEntity->GetHitboxPos(0);
		const Vec3 vAngTo = Math::CalcAngle(vEyePos, vBonePos);
		const float flFoVTo = Math::CalcFov(vAngles, vAngTo);
		if (flFoVTo < 0) { continue; }

		vTargetRecords.push_back({ flFoVTo, 0, pEntity});
	}
}

//	Scans points and returns the most appropriate one.
Vec3 CProjectileAimbot::GetPoint(const int nHitbox)
{
	float flMult;
	if (G::WeaponCanHeadShot) { flMult = 0.99f; }
	else { flMult = 0.70; }

	const Vec3 vMaxs = iTargetInfo.vMaxs * flMult;
	const Vec3 vMins = iTargetInfo.vMins * flMult;
	const Vec3 vPos = iTargetInfo.vAbsOrigin;

	if (nHitbox >= 0) {
		return iTargetInfo.pEntity->GetBonePos(nHitbox);	//	there is no point in vischecking this as we only ever want to target this hitbox.
	}

	const std::vector<Vec3> vPoints{
		//	top 4 points
		{vMaxs.x, vMaxs.y, vMaxs.z},
		{ vMins.x, vMaxs.y, vMaxs.z },
		{ vMins.x, vMaxs.y, vMins.z },
		{ vMaxs.x, vMaxs.y, vMins.z },
			//	bottom 4 points		
		{ vMaxs.x, vMins.y, vMaxs.z },
		{ vMins.x, vMins.y, vMaxs.z },
		{ vMins.x, vMins.y, vMins.z },
		{ vMaxs.x, vMins.y, vMins.z },
	};

	const matrix3x4 vTransform = {
		{1.f, 0, 0, vPos.x},
		{0, 1.f, 0, vPos.y},
		{0, 0, 1.f, iTargetInfo.pEntity->GetVecVelocity().IsZero() ? iTargetInfo.vBackupAbsOrigin.z : vPos.z}
	};

	for (const Vec3& vPoint : vPoints) {
		Vec3 vTransformedPoint{};
		Math::VectorTransform(vPoint, vTransform, vTransformedPoint);
		if (!IsPointVisibleFromPoint(vTransformedPoint, iWeapon.vInitialLocation)) { continue; }
		if (!HullVisibility(vTransformedPoint, iWeapon.vInitialLocation, iWeapon.vMaxs)) { continue; }
		return vTransformedPoint;
	}

	//	we have failed to get a point, return the abs origin as visibility will be checked later
	return iTargetInfo.vAbsOrigin;
}

bool CProjectileAimbot::IsTargetVisible() {
	float flMult;
	if (G::WeaponCanHeadShot) { flMult = 0.99f; }
	else { flMult = 0.70; }

	const Vec3 vMaxs = iTargetInfo.vMaxs * flMult;
	const Vec3 vMins = iTargetInfo.vMins * flMult;
	const Vec3 vPos = iTargetInfo.vAbsOrigin;

	const std::vector<Vec3> vPoints{
		//	top 4 points
		{vMaxs.x, vMaxs.y, vMaxs.z},
		{vMins.x, vMaxs.y, vMaxs.z},
		{vMins.x, vMaxs.y, vMins.z},
		{vMaxs.x, vMaxs.y, vMins.z},
		//	bottom 4 points		
		{vMaxs.x, vMins.y, vMaxs.z},
		{vMins.x, vMins.y, vMaxs.z},
		{vMins.x, vMins.y, vMins.z},
		{vMaxs.x, vMins.y, vMins.z},
	};

	const matrix3x4 vTransform = {
		{1.f, 0, 0, vPos.x},
		{0, 1.f, 0, vPos.y},
		{0, 0, 1.f, iTargetInfo.pEntity->GetVecVelocity().IsZero() ? iTargetInfo.vBackupAbsOrigin.z : vPos.z}
	};

	for (uint16 iCurPoint = 0; iCurPoint < vPoints.size(); iCurPoint += 2) {
		Vec3 vTransformedPoint{};
		if (!IsPointVisibleFromPoint(vTransformedPoint, iWeapon.vInitialLocation)) { continue; }
		return true;
	}

	return false;
}

int CProjectileAimbot::GetHitbox()	//	only for bone aimbot!!!
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	const int iAimMode = Vars::Aimbot::Projectile::AimPosition.Value;
	if (pLocal->GetClassNum() == ETFClass::CLASS_SNIPER && iAimMode == 3) { return 0; }
	return -1;
}

bool CProjectileAimbot::ShouldBounce()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	return kBounce.Down() ? (pLocal->GetClassNum() == ETFClass::CLASS_SOLDIER || pLocal->GetClassNum() == ETFClass::CLASS_DEMOMAN) : false;
}

bool CProjectileAimbot::CanSeePoint()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	Utils::VisPosMask(pLocal, iTarget.pEntity, iWeapon.vInitialLocation, iTarget.vShootPos, MASK_SHOT_HULL);
}

//	will do a hull trace
bool CProjectileAimbot::BoxTraceEnd() 
{
	CGameTrace tTrace = {};
	static CTraceFilterWorldAndPropsOnly tTraceFilter = {};
	tTraceFilter.pSkip = iTarget.pEntity;
	Utils::TraceHull(iWeapon.vInitialLocation, iTarget.vShootPos, iWeapon.vMaxs, iWeapon.vMins, MASK_SHOT_HULL, &tTraceFilter, &tTrace);

	return !tTrace.DidHit() /*&& !tTrace.entity*/;
}

Vec3 CProjectileAimbot::GetHitboxOffset(const Vec3 vBase, const int nHitbox) {
	return GetPointOffset(vBase, iTarget.pEntity->GetHitboxPos(nHitbox));
}

Vec3 CProjectileAimbot::GetPointOffset(const Vec3 vBase, const Vec3 vPoint)
{
	return vPoint - vBase;
}

Vec3 CProjectileAimbot::GetClosestPoint(const Vec3 vCompare)
{
	Vec3 vPos{};
	iTarget.pEntity->GetCollision()->CalcNearestPoint(vCompare, &vPos);
	return vPos;
}
