#include "../Hooks.h"

#include "../../Features/Glow/Glow.h"
#include "../../Features/Chams/Chams.h"
#include "../../Features/Chams/DMEChams.h"

#include <mutex>

MAKE_HOOK(ViewRender_RenderView, Utils::GetVFuncPtr(I::ViewRender, 6), void, __fastcall,
		  void* ecx, void* edx, const CViewSetup& view, ClearFlags_t nClearFlags, RenderViewInfo_t whatToDraw)
{
	static std::once_flag onceFlag;
	std::call_once(onceFlag, []
				   {
					   F::Glow.Init();
					   F::Chams.Init();
					   F::DMEChams.Init();
				   });

	Hook.Original<void(__thiscall*)(void*, const CViewSetup&, int, int)>()(ecx, view, nClearFlags, whatToDraw);
}