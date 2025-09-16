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
#ifndef GAME_CLIENT_COMPONENTS_ITEMS_H
#define GAME_CLIENT_COMPONENTS_ITEMS_H
#include <game/client/component.h>

class CItems : public CComponent
{
	void RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID);
	void RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent);
	void RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameDataFlag *pPrevGameDataFlag, const CNetObj_GameDataFlag *pCurGameDataFlag);
	void RenderLaser(const struct CNetObj_Laser *pCurrent);

public:
	void OnRender() override;
};

#endif
