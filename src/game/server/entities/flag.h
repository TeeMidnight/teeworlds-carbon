/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <game/server/entity.h>

class CFlag : public CEntity
{
private:
	/* Identity */
	int m_Team;
	vec2 m_StandPos;

	/* State */
	bool m_AtStand;
	CCharacter *m_pCarrier;
	vec2 m_Vel;
	int m_GrabTick;
	int m_DropTick;

public:
	/* Constants */
	static int const ms_PhysSize = 14;

	/* Constructor */
	CFlag(CGameWorld *pGameWorld, int Team, vec2 StandPos);

	/* Getters */
	int GetTeam() const { return m_Team; }
	bool IsAtStand() const { return m_AtStand; }
	CCharacter *GetCarrier() const { return m_pCarrier; }
	int GetGrabTick() const { return m_GrabTick; }
	int GetDropTick() const { return m_DropTick; }

	/* CEntity functions */
	void Reset() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;
	void TickDefered() override;

	/* Functions */
	void Grab(class CCharacter *pChar);
	void Drop();
};

#endif
