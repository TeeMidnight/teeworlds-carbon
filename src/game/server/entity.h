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
#ifndef GAME_SERVER_ENTITY_H
#define GAME_SERVER_ENTITY_H

#include <base/uuid.h>
#include <base/vmath.h>

#include "alloc.h"
#include "gameworld.h"

/*
	Class: Entity
		Basic entity class.
*/
class CEntity
{
	MACRO_ALLOC_HEAP()

private:
	/* Friend classes */
	friend class CGameWorld; // for entity list handling

	/* Identity */
	class CGameWorld *m_pGameWorld;

	CEntity *m_pPrevTypeEntity;
	CEntity *m_pNextTypeEntity;

	int m_ID;
	int m_ObjType;

	/*
		Variable: m_ProximityRadius
			Contains the physical size of the entity.
	*/
	float m_ProximityRadius;

	/* State */
	bool m_MarkedForDestroy;

protected:
	/* State */

	/*
		Variable: m_Pos
			Contains the current posititon of the entity.
	*/
	vec2 m_Pos;

	/* Getters */
	int GetID() const { return m_ID; }

public:
	/* Constructor */
	CEntity(CGameWorld *pGameWorld, int Objtype, vec2 Pos, int ProximityRadius = 0);

	/* Destructor */
	virtual ~CEntity();

	/* Objects */
	class CGameWorld *GameWorld() { return m_pGameWorld; }
	class CConfig *Config() { return m_pGameWorld->Config(); }
	class CGameContext *GameServer() { return m_pGameWorld->GameServer(); }
	class IGameController *GameController() { return m_pGameWorld->GameController(); }
	class IServer *Server() { return m_pGameWorld->Server(); }

	/* Getters */
	CEntity *TypeNext() { return m_pNextTypeEntity; }
	CEntity *TypePrev() { return m_pPrevTypeEntity; }
	const vec2 &GetPos() const { return m_Pos; }
	float GetProximityRadius() const { return m_ProximityRadius; }
	bool IsMarkedForDestroy() const { return m_MarkedForDestroy; }

	int GetObjType() const { return m_ObjType; }

	/* Setters */
	void MarkForDestroy() { m_MarkedForDestroy = true; }

	/* Other functions */

	/*
		Function: Destroy
			Destroys the entity.
	*/
	virtual void Destroy() { delete this; }

	/*
		Function: Reset
			Called when the game resets the map. Puts the entity
			back to its starting state or perhaps destroys it.
	*/
	virtual void Reset() {}

	/*
		Function: Tick
			Called to progress the entity to the next tick. Updates
			and moves the entity to its new state and position.
	*/
	virtual void Tick() {}

	/*
		Function: TickDefered
			Called after all entities Tick() function has been called.
	*/
	virtual void TickDefered() {}

	/*
		Function: TickPaused
			Called when the game is paused, to freeze the state and position of the entity.
	*/
	virtual void TickPaused() {}

	/*
		Function: Snap
			Called when a new snapshot is being generated for a specific
			client.

		Arguments:
			SnappingClient - ID of the client which snapshot is
				being generated. Could be -1 to create a complete
				snapshot of everything in the game for demo
				recording.
	*/
	virtual void Snap(int SnappingClient) {}

	virtual void PostSnap() {}

	/*
		Function: networkclipped(int snapping_client)
			Performs a series of test to see if a client can see the
			entity.

		Arguments:
			SnappingClient - ID of the client which snapshot is
				being generated. Could be -1 to create a complete
				snapshot of everything in the game for demo
				recording.

		Returns:
			Non-zero if the entity doesn't have to be in the snapshot.
	*/
	int NetworkClipped(int SnappingClient);
	int NetworkClipped(int SnappingClient, vec2 CheckPos);

	bool GameLayerClipped(vec2 CheckPos);
};

class COwnerComponent
{
	CEntity *m_pOwner;
	CEntity *m_pThis;

protected:
	void SetOwner(CEntity *pOwner) { m_pOwner = pOwner; }

public:
	COwnerComponent(CEntity *pThis);

	static unsigned GetTypeHash() { return str_quickhash("Owner"); }

	CEntity *GetOwner() const { return m_pOwner; }
};

class CHealthComponent
{
	CEntity *m_pThis;

protected:
	bool m_Alive;

	int m_Health;
	int m_MaxHealth;

	int m_Armor;
	int m_MaxArmor;

public:
	CHealthComponent(CEntity *pThis);

	static unsigned GetTypeHash() { return str_quickhash("Health"); }

	virtual ~CHealthComponent();

	virtual void Die(CEntity *pKiller, int Weapon);
	virtual bool IsFriendlyDamage(CEntity *pFrom);
	virtual bool TakeDamage(vec2 Force, vec2 Source, int Dmg, CEntity *pFrom, int Weapon);

	inline bool IncreaseHealth(int Amount)
	{
		if(m_Health >= m_MaxHealth)
			return false;
		m_Health = clamp(m_Health + Amount, 0, m_MaxHealth);
		return true;
	};

	inline bool IncreaseArmor(int Amount)
	{
		if(m_Armor >= m_MaxArmor)
			return false;
		m_Armor = clamp(m_Armor + Amount, 0, m_MaxArmor);
		return true;
	};

	// Getter
	int GetHealth() const { return m_Health; }
	int GetArmor() const { return m_Armor; }

	int GetMaxHealth() const { return m_MaxHealth; }
	int GetMaxArmor() const { return m_MaxArmor; }

	inline bool IsAlive() { return m_Alive; }
	// Setter
	void SetHealth(int Amount) { m_Health = Amount; }
	void SetArmor(int Amount) { m_Armor = Amount; }

	void SetMaxHealth(int Amount) { m_MaxHealth = Amount; }
	void SetMaxArmor(int Amount) { m_MaxArmor = Amount; }
};

#endif
