#pragma once
#include "../../SDK/SDK.h"

#ifdef DEBUG
#include <iostream>
#include <fstream>
#endif

class CMisc
{
	void AccurateMovement(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoJump(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoStrafe(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd);
	void LegJitter(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal);
	void StopMovement(CUserCmd* pCmd, bool* pSendPacket);
	void FastDeltaMove(CUserCmd* pCmd, bool* pSendPacket);

	void AntiAFK(CUserCmd* pCmd);
	void CheatsBypass();
	void PingReducer();
	void WeaponSway();
	void DetectChoke();
	void ChokeCheck(bool* pSendPacket);
	void DoubletapPacket(bool* pSendPacket);

	void PrintProjAngles(CBaseEntity* pLocal);

	// Late
	void FastStop(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastAccel(CUserCmd* pCmd, CBaseEntity* pLocal, bool* pSendPacket);

	bool SteamCleared = false;
	bool bMovementScuffed = false;
public:
#ifdef DEBUG
	void DumpClassIDS();
#endif

	bool TauntControl(CUserCmd* pCmd);
	void RunPre(CUserCmd* pCmd, bool* pSendPacket);
	void RunPost(CUserCmd* pCmd, bool* pSendPacket);

	void SteamRPC();
	void UnlockAchievements();

	bool bAntiWarp = false;
	bool bFastAccel = false;
	bool bMovementStopped = false;

	Vec3 PeekReturnPos;
};

ADD_FEATURE(CMisc, Misc)
