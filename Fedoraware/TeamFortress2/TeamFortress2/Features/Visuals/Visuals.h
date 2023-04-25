#pragma once
#include "../../SDK/SDK.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = { 0,0,0 };
	Vec3 m_vEnd = { 0,0,0 };
	Color_t m_Color = { 0,0,0,0 };
	bool m_bDraw = false;
};

struct BulletTracer_t
{
	Vec3 m_vStartPos;
	Vec3 m_vEndPos;
	Color_t m_Color;
	float m_flTimeCreated;
};


class CVisuals
{
private:
	int m_nHudZoom = 0;
	int m_nHudMotd = 0;
	IMaterial* m_pMatDev;

public:
	bool RemoveScope(int nPanel);
	void FOV(CViewSetup* pView);
	void ThirdPerson(CViewSetup* pView);
	void ModulateWorld();
	void RestoreWorldModulation();
	void OverrideWorldTextures();
	void PickupTimers();
	void DrawHitboxMatrix(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time);

	void DrawOnScreenConditions(CBaseEntity* pLocal);
	void DrawOnScreenPing(CBaseEntity* pLocal);
	void SkyboxChanger();
	void BulletTrace(CBaseEntity* pEntity, Color_t color);
	void DrawAimbotFOV(CBaseEntity* pLocal);
	void DrawDebugInfo(CBaseEntity* pLocal);
	void DrawAntiAim(CBaseEntity* pLocal);
	void DrawTickbaseInfo(CBaseEntity* pLocal);
	void DrawMenuSnow();
	void DrawMovesimLine();
	void ManualNetwork(const StartSoundParams_t& params); // Credits: reestart
	void RenderLine(const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer);
	void DrawSightlines();
	void FillSightlines();
	void AddBulletTracer(const Vec3& vFrom, const Vec3& vTo, const Color_t& clr);
	void PruneBulletTracers();
	void DrawBulletTracers();
	void DrawProjectileTracer(CBaseEntity* pLocal, const Vec3& position);
	void DrawServerHitboxes();

	std::vector<BulletTracer_t> m_vecBulletTracers;

	float arrowUp = 0.f;
	float arrowRight = 0.f;

	std::array<Sightline_t, 64> m_SightLines;

	struct PickupData
	{
		int Type = 0;
		float Time = 0.f;
		Vec3 Location;
	};
	std::vector<PickupData> PickupDatas;

	struct MaterialHandleData
	{
		enum class EMatGroupType
		{
			GROUP_OTHER,
			GROUP_WORLD,
			GROUP_SKY
		};

		MaterialHandle_t Handle;
		IMaterial* Material;
		std::string_view Group;
		std::string_view Name;
		EMatGroupType	 GroupType;
		bool			 ShouldOverrideTextures;
	};
	std::vector<MaterialHandleData> MaterialHandleDatas;

	void StoreMaterialHandles();
	void ClearMaterialHandles();
};

ADD_FEATURE(CVisuals, Visuals)