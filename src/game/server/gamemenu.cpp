/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#include <engine/config.h>
#include <engine/localization.h>
#include <engine/shared/memheap.h>

#include "gamecontext.h"
#include "gamemenu.h"
#include "player.h"

#include <cstdarg>
#include <cstdio>

CConfig *CGameMenu::Config() const { return GameServer()->Config(); }
IServer *CGameMenu::Server() const { return GameServer()->Server(); }

CGameMenu::CGameMenu(CGameContext *pGameServer) :
	m_pGameServer(pGameServer)
{
	for(auto &Data : m_aPlayerData)
	{
		Data.Reset();
	}

	m_CurrentClientID = -1;

	Register("MAIN", _C("Main Menu", "Vote Menu"), MenuMain, nullptr);
	Register("LANGUAGE", _C("Language Settings", "Vote Menu"), MenuLanguage, nullptr);
}

void CGameMenu::Register(const char *pPageName, const char *pTitle, const char *pContext, FMenuCallback pfnFunc, void *pUser, const char *pParent)
{
	dbg_assert(pPageName && pPageName[0], "Page must have a name");
	dbg_assert(pTitle && pTitle[0], "Page must have a title");

	std::shared_ptr<SMenuPage> pPage = std::make_shared<SMenuPage>();
	pPage->m_pfnCallback = pfnFunc;
	pPage->m_pUserData = pUser;
	pPage->m_Hash = str_quickhash(pPageName);
	pPage->m_ParentHash = str_quickhash(pParent);
	str_copy(pPage->m_aTitle, pTitle, sizeof(pPage->m_aTitle));
	str_copy(pPage->m_aContext, pContext, sizeof(pPage->m_aContext));
	m_upMenuPages[pPage->m_Hash] = pPage;
}

void CGameMenu::OnClientEntered(int ClientID)
{
	m_aPlayerData[ClientID].Reset(true);
	SetPlayerPage(ClientID, "MAIN");
}

void CGameMenu::OnMenuVote(int ClientID, SCallVoteStatus &VoteStatus, bool Sound)
{
	if(ClientID < 0 || ClientID >= SERVER_MAX_CLIENTS)
		return;

	if(VoteStatus.m_Force && !Server()->IsAuthed(ClientID))
		return;

	m_CurrentClientID = ClientID;

	unsigned &CurrentPage = m_aPlayerData[ClientID].m_CurrentPage;
	if(!m_upMenuPages.count(CurrentPage))
	{
		CurrentPage = MENU_MAIN_PAGE_ID;
		VoteStatus.m_aDesc[0] = '\0';
		VoteStatus.m_aReason[0] = '\0';
	}

	// find command
	str_copy(VoteStatus.m_aCmd, "DISPLAY", sizeof(VoteStatus.m_aCmd));
	if(VoteStatus.m_aDesc[0])
	{
		for(CVoteOptionServer *pOption = m_aPlayerData[ClientID].m_pVoteOptionFirst; pOption; pOption = pOption->m_pNext)
		{
			if(str_comp_nocase(VoteStatus.m_aDesc, pOption->m_aDescription) == 0)
			{
				str_format(VoteStatus.m_aCmd, sizeof(VoteStatus.m_aCmd), "%s", pOption->m_aCommand);
				break;
			}
		}
	}

	if(Sound)
		GameServer()->SendSoundTarget(ClientID, SOUND_WEAPON_NOAMMO);

	if(str_comp(VoteStatus.m_aCmd, "PREPAGE") == 0)
	{
		SetPlayerPage(ClientID, m_upMenuPages[CurrentPage]->m_ParentHash);
		return;
	}
	else if(str_comp(VoteStatus.m_aCmd, "NONE") == 0)
	{
		return;
	}

	// call back
	if(!m_upMenuPages[CurrentPage]->m_pfnCallback(ClientID, VoteStatus, this, m_upMenuPages[CurrentPage]->m_pUserData))
		return;
	// add back page
	if(CurrentPage != MENU_MAIN_PAGE_ID)
	{
		AddHorizontalRule();
		AddOptionLocalize(_C("Previous Page", "Vote Menu"), "PREPAGE", "=");
	}

	CVoteOptionServer *pCurrent = m_aPlayerData[ClientID].m_pVoteOptionFirst;
	while(pCurrent)
	{
		// count options for actual packet
		int NumOptions = 0;
		for(CVoteOptionServer *p = pCurrent; p && NumOptions < MAX_VOTE_OPTION_ADD; p = p->m_pNext, ++NumOptions)
			;

		// pack and send vote list packet
		CMsgPacker Msg(NETMSGTYPE_SV_VOTEOPTIONLISTADD);
		Msg.AddInt(NumOptions);
		while(pCurrent && NumOptions--)
		{
			Msg.AddString(pCurrent->m_aDescription, VOTE_DESC_LENGTH);
			pCurrent = pCurrent->m_pNext;
		}
		Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	}
}

void CGameMenu::SendMenuChat(int ClientID, const char *pChat)
{
	if(ClientID == -1)
	{
		for(int i = 0; i < SERVER_MAX_CLIENTS; i++)
		{
			if(Server()->ClientIngame(i))
				SendMenuChat(ClientID, pChat);
		}
		return;
	}

	str_copy(m_aPlayerData[ClientID].m_aMenuChat, pChat, ClientID);
	SCallVoteStatus VoteStatus;
	OnMenuVote(ClientID, VoteStatus);
}

void CGameMenu::ClearOptions(int ClientID)
{
	GameServer()->SendVoteClearOptions(ClientID);

	m_aPlayerData[ClientID].m_pVoteOptionHeap->Reset();
	m_aPlayerData[ClientID].m_pVoteOptionFirst = nullptr;
	m_aPlayerData[ClientID].m_pVoteOptionLast = nullptr;
	m_aPlayerData[ClientID].m_NumVoteOptions = 0;
}

void CGameMenu::SetPlayerPage(int ClientID, unsigned Page)
{
	if(ClientID < 0 || ClientID >= SERVER_MAX_CLIENTS)
		return;
	m_aPlayerData[ClientID].m_CurrentPage = Page;
	SCallVoteStatus VoteStatus;
	OnMenuVote(ClientID, VoteStatus);
}

void CGameMenu::SetPlayerPage(int ClientID, const char *pPage)
{
	if(!pPage || !pPage[0])
		return;
	SetPlayerPage(ClientID, str_quickhash(pPage));
}
// static
bool CGameMenu::MenuMain(int ClientID, SCallVoteStatus &VoteStatus, class CGameMenu *pMenu, void *pUserData)
{
	// refresh
	bool DisplayAddr = false;
	if(VoteStatus.m_aCmd[0])
	{
		if(str_comp(VoteStatus.m_aCmd, "PAGE SERVER VOTE") == 0)
		{
			pMenu->SetPlayerPage(ClientID, "SERVER VOTE");
			return false;
		}
		else if(str_comp(VoteStatus.m_aCmd, "PAGE LANGUAGE") == 0)
		{
			pMenu->SetPlayerPage(ClientID, "LANGUAGE");
			return false;
		}
		else if(str_comp(VoteStatus.m_aCmd, "DISPLAY ADDR") == 0)
		{
			DisplayAddr = true;
		}
		else if(str_comp(VoteStatus.m_aCmd, "HIDDEN") == 0)
		{
			pMenu->GameServer()->m_apPlayers[ClientID]->m_HideTip = true;
		}
	}

	pMenu->ClearOptions(ClientID);
	pMenu->AddPageTitle();
	// TIP
	if(!pMenu->GameServer()->m_apPlayers[ClientID]->m_HideTip)
	{
		pMenu->AddOptionLocalize(_C("If you don't want to close menu when you use a option,", "Vote Main Menu"), "DISPLAY", "");
		pMenu->AddOptionLocalize(_C("then you can input this in your console:", "Vote Main Menu"), "DISPLAY", "");
		pMenu->AddOption("ui_close_window_after_changing_setting 0", "DISPLAY", "");

		pMenu->AddOptionLocalize(_C("(Click this to hide this tip)", "Vote Main Menu"), "HIDDEN", "");

		pMenu->AddHorizontalRule();
	}
	// player stats
	{
		pMenu->AddOptionLocalizeFormat(_C("Name: {}", "Vote Main Menu"), "DISPLAY", "-", pMenu->Server()->ClientName(ClientID));
		pMenu->AddOptionLocalizeFormat(_C("Level: {}", "Vote Main Menu"), "DISPLAY", "-", pMenu->Server()->ClientScore(ClientID));
		if(DisplayAddr)
		{
			char aAddr[NETADDR_MAXSTRSIZE];
			pMenu->Server()->GetClientAddr(ClientID, aAddr, sizeof(aAddr));
			pMenu->AddOptionLocalizeFormat(_C("IP Address: {} (Click to hide)", "Vote Main Menu"), "DISPLAY", "-", aAddr);
		}
		else
		{
			pMenu->AddOptionLocalize(_C("IP Address: (Click to display)", "Vote Main Menu"), "DISPLAY ADDR", "-");
		}
	}
	pMenu->AddHorizontalRule();
	// options
	{
		pMenu->AddOptionLocalize(_C("Server Vote", "Vote Main Menu"), "PAGE SERVER VOTE", "★");
		pMenu->AddOptionLocalize(_C("Language Settings", "Vote Main Menu"), "PAGE LANGUAGE", "★");
	}

	return true;
}

bool CGameMenu::MenuLanguage(int ClientID, SCallVoteStatus &VoteStatus, class CGameMenu *pMenu, void *pUserData)
{
	// refresh
	if(VoteStatus.m_aCmd[0])
	{
		if(str_startswith(VoteStatus.m_aCmd, "LANGUAGE "))
		{
			const char *pCode = VoteStatus.m_aCmd + str_length("LANGUAGE ");
			if(str_comp(pMenu->Server()->ClientLanguage(ClientID), pCode) == 0)
				return false;
			pMenu->Server()->SetClientLanguage(ClientID, pCode);
		}
	}

	pMenu->ClearOptions(ClientID);
	pMenu->AddPageTitle();
	{
		SLanguageInfo *pInfo = nullptr;
		if(size_t LanguagesNum = pMenu->Server()->GetLanguagesInfo(&pInfo))
		{
			char aCommand[VOTE_CMD_LENGTH];
			for(size_t Index = 0; Index < LanguagesNum; Index++)
			{
				str_format(aCommand, sizeof(aCommand), "LANGUAGE %s", pInfo[Index].m_pCode);
				if(str_comp(pMenu->Server()->ClientLanguage(ClientID), pInfo[Index].m_pCode) == 0)
					pMenu->AddOption(pInfo[Index].m_pName, aCommand, ">");
				else
					pMenu->AddOption(pInfo[Index].m_pName, aCommand, "-");
			}
			delete[] pInfo;
		}
		else
		{
			pMenu->AddOptionLocalize(_("Oops, couldn't find any language. (Click to refresh)"), "DISPLAY");
		}
	}

	return true;
}

void CGameMenu::AddPageTitle()
{
	if(m_CurrentClientID < 0 || m_CurrentClientID >= SERVER_MAX_CLIENTS)
		return;
	if(!m_upMenuPages.count(m_aPlayerData[m_CurrentClientID].m_CurrentPage))
		return;

	AddOption("===============================", "NONE", "");
	AddOptionLocalize(m_upMenuPages[m_aPlayerData[m_CurrentClientID].m_CurrentPage]->m_aTitle, m_upMenuPages[m_aPlayerData[m_CurrentClientID].m_CurrentPage]->m_aContext, "NONE", "=");
	AddOption("===============================", "NONE", "");
}

void CGameMenu::AddSpace()
{
	AddOption(" ", "NONE");
}

void CGameMenu::AddHorizontalRule()
{
	AddOption("−−−−−−−−−−−−−−−−−−−−−", "NONE");
}

void CGameMenu::AddOption(const char *pDesc, const char *pCommand, const char *pPrefix)
{
	if(m_CurrentClientID < 0 || m_CurrentClientID >= SERVER_MAX_CLIENTS)
		return;
	if(!pDesc || !pCommand)
		return;
	if(!pDesc[0] || !pCommand[0])
		return;
	// add the option
	++m_aPlayerData[m_CurrentClientID].m_NumVoteOptions;
	int Len = str_length(pCommand);

	CVoteOptionServer *pOption = (CVoteOptionServer *) m_aPlayerData[m_CurrentClientID].m_pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
	pOption->m_pNext = 0;
	pOption->m_pPrev = m_aPlayerData[m_CurrentClientID].m_pVoteOptionLast;
	if(pOption->m_pPrev)
		pOption->m_pPrev->m_pNext = pOption;
	m_aPlayerData[m_CurrentClientID].m_pVoteOptionLast = pOption;
	if(!m_aPlayerData[m_CurrentClientID].m_pVoteOptionFirst)
		m_aPlayerData[m_CurrentClientID].m_pVoteOptionFirst = pOption;

	if(pPrefix && pPrefix[0])
		str_format(pOption->m_aDescription, sizeof(pOption->m_aDescription), "%s %s", pPrefix, pDesc);
	else
		str_copy(pOption->m_aDescription, pDesc, sizeof(pOption->m_aDescription));
	mem_copy(pOption->m_aCommand, pCommand, Len + 1);
}

void CGameMenu::AddOptionLocalize(const char *pDesc, const char *pContext, const char *pCommand, const char *pPrefix)
{
	if(m_CurrentClientID < 0 || m_CurrentClientID >= SERVER_MAX_CLIENTS)
		return;
	if(!pDesc || !pCommand)
		return;
	if(!pDesc[0] || !pCommand[0])
		return;
	AddOption(Localize(pDesc, pContext), pCommand, pPrefix);
}

const char *CGameMenu::Localize(const char *pStr, const char *pContext)
{
	if(m_CurrentClientID == -1)
		return pStr;

	return Server()->Localize(m_CurrentClientID, pStr, pContext);
}

CGameMenu::CPlayerData::CPlayerData()
{
	m_pVoteOptionHeap = new CHeap();
	Reset(true);
}

CGameMenu::CPlayerData::~CPlayerData()
{
	delete m_pVoteOptionHeap;
}
