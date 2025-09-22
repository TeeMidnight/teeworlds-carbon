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
#ifndef GAME_SERVER_GAMEWORLD_H
#define GAME_SERVER_GAMEWORLD_H

#include <game/gamecore.h>

#include <memory>

#define MAX_CHECK_ENTITY 128

class CEntity;
class IEntityComponent;

/*
	Class: Game World
		Tracks all entities in the game. Propagates tick and
		snap calls to all entities.
*/
class CGameWorld
{
public:
	enum
	{
		ENTTYPE_PROJECTILE = 0,
		ENTTYPE_LASER,
		ENTTYPE_PICKUP,
		ENTTYPE_CHARACTER,
		ENTTYPE_FLAG,
		ENTTYPE_BOTENTITY,
		NUM_ENTTYPES
	};

private:
	void Reset();
	void RemoveEntities();

	CEntity *m_pNextTraverseEntity;
	CEntity *m_apFirstEntityTypes[NUM_ENTTYPES];

	class IGameController *m_pGameController;
	class CGameContext *m_pGameServer;
	class CConfig *m_pConfig;
	class IServer *m_pServer;

	std::shared_ptr<CCollision> m_pCollision;
	std::unordered_map<CEntity *, std::unordered_map<unsigned, void *>> m_uupComponents;

	class CBotManager *m_pBotManager;

public:
	class CBotManager *BotManager() const { return m_pBotManager; }
	class CEventHandler *EventHandler();
	class IGameController *GameController() { return m_pGameController; }
	class CGameContext *GameServer() { return m_pGameServer; }
	class CConfig *Config() { return m_pConfig; }
	class IServer *Server() { return m_pServer; }
	CCollision *Collision() { return m_pCollision.get(); }

	vec2 m_aaSpawnPoints[3][64];
	unsigned m_aNumSpawnPoints[3];

	bool m_ResetRequested;
	bool m_Paused;
	CWorldCore m_Core;

	Uuid m_WorldUuid;

	CGameWorld();
	~CGameWorld();

	void SetCollision(std::shared_ptr<CCollision> pCollision);
	void SetGameServer(CGameContext *pGameServer);
	void SetGameController(IGameController *pGameController);

	CEntity *FindFirst(int Type);

	/*
		Function: find_entities
			Finds entities close to a position and returns them in a list.

		Arguments:
			pos - Position.
			radius - How close the entities have to be.
			ents - Pointer to a list that should be filled with the pointers
				to the entities.
			max - Number of entities that fits into the ents array.
			type - Type of the entities to find.

		Returns:
			Number of entities found and added to the ents array.
	*/
	template<typename F>
	int FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, F Flag);

	/*
		Function: closest_CEntity
			Finds the closest CEntity of a type to a specific point.

		Arguments:
			pos - The center position.
			radius - How far off the CEntity is allowed to be
			type - Type of the entities to find.
			notthis - Entity to ignore

		Returns:
			Returns a pointer to the closest CEntity or NULL if no CEntity is close enough.
	*/
	template<typename F>
	CEntity *ClosestEntity(vec2 Pos, float Radius, F Flag, CEntity *pNotThis);

	/*
		Function: interserct_CCharacter
			Finds the closest CCharacter that intersects the line.

		Arguments:
			pos0 - Start position
			pos2 - End position
			radius - How for from the line the CCharacter is allowed to be.
			new_pos - Intersection position
			notthis - Entity to ignore intersecting with

		Returns:
			Returns a pointer to the closest hit or NULL of there is no intersection.
	*/
	template<typename F>
	CEntity *IntersectEntity(vec2 Pos0, vec2 Pos1, float Radius, F Flag, vec2 &NewPos, class CEntity *pNotThis = 0);

	/*
		Function: insert_entity
			Adds an entity to the world.

		Arguments:
			entity - Entity to add
	*/
	void InsertEntity(CEntity *pEntity);

	/*
		Function: remove_entity
			Removes an entity from the world.

		Arguments:
			entity - Entity to remove
	*/
	void RemoveEntity(CEntity *pEntity);

	/*
		Function: destroy_entity
			Destroys an entity in the world.

		Arguments:
			entity - Entity to destroy
	*/
	void DestroyEntity(CEntity *pEntity);

	void RegisterEntityComponent(CEntity *pThis, unsigned TypeHash, void *pComponent);

	template<typename Target, typename Component>
	void RegisterEntityComponent(CEntity *pThis, Component *pComponent)
	{
		RegisterEntityComponent(pThis, Target::GetTypeHash(), static_cast<Target *>(pComponent));
	}

	template<typename Target, typename Entity>
	void RegisterEntitySelfAsComponent(Entity *pThis)
	{
		RegisterEntityComponent(pThis, Target::GetTypeHash(), static_cast<Target *>(pThis));
	}

	/*
		Function: snap
			Calls snap on all the entities in the world to create
			the snapshot.

		Arguments:
			snapping_client - ID of the client which snapshot
			is being created.
	*/
	void Snap(int SnappingClient);

	void PostSnap();

	/*
		Function: tick
			Calls tick on all the entities in the world to progress
			the world to the next tick.

	*/
	void Tick();

	int64_t CmaskAllInWorld();
	int64_t CmaskAllInWorldExceptOne(int ClientID);

	// helper functions
	void CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self, int64_t Mask);
	void CreateExplosion(vec2 Pos, CEntity *pFrom, int Weapon, int MaxDamage, int64_t Mask);
	void CreateHammerHit(vec2 Pos, int64_t Mask);
	void CreatePlayerSpawn(vec2 Pos, int64_t Mask);
	void CreateDeath(vec2 Pos, int Who, int64_t Mask);
	void CreateSound(vec2 Pos, int Sound, int64_t Mask);

	void CreateDamage(vec2 Pos, int Id, vec2 Source, int HealthAmount, int ArmorAmount, bool Self);
	void CreateExplosion(vec2 Pos, CEntity *pFrom, int Weapon, int MaxDamage);
	void CreateHammerHit(vec2 Pos);
	void CreatePlayerSpawn(vec2 Pos);
	void CreateDeath(vec2 Pos, int Who);
	void CreateSound(vec2 Pos, int Sound);

	void *GetComponent(CEntity *pEntity, unsigned Hash)
	{
		auto IterEntity = m_uupComponents.find(pEntity);
		if(IterEntity == m_uupComponents.end())
			return nullptr;

		auto IterComponent = IterEntity->second.find(Hash);
		if(IterComponent == IterEntity->second.end())
			return nullptr;
		return IterComponent->second;
	}

	template<typename T>
	T *GetComponent(CEntity *pEntity)
	{
		static_assert(std::is_same_v<decltype(T::GetTypeHash()), unsigned>, "T must have static unsigned GetTypeHash()");
		return static_cast<T *>(GetComponent(pEntity, T::GetTypeHash()));
	}
};

#endif
