/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include <game/server/entities/laser.h>
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gameworld.h>
#include <game/server/weapons.h>
#include <generated/server_data.h>

class CLaserWeapon : public IWeaponInterface
{
public:
	CLaserWeapon() { WeaponManager()->RegisterWeapon("vanilla.laser", this); }

	//
	void OnFire(class CEntity *pFrom, class CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer = nullptr) override;
	const char *Name() override { return _N("Laser"); }
	bool FullAuto() override { return true; }
	int FireDelay() override { return g_pData->m_Weapons.m_Laser.m_pBase->m_Firedelay; }
	int SnapStyle() override { return WEAPON_LASER; }

	// Ammo
	int AmmoRegenTime() override { return g_pData->m_Weapons.m_Laser.m_pBase->m_Ammoregentime; }
	int DefaultAmmo() override { return g_pData->m_Weapons.m_Laser.m_pBase->m_Maxammo; }
	int MaxAmmo() override { return g_pData->m_Weapons.m_Laser.m_pBase->m_Maxammo; }
};

void CLaserWeapon::OnFire(CEntity *pFrom, CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer)
{
	new CLaser(pWorld, Pos, Direction, pWorld->GameServer()->Tuning()->m_LaserReach, pFrom);
	pWorld->GameServer()->CreateSound(Pos, SOUND_LASER_FIRE);
}

static CLaserWeapon gs_WeaponLaser;
