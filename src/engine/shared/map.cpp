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
#include <base/system.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <game/mapitems.h>
#include "datafile.h"

class CMap : public IEngineMap
{
	CDataFileReader m_DataFile;

public:
	CMap() {}

	void *GetData(int Index) override { return m_DataFile.GetData(Index); }
	void *GetDataSwapped(int Index) override { return m_DataFile.GetDataSwapped(Index); }
	void UnloadData(int Index) override { m_DataFile.UnloadData(Index); }
	void *GetItem(int Index, int *pType, int *pID) override { return m_DataFile.GetItem(Index, pType, pID); }
	void GetType(int Type, int *pStart, int *pNum) override { m_DataFile.GetType(Type, pStart, pNum); }
	void *FindItem(int Type, int ID) override { return m_DataFile.FindItem(Type, ID); }
	int NumItems() override { return m_DataFile.NumItems(); }
	int GetDataSize(int Index) override { return m_DataFile.GetDataSize(Index); }

	void Unload() override
	{
		m_DataFile.Close();
	}

	bool Load(const char *pMapName, IStorage *pStorage) override
	{
		if(!pStorage)
			pStorage = Kernel()->RequestInterface<IStorage>();
		if(!pStorage)
			return false;
		if(!m_DataFile.Open(pStorage, pMapName, IStorage::TYPE_ALL))
			return false;
		// check version
		CMapItemVersion *pItem = (CMapItemVersion *) m_DataFile.FindItem(MAPITEMTYPE_VERSION, 0);
		if(!pItem || pItem->m_Version != CMapItemVersion::CURRENT_VERSION)
			return false;

		// replace compressed tile layers with uncompressed ones
		int GroupsStart, GroupsNum, LayersStart, LayersNum;
		m_DataFile.GetType(MAPITEMTYPE_GROUP, &GroupsStart, &GroupsNum);
		m_DataFile.GetType(MAPITEMTYPE_LAYER, &LayersStart, &LayersNum);
		for(int g = 0; g < GroupsNum; g++)
		{
			CMapItemGroup *pGroup = static_cast<CMapItemGroup *>(m_DataFile.GetItem(GroupsStart + g, 0, 0));
			for(int l = 0; l < pGroup->m_NumLayers; l++)
			{
				CMapItemLayer *pLayer = static_cast<CMapItemLayer *>(m_DataFile.GetItem(LayersStart + pGroup->m_StartLayer + l, 0, 0));

				if(pLayer->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);

					if(pTilemap->m_Version > 3)
					{
						const int TilemapCount = pTilemap->m_Width * pTilemap->m_Height;
						const int TilemapSize = TilemapCount * sizeof(CTile);

						if((TilemapCount / pTilemap->m_Width != pTilemap->m_Height) || (TilemapSize / (int) sizeof(CTile) != TilemapCount))
						{
							dbg_msg("engine", "map layer too big (%d * %d * %u causes an integer overflow)", pTilemap->m_Width, pTilemap->m_Height, unsigned(sizeof(CTile)));
							return false;
						}
						CTile *pTiles = static_cast<CTile *>(mem_alloc(TilemapSize));
						if(!pTiles)
							return false;

						// extract original tile data
						int i = 0;
						CTile *pSavedTiles = static_cast<CTile *>(m_DataFile.GetData(pTilemap->m_Data));
						while(i < TilemapCount)
						{
							for(unsigned Counter = 0; Counter <= pSavedTiles->m_Skip && i < TilemapCount; Counter++)
							{
								pTiles[i] = *pSavedTiles;
								pTiles[i++].m_Skip = 0;
							}

							pSavedTiles++;
						}

						m_DataFile.ReplaceData(pTilemap->m_Data, reinterpret_cast<char *>(pTiles), TilemapSize);
					}
				}
			}
		}

		return true;
	}

	bool IsLoaded() override
	{
		return m_DataFile.IsOpen();
	}

	SHA256_DIGEST Sha256() override
	{
		return m_DataFile.Sha256();
	}

	unsigned Crc() override
	{
		return m_DataFile.Crc();
	}
};

extern IEngineMap *CreateEngineMap() { return new CMap; }
