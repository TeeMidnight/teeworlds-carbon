/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2022-2025 Dennis Felsing
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include "register.h"

#include <base/types.h>
#include <base/uuid.h>

#include <engine/console.h>
#include <engine/engine.h>
#include <engine/shared/config.h>
#include <engine/shared/http.h>
#include <engine/shared/jsonparser.h>
#include <engine/shared/masterserver.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>

class CRegister : public IRegister
{
	enum
	{
		STATUS_NONE = 0,
		STATUS_OK,
		STATUS_NEEDCHALLENGE,
		STATUS_NEEDINFO,
		STATUS_ERROR,

		PROTOCOL_TW6_IPV6 = 0,
		PROTOCOL_TW6_IPV4,
		PROTOCOL_TW7_IPV6,
		PROTOCOL_TW7_IPV4,
		NUM_PROTOCOLS,
	};

	static bool StatusFromString(int *pResult, const char *pString);
	static const char *ProtocolToScheme(int Protocol);
	static const char *ProtocolToString(int Protocol);
	static bool ProtocolFromString(int *pResult, const char *pString);
	static const char *ProtocolToSystem(int Protocol);
	static IPRESOLVE ProtocolToIpresolve(int Protocol);

	static void ConchainOnConfigChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	class CGlobal
	{
	public:
		LOCK m_Lock;
		int m_InfoSerial;
		int m_LatestSuccessfulInfoSerial;

		CGlobal()
		{
			m_Lock = lock_create();
		}

		~CGlobal()
		{
			lock_destroy(m_Lock);
		}
	};

	class CProtocol
	{
		class CShared
		{
		public:
			CShared(CGlobal *pGlobal) :
				m_pGlobal(pGlobal)
			{
				m_Lock = lock_create();
			}

			CShared()
			{
				lock_destroy(m_Lock);
			}

			CGlobal *m_pGlobal;
			LOCK m_Lock;
			int m_NumTotalRequests;
			int m_LatestResponseStatus;
			int m_LatestResponseIndex;
		};

		struct SRequestJob
		{
			int m_Protocol;
			int m_ServerPort;
			int m_Index;
			int m_InfoSerial;
			CShared *m_pShared;
			CHttpRequest *m_pRegister;
			IHttp *m_pHttp;
			CJob m_Job;
		};
		SRequestJob m_Job;

		static int RequestThread(void *pUser);

		CRegister *m_pParent;
		int m_Protocol;

		CShared *m_pShared;
		bool m_NewChallengeToken;
		bool m_HaveChallengeToken;
		char m_aChallengeToken[128];

		void CheckChallengeStatus();

	public:
		int64_t m_PrevRegister;
		int64_t m_NextRegister;

		CProtocol(CRegister *pParent, int Protocol);
		~CProtocol()
		{
			delete m_pShared;
		}
		void OnToken(const char *pToken);
		void SendRegister();
		void SendDeleteIfRegistered(bool Shutdown);
		void Update();
	};

	CConfig *m_pConfig;
	IConsole *m_pConsole;
	IEngine *m_pEngine;
	IHttp *m_pHttp;

	// Don't start sending registers before the server has initialized
	// completely.
	bool m_GotFirstUpdateCall;
	int m_ServerPort;
	char m_aConnlessTokenHex[16];

	CGlobal *m_pGlobal;
	bool m_aProtocolEnabled[NUM_PROTOCOLS];
	CProtocol *m_apProtocols[NUM_PROTOCOLS];

	int m_NumExtraHeaders;
	char m_aaExtraHeaders[8][128];

	char m_aVerifyPacketPrefix[sizeof(SERVERBROWSE_CHALLENGE) + UUID_MAXSTRSIZE];
	Uuid m_Secret;
	Uuid m_ChallengeSecret;
	bool m_GotServerInfo;
	char m_aServerInfo[16384];

public:
	CRegister(CConfig *pConfig, IConsole *pConsole, IEngine *pEngine, IHttp *pHttp, int ServerPort, unsigned SixupSecurityToken);
	~CRegister()
	{
		delete m_pGlobal;
		for(auto &pProtocol : m_apProtocols)
			delete pProtocol;
	}
	void Update() override;
	void OnConfigChange() override;
	bool OnPacket(const CNetChunk *pPacket) override;
	void OnNewInfo(const char *pInfo) override;
	void OnShutdown() override;
};

bool CRegister::StatusFromString(int *pResult, const char *pString)
{
	if(str_comp(pString, "success") == 0)
	{
		*pResult = STATUS_OK;
	}
	else if(str_comp(pString, "need_challenge") == 0)
	{
		*pResult = STATUS_NEEDCHALLENGE;
	}
	else if(str_comp(pString, "need_info") == 0)
	{
		*pResult = STATUS_NEEDINFO;
	}
	else if(str_comp(pString, "error") == 0)
	{
		*pResult = STATUS_ERROR;
	}
	else
	{
		*pResult = -1;
		return true;
	}
	return false;
}

const char *CRegister::ProtocolToScheme(int Protocol)
{
	switch(Protocol)
	{
	case PROTOCOL_TW6_IPV6: return "tw-0.6+udp://";
	case PROTOCOL_TW6_IPV4: return "tw-0.6+udp://";
	case PROTOCOL_TW7_IPV6: return "tw-0.7+udp://";
	case PROTOCOL_TW7_IPV4: return "tw-0.7+udp://";
	}
	dbg_assert(0, "invalid protocol");
	dbg_break();

	return "";
}

const char *CRegister::ProtocolToString(int Protocol)
{
	switch(Protocol)
	{
	case PROTOCOL_TW6_IPV6: return "tw0.6/ipv6";
	case PROTOCOL_TW6_IPV4: return "tw0.6/ipv4";
	case PROTOCOL_TW7_IPV6: return "tw0.7/ipv6";
	case PROTOCOL_TW7_IPV4: return "tw0.7/ipv4";
	}
	dbg_assert(0, "invalid protocol");
	dbg_break();

	return "";
}

bool CRegister::ProtocolFromString(int *pResult, const char *pString)
{
	if(str_comp(pString, "tw0.6/ipv6") == 0)
	{
		*pResult = PROTOCOL_TW6_IPV6;
	}
	else if(str_comp(pString, "tw0.6/ipv4") == 0)
	{
		*pResult = PROTOCOL_TW6_IPV4;
	}
	else if(str_comp(pString, "tw0.7/ipv6") == 0)
	{
		*pResult = PROTOCOL_TW7_IPV6;
	}
	else if(str_comp(pString, "tw0.7/ipv4") == 0)
	{
		*pResult = PROTOCOL_TW7_IPV4;
	}
	else
	{
		*pResult = -1;
		return true;
	}
	return false;
}

const char *CRegister::ProtocolToSystem(int Protocol)
{
	switch(Protocol)
	{
	case PROTOCOL_TW6_IPV6: return "register/6/ipv6";
	case PROTOCOL_TW6_IPV4: return "register/6/ipv4";
	case PROTOCOL_TW7_IPV6: return "register/7/ipv6";
	case PROTOCOL_TW7_IPV4: return "register/7/ipv4";
	}
	dbg_assert(0, "invalid protocol");
	dbg_break();

	return "";
}

IPRESOLVE CRegister::ProtocolToIpresolve(int Protocol)
{
	switch(Protocol)
	{
	case PROTOCOL_TW6_IPV6: return IPRESOLVE::V6;
	case PROTOCOL_TW6_IPV4: return IPRESOLVE::V4;
	case PROTOCOL_TW7_IPV6: return IPRESOLVE::V6;
	case PROTOCOL_TW7_IPV4: return IPRESOLVE::V4;
	}
	dbg_assert(0, "invalid protocol");
	dbg_break();

	return IPRESOLVE::WHATEVER;
}

void CRegister::ConchainOnConfigChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		((CRegister *) pUserData)->OnConfigChange();
	}
}

void CRegister::CProtocol::SendRegister()
{
	int64_t Now = time_get();
	int64_t Freq = time_freq();

	char aAddress[64];
	str_format(aAddress, sizeof(aAddress), "%sconnecting-address.invalid:%d", ProtocolToScheme(m_Protocol), m_pParent->m_ServerPort);

	char aSecret[UUID_MAXSTRSIZE];
	FormatUuid(m_pParent->m_Secret, aSecret, sizeof(aSecret));

	char aChallengeUuid[UUID_MAXSTRSIZE];
	FormatUuid(m_pParent->m_ChallengeSecret, aChallengeUuid, sizeof(aChallengeUuid));
	char aChallengeSecret[64];
	str_format(aChallengeSecret, sizeof(aChallengeSecret), "%s:%s", aChallengeUuid, ProtocolToString(m_Protocol));
	int InfoSerial;
	bool SendInfo;

	lock_wait(m_pShared->m_pGlobal->m_Lock);
	InfoSerial = m_pShared->m_pGlobal->m_InfoSerial;
	SendInfo = InfoSerial > m_pShared->m_pGlobal->m_LatestSuccessfulInfoSerial;
	lock_unlock(m_pShared->m_pGlobal->m_Lock);

	CHttpRequest *pRegister;
	if(SendInfo)
	{
		pRegister = HttpPostJson(m_pParent->m_pConfig->m_SvRegisterUrl, m_pParent->m_pConfig, m_pParent->m_aServerInfo);
	}
	else
	{
		pRegister = HttpPost(m_pParent->m_pConfig->m_SvRegisterUrl, m_pParent->m_pConfig, (unsigned char *) "", 0);
	}
	pRegister->HeaderString("Address", aAddress);
	pRegister->HeaderString("Secret", aSecret);
	if(m_Protocol == PROTOCOL_TW7_IPV6 || m_Protocol == PROTOCOL_TW7_IPV4)
	{
		pRegister->HeaderString("Connless-Token", m_pParent->m_aConnlessTokenHex);
	}
	pRegister->HeaderString("Challenge-Secret", aChallengeSecret);
	if(m_HaveChallengeToken)
	{
		pRegister->HeaderString("Challenge-Token", m_aChallengeToken);
	}
	pRegister->HeaderInt("Info-Serial", InfoSerial);
	for(int i = 0; i < m_pParent->m_NumExtraHeaders; i++)
	{
		pRegister->Header(m_pParent->m_aaExtraHeaders[i]);
	}
	pRegister->LogProgress(HTTPLOG::FAILURE);
	pRegister->IpResolve(ProtocolToIpresolve(m_Protocol));
	pRegister->FailOnErrorStatus(false);

	int RequestIndex;
	lock_wait(m_pShared->m_Lock);
	if(m_pShared->m_LatestResponseStatus != STATUS_OK)
	{
		dbg_msg(ProtocolToSystem(m_Protocol), "registering...");
	}
	RequestIndex = m_pShared->m_NumTotalRequests;
	m_pShared->m_NumTotalRequests += 1;
	lock_unlock(m_pShared->m_Lock);

	m_Job = {m_Protocol, m_pParent->m_ServerPort, RequestIndex, InfoSerial, m_pShared, std::move(pRegister), m_pParent->m_pHttp, CJob()};
	m_pParent->m_pEngine->AddJob(&m_Job.m_Job, CProtocol::RequestThread, &m_Job);
	m_NewChallengeToken = false;

	m_PrevRegister = Now;
	m_NextRegister = Now + 15 * Freq;
}

void CRegister::CProtocol::SendDeleteIfRegistered(bool Shutdown)
{
	lock_wait(m_pShared->m_Lock);
	const bool ShouldSendDelete = m_pShared->m_LatestResponseStatus == STATUS_OK;
	m_pShared->m_LatestResponseStatus = STATUS_NONE;
	lock_unlock(m_pShared->m_Lock);

	if(!ShouldSendDelete)
		return;

	char aAddress[64];
	str_format(aAddress, sizeof(aAddress), "%sconnecting-address.invalid:%d", ProtocolToScheme(m_Protocol), m_pParent->m_ServerPort);

	char aSecret[UUID_MAXSTRSIZE];
	FormatUuid(m_pParent->m_Secret, aSecret, sizeof(aSecret));

	CHttpRequest *pDelete = HttpPost(m_pParent->m_pConfig->m_SvRegisterUrl, m_pParent->m_pConfig, (const unsigned char *) "", 0);
	pDelete->HeaderString("Action", "delete");
	pDelete->HeaderString("Address", aAddress);
	pDelete->HeaderString("Secret", aSecret);
	for(int i = 0; i < m_pParent->m_NumExtraHeaders; i++)
	{
		pDelete->Header(m_pParent->m_aaExtraHeaders[i]);
	}
	pDelete->LogProgress(HTTPLOG::FAILURE);
	pDelete->IpResolve(ProtocolToIpresolve(m_Protocol));
	if(Shutdown)
	{
		// On shutdown, wait at most 1 second for the delete requests.
		pDelete->Timeout(CTimeout{1000, 1000, 0, 0});
	}
	dbg_msg(ProtocolToSystem(m_Protocol), "deleting...");
	m_pParent->m_pHttp->Run(pDelete);
	delete pDelete;
}

CRegister::CProtocol::CProtocol(CRegister *pParent, int Protocol) :
	m_pParent(pParent),
	m_Protocol(Protocol),
	m_pShared(new CShared(pParent->m_pGlobal)),
	m_NewChallengeToken(false),
	m_HaveChallengeToken(false),
	m_PrevRegister(-1),
	m_NextRegister(-1)
{
	mem_zero(m_aChallengeToken, sizeof(m_aChallengeToken));
}

void CRegister::CProtocol::CheckChallengeStatus()
{
	lock_wait(m_pShared->m_Lock);
	// No requests in flight?
	if(m_pShared->m_LatestResponseIndex == m_pShared->m_NumTotalRequests - 1)
	{
		switch(m_pShared->m_LatestResponseStatus)
		{
		case STATUS_NEEDCHALLENGE:
			if(m_NewChallengeToken)
			{
				// Immediately resend if we got the token.
				m_NextRegister = time_get();
			}
			break;
		case STATUS_NEEDINFO:
			// Act immediately if the master requests more info.
			m_NextRegister = time_get();
			break;
		}
	}
	lock_unlock(m_pShared->m_Lock);
}

void CRegister::CProtocol::Update()
{
	CheckChallengeStatus();
	if(time_get() >= m_NextRegister)
	{
		SendRegister();
	}
}

void CRegister::CProtocol::OnToken(const char *pToken)
{
	m_NewChallengeToken = true;
	m_HaveChallengeToken = true;
	str_copy(m_aChallengeToken, pToken, sizeof(m_aChallengeToken));

	CheckChallengeStatus();
	if(time_get() >= m_NextRegister)
	{
		SendRegister();
	}
}

int CRegister::CProtocol::RequestThread(void *pUser)
{
	SRequestJob *pJob = (SRequestJob *) pUser;

	pJob->m_pHttp->Run(pJob->m_pRegister);
	pJob->m_pRegister->Wait();
	if(pJob->m_pRegister->State() != EHttpState::DONE)
	{
		// TODO: exponential backoff
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "error sending request to master");
		return -1;
	}
	json_value *pJson = pJob->m_pRegister->ResultJson();
	if(!pJson)
	{
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "non-JSON response from master");
		return -1;
	}
	const json_value &Json = *pJson;
	const json_value &StatusString = Json["status"];
	if(StatusString.type != json_string)
	{
		json_value_free(pJson);
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "invalid JSON response from master");
		return -1;
	}
	int Status;
	if(StatusFromString(&Status, StatusString))
	{
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "invalid status from master: %s", (const char *) StatusString);
		json_value_free(pJson);
		return -1;
	}
	if(Status == STATUS_ERROR)
	{
		const json_value &Message = Json["message"];
		if(Message.type != json_string)
		{
			json_value_free(pJson);
			dbg_msg(ProtocolToSystem(pJob->m_Protocol), "invalid JSON error response from master");
			return -1;
		}
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "error response from master: %d: %s", pJob->m_pRegister->StatusCode(), (const char *) Message);
		json_value_free(pJson);
		return -1;
	}
	if(pJob->m_pRegister->StatusCode() >= 400)
	{
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "non-success status code %d from master without error code", pJob->m_pRegister->StatusCode());
		json_value_free(pJson);
		return -1;
	}

	lock_wait(pJob->m_pShared->m_Lock);
	if(Status != STATUS_OK || Status != pJob->m_pShared->m_LatestResponseStatus)
	{
		if(pJob->m_pRegister->Config()->m_Debug)
			dbg_msg(ProtocolToSystem(pJob->m_Protocol), "status: %s", (const char *) StatusString);
	}
	if(Status == pJob->m_pShared->m_LatestResponseStatus && Status == STATUS_NEEDCHALLENGE)
	{
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "ERROR: the master server reports that clients can not connect to this server.");
		dbg_msg(ProtocolToSystem(pJob->m_Protocol), "ERROR: configure your firewall/nat to let through udp on port %d.", pJob->m_ServerPort);
	}
	json_value_free(pJson);
	if(pJob->m_Index > pJob->m_pShared->m_LatestResponseIndex)
	{
		pJob->m_pShared->m_LatestResponseIndex = pJob->m_Index;
		pJob->m_pShared->m_LatestResponseStatus = Status;
	}
	lock_unlock(pJob->m_pShared->m_Lock);

	if(Status == STATUS_OK)
	{
		lock_wait(pJob->m_pShared->m_pGlobal->m_Lock);
		if(pJob->m_InfoSerial > pJob->m_pShared->m_pGlobal->m_LatestSuccessfulInfoSerial)
		{
			pJob->m_pShared->m_pGlobal->m_LatestSuccessfulInfoSerial = pJob->m_InfoSerial;
		}
		lock_unlock(pJob->m_pShared->m_pGlobal->m_Lock);
	}
	else if(Status == STATUS_NEEDINFO)
	{
		lock_wait(pJob->m_pShared->m_pGlobal->m_Lock);
		if(pJob->m_InfoSerial == pJob->m_pShared->m_pGlobal->m_LatestSuccessfulInfoSerial)
		{
			// Tell other requests that they need to send the info again.
			pJob->m_pShared->m_pGlobal->m_LatestSuccessfulInfoSerial -= 1;
		}
		lock_unlock(pJob->m_pShared->m_pGlobal->m_Lock);
	}
	return 0;
}

CRegister::CRegister(CConfig *pConfig, IConsole *pConsole, IEngine *pEngine, IHttp *pHttp, int ServerPort, unsigned SixupSecurityToken) :
	m_pConfig(pConfig),
	m_pConsole(pConsole),
	m_pEngine(pEngine),
	m_pHttp(pHttp),
	m_GotFirstUpdateCall(false),
	m_ServerPort(ServerPort),
	m_pGlobal(new CGlobal()),
	m_NumExtraHeaders(0),
	m_GotServerInfo(false)
{
	const int HEADER_LEN = sizeof(SERVERBROWSE_CHALLENGE);
	mem_copy(m_aVerifyPacketPrefix, SERVERBROWSE_CHALLENGE, HEADER_LEN);
	m_Secret = RandomUuid();
	m_ChallengeSecret = RandomUuid();
	FormatUuid(m_ChallengeSecret, m_aVerifyPacketPrefix + HEADER_LEN, sizeof(m_aVerifyPacketPrefix) - HEADER_LEN);
	m_aVerifyPacketPrefix[HEADER_LEN + UUID_MAXSTRSIZE - 1] = ':';

	// The DDNet code uses the `unsigned` security token in big-endian byte order.
	str_format(m_aConnlessTokenHex, sizeof(m_aConnlessTokenHex), "%08x", SixupSecurityToken);

	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		m_aProtocolEnabled[i] = true;
		m_apProtocols[i] = new CProtocol(this, i);
	}
	m_aProtocolEnabled[PROTOCOL_TW6_IPV6] = false;
	m_aProtocolEnabled[PROTOCOL_TW6_IPV4] = false;

	mem_zero(m_aaExtraHeaders, sizeof(m_aaExtraHeaders));
	mem_zero(m_aServerInfo, sizeof(m_aServerInfo));

	m_pConsole->Chain("sv_register", ConchainOnConfigChange, this);
	m_pConsole->Chain("sv_register_extra", ConchainOnConfigChange, this);
	m_pConsole->Chain("sv_register_url", ConchainOnConfigChange, this);
}

void CRegister::Update()
{
	if(!m_GotFirstUpdateCall)
	{
		bool Ipv6 = m_aProtocolEnabled[PROTOCOL_TW6_IPV6] || m_aProtocolEnabled[PROTOCOL_TW7_IPV6];
		bool Ipv4 = m_aProtocolEnabled[PROTOCOL_TW6_IPV4] || m_aProtocolEnabled[PROTOCOL_TW7_IPV4];
		if(Ipv6 && Ipv4)
		{
			dbg_assert(!HttpHasIpresolveBug(), "curl version < 7.77.0 does not support registering via both IPv4 and IPv6, set `sv_register ipv6` or `sv_register ipv4`");
		}
		m_GotFirstUpdateCall = true;
	}
	if(!m_GotServerInfo)
	{
		return;
	}
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		if(!m_aProtocolEnabled[i])
		{
			continue;
		}
		m_apProtocols[i]->Update();
	}
}

void CRegister::OnConfigChange()
{
	bool aOldProtocolEnabled[NUM_PROTOCOLS];
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		aOldProtocolEnabled[i] = m_aProtocolEnabled[i];
	}
	const char *pProtocols = m_pConfig->m_SvRegister;
	if(str_comp(pProtocols, "1") == 0)
	{
		for(int i = 0; i < NUM_PROTOCOLS; i++)
		{
			m_aProtocolEnabled[i] = true;
		}
	}
	else if(str_comp(pProtocols, "0") == 0)
	{
		for(int i = 0; i < NUM_PROTOCOLS; i++)
		{
			m_aProtocolEnabled[i] = false;
		}
	}
	else
	{
		for(int i = 0; i < NUM_PROTOCOLS; i++)
		{
			m_aProtocolEnabled[i] = false;
		}
		char aBuf[16];
		while((pProtocols = str_next_token(pProtocols, ",", aBuf, sizeof(aBuf))))
		{
			int Protocol;
			if(str_comp(aBuf, "ipv6") == 0)
			{
				m_aProtocolEnabled[PROTOCOL_TW6_IPV6] = true;
				m_aProtocolEnabled[PROTOCOL_TW7_IPV6] = true;
			}
			else if(str_comp(aBuf, "ipv4") == 0)
			{
				m_aProtocolEnabled[PROTOCOL_TW6_IPV4] = true;
				m_aProtocolEnabled[PROTOCOL_TW7_IPV4] = true;
			}
			else if(str_comp(aBuf, "tw0.6") == 0)
			{
				m_aProtocolEnabled[PROTOCOL_TW6_IPV6] = true;
				m_aProtocolEnabled[PROTOCOL_TW6_IPV4] = true;
			}
			else if(str_comp(aBuf, "tw0.7") == 0)
			{
				m_aProtocolEnabled[PROTOCOL_TW7_IPV6] = true;
				m_aProtocolEnabled[PROTOCOL_TW7_IPV4] = true;
			}
			else if(!ProtocolFromString(&Protocol, aBuf))
			{
				m_aProtocolEnabled[Protocol] = true;
			}
			else
			{
				dbg_msg("register", "unknown protocol '%s'", aBuf);
				continue;
			}
		}
	}
	m_aProtocolEnabled[PROTOCOL_TW6_IPV6] = false;
	m_aProtocolEnabled[PROTOCOL_TW6_IPV4] = false;

	m_NumExtraHeaders = 0;
	const char *pRegisterExtra = m_pConfig->m_SvRegisterExtra;
	char aHeader[128];
	while((pRegisterExtra = str_next_token(pRegisterExtra, ",", aHeader, sizeof(aHeader))))
	{
		if(m_NumExtraHeaders == 8)
		{
			dbg_msg("register", "reached maximum of %d extra headers, dropping '%s' and all further headers", m_NumExtraHeaders, aHeader);
			break;
		}
		if(!str_find(aHeader, ": "))
		{
			dbg_msg("register", "header '%s' doesn't contain mandatory ': ', ignoring", aHeader);
			continue;
		}
		str_copy(m_aaExtraHeaders[m_NumExtraHeaders], aHeader, sizeof(m_aaExtraHeaders[m_NumExtraHeaders]));
		m_NumExtraHeaders += 1;
	}
	// Don't start registering before the first `CRegister::Update` call.
	if(!m_GotFirstUpdateCall)
	{
		return;
	}
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		if(aOldProtocolEnabled[i] == m_aProtocolEnabled[i])
		{
			continue;
		}
		if(m_aProtocolEnabled[i])
		{
			m_apProtocols[i]->SendRegister();
		}
		else
		{
			m_apProtocols[i]->SendDeleteIfRegistered(false);
		}
	}
}

bool CRegister::OnPacket(const CNetChunk *pPacket)
{
	if((pPacket->m_Flags & NETSENDFLAG_CONNLESS) == 0)
	{
		return false;
	}
	if(pPacket->m_DataSize >= (int) sizeof(m_aVerifyPacketPrefix) &&
		mem_comp(pPacket->m_pData, m_aVerifyPacketPrefix, sizeof(m_aVerifyPacketPrefix)) == 0)
	{
		CUnpacker Unpacker;
		Unpacker.Reset(pPacket->m_pData, pPacket->m_DataSize);
		Unpacker.GetRaw(sizeof(m_aVerifyPacketPrefix));
		const char *pProtocol = Unpacker.GetString(0);
		const char *pToken = Unpacker.GetString(0);
		if(Unpacker.Error())
		{
			dbg_msg("register", "got erroneous challenge packet from master");
			return true;
		}

		if(m_pConfig->m_Debug)
			dbg_msg("register", "got challenge token, protocol='%s' token='%s'", pProtocol, pToken);
		int Protocol;
		if(ProtocolFromString(&Protocol, pProtocol))
		{
			dbg_msg("register", "got challenge packet with unknown protocol");
			return true;
		}
		m_apProtocols[Protocol]->OnToken(pToken);
		return true;
	}
	return false;
}

void CRegister::OnNewInfo(const char *pInfo)
{
	if(m_pConfig->m_Debug)
		dbg_msg("register", "info: %s", pInfo);
	if(m_GotServerInfo && str_comp(m_aServerInfo, pInfo) == 0)
	{
		return;
	}

	m_GotServerInfo = true;
	str_copy(m_aServerInfo, pInfo, sizeof(m_aServerInfo));
	lock_wait(m_pGlobal->m_Lock);
	m_pGlobal->m_InfoSerial += 1;
	lock_unlock(m_pGlobal->m_Lock);

	// Don't start registering before the first `CRegister::Update` call.
	if(!m_GotFirstUpdateCall)
	{
		return;
	}

	// Immediately send new info if it changes, but at most once per second.
	int64_t Now = time_get();
	int64_t Freq = time_freq();
	int64_t MaximumPrevRegister = -1;
	int64_t MinimumNextRegister = -1;
	int MinimumNextRegisterProtocol = -1;
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		if(!m_aProtocolEnabled[i])
		{
			continue;
		}
		if(m_apProtocols[i]->m_NextRegister == -1)
		{
			m_apProtocols[i]->m_NextRegister = Now;
			continue;
		}
		if(m_apProtocols[i]->m_PrevRegister > MaximumPrevRegister)
		{
			MaximumPrevRegister = m_apProtocols[i]->m_PrevRegister;
		}
		if(MinimumNextRegisterProtocol == -1 || m_apProtocols[i]->m_NextRegister < MinimumNextRegister)
		{
			MinimumNextRegisterProtocol = i;
			MinimumNextRegister = m_apProtocols[i]->m_NextRegister;
		}
	}
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		if(!m_aProtocolEnabled[i])
		{
			continue;
		}
		if(i == MinimumNextRegisterProtocol)
		{
			int64_t NextRegister = m_apProtocols[i]->m_NextRegister;
			if(MaximumPrevRegister + Freq < NextRegister)
				NextRegister = MaximumPrevRegister + Freq;
			m_apProtocols[i]->m_NextRegister = NextRegister;
		}
		if(Now >= m_apProtocols[i]->m_NextRegister)
		{
			m_apProtocols[i]->SendRegister();
		}
	}
}

void CRegister::OnShutdown()
{
	for(int i = 0; i < NUM_PROTOCOLS; i++)
	{
		if(!m_aProtocolEnabled[i])
		{
			continue;
		}
		m_apProtocols[i]->SendDeleteIfRegistered(true);
	}
}

IRegister *CreateRegister(CConfig *pConfig, IConsole *pConsole, IEngine *pEngine, IHttp *pHttp, int ServerPort, unsigned SixupSecurityToken)
{
	return new CRegister(pConfig, pConsole, pEngine, pHttp, ServerPort, SixupSecurityToken);
}
