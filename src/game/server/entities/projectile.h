/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

#include <game/server/entity.h>

enum
{
	PLAYER_TEAM_BLUE = -2,
	PLAYER_TEAM_RED = -1
};

class CProjectile : public CBaseOwnerEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Type, CEntity *pFrom, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	void LoseOwner();

	void Reset() override;
	void Tick() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;

private:
	vec2 m_Direction;
	int m_LifeSpan;
	int m_Type;
	int m_Damage;
	int m_SoundImpact;
	int m_Weapon;
	float m_Force;
	int m_StartTick;
	bool m_Explosive;
};

#endif
