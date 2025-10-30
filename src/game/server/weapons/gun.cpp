/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#include <game/server/entities/projectile.h>
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gameworld.h>
#include <game/server/weapons.h>
#include <generated/server_data.h>

class CGun : public IWeaponInterface
{
public:
	CGun() { WeaponManager()->RegisterWeapon("vanilla.gun", this); }

	//
	void OnFire(class CEntity *pFrom, class CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer = nullptr) override;
	const char *Name() override { return _N("Gun"); }
	bool FullAuto() override { return false; }
	int FireDelay() override { return g_pData->m_Weapons.m_Gun.m_pBase->m_Firedelay; }
	int SnapStyle() override { return WEAPON_GUN; }

	// Ammo
	int AmmoRegenTime() override { return g_pData->m_Weapons.m_Gun.m_pBase->m_Ammoregentime; }
	int DefaultAmmo() override { return g_pData->m_Weapons.m_Gun.m_pBase->m_Maxammo; }
	int MaxAmmo() override { return g_pData->m_Weapons.m_Gun.m_pBase->m_Maxammo; }
};

void CGun::OnFire(CEntity *pFrom, CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer)
{
	new CProjectile(pWorld, WEAPON_GUN,
		pFrom,
		Pos,
		Direction,
		(int) (pWorld->Server()->TickSpeed() * pWorld->GameServer()->Tuning()->m_GunLifetime),
		g_pData->m_Weapons.m_Gun.m_pBase->m_Damage, false, 0, -1, WEAPON_GUN);

	pWorld->CreateSound(Pos, SOUND_GUN_FIRE);
}

static CGun gs_WeaponGun;
