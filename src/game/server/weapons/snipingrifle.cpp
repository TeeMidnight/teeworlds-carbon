/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include <game/server/entities/projectile.h>
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gameworld.h>
#include <game/server/weapons.h>
#include <generated/server_data.h>

class CSnipingRifle : public IWeaponInterface
{
public:
	CSnipingRifle() { WeaponManager()->RegisterWeapon("carbon.sniping_rifle", this); }

	//
	void OnFire(class CEntity *pFrom, class CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer = nullptr) override;
	const char *Name() override { return _N("Sniping Rifle"); }
	bool FullAuto() override { return false; }
	int FireDelay() override { return 1000; }
	int SnapStyle() override { return WEAPON_LASER; }

	// Ammo
	int AmmoRegenTime() override { return 0; }
	int DefaultAmmo() override { return 10; }
	int MaxAmmo() override { return 10; }
};

void CSnipingRifle::OnFire(CEntity *pFrom, CGameWorld *pWorld, vec2 Pos, vec2 Direction, int *pReloadTimer)
{
	new CProjectile(pWorld, WEAPON_SHOTGUN,
		pFrom,
		Pos,
		Direction * 2.5f,
		round_to_int(pWorld->Server()->TickSpeed() * 0.5f),
		9, false, 2, -1, SnapStyle());

	pWorld->GameServer()->CreateSound(Pos, SOUND_GUN_FIRE);
}

static CSnipingRifle gs_WeaponSnipingRifle;
