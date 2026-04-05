#include "Radar.h"

#include "Radar.h"

// 1. Core Includes - Order matters here!
#include "../../../SDK/SDK.h"
#include "../../../SDK/Vars.h"

// 2. Feature Dependencies
#include "../../Players/PlayerUtils.h"
#include "../../ImGui/Menu/Menu.h"

#ifndef TF_TEAM_RED
#define TF_TEAM_RED 2
#endif
#ifndef TF_TEAM_BLU
#define TF_TEAM_BLU 3
#endif

bool CRadar::GetDrawPosition(CTFPlayer* pLocal, CBaseEntity* pEntity, int& x, int& y, int& z)
{
	const float flRange = Vars::Radar::Main::Range.Value;
	const float flYaw = -DEG2RAD(I::EngineClient->GetViewAngles().y);
	const float flSin = sinf(flYaw), flCos = cosf(flYaw);

	Vec3 vDelta = pLocal->GetAbsOrigin() - pEntity->GetAbsOrigin();
	Vec2 vPos = { vDelta.x * flSin + vDelta.y * flCos, vDelta.x * flCos - vDelta.y * flSin };

	switch (Vars::Radar::Main::Style.Value)
	{
	case Vars::Radar::Main::StyleEnum::Circle:
	{
		const float flDist = vDelta.Length2D();
		if (flDist > flRange)
		{
			if (!Vars::Radar::Main::DrawOutOfRange.Value)
				return false;

			vPos *= flRange / flDist;
		}
		break;
	}
	case Vars::Radar::Main::StyleEnum::Rectangle:
		if (fabs(vPos.x) > flRange || fabs(vPos.y) > flRange)
		{
			if (!Vars::Radar::Main::DrawOutOfRange.Value)
				return false;

			Vec2 a = { -flRange / vPos.x, -flRange / vPos.y };
			Vec2 b = { flRange / vPos.x, flRange / vPos.y };
			Vec2 c = { std::min(a.x, b.x), std::min(a.y, b.y) };
			vPos *= fabsf(std::max(c.x, c.y));
		}
	}

	auto& tWindowBox = Vars::Radar::Main::Window.Value;
	x = tWindowBox.x + vPos.x / flRange * tWindowBox.w / 2;
	y = tWindowBox.y + vPos.y / flRange * tWindowBox.w / 2 + tWindowBox.h / 2.f;
	z = vDelta.z;

	return true;
}

void CRadar::DrawBackground()
{
	auto& tWindowBox = Vars::Radar::Main::Window.Value;
	Color_t& tThemeBack = Vars::Menu::Theme::Background.Value;
	Color_t& tThemeAccent = Vars::Menu::Theme::Accent.Value;
	Color_t tColorBackground = { tThemeBack.r, tThemeBack.g, tThemeBack.b, byte(Vars::Radar::Main::BackgroundAlpha.Value) };
	Color_t tColorAccent = { tThemeAccent.r, tThemeAccent.g, tThemeAccent.b, byte(Vars::Radar::Main::LineAlpha.Value) };

	// Calculate center coordinates for the center dot
	const int centerX = tWindowBox.x;
	const int centerY = tWindowBox.y + tWindowBox.h / 2;

	switch (Vars::Radar::Main::Style.Value)
	{
	case Vars::Radar::Main::StyleEnum::Circle:
	{
		const float flRadius = tWindowBox.w / 2.f;
		H::Draw.FillCircle(tWindowBox.x, centerY, flRadius, 100, tColorBackground);
		H::Draw.LineCircle(tWindowBox.x, centerY, flRadius + 1.0f, 100, { 0, 0, 0, 255 }); // Black Outline
		H::Draw.LineCircle(tWindowBox.x, centerY, flRadius, 100, tColorAccent);            // Colored Border
		break;
	}
	case Vars::Radar::Main::StyleEnum::Rectangle:
	{
		int left = tWindowBox.x - tWindowBox.w / 2;
		int top = tWindowBox.y;
		int width = tWindowBox.w;
		int height = tWindowBox.h;

		// 1. Draw solid dark background
		H::Draw.FillRect(left, top, width, height, tColorBackground);

		// 2. Draw accent outer outline (2px outside) - FIXED to use modern Amalgam colors!
		H::Draw.LineRect(left - 2, top - 2, width + 4, height + 4, tThemeAccent);

		// 3. Draw crisp colored accent border
		H::Draw.LineRect(left, top, width, height, tColorAccent);
		break;
	}
	}

	// Draw the minimalist center dot from your image (pale pink with black outline)
	H::Draw.FillCircle(centerX, centerY, 2.0f, 12, { 255, 175, 175, 255 });
	H::Draw.LineCircle(centerX, centerY, 2.0f, 12, { 0, 0, 0, 255 });
}

void CRadar::DrawPoints(CTFPlayer* pLocal)
{
	if (Vars::Radar::World::Enabled.Value)
	{
		const int iSize = Vars::Radar::World::Size.Value;

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Gargoyle)
		{
			for (auto pGargy : H::Entities.GetGroup(EntityEnum::PickupGargoyle))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pGargy, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 150, 50, 200, 255 });
					}
					// World item textures are custom, removing .vtf allows the engine to try and find them safely
					H::Draw.Texture("hud/pickup_gargoyle", x, y, iSize, iSize);
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Spellbook)
		{
			for (auto pBook : H::Entities.GetGroup(EntityEnum::PickupSpellbook))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pBook, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 150, 50, 200, 255 });
					}
					H::Draw.Texture("hud/pickup_spellbook", x, y, iSize, iSize);
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Powerup)
		{
			for (auto pPower : H::Entities.GetGroup(EntityEnum::PickupPowerup))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pPower, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 255, 100, 0, 255 });
					}
					H::Draw.Texture("hud/pickup_powerup", x, y, iSize, iSize);
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Bombs)
		{
			for (auto bBomb : H::Entities.GetGroup(EntityEnum::WorldBomb))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, bBomb, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 150, 50, 200, 255 });
					}
					H::Draw.Texture("hud/pickup_bomb", x, y, iSize, iSize);
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Money)
		{
			for (auto pBook : H::Entities.GetGroup(EntityEnum::PickupMoney))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pBook, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 0, 255, 0, 255 });
					}
					H::Draw.Texture("hud/pickup_money", x, y, iSize, iSize);
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Ammo)
		{
			for (auto pAmmo : H::Entities.GetGroup(EntityEnum::PickupAmmo))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pAmmo, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 150, 150, 150, 255 });
					}
					H::Draw.Texture("vgui/hud/ammo_box", x, y, iSize, iSize); // Fixed path
				}
			}
		}

		if (Vars::Radar::World::Draw.Value & Vars::Radar::World::DrawEnum::Health)
		{
			for (auto pHealth : H::Entities.GetGroup(EntityEnum::PickupHealth))
			{
				int x, y, z;
				if (GetDrawPosition(pLocal, pHealth, x, y, z))
				{
					if (Vars::Radar::World::Background.Value)
					{
						const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
						H::Draw.FillCircle(x, y, flRadius, 20, { 0, 255, 0, 255 });
					}
					H::Draw.Texture("vgui/hud/health_color", x, y, iSize, iSize); // Fixed path
				}
			}
		}
	}

	if (Vars::Radar::Building::Enabled.Value)
	{
		const int iSize = Vars::Radar::Building::Size.Value;

		for (auto pEntity : H::Entities.GetGroup(EntityEnum::BuildingAll))
		{
			auto pBuilding = pEntity->As<CBaseObject>();

			if (!pBuilding->m_bWasMapPlaced())
			{
				auto pOwner = pBuilding->m_hBuilder().Get();
				if (pOwner)
				{
					const int nIndex = pOwner->entindex();
					if (pLocal->m_iObserverMode() == OBS_MODE_FIRSTPERSON ? pLocal->m_hObserverTarget().Get() == pOwner : nIndex == I::EngineClient->GetLocalPlayer())
					{
						if (!(Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Local))
							continue;
					}
					else
					{
						if (!(Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Prioritized && H::Entities.GetPriority(nIndex))
							&& !(Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Friends && H::Entities.IsFriend(nIndex))
							&& !(Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Party && H::Entities.InParty(nIndex))
							&& !(pOwner->As<CTFPlayer>()->m_iTeamNum() != pLocal->m_iTeamNum() ? Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Enemy : Vars::Radar::Building::Draw.Value & Vars::Radar::Building::DrawEnum::Team))
							continue;
					}
				}
			}

			int x, y, z;
			if (GetDrawPosition(pLocal, pBuilding, x, y, z))
			{
				const Color_t tColor = pBuilding->m_iTeamNum() == TF_TEAM_RED ? Color_t(255, 100, 100, 255) : (pBuilding->m_iTeamNum() == TF_TEAM_BLU ? Color_t(100, 150, 255, 255) : Color_t(255, 255, 255, 255));

				int iBounds = iSize;
				if (Vars::Radar::Building::Background.Value)
				{
					const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
					H::Draw.FillCircle(x, y, flRadius, 20, tColor);
					iBounds = flRadius * 2;
				}

				// FIXED BUILDING TEXTURES
				switch (pBuilding->GetClassID())
				{
				case ETFClassID::CObjectSentrygun:
					H::Draw.Texture("hud/eng_build_sentry", x, y, iSize, iSize);
					break;
				case ETFClassID::CObjectDispenser:
					H::Draw.Texture("hud/eng_build_dispenser", x, y, iSize, iSize);
					break;
				case ETFClassID::CObjectTeleporter:
					H::Draw.Texture(pBuilding->m_iObjectMode() ? "hud/eng_build_tele_exit" : "hud/eng_build_tele_enter", x, y, iSize, iSize);
					break;
				}

				if (Vars::Radar::Building::Health.Value)
				{
					const int iMaxHealth = pBuilding->m_iMaxHealth(), iHealth = pBuilding->m_iHealth();

					float flRatio = std::clamp(float(iHealth) / iMaxHealth, 0.f, 1.f);
					Color_t cColor = Vars::Colors::IndicatorBad.Value.Lerp(Vars::Colors::IndicatorGood.Value, flRatio);
					H::Draw.FillRectPercent(x - iBounds / 2, y - iBounds / 2, 2, iBounds, flRatio, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);
				}
			}
		}
	}

	if (Vars::Radar::Player::Enabled.Value)
	{
		const int iSize = Vars::Radar::Player::Size.Value;

		for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerAll))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer->IsDormant() && !H::Entities.GetDormancy(pPlayer->entindex()) || !pPlayer->IsAlive() || pPlayer->IsAGhost())
				continue;

			const int nIndex = pPlayer->entindex();
			if (pLocal->m_iObserverMode() == OBS_MODE_FIRSTPERSON ? pLocal->m_hObserverTarget().Get() == pPlayer : nIndex == I::EngineClient->GetLocalPlayer())
			{
				if (!(Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Local))
					continue;
			}
			else
			{
				if (!(Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Prioritized && H::Entities.GetPriority(nIndex))
					&& !(Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Friends && H::Entities.IsFriend(nIndex))
					&& !(Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Party && H::Entities.InParty(nIndex))
					&& !(pPlayer->m_iTeamNum() != pLocal->m_iTeamNum() ? Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Enemy : Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Team))
					continue;
			}
			if (!(Vars::Radar::Player::Draw.Value & Vars::Radar::Player::DrawEnum::Cloaked) && pPlayer->m_flInvisibility() >= 1.f)
				continue;

			int x, y, z;
			if (GetDrawPosition(pLocal, pPlayer, x, y, z))
			{
				const Color_t tColor = pPlayer->m_iTeamNum() == TF_TEAM_RED ? Color_t(255, 100, 100, 255) : (pPlayer->m_iTeamNum() == TF_TEAM_BLU ? Color_t(100, 150, 255, 255) : Color_t(255, 255, 255, 255));

				int iBounds = iSize;
				if (Vars::Radar::Player::Background.Value)
				{
					const float flRadius = sqrtf(pow(iSize, 2) * 2) / 2;
					H::Draw.FillCircle(x, y, flRadius, 20, tColor);
					iBounds = flRadius * 2;
				}

				// FIXED PLAYER TEXTURES
				switch (Vars::Radar::Player::Icon.Value)
				{
				case Vars::Radar::Player::IconEnum::Avatars:
				{
					player_info_t pi{};
					if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) && !pi.fakeplayer)
					{
						H::Draw.Avatar(x, y, iSize, iSize, pi.friendsID);
						break;
					}
					[[fallthrough]];
				}
				case Vars::Radar::Player::IconEnum::Portraits:
				{
					int iClass = pPlayer->m_iClass();
					if (iClass >= 1 && iClass <= 9)
					{
						static const char* szPortraitsRed[] = { "", "class_portraits/scout_red", "class_portraits/sniper_red", "class_portraits/soldier_red", "class_portraits/demoman_red", "class_portraits/medic_red", "class_portraits/heavy_red", "class_portraits/pyro_red", "class_portraits/spy_red", "class_portraits/engineer_red" };
						static const char* szPortraitsBlu[] = { "", "class_portraits/scout_blue", "class_portraits/sniper_blue", "class_portraits/soldier_blue", "class_portraits/demoman_blue", "class_portraits/medic_blue", "class_portraits/heavy_blue", "class_portraits/pyro_blue", "class_portraits/spy_blue", "class_portraits/engineer_blue" };

						const char* szTexture = pPlayer->m_iTeamNum() == TF_TEAM_RED ? szPortraitsRed[iClass] : szPortraitsBlu[iClass];
						H::Draw.Texture(szTexture, x, y, iSize, iSize);
						break;
					}
					[[fallthrough]];
				}
				case Vars::Radar::Player::IconEnum::Icons:
				{
					int iClass = pPlayer->m_iClass();
					if (iClass >= 1 && iClass <= 9)
					{
						static const char* szIcons[] = { "", "hud/leaderboard_class_scout", "hud/leaderboard_class_sniper", "hud/leaderboard_class_soldier", "hud/leaderboard_class_demo", "hud/leaderboard_class_medic", "hud/leaderboard_class_heavy", "hud/leaderboard_class_pyro", "hud/leaderboard_class_spy", "hud/leaderboard_class_engineer" };
						H::Draw.Texture(szIcons[iClass], x, y, iSize, iSize);
					}
					break;
				}
				}

				if (Vars::Radar::Player::Health.Value)
				{
					const int iMaxHealth = pPlayer->GetMaxHealth(), iHealth = pPlayer->m_iHealth();

					float flRatio = std::clamp(float(iHealth) / iMaxHealth, 0.f, 1.f);
					Color_t cColor = Vars::Colors::IndicatorBad.Value.Lerp(Vars::Colors::IndicatorGood.Value, flRatio);
					H::Draw.FillRectPercent(x - iBounds / 2, y - iBounds / 2, 2, iBounds, flRatio, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);

					if (iHealth > iMaxHealth)
					{
						const float flMaxOverheal = floorf(iMaxHealth / 10.f) * 5;
						flRatio = std::clamp((iHealth - iMaxHealth) / flMaxOverheal, 0.f, 1.f);
						cColor = Vars::Colors::IndicatorMisc.Value;
						H::Draw.FillRectPercent(x - iBounds / 2, y - iBounds / 2, 2, iBounds, flRatio, cColor, { 0, 0, 0, 0 }, ALIGN_BOTTOM, true);
					}
				}

				if (Vars::Radar::Player::Height.Value && std::abs(z) > 80.f)
				{
					const int m = x - iSize / 2;
					const int iOffset = z < 0 ? -5 : 5;
					const int yPos = z < 0 ? y - iBounds / 2 - 2 : y + iBounds / 2 + 2;

					H::Draw.FillPolygon({ Vec2(m, yPos), Vec2(m + iSize * 0.5f, yPos + iOffset), Vec2(m + iSize, yPos) }, tColor);
				}
			}
		}
	}
}

void CRadar::Run(CTFPlayer* pLocal)
{
	if (!Vars::Radar::Main::Enabled.Value || I::MatSystemSurface->IsCursorVisible() && !I::EngineClient->IsPlayingDemo() && !F::Menu.m_bIsOpen)
		return;

	DrawBackground();
	DrawPoints(pLocal);
}