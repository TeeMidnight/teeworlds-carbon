/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_GAMEWORLD_INL
#define GAME_SERVER_GAMEWORLD_INL

#include "entity.h"
#include "gameworld.h"

template<typename F>
int CGameWorld::FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, F Flag)
{
	int Num = 0;
	auto pfnTest = [&](int Type)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[Type]; pEnt; pEnt = pEnt->TypeNext())
		{
            if(!Flag(pEnt))
                continue;

			if(distance(pEnt->m_Pos, Pos) < Radius + pEnt->m_ProximityRadius)
			{
				if(ppEnts)
					ppEnts[Num] = pEnt;
				Num++;
				if(Num == Max)
					break;
			}
		}
	};
    if constexpr (F::Indexable())
    {
        pfnTest(Flag.TypeIndex());
    }
    else
    {
        for(int i = 0; i < NUM_ENTTYPES; i++)
        {
            pfnTest(i);
        }
    }

	return Num;
}

template<typename F>
CEntity *CGameWorld::IntersectEntity(vec2 Pos0, vec2 Pos1, float Radius, F Flag, vec2 &NewPos, CEntity *pNotThis)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CEntity *pClosest = 0;

	auto pfnTest = [&](int Type)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[Type]; pEnt; pEnt = pEnt->TypeNext())
		{
			if(!Flag(pEnt) || pEnt == pNotThis)
				continue;

			vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, pEnt->m_Pos);
			float Len = distance(pEnt->m_Pos, IntersectPos);
			if(Len < pEnt->m_ProximityRadius + Radius)
			{
				Len = distance(Pos0, IntersectPos);
				if(Len < ClosestLen)
				{
					NewPos = IntersectPos;
					ClosestLen = Len;
					pClosest = pEnt;
				}
			}
		}
	};
    if constexpr (F::Indexable())
    {
        pfnTest(Flag.TypeIndex());
    }
    else
    {
        for(int i = 0; i < NUM_ENTTYPES; i++)
        {
            pfnTest(i);
        }
    }

	return pClosest;
}

template<typename F>
CEntity *CGameWorld::ClosestEntity(vec2 Pos, float Radius, F Flag, CEntity *pNotThis)
{
	// Find other players
	float ClosestRange = Radius * 2;
	CEntity *pClosest = 0;

	auto pfnTest = [&](int Type)
	{
		for(CEntity *pEnt = m_apFirstEntityTypes[Type]; pEnt; pEnt = pEnt->TypeNext())
		{
			if(!Flag(pEnt)|| pEnt == pNotThis)
				continue;

			float Len = distance(Pos, pEnt->m_Pos);
			if(Len < pEnt->m_ProximityRadius + Radius)
			{
				if(Len < ClosestRange)
				{
					ClosestRange = Len;
					pClosest = pEnt;
				}
			}
		}
	};
    if constexpr (F::Indexable())
    {
        pfnTest(Flag.TypeIndex());
    }
    else
    {
        for(int i = 0; i < NUM_ENTTYPES; i++)
        {
            pfnTest(i);
        }
    }

	return pClosest;
}

namespace GameWorldCheck
{
    struct EntityType
    {
        int m_Type;
        EntityType(int Type) : m_Type(Type) {}
        bool operator()(CEntity* pEnt) const { return pEnt->GetObjType() == m_Type; }

        static constexpr bool Indexable() { return true; }
        int TypeIndex() const { return m_Type; }
    };

    struct EntityComponent
    {
        CGameWorld *m_pGameWorld;
        unsigned m_ComponentHash;
        EntityComponent(CGameWorld *pWorld, int Hash) : m_pGameWorld(pWorld), m_ComponentHash(Hash) {}
        bool operator()(CEntity* pEnt) const { return m_pGameWorld->GetComponent(pEnt, m_ComponentHash) != nullptr; }

        static constexpr bool Indexable() { return false; }
    };
};
#endif // GAME_SERVER_GAMEWORLD_INL
