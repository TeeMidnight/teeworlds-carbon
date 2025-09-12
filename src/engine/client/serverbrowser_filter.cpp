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
#include <algorithm> // sort  TODO: remove this

#include <base/math.h>

#include <engine/client/contacts.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>

#include "serverbrowser_entry.h"
#include "serverbrowser_filter.h"

class SortWrap
{
	typedef int (CServerBrowserFilter::CServerFilter::*SortFunc)(int, int) const;
	SortFunc m_pfnSort;
	CServerBrowserFilter::CServerFilter *m_pThis;

public:
	SortWrap() {};
	SortWrap(CServerBrowserFilter::CServerFilter *t, SortFunc f) :
		m_pfnSort(f), m_pThis(t) {}
	int operator()(int a, int b) { return (m_pThis->Config()->m_BrSortOrder ? (m_pThis->*m_pfnSort)(b, a) : (m_pThis->*m_pfnSort)(a, b)); }
};

class SortWarpCollection
{
	array<SortWrap> m_aSortWarps;

public:
	SortWarpCollection() { m_aSortWarps.clear(); }

	void Add(const SortWrap &Warp)
	{
		m_aSortWarps.add(Warp);
	}

	bool operator()(int a, int b)
	{
		int FlagA, FlagB;
		FlagA = 0;
		FlagB = 0;
		for(auto &Warp : m_aSortWarps)
		{
			int Result;
			if((Result = Warp(a, b)) > 0)
				FlagA += Result;
			else if(Result < 0)
				FlagB -= Result;
		}
		return FlagA > FlagB;
	}
};

//	CServerFilter
CServerBrowserFilter::CServerFilter::CServerFilter()
{
	m_pServerBrowserFilter = 0;

	m_FilterInfo.m_SortHash = 0;
	m_FilterInfo.m_Ping = 0;
	m_FilterInfo.m_Country = 0;
	m_FilterInfo.m_ServerLevel = 0;
	for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
	{
		m_FilterInfo.m_aGametype[i][0] = 0;
		m_FilterInfo.m_aGametypeExclusive[i] = false;
	}
	m_FilterInfo.m_aAddress[0] = 0;

	m_NumSortedPlayers = 0;
	m_NumSortedServers = 0;
	m_SortedServersCapacity = 0;

	m_pSortedServerlist = 0;
}

CServerBrowserFilter::CServerFilter::~CServerFilter()
{
	if(m_pSortedServerlist)
		mem_free(m_pSortedServerlist);
}

CServerBrowserFilter::CServerFilter &CServerBrowserFilter::CServerFilter::operator=(const CServerBrowserFilter::CServerFilter &Other)
{
	if(&Other != this)
	{
		m_pServerBrowserFilter = Other.m_pServerBrowserFilter;
		m_FilterInfo.Set(&Other.m_FilterInfo);
		m_NumSortedPlayers = Other.m_NumSortedPlayers;
		m_NumSortedServers = Other.m_NumSortedServers;
		m_SortedServersCapacity = Other.m_SortedServersCapacity;

		if(m_pSortedServerlist)
			mem_free(m_pSortedServerlist);
		m_pSortedServerlist = (int *) mem_alloc(m_SortedServersCapacity * sizeof(int));
		for(int i = 0; i < m_SortedServersCapacity; ++i)
			m_pSortedServerlist[i] = Other.m_pSortedServerlist[i];
	}
	return *this;
}

void CServerBrowserFilter::CServerFilter::Filter()
{
	int NumServers = m_pServerBrowserFilter->m_NumServers;
	m_NumSortedServers = 0;
	m_NumSortedPlayers = 0;

	// allocate the sorted list
	if(m_SortedServersCapacity < NumServers)
	{
		if(m_pSortedServerlist)
			mem_free(m_pSortedServerlist);
		m_SortedServersCapacity = maximum(1000, NumServers + NumServers / 2);
		m_pSortedServerlist = (int *) mem_alloc(m_SortedServersCapacity * sizeof(int));
	}

	// filter the servers
	for(int i = 0; i < NumServers; i++)
	{
		bool Filtered = false;

		int RelevantClientCount = (m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS) ? m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumPlayers : m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumClients;
		if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_BOTS)
		{
			RelevantClientCount -= m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumBotPlayers;
			if(!(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS))
				RelevantClientCount -= m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumBotSpectators;
		}

		if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_EMPTY && RelevantClientCount == 0)
			Filtered = true;
		else if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FULL && ((m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS && m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumPlayers == m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_MaxPlayers) ||
											 m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumClients == m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_MaxClients))
			Filtered = true;
		else if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_PW && m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_Flags & IServerBrowser::FLAG_PASSWORD)
			Filtered = true;
		else if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FAVORITE && !m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_Favorite)
			Filtered = true;
		else if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_PURE && !(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_Flags & IServerBrowser::FLAG_PURE))
			Filtered = true;
		else if(m_FilterInfo.m_Ping < m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_Latency)
			Filtered = true;
		else if(m_FilterInfo.m_aAddress[0] && !str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aAddress, m_FilterInfo.m_aAddress))
			Filtered = true;
		else if(m_FilterInfo.IsLevelFiltered(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_ServerLevel))
			Filtered = true;
		else
		{
			if(m_FilterInfo.m_aGametype[0][0])
			{
				bool Excluded = false, DoInclude = false, Included = false;
				for(int Index = 0; Index < CServerFilterInfo::MAX_GAMETYPES; ++Index)
				{
					if(!m_FilterInfo.m_aGametype[Index][0])
						break;
					if(m_FilterInfo.m_aGametypeExclusive[Index])
					{
						if(!str_comp_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aGameType, m_FilterInfo.m_aGametype[Index]))
						{
							Excluded = true;
							break;
						}
					}
					else
					{
						DoInclude = true;
						if(!str_comp_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aGameType, m_FilterInfo.m_aGametype[Index]))
						{
							Included = true;
							break;
						}
					}
				}
				Filtered = Excluded || (DoInclude && !Included);
			}

			if(!Filtered && m_FilterInfo.m_SortHash & IServerBrowser::FILTER_COUNTRY)
			{
				Filtered = true;
				// match against player country
				for(int p = 0; p < m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumClients; p++)
				{
					if(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_Country == m_FilterInfo.m_Country)
					{
						Filtered = false;
						break;
					}
				}
			}

			if(!Filtered && Config()->m_BrFilterString[0] != 0)
			{
				m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit = 0;

				// match against server name
				if(str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aName, Config()->m_BrFilterString))
					m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_SERVERNAME;

				// match against players
				for(int p = 0; p < m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumClients; p++)
				{
					if(str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_aName, Config()->m_BrFilterString) ||
						str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_aClan, Config()->m_BrFilterString))
					{
						m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_PLAYER;
						break;
					}
				}

				// match against map
				if(str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aMap, Config()->m_BrFilterString))
					m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_MAPNAME;

				// match against game type
				if(str_find_nocase(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aGameType, Config()->m_BrFilterString))
					m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_GAMETYPE;

				if(!m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_QuickSearchHit)
					Filtered = true;
			}
		}

		if(!Filtered)
		{
			// check for friend
			m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_FriendState = CContactInfo::CONTACT_NO;
			for(int p = 0; p < m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_NumClients; p++)
			{
				m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_FriendState = m_pServerBrowserFilter->m_pFriends->GetFriendState(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_aName,
					m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_aClan);
				m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_FriendState = maximum(m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_FriendState, m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_aClients[p].m_FriendState);
			}

			if(!(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FRIENDS) || m_pServerBrowserFilter->m_ppServerlist[i]->m_Info.m_FriendState != CContactInfo::CONTACT_NO)
			{
				m_pSortedServerlist[m_NumSortedServers++] = i;
				m_NumSortedPlayers += RelevantClientCount;
			}
		}
	}
}

int CServerBrowserFilter::CServerFilter::GetSortHash() const
{
	int i = Config()->m_BrSort & 0x7;
	i |= Config()->m_BrSortOrder << 3;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_BOTS)
		i |= 1 << 4;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_EMPTY)
		i |= 1 << 5;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FULL)
		i |= 1 << 6;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS)
		i |= 1 << 7;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FRIENDS)
		i |= 1 << 8;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_PW)
		i |= 1 << 9;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FAVORITE)
		i |= 1 << 10;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_PURE)
		i |= 1 << 11;
	if(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_COUNTRY)
		i |= 1 << 12;
	return i;
}

void CServerBrowserFilter::CServerFilter::Sort()
{
	// create filtered list
	Filter();
	SortWarpCollection FuncCollection;
	// sort
	if(Config()->m_BrSort & IServerBrowser::SORT_NAME)
		FuncCollection.Add(SortWrap(this, &CServerBrowserFilter::CServerFilter::SortCompareName));
	if(Config()->m_BrSort & IServerBrowser::SORT_PING)
		FuncCollection.Add(SortWrap(this, &CServerBrowserFilter::CServerFilter::SortComparePing));
	if(Config()->m_BrSort & IServerBrowser::SORT_MAP)
		FuncCollection.Add(SortWrap(this, &CServerBrowserFilter::CServerFilter::SortCompareMap));
	if(Config()->m_BrSort & IServerBrowser::SORT_NUMPLAYERS)
	{
		if(!(m_FilterInfo.m_SortHash & IServerBrowser::FILTER_BOTS))
			FuncCollection.Add(SortWrap(this, (m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS) ? &CServerBrowserFilter::CServerFilter::SortCompareNumPlayers : &CServerBrowserFilter::CServerFilter::SortCompareNumClients));
		else
			FuncCollection.Add(SortWrap(this, (m_FilterInfo.m_SortHash & IServerBrowser::FILTER_SPECTATORS) ? &CServerBrowserFilter::CServerFilter::SortCompareNumRealPlayers : &CServerBrowserFilter::CServerFilter::SortCompareNumRealClients));
	}
	if(Config()->m_BrSort & IServerBrowser::SORT_GAMETYPE)
		FuncCollection.Add(SortWrap(this, &CServerBrowserFilter::CServerFilter::SortCompareGametype));
	std::stable_sort(m_pSortedServerlist, m_pSortedServerlist + m_NumSortedServers, FuncCollection);

	m_FilterInfo.m_SortHash = GetSortHash();
}

int CServerBrowserFilter::CServerFilter::SortCompareName(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	//	make sure empty entries are listed last
	return (a->m_InfoState == CServerEntry::STATE_READY && b->m_InfoState == CServerEntry::STATE_READY) ? 0 : ((a->m_InfoState != CServerEntry::STATE_READY && b->m_InfoState != CServerEntry::STATE_READY) ? str_comp_nocase(a->m_Info.m_aName, b->m_Info.m_aName) : (a->m_InfoState == CServerEntry::STATE_READY ? -1 : 0));
}

int CServerBrowserFilter::CServerFilter::SortCompareMap(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	int Result = str_comp_nocase(a->m_Info.m_aMap, b->m_Info.m_aMap);
	return Result;
}

int CServerBrowserFilter::CServerFilter::SortComparePing(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return b->m_Info.m_Latency - a->m_Info.m_Latency;
}

int CServerBrowserFilter::CServerFilter::SortCompareGametype(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return str_comp_nocase(a->m_Info.m_aGameType, b->m_Info.m_aGameType);
}

int CServerBrowserFilter::CServerFilter::SortCompareNumPlayers(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return a->m_Info.m_NumPlayers - b->m_Info.m_NumPlayers;
}

int CServerBrowserFilter::CServerFilter::SortCompareNumRealPlayers(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return (a->m_Info.m_NumPlayers - a->m_Info.m_NumBotPlayers) - (b->m_Info.m_NumPlayers - b->m_Info.m_NumBotPlayers);
}

int CServerBrowserFilter::CServerFilter::SortCompareNumClients(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return a->m_Info.m_NumClients - b->m_Info.m_NumClients;
}

int CServerBrowserFilter::CServerFilter::SortCompareNumRealClients(int Index1, int Index2) const
{
	CServerEntry *a = m_pServerBrowserFilter->m_ppServerlist[Index1];
	CServerEntry *b = m_pServerBrowserFilter->m_ppServerlist[Index2];
	return (a->m_Info.m_NumClients - a->m_Info.m_NumBotPlayers - a->m_Info.m_NumBotSpectators) - (b->m_Info.m_NumClients - b->m_Info.m_NumBotPlayers - b->m_Info.m_NumBotSpectators);
}

//	CServerBrowserFilter
void CServerBrowserFilter::Init(CConfig *pConfig, IFriends *pFriends, const char *pNetVersion)
{
	m_pConfig = pConfig;
	m_pFriends = pFriends;
	str_copy(m_aNetVersion, pNetVersion, sizeof(m_aNetVersion));
}

void CServerBrowserFilter::Clear()
{
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		m_lFilters[i].m_NumSortedServers = 0;
		m_lFilters[i].m_NumSortedPlayers = 0;
	}
}

void CServerBrowserFilter::Sort(CServerEntry **ppServerlist, int NumServers, int ResortFlags)
{
	m_ppServerlist = ppServerlist;
	m_NumServers = NumServers;
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		// check if we need to resort
		CServerFilter *pFilter = &m_lFilters[i];
		if((ResortFlags & RESORT_FLAG_FORCE) || ((ResortFlags & RESORT_FLAG_FAV) && pFilter->m_FilterInfo.m_SortHash & IServerBrowser::FILTER_FAVORITE) || pFilter->m_FilterInfo.m_SortHash != pFilter->GetSortHash())
			pFilter->Sort();
	}
}

void CServerFilterInfo::Set(const CServerFilterInfo *pSrc)
{
	m_SortHash = pSrc->m_SortHash;
	m_Ping = pSrc->m_Ping;
	m_Country = pSrc->m_Country;
	m_ServerLevel = pSrc->m_ServerLevel;
	for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
	{
		str_copy(m_aGametype[i], pSrc->m_aGametype[i], sizeof(m_aGametype[i]));
		m_aGametypeExclusive[i] = m_aGametype[i][0] && pSrc->m_aGametypeExclusive[i];
	}
	str_copy(m_aAddress, pSrc->m_aAddress, sizeof(m_aAddress));
}

int CServerBrowserFilter::AddFilter(const CServerFilterInfo *pFilterInfo)
{
	CServerFilter Filter;
	Filter.m_FilterInfo.Set(pFilterInfo);
	Filter.m_pSortedServerlist = 0;
	Filter.m_NumSortedPlayers = 0;
	Filter.m_NumSortedServers = 0;
	Filter.m_SortedServersCapacity = 0;
	Filter.m_pServerBrowserFilter = this;
	m_lFilters.add(Filter);

	return m_lFilters.size() - 1;
}

void CServerBrowserFilter::GetFilter(int Index, CServerFilterInfo *pFilterInfo) const
{
	pFilterInfo->Set(&m_lFilters[Index].m_FilterInfo);
}

void CServerBrowserFilter::SetFilter(int Index, const CServerFilterInfo *pFilterInfo)
{
	CServerFilter *pFilter = &m_lFilters[Index];
	pFilter->m_FilterInfo.Set(pFilterInfo);
	pFilter->Sort();
}

void CServerBrowserFilter::RemoveFilter(int Index)
{
	m_lFilters.remove_index(Index);
}
