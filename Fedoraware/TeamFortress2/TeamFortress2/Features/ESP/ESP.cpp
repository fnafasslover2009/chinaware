#include "ESP.h"
#include "../AntiHack/CheaterDetection/CheaterDetection.h"
#include "../Backtrack/Backtrack.h"
#include "GetWeaponName/GetWeaponName.h"
#include "../Vars.h"

bool CESP::ShouldRun()
{
	if (!(I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value) && I::EngineVGui->IsGameUIVisible())
	{
		return false;
	}

	return true;
}

void CESP::Run()
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (ShouldRun())
		{
			DrawWorld();
			DrawBuildings(pLocal);
			DrawPlayers(pLocal);
		}
	}
}

bool CESP::GetDrawBounds(CBaseEntity* pEntity, Vec3* vTrans, int& x, int& y, int& w, int& h)
{
	bool bIsPlayer = false;
	Vec3 vMins, vMaxs;

	if (pEntity->IsPlayer())
	{
		bIsPlayer = true;
		const bool bIsDucking = pEntity->IsDucking();
		vMins = I::GameMovement->GetPlayerMins(bIsDucking);
		vMaxs = I::GameMovement->GetPlayerMaxs(bIsDucking);
	}

	else
	{
		vMins = pEntity->GetCollideableMins();
		vMaxs = pEntity->GetCollideableMaxs();
	}

	const matrix3x4& transform = pEntity->GetRgflCoordinateFrame();

	const Vec3 vPoints[] =
	{
		Vec3(vMins.x, vMins.y, vMins.z),
		Vec3(vMins.x, vMaxs.y, vMins.z),
		Vec3(vMaxs.x, vMaxs.y, vMins.z),
		Vec3(vMaxs.x, vMins.y, vMins.z),
		Vec3(vMaxs.x, vMaxs.y, vMaxs.z),
		Vec3(vMins.x, vMaxs.y, vMaxs.z),
		Vec3(vMins.x, vMins.y, vMaxs.z),
		Vec3(vMaxs.x, vMins.y, vMaxs.z)
	};

	for (int n = 0; n < 8; n++)
	{
		Math::VectorTransform(vPoints[n], transform, vTrans[n]);
	}

	Vec3 flb, brt, blb, frt, frb, brb, blt, flt;

	if (Utils::W2S(vTrans[3], flb) && Utils::W2S(vTrans[5], brt)
		&& Utils::W2S(vTrans[0], blb) && Utils::W2S(vTrans[4], frt)
		&& Utils::W2S(vTrans[2], frb) && Utils::W2S(vTrans[1], brb)
		&& Utils::W2S(vTrans[6], blt) && Utils::W2S(vTrans[7], flt))
	{
		const Vec3 arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

		float left = flb.x;
		float top = flb.y;
		float righ = flb.x;
		float bottom = flb.y;

		for (int n = 1; n < 8; n++)
		{
			if (left > arr[n].x)
			{
				left = arr[n].x;
			}

			if (top < arr[n].y)
			{
				top = arr[n].y;
			}

			if (righ < arr[n].x)
			{
				righ = arr[n].x;
			}

			if (bottom > arr[n].y)
			{
				bottom = arr[n].y;
			}
		}

		float x_ = left;
		const float y_ = bottom;
		float w_ = righ - left;
		const float h_ = top - bottom;

		if (bIsPlayer && Vars::ESP::Players::Box.Value != 3)
		{
			x_ += (righ - left) / 8.0f;
			w_ -= (righ - left) / 8.0f * 2.0f;
		}

		x = static_cast<int>(x_);
		y = static_cast<int>(y_);
		w = static_cast<int>(w_);
		h = static_cast<int>(h_);

		return !(x > g_ScreenSize.w || x + w < 0 || y > g_ScreenSize.h || y + h < 0);
	}

	return false;
}

void CESP::DrawPlayers(CBaseEntity* pLocal)
{
	if (!Vars::ESP::Players::Active.Value || !Vars::ESP::Main::Active.Value)
	{
		return;
	}

	CTFPlayerResource* cResource = g_EntityCache.GetPR();
	if (!cResource) { return; }

	for (const auto& Player : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (!Player->IsAlive() || Player->IsAGhost())
		{
			continue;
		}

		// distance things
		const Vec3 vDelta = Player->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::Players::Alpha.Value);

		if (Player->GetDormant()) {
			Player->m_iHealth() = cResource->GetHealth(Player->GetIndex());
		}

		int nIndex = Player->GetIndex();
		bool bIsLocal = nIndex == I::EngineClient->GetLocalPlayer();

		if (!bIsLocal)
		{
			if (Vars::ESP::Players::IgnoreCloaked.Value)
			{
				if (Player->IsCloaked()) { continue; }
			}

			if (Vars::ESP::Players::IgnoreTeammates.Value)
			{
				if (Vars::ESP::Players::IgnoreFriends.Value)
				{
					if (Player->GetTeamNum() == pLocal->GetTeamNum() && g_EntityCache.IsFriend(nIndex)) { continue; }
				}
				else
					if (Player->GetTeamNum() == pLocal->GetTeamNum() && !g_EntityCache.IsFriend(nIndex)) { continue; }
			}
		}

		else
		{
			if (Vars::ESP::Players::IgnoreLocal.Value)
			{
				continue;
			}
		}

		Color_t drawColor = Utils::GetTeamColor(Player->GetTeamNum(), Vars::ESP::Main::EnableTeamEnemyColors.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(Player, vTrans, x, y, w, h))
		{
			int nHealth = Player->GetHealth(), nMaxHealth = Player->GetMaxHealth();
			Color_t healthColor = Utils::GetHealthColor(nHealth, nMaxHealth);

			size_t FONT = FONT_ESP, FONT_NAME = FONT_ESP_NAME;

			int nTextX = x + w + 3, nTextOffset = 0, nClassNum = Player->GetClassNum();

			if (Vars::ESP::Players::Uber.Value == 2 && nClassNum == CLASS_MEDIC)
			{
				if (const auto& pMedGun = Player->GetWeaponFromSlot(SLOT_SECONDARY))
				{
					nTextX += 5;
				}
			}

			// Bone ESP
			if (Vars::ESP::Players::Bones.Value)
			{
				const Color_t clrBone = Vars::ESP::Players::Bones.Value == 1 ? Colors::Bones : healthColor;

				DrawBones(Player, { 8, 7, 6, 4 }, clrBone);
				DrawBones(Player, { 11, 10, 9, 4 }, clrBone);
				DrawBones(Player, { 0, 4, 1 }, clrBone);
				DrawBones(Player, { 14, 13, 1 }, clrBone);
				DrawBones(Player, { 17, 16, 1 }, clrBone);
			}

			// Box ESP (Outlined, Rect, 3D)
			switch (Vars::ESP::Players::Box.Value)
			{
			case 1:
			{
				int height = h + 1; //don't ask me /shrug

				g_Draw.OutlinedRect(x, y, w, height, drawColor);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, height + 2, Colors::OutlineESP);

				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, drawColor);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, drawColor);
				break;
			}
			default:
				break;
			}

			// Health Text
			if (Vars::ESP::Players::HealthText.Value == 1)
			{
				if (nHealth > nMaxHealth) {
					g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::Overheal, ALIGN_DEFAULT, L"+%d HP", nHealth - nMaxHealth);
				} else {
					g_Draw.String(FONT, nTextX, y + nTextOffset, healthColor, ALIGN_DEFAULT, L"%d HP", nHealth);
				}
				nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
			}

			// if (Vars::Debug::DebugInfo.Value)
			// {
			// 	Vec3 vPlayerVelocity{};
			// 	Player->EstimateAbsVelocity(vPlayerVelocity);
			// 	g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::White, ALIGN_DEFAULT, L"SPEED (%.0f)", vPlayerVelocity.Length());
			// }

			// Ubercharge status/bar
			if (Vars::ESP::Players::Uber.Value && nClassNum == CLASS_MEDIC)
			{
				if (const auto& pMedGun = Player->GetWeaponFromSlot(SLOT_SECONDARY))
				{
					if (Vars::ESP::Players::Uber.Value == 1)
					{
						g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::UberColor, ALIGN_DEFAULT, L"%.0f%%",
							pMedGun->GetUberCharge() * 100.0f);
						nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
					}

					float flUber = pMedGun->GetUberCharge() * (pMedGun->GetItemDefIndex() == Medic_s_TheVaccinator
						? 400.0f
						: 100.0f);

					float flMaxUber = (pMedGun->GetItemDefIndex() == Medic_s_TheVaccinator ? 400.0f : 100.0f);

					if (flUber > flMaxUber)
					{
						flUber = flMaxUber;
					}

					static constexpr int RECT_WIDTH = 2;
					int nHeight = h + (flUber < flMaxUber ? 2 : 1);
					int nHeight2 = h + 1;
					float ratio = flUber / flMaxUber;
					int bar = static_cast<int>(std::round((w - 2) * ratio));

					if (Vars::ESP::Players::Uber.Value == 2 && pMedGun->GetUberCharge())
					{
						x += w + 1;

						g_Draw.RectOverlay(x - w, y + h + 3, bar, 2, 4, Colors::UberColor, Colors::OutlineESP, false);

						x -= w + 1;
					}

					if (Vars::ESP::Players::Uber.Value == 3)
					{
						if (pMedGun->GetUberCharge())
						{
							x += w + 1;
							g_Draw.RectOverlay(x - w, y + h + 3, bar, 2, 4, Colors::UberColor, Colors::OutlineESP, false);
							x -= w + 1;
						}
						g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::UberColor, ALIGN_DEFAULT, L"%.0f%%",
							pMedGun->GetUberCharge() * 100.0f);
						nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
					}
				}
			}

			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(nIndex, &pi))
			{
				// Name ESP
				int middle = x + w / 2;
				if (Vars::ESP::Players::Name.Value)
				{
					int offset = g_Draw.m_vecFonts[FONT_NAME].nTall + g_Draw.m_vecFonts[FONT_NAME].nTall / 4;
					g_Draw.String(FONT_NAME, middle, y - offset, Vars::ESP::Players::NameCustom.Value ? Vars::ESP::Players::NameColor : drawColor, ALIGN_CENTERHORIZONTAL,
						Utils::ConvertUtf8ToWide(pi.name).data());
				}

				if (Vars::ESP::Players::PriorityText.Value)
				{
					switch (G::PlayerPriority[pi.friendsID].Mode)
					{
					case 4:
					{
						if (Vars::ESP::Players::PriorityText.Value)
						{
							if (!g_EntityCache.IsFriend(nIndex))
								g_Draw.String(FONT_NAME, middle, y - 28, { 255, 0, 0, 255 }, ALIGN_CENTERHORIZONTAL, "CHEATER");
						}
						break;
					}
					case 3:
					{
						g_Draw.String(FONT_NAME, middle, y - 28, { 255, 255, 0, 255 }, ALIGN_CENTERHORIZONTAL, "RAGE");
						break;
					}
					case 1:
					{
						g_Draw.String(FONT_NAME, middle, y - 28, { 255, 255, 255, 255 }, ALIGN_CENTERHORIZONTAL, "IGNORED");
						break;
					}
					case 0:
					{
						if (Player != pLocal)
						{
							if (g_EntityCache.IsFriend(nIndex))
							{
								g_Draw.String(FONT_NAME, middle, y - 28, Colors::Friend, ALIGN_CENTERHORIZONTAL, "FRIEND");
								nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
							}							
						}
						break;
					}
					default: break;
					}
				}
			}

			// Class ESP
			if (Vars::ESP::Players::Class.Value)
			{
				if (Vars::ESP::Players::Class.Value == 1 || Vars::ESP::Players::Class.Value == 3)
				{
					int offset = Vars::ESP::Players::Name.Value
						? g_Draw.m_vecFonts[FONT_NAME].nTall + g_Draw.m_vecFonts[FONT_NAME].nTall * 0.3f
						: 0;

					static constexpr int TEXTURE_SIZE = 18;
					if (Vars::ESP::Players::PriorityText.Value && G::PlayerPriority[pi.friendsID].Mode != 2)
					{
						g_Draw.Texture(x + w / 2 - TEXTURE_SIZE / 2, y - 30 - TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE, Colors::White,
							nClassNum);
					}
					else
						g_Draw.Texture(x + w / 2 - TEXTURE_SIZE / 2, y - offset - TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE, Colors::White,
							nClassNum);
				}

				if (Vars::ESP::Players::Class.Value >= 2)
				{
					g_Draw.String(FONT, nTextX, y + nTextOffset, drawColor, ALIGN_DEFAULT, L"%ls",
						GetPlayerClass(nClassNum));
					nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
				}
			}

			const auto& pWeapon = Player->GetActiveWeapon();
			int weaponoffset = 0;
			if (pWeapon)
			{
				// Weapon text
				if (Vars::ESP::Players::WeaponText.Value)
				{
					g_Draw.String(FONT_ESP, x + (w / 2), y + h + (Vars::ESP::Players::WeaponIcon.Value ? 25 : 0), Colors::WeaponIcon, ALIGN_CENTERHORIZONTAL, 
						"%s", F::WESP.GetWeaponName(pWeapon->GetWeaponID(), G::CurItemDefIndex, G::CurWeaponType));
				}

				// Weapon icons
				if (Vars::ESP::Players::WeaponIcon.Value)
				{
					CHudTexture* pIcon = pWeapon->GetWeaponIcon();
					if (pIcon)
					{
						int offset = 0;
						if (Vars::ESP::Players::Distance.Value)
						{
							offset = 10;
						}

						float fx, fy, fw, fh;
						fx = static_cast<float>(x);
						fy = static_cast<float>(y);
						fw = static_cast<float>(w);
						fh = static_cast<float>(h);
						const auto iconWidth = static_cast<float>(pIcon->Width());
						// lol
						const float scale = std::clamp(fw / iconWidth, 0.5f, 0.75f);
						static float easedScale = 0.5f;
						scale > easedScale
							? easedScale = g_Draw.EaseOut(scale, easedScale, 0.99f)
							: easedScale = g_Draw.EaseIn(easedScale, scale, 0.99f);
						g_Draw.DrawHudTexture(fx + fw / 2.f - iconWidth / 2.f * scale, fy + fh + 1.f + weaponoffset, scale, pIcon, 
							Colors::WeaponIcon);
					}
				}
			}

			//Distance ESP
			if (Vars::ESP::Players::Distance.Value)
			{				
				if (Player != pLocal)
				{
					//this code sucks!!!
					int offset = 0;
					if (Vars::ESP::Players::WeaponIcon.Value && Vars::ESP::Players::WeaponText.Value) { offset = 26; }
					else if (Vars::ESP::Players::WeaponText.Value) { offset = 1; }
					else if (Vars::ESP::Players::WeaponIcon.Value) { offset = 25; }
					weaponoffset += Vars::Fonts::FONT_ESP::nTall.Value;

					const int Distance = round(flDistance / 52.49);
					g_Draw.String(FONT_ESP, x + (w / 2), y + h + weaponoffset + offset, drawColor, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
				}
			}

			// Player conditions
			if (Vars::ESP::Players::Conditions::Enabled.Value)
			{
				size_t FONT = FONT_ESP_COND;
				int offset = g_Draw.m_vecFonts[FONT].nTall / 4;

				if (Vars::ESP::Players::Conditions::Ping.Value)
				{
					// ping warning, idea from nitro
					int ping = cResource->GetPing(Player->GetIndex());
					if (const INetChannel* netChannel = I::EngineClient->GetNetChannelInfo()) //safety net
					{
						if (!netChannel->IsLoopback()) // dont draw if in a local server, since every ping will be below 10 anyways, also reduces clutter 
						{
							if (ping != 0 && (ping >= 200 || ping <= 20))
							{
								g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 95, 95, 255 }, ALIGN_DEFAULT, "%dMS", ping);
								nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
							}
						}
					}
				}

				if (Vars::ESP::Players::Conditions::KD.Value)
				{
					const int kills = cResource->GetKills(Player->GetIndex());
					const int deaths = cResource->GetDeaths(Player->GetIndex());
					if (deaths > 1) //idea from rijin
					{
						const int kd = kills / deaths;
						if (kills >= 12 && kd >= 6) //dont just say they have a high kd because they just joined and got a couple kills
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 95, 95, 255 }, ALIGN_DEFAULT, "HIGH K/D [%d]", kd);
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
					}
					else
					{
						if (kills >= 12)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 95, 95, 255 }, ALIGN_DEFAULT, "HIGH K/D [%d]", kills);
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
					}
				}
				
				if (Vars::ESP::Players::Conditions::LagComp.Value)
				{
					//lagcomp cond, idea from nitro
					const float flSimTime = Player->GetSimulationTime(), flOldSimTime = Player->GetOldSimulationTime();
					if (flSimTime != flOldSimTime) //stolen from CBacktrack::MakeRecords()
					{
						if (!F::Backtrack.mRecords[Player].empty())
						{
							const Vec3 vPrevOrigin = F::Backtrack.mRecords[Player].front().vOrigin;
							const Vec3 vDelta = Player->m_vecOrigin() - vPrevOrigin;
							if (vDelta.Length2DSqr() > 4096.f)
							{
								g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 95, 95, 255 }, ALIGN_DEFAULT, "LAGCOMP");
								nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
							}
						}
					}
				}

				if (Vars::ESP::Players::Conditions::Dormant.Value)
				{
					if (Player->GetDormant())
					{
						g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 128, 128, 128, 255 }, ALIGN_DEFAULT, "DORMANT");
						nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
					}
				}
								
				const int nCond = Player->GetCond();
				const int nCondEx = Player->GetCondEx();
				const int nCondEx2 = Player->GetCondEx2();

				//colors
				const Color_t teamColors = Utils::GetTeamColor(Player->GetTeamNum(), Vars::ESP::Main::EnableTeamEnemyColors.Value);
				const static Color_t pink = { 255, 100, 200, 255 };
				const static Color_t green = { 0, 255, 0, 255 };
				const static Color_t yellow = { 255, 255, 0, 255 };

				{ //this is here just so i can collapse this entire section to reduce clutter
					if (Vars::ESP::Players::Conditions::Buffs.Value)
					{
						if (nCond & TFCond_Ubercharged || nCondEx & TFCondEx_UberchargedHidden || nCondEx & TFCondEx_UberchargedCanteen)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::UberColor, ALIGN_DEFAULT, "UBER");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						else if (nCond & TFCond_Bonked)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Invuln, ALIGN_DEFAULT, "BONK");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						} // no need to show bonk effect if they are ubered, right?

						/* vaccinator effects */
						if (nCondEx & TFCondEx_BulletCharge)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "BULLET CHARGE");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						if (nCondEx & TFCondEx_ExplosiveCharge)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "BLAST CHARGE");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						if (nCondEx & TFCondEx_FireCharge)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "FIRE CHARGE");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						if (nCondEx & TFCondEx_BulletResistance)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "BULLET RESIST");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						if (nCondEx & TFCondEx_ExplosiveResistance)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "BLAST RESIST");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						if (nCondEx & TFCondEx_FireResistance)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "FIRE RESIST");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_MegaHeal)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, green, ALIGN_DEFAULT, "MEGAHEAL");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (Player->IsCritBoosted())
						{															//light red
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 107, 108, 255 }, ALIGN_DEFAULT, "CRITS");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_Buffed)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, teamColors, ALIGN_DEFAULT, "BUFF BANNER");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_CritCola || nCond & TFCond_NoHealingDamageBuff)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, yellow, ALIGN_DEFAULT, "MINI-CRITS");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_DefenseBuffed)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, teamColors, ALIGN_DEFAULT, "BATTALIONS");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_RegenBuffed)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, teamColors, ALIGN_DEFAULT, "CONCHEROR");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCondEx_FocusBuff)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, "FOCUS");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_Healing || nCond & TFCond_MegaHeal || Player->IsKingBuffed())
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Overheal, ALIGN_DEFAULT, "HP++");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
						else if (Player->GetHealth() > Player->GetMaxHealth())
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Overheal, ALIGN_DEFAULT, "HP+");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

					}
					
					if (Vars::ESP::Players::Conditions::Debuffs.Value)
					{
						if (nCond & TFCond_Jarated)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, yellow, ALIGN_DEFAULT, "JARATE");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_MarkedForDeath || nCondEx & TFCondEx_MarkedForDeathSilent)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, yellow, ALIGN_DEFAULT, "MARKED");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_OnFire)
						{															//orange
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 239, 129, 90, 255 }, ALIGN_DEFAULT, "BURNING");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_Milked)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::White, ALIGN_DEFAULT, "MILK");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
					}
					
					if (Vars::ESP::Players::Conditions::Other.Value)
					{
						if (Vars::Visuals::RemoveTaunts.Value) // i dont really see a need for this condition unless you have this enabled
						{
							if (nCond & TFCond_Taunting)
							{
								g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, pink, ALIGN_DEFAULT, "TAUNTING");
								nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
							}
						}

						if (Player->GetFeignDeathReady())
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, "DR");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
				
						if (nCond & TFCond_Slowed)
						{
							if (const auto& pWeapon = Player->GetActiveWeapon())
							{
								if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
								{															//gray
									g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 128, 128, 128, 255 }, ALIGN_DEFAULT, "REV");
									nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
								}

								if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
								{
									bool charged = (I::GlobalVars->curtime - pWeapon->GetChargeBeginTime()) >= 1.0f;
									if (charged)
									{
										g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, "CHARGED");
									}
									else
									{
										g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, "CHARGING");
									}
									nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall; //just put this here since it should draw something regardless
								}

								if (pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON)
								{
									g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, "CHARGING");
									nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
								}
							}
						}

						if (nCond & TFCond_Zoomed)
						{															//aqua
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 0, 255, 255, 255 }, ALIGN_DEFAULT, "ZOOMED");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_Cloaked || nCond & TFCond_CloakFlicker || nCondEx2 & TFCondEx2_Stealthed || nCondEx2 & TFCondEx2_StealthedUserBuffFade)
						{
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, Colors::Cloak, ALIGN_DEFAULT, L"INVIS %.0f%%", Player->GetInvisPercentage());
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}

						if (nCond & TFCond_Disguising || nCondEx & TFCondEx_DisguisedRemoved || nCond & TFCond_Disguised)
						{															//gray
							g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 128, 128, 128, 255 }, ALIGN_DEFAULT, "DISGUISED");
							nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
						}
					}																				
				}
			}

			// Health bar
			if (Vars::ESP::Players::HealthBar.Value)
			{
				x -= 1;

				auto flHealth = static_cast<float>(nHealth);
				auto flMaxHealth = static_cast<float>(nMaxHealth);

				float SPEED_FREQ = 145 / 0.65f;
				int player_hp = flHealth;
				int player_hp_max = flMaxHealth;
				static float prev_player_hp[75];

				if (prev_player_hp[Player->GetIndex()] > player_hp)
					prev_player_hp[Player->GetIndex()] -= SPEED_FREQ * I::GlobalVars->frametime;
				else
					prev_player_hp[Player->GetIndex()] = player_hp;

				Gradient_t clr = flHealth > flMaxHealth ? Colors::GradientOverhealBar : Colors::GradientHealthBar;

				Color_t HealthColor = flHealth > flMaxHealth ? Colors::Overheal : Utils::GetHealthColor(nHealth, nMaxHealth);

				if (!Player->IsVulnerable())
				{
					clr = { Colors::Invuln, Colors::Invuln };
				}

				if (flHealth > flMaxHealth)
				{
					flHealth = flMaxHealth;
				}

				float ratio = flHealth / flMaxHealth;

				if (Vars::ESP::Players::HealthBarStyle.Value == 0 && Vars::ESP::Players::HealthBar.Value)
				{
					g_Draw.OutlinedGradientBar(x - 4, y + h, 2, h, ratio, clr.startColour, clr.endColour, Colors::OutlineESP, false);
				}

				else if (Vars::ESP::Players::HealthBarStyle.Value == 1 && Vars::ESP::Players::HealthBar.Value)
				{
					g_Draw.RectOverlay(x - 4, y + h, 2, h, ratio, HealthColor, Colors::OutlineESP, false);
				}

				if (Vars::ESP::Players::HealthText.Value == 2)
				{
					if (nHealth > nMaxHealth)
					{
						g_Draw.String(FONT, x - 6, (y + h) - (ratio * h) - 4, Colors::White, ALIGN_REVERSE, "+%d", nHealth - nMaxHealth);
					}
					else
					{
						g_Draw.String(FONT, x - 6, (y + h) - (ratio * h) - 4, Colors::White, ALIGN_REVERSE, "%d", nHealth);
					}
				}

				x += 1;
			}
		}
	}
	I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
}

void CESP::DrawBuildings(CBaseEntity* pLocal) const
{
	if (!Vars::ESP::Buildings::Active.Value || !Vars::ESP::Main::Active.Value)
	{
		return;
	}

	for (const auto& pBuilding : g_EntityCache.GetGroup(Vars::ESP::Buildings::IgnoreTeammates.Value ? EGroupType::BUILDINGS_ENEMIES : EGroupType::BUILDINGS_ALL))
	{
		if (!pBuilding->IsAlive())
		{
			continue;
		}

		// distance things
		const Vec3 vDelta = pBuilding->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::Buildings::Alpha.Value);

		const auto& building = reinterpret_cast<CBaseObject*>(pBuilding);

		Color_t drawColor = Utils::GetTeamColor(building->GetTeamNum(), Vars::ESP::Main::EnableTeamEnemyColors.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(building, vTrans, x, y, w, h))
		{
			const auto nHealth = building->GetHealth();
			const auto nMaxHealth = building->GetMaxHealth();
			auto nTextOffset = 0, nTextTopOffset = 0;
			const auto nTextX = x + w + 3;

			Color_t healthColor = Utils::GetHealthColor(nHealth, nMaxHealth);

			const auto nType = static_cast<EBuildingType>(building->GetType());

			size_t FONT = FONT_ESP, FONT_NAME = FONT_ESP_NAME, FONT_COND = FONT_ESP_COND;

			const bool bIsMini = building->GetMiniBuilding();

			// Box ESP (Rect, Corners, 3D)
			switch (Vars::ESP::Buildings::Box.Value)
			{
			case 1:
			{
				h += 1;

				g_Draw.OutlinedRect(x, y, w, h, drawColor);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Colors::OutlineESP);

				h -= 1;
				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, drawColor);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, drawColor);
				break;
			}
			default:
				break;
			}

			// Name ESP
			if (Vars::ESP::Buildings::Name.Value)
			{
				const wchar_t* szName;

				switch (nType)
				{
				case EBuildingType::SENTRY:
				{
					if (bIsMini)
					{
						szName = L"Mini-Sentry";
					}
					else
					{
						szName = L"Sentry";
					}
					break;
				}
				case EBuildingType::DISPENSER:
				{
					szName = L"Dispenser";
					break;
				}
				case EBuildingType::TELEPORTER:
				{
					if (building->GetObjectMode())
					{
						szName = L"Teleporter Exit";
					}
					else
					{
						szName = L"Teleporter Entrance";
					}
					break;
				}
				default:
				{
					szName = L"Unknown";
					break;
				}
				}

				nTextTopOffset += g_Draw.m_vecFonts[FONT_NAME].nTall + g_Draw.m_vecFonts[FONT_NAME].nTall / 4;
				g_Draw.String(FONT_NAME, x + w / 2, y - nTextTopOffset, Vars::ESP::Buildings::NameCustom.Value ? Vars::ESP::Buildings::NameColor : drawColor, ALIGN_CENTERHORIZONTAL,
					szName);
			}

			//Distance ESP
			if (Vars::ESP::Buildings::Distance.Value)
			{
				const int Distance = round(flDistance / 52.49);
				g_Draw.String(FONT_ESP, x + (w / 2), y + h, Colors::White, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
			}

			// Building owner ESP
			if (Vars::ESP::Buildings::Owner.Value && !building->GetMapPlaced())
			{
				if (const auto& pOwner = building->GetOwner())
				{
					PlayerInfo_t pi;
					if (I::EngineClient->GetPlayerInfo(pOwner->GetIndex(), &pi))
					{
						nTextTopOffset += g_Draw.m_vecFonts[FONT_NAME].nTall + g_Draw.m_vecFonts[FONT_NAME].nTall /
							4;
						g_Draw.String(FONT_COND, x + w / 2, y - nTextTopOffset, Colors::Cond, ALIGN_CENTERHORIZONTAL,
							L"Built by: %ls", Utils::ConvertUtf8ToWide(pi.name).data());
					}
				}
			}

			// Health text
			if (Vars::ESP::Buildings::Health.Value)
			{
				g_Draw.String(FONT, nTextX, y + nTextOffset, healthColor, ALIGN_DEFAULT, L"%d HP", nHealth, nMaxHealth);
				nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
			}

			// Building level
			if (Vars::ESP::Buildings::Level.Value && !bIsMini)
			{
				g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, L"%d/%d",
					building->GetLevel(), building->GetHighestLevel());
				nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
			}

			// Building conditions
			if (Vars::ESP::Buildings::Cond.Value)
			{
				std::vector<std::wstring> condStrings{};

				const float flConstructed = building->GetConstructed() * 100.0f;
				if (flConstructed < 100.0f && static_cast<int>(flConstructed) != 0)
				{
					g_Draw.String(FONT, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, L"BUILDING: %0.f%%",
						flConstructed);
					nTextOffset += g_Draw.m_vecFonts[FONT].nTall;
				}

				if (nType == EBuildingType::SENTRY && building->GetControlled())
				{
					condStrings.emplace_back(L"WRANGLED");
				}

				if (building->GetSapped())
				{
					condStrings.emplace_back(L"SAPPED");
				}
				else if (building->GetDisabled()) //Building->IsSpook()
				{
					condStrings.emplace_back(L"DISABLED");
				}

				if (building->IsSentrygun() && !building->GetConstructing())
				{
					int iShells;
					int iMaxShells;
					int iRockets;
					int iMaxRockets;

					building->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

					if (iShells == 0)
						condStrings.emplace_back(L"NO AMMO");

					if (!bIsMini && iRockets == 0)
						condStrings.emplace_back(L"NO ROCKETS");
				}

				if (!condStrings.empty())
				{
					for (auto& condString : condStrings)
					{
						g_Draw.String(FONT_COND, nTextX, y + nTextOffset, Colors::Cond, ALIGN_DEFAULT, condString.data());
						nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
					}
				}
			}

			// Healthbar
			if (Vars::ESP::Buildings::HealthBar.Value)
			{
				x -= 1;

				auto flHealth = static_cast<float>(nHealth);
				const auto flMaxHealth = static_cast<float>(nMaxHealth);

				if (flHealth > flMaxHealth)
				{
					flHealth = flMaxHealth;
				}

				const int nHeight = h + (flHealth < flMaxHealth ? 2 : 1);
				int nHeight2 = h + 1;

				const float ratio = flHealth / flMaxHealth;

				g_Draw.RectOverlay(x - 4, y + h, 2, h, ratio, healthColor, Colors::OutlineESP, false);
				g_Draw.OutlinedRect(x - 5, y + nHeight - nHeight * ratio - 1, 4,
					nHeight * ratio + 1, Colors::OutlineESP);

				if (Vars::ESP::Players::HealthText.Value == 2)
				{
					g_Draw.String(FONT, x - 6, (y + h) - (ratio * h) - 4, Colors::White, ALIGN_REVERSE, "%d", nHealth);
				}

				x += 1;
			}
		}
	}
	I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
}

void CESP::DrawWorld() const
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (!Vars::ESP::World::Active.Value || !Vars::ESP::Main::Active.Value)
	{
		return;
	}

	Vec3 vScreen = {};
	constexpr size_t FONT = FONT_ESP_PICKUPS;

	I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

	for (const auto& health : g_EntityCache.GetGroup(EGroupType::WORLD_HEALTH))
	{
		// distance things
		const Vec3 vDelta = health->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(health, vTrans, x, y, w, h))
		{
			if (Vars::ESP::World::HealthName.Value)
			{
				if (Utils::W2S(health->GetVecOrigin(), vScreen))
					g_Draw.String(FONT, vScreen.x, y, Colors::Health, ALIGN_CENTER, L"Health");
			}

			if (Vars::ESP::World::HealthLine.Value)
			{
				Vec3 vScreen, vOrigin = Vec3(g_ScreenSize.c, g_ScreenSize.h, 0.0f);

				if (I::Input->CAM_IsThirdPerson())
					Utils::W2S(pLocal->GetAbsOrigin(), vOrigin);

				if (Utils::W2S(health->GetAbsOrigin(), vScreen))
					g_Draw.Line(vOrigin.x, vOrigin.y, vScreen.x, vScreen.y, Colors::Health);
			}

			//Distance ESP
			if (Vars::ESP::World::HealthDistance.Value)
			{
				const int Distance = round(flDistance / 52.49); 
				g_Draw.String(FONT_ESP, x + (w / 2), y + h, Colors::Health, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
			}

			switch (Vars::ESP::World::HealthBox.Value)
			{
			case 1:
			{
				h += 1;

				g_Draw.OutlinedRect(x, y, w, h, Colors::Health);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Colors::OutlineESP);

				h -= 1;
				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, Colors::Health);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, Colors::Health);
				break;
			}
			default: break;
			}
		}
	}

	for(const auto& ammo : g_EntityCache.GetGroup(EGroupType::WORLD_AMMO))
	{
		// distance things
		const Vec3 vDelta = ammo->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(ammo, vTrans, x, y, w, h))
		{
			if (Vars::ESP::World::AmmoName.Value)
			{
				if (Utils::W2S(ammo->GetVecOrigin(), vScreen))
					g_Draw.String(FONT, vScreen.x, y, Colors::Ammo, ALIGN_CENTER, L"Ammo");
			}

			if (Vars::ESP::World::AmmoLine.Value)
			{
				Vec3 vScreen, vOrigin = Vec3(g_ScreenSize.c, g_ScreenSize.h, 0.0f);

				if (I::Input->CAM_IsThirdPerson())
					Utils::W2S(pLocal->GetAbsOrigin(), vOrigin);

				if (Utils::W2S(ammo->GetAbsOrigin(), vScreen))
					g_Draw.Line(vOrigin.x, vOrigin.y, vScreen.x, vScreen.y, Colors::Ammo);
			}

			//Distance ESP
			if (Vars::ESP::World::AmmoDistance.Value)
			{
				const int Distance = round(flDistance / 52.49);
				g_Draw.String(FONT_ESP, x + (w / 2), y + h, Colors::Ammo, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
			}

			switch (Vars::ESP::World::AmmoBox.Value)
			{
			case 1:
			{
				h += 1;

				g_Draw.OutlinedRect(x, y, w, h, Colors::Ammo);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Colors::OutlineESP);

				h -= 1;
				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, Colors::Ammo);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, Colors::Ammo);
				break;
			}
			default: break;
			}
		}
	}

	for (const auto& NPC : g_EntityCache.GetGroup(EGroupType::WORLD_NPC))
	{
		// distance things
		const Vec3 vDelta = NPC->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(NPC, vTrans, x, y, w, h))
		{
			int nTextTopOffset = 0;

			if (Vars::ESP::World::NPCName.Value)
			{
				const wchar_t* szName;

				switch (NPC->GetClassID())
				{
				case ETFClassID::CHeadlessHatman:
				{
					szName = L"Horseless Headless Horsemann";
					break;
				}
				case ETFClassID::CTFTankBoss:
				{
					szName = L"Tank";
					break;
				}
				case ETFClassID::CMerasmus:
				{
					szName = L"Merasmus";
					break;
				}
				case ETFClassID::CZombie:
				{
					szName = L"Skeleton";
					break;
				}
				case ETFClassID::CEyeballBoss:
				{
					szName = L"Monoculus";
					break;
				}
				default:
				{
					szName = L"Unknown";
					break;
				}
				}

				nTextTopOffset += g_Draw.m_vecFonts[FONT].nTall + g_Draw.m_vecFonts[FONT].nTall / 4;
				g_Draw.String(FONT_ESP_NAME, x + w / 2, y - nTextTopOffset, Colors::NPC, ALIGN_CENTERHORIZONTAL, szName);
			}

			if (Vars::ESP::World::NPCLine.Value)
			{
				Vec3 vScreen, vOrigin = Vec3(g_ScreenSize.c, g_ScreenSize.h, 0.0f);

				if (I::Input->CAM_IsThirdPerson())
					Utils::W2S(pLocal->GetAbsOrigin(), vOrigin);

				if (Utils::W2S(NPC->GetAbsOrigin(), vScreen))
					g_Draw.Line(vOrigin.x, vOrigin.y, vScreen.x, vScreen.y, Colors::NPC);
			}

			//Distance ESP
			if (Vars::ESP::World::NPCDistance.Value)
			{
				const int Distance = round(flDistance / 52.49);
				g_Draw.String(FONT_ESP, x + (w / 2), y + h, Colors::NPC, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
			}

			switch (Vars::ESP::World::NPCBox.Value)
			{
			case 1:
			{
				h += 1;

				g_Draw.OutlinedRect(x, y, w, h, Colors::NPC);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Colors::OutlineESP);

				h -= 1;
				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, Colors::NPC);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, Colors::NPC);
				break;
			}
			default: break;
			}
		}
	}

	for (const auto& Bombs : g_EntityCache.GetGroup(EGroupType::WORLD_BOMBS))
	{
		// distance things
		const Vec3 vDelta = Bombs->GetAbsOrigin() - pLocal->GetAbsOrigin();
		const float flDistance = vDelta.Length2D();
		I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(Bombs, vTrans, x, y, w, h))
		{
			int nTextTopOffset = 0;

			if (Vars::ESP::World::BombName.Value)
			{
				const wchar_t* szName;

				switch (Bombs->GetClassID())
				{
				case ETFClassID::CTFPumpkinBomb:
				{
					szName = L"Pumpkin Bomb";
					break;
				}
				case ETFClassID::CTFGenericBomb:
				{
					szName = L"Bomb";
					break;
				}
				default:
				{
					szName = L"Unknown";
					break;
				}
				}

				nTextTopOffset += g_Draw.m_vecFonts[FONT_ESP_NAME].nTall + g_Draw.m_vecFonts[FONT_ESP_NAME].nTall / 4;
				g_Draw.String(FONT_ESP_NAME, x + w / 2, y - nTextTopOffset, Colors::Bomb, ALIGN_CENTERHORIZONTAL, szName);
			}

			if (Vars::ESP::World::BombLine.Value)
			{
				Vec3 vScreen, vOrigin = Vec3(g_ScreenSize.c, g_ScreenSize.h, 0.0f);

				if (I::Input->CAM_IsThirdPerson())
					Utils::W2S(pLocal->GetAbsOrigin(), vOrigin);

				if (Utils::W2S(Bombs->GetAbsOrigin(), vScreen))
					g_Draw.Line(vOrigin.x, vOrigin.y, vScreen.x, vScreen.y, Colors::Bomb);
			}

			if (Vars::ESP::World::BombDistance.Value)
			{
				const int Distance = round(flDistance / 52.49);
				g_Draw.String(FONT_ESP, x + (w / 2), y + h, Colors::Bomb, ALIGN_CENTERHORIZONTAL, L"%dM", Distance);
			}

			switch (Vars::ESP::World::BombBox.Value)
			{
			case 1:
			{
				h += 1;

				g_Draw.OutlinedRect(x, y, w, h, Colors::Bomb);
				g_Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Colors::OutlineESP);

				h -= 1;
				break;
			}
			case 2:
			{
				g_Draw.CornerRect(x, y, w, h, 3, 5, Colors::Bomb);
				g_Draw.CornerRect(x - 1, y - 1, w + 2, h + 2, 3, 5, Colors::OutlineESP);

				break;
			}
			case 3:
			{
				Draw3DBox(vTrans, Colors::Bomb);
				break;
			}
			default: break;
			}
		}
	}

	I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
}

using ETFCond = int;

template <typename tIntType>
class CConditionVars
{
public:
	CConditionVars(tIntType& nPlayerCond, tIntType& nPlayerCondEx, tIntType& nPlayerCondEx2, tIntType& nPlayerCondEx3, ETFCond eCond)
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

const wchar_t* CESP::GetPlayerClass(int nClassNum)
{
	static const wchar_t* szClasses[] = {
		L"unknown", L"scout", L"sniper", L"soldier", L"demoman",
		L"medic", L"heavy", L"pyro", L"spy", L"engineer"
	};

	return nClassNum < 10 && nClassNum > 0 ? szClasses[nClassNum] : szClasses[0];
}

//Got this from dude719, who got it from somewhere else
void CESP::Draw3DBox(const Vec3* vPoints, Color_t clr)
{
	Vector vStart, vEnd;

	for (int i = 0; i < 3; i++)
	{
		if (Utils::W2S(vPoints[i], vStart))
		{
			if (Utils::W2S(vPoints[i + 1], vEnd))
			{
				g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
			}
		}
	}

	if (Utils::W2S(vPoints[0], vStart))
	{
		if (Utils::W2S(vPoints[3], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}

	for (int i = 4; i < 7; i++)
	{
		if (Utils::W2S(vPoints[i], vStart))
		{
			if (Utils::W2S(vPoints[i + 1], vEnd))
			{
				g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
			}
		}
	}

	if (Utils::W2S(vPoints[4], vStart))
	{
		if (Utils::W2S(vPoints[7], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}

	if (Utils::W2S(vPoints[0], vStart))
	{
		if (Utils::W2S(vPoints[6], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}

	if (Utils::W2S(vPoints[1], vStart))
	{
		if (Utils::W2S(vPoints[5], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}

	if (Utils::W2S(vPoints[2], vStart))
	{
		if (Utils::W2S(vPoints[4], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}

	if (Utils::W2S(vPoints[3], vStart))
	{
		if (Utils::W2S(vPoints[7], vEnd))
		{
			g_Draw.Line(vStart.x, vStart.y, vEnd.x, vEnd.y, clr);
		}
	}
}

void CESP::DrawBones(CBaseEntity* pPlayer, const std::vector<int>& vecBones, Color_t clr)
{
	const size_t nMax = vecBones.size(), nLast = nMax - 1;
	for (size_t n = 0; n < nMax; n++)
	{
		if (n == nLast)
		{
			continue;
		}

		const auto vBone = pPlayer->GetHitboxPos(vecBones[n]);
		const auto vParent = pPlayer->GetHitboxPos(vecBones[n + 1]);

		Vec3 vScreenBone, vScreenParent;

		if (Utils::W2S(vBone, vScreenBone) && Utils::W2S(vParent, vScreenParent))
		{
			g_Draw.Line(vScreenBone.x, vScreenBone.y, vScreenParent.x, vScreenParent.y, clr);
		}
	}
}
