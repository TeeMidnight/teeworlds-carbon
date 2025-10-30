/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#ifndef GAME_CLIENT_COMPONENTS_EFFECTS_H
#define GAME_CLIENT_COMPONENTS_EFFECTS_H
#include <game/client/component.h>

class CEffects : public CComponent
{
	bool m_Add50hz;
	bool m_Add100hz;

	int m_aDamageTaken[MAX_CLIENTS];
	float m_aDamageTakenTick[MAX_CLIENTS];

public:
	CEffects();

	void AirJump(vec2 Pos);
	void DamageIndicator(vec2 Pos, int Amount, float Angle, int ClientID);
	void PowerupShine(vec2 Pos, vec2 Size);
	void SmokeTrail(vec2 Pos, vec2 Vel);
	void SkidTrail(vec2 Pos, vec2 Vel);
	void BulletTrail(vec2 Pos);
	void PlayerSpawn(vec2 Pos);
	void PlayerDeath(vec2 Pos, int ClientID);
	void Explosion(vec2 Pos);
	void HammerHit(vec2 Pos);

	void OnRender() override;
};
#endif
