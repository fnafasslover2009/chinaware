#pragma once
#include "../AutoGlobal/AutoGlobal.h"

enum EDetTargets
{
	PLAYER = 1 << 0,
	SENTRY = 1 << 1,
	DISPENSER = 1 << 2,
	TELEPORTER = 1 << 3,
	NPC = 1 << 4,
	BOMB = 1 << 5,
	STICKY = 1 << 6
};

class CAutoDetonate
{
	bool CheckDetonation(CBaseEntity* pLocal, const std::vector<CBaseEntity*>& entityGroup, float radius, CUserCmd* pCmd);

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoDetonate, AutoDetonate)