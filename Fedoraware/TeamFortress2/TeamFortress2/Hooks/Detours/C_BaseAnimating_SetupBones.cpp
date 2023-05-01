#include "../Hooks.h"
#include "../HookManager.h"



MAKE_HOOK(C_BaseAnimating_SetupBones, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 33 C9 33 D2 89 4D EC 89 55 F0 8B 46 08 85 C0 74 3B"), bool, __fastcall,
	void* ecx, void* edx, matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	const auto C_BaseAnimating_InvalidateBoneCaches = g_HookManager.GetMapHooks()["C_BaseAnimating_InvalidateBoneCaches"];
	if (C_BaseAnimating_InvalidateBoneCaches){
	C_BaseAnimating_InvalidateBoneCaches->Original<void(__cdecl*)()>()();
	}
	return Hook.Original<FN>()(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}
