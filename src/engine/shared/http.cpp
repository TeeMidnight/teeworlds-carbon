/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2018-2025 Dennis Felsing
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#include "http.h"

#include <base/math.h>
#include <base/tl/threading.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>
#include <engine/storage.h>
#include <game/version.h>

#include <climits>

#if !defined(CONF_FAMILY_WINDOWS)
#include <csignal>
#endif

// There is a stray constant on Windows/MSVC...
#ifdef ERROR
#undef ERROR
#endif

#include <curl/curl.h>

int CurlDebug(CURL *pHandle, curl_infotype Type, char *pData, size_t DataSize, void *pUser)
{
	char TypeChar;
	switch(Type)
	{
	case CURLINFO_TEXT:
		TypeChar = '*';
		break;
	case CURLINFO_HEADER_OUT:
		TypeChar = '<';
		break;
	case CURLINFO_HEADER_IN:
		TypeChar = '>';
		break;
	default:
		return 0;
	}
	while(const char *pLineEnd = (const char *) memchr(pData, '\n', DataSize))
	{
		int LineLength = pLineEnd - pData;
		dbg_msg("curl", "%c %.*s", TypeChar, LineLength, pData);
		pData += LineLength + 1;
		DataSize -= LineLength + 1;
	}
	return 0;
}

void EscapeUrl(char *pBuf, int Size, const char *pStr)
{
	char *pEsc = curl_easy_escape(nullptr, pStr, 0);
	str_copy(pBuf, pEsc, Size);
	curl_free(pEsc);
}

bool HttpHasIpresolveBug()
{
	// curl < 7.77.0 doesn't use CURLOPT_IPRESOLVE correctly wrt.
	// connection caches.
	return curl_version_info(CURLVERSION_NOW)->version_num < 0x074d00;
}

CHttpRequest::CHttpRequest(const char *pUrl, CConfig *pConfig) :
	m_pConfig(pConfig)
{
	str_copy(m_aUrl, pUrl, sizeof(m_aUrl));
	sha256_init(&m_ActualSha256Ctx);
	m_pWaitLock = new lock();
	m_pWaitSema = new semaphore();
}

CHttpRequest::~CHttpRequest()
{
	dbg_assert(m_File == nullptr, "HTTP request file was not closed");
	mem_free(m_pBuffer);
	curl_slist_free_all((curl_slist *) m_pHeaders);
	mem_free(m_pBody);
	if(m_State == EHttpState::DONE && m_ValidateBeforeOverwrite)
	{
		OnValidation(false);
	}
	delete m_pWaitLock;
	delete m_pWaitSema;
}

static bool CalculateSha256(const char *pAbsoluteFilename, SHA256_DIGEST *pSha256)
{
	IOHANDLE File = io_open(pAbsoluteFilename, IOFLAG_READ);
	if(!File)
	{
		return false;
	}
	SHA256_CTX Sha256Ctxt;
	sha256_init(&Sha256Ctxt);
	unsigned char aBuffer[64 * 1024];
	while(true)
	{
		unsigned Bytes = io_read(File, aBuffer, sizeof(aBuffer));
		if(Bytes == 0)
			break;
		sha256_update(&Sha256Ctxt, aBuffer, Bytes);
	}
	io_close(File);
	*pSha256 = sha256_finish(&Sha256Ctxt);
	return true;
}

bool CHttpRequest::ShouldSkipRequest()
{
	if(m_WriteToFile && m_ExpectedSha256 != SHA256_ZEROED)
	{
		SHA256_DIGEST Sha256;
		if(CalculateSha256(m_aDestAbsolute, &Sha256) && Sha256 == m_ExpectedSha256)
		{
			dbg_msg("http", "skipping download because expected file already exists: %s", m_aDest);
			return true;
		}
	}
	return false;
}

bool CHttpRequest::BeforeInit()
{
	if(m_WriteToFile)
	{
		if(m_SkipByFileTime)
		{
			time_t FileCreatedTime, FileModifiedTime;
			if(fs_file_time(m_aDestAbsolute, &FileCreatedTime, &FileModifiedTime) == 0)
			{
				m_IfModifiedSince = FileModifiedTime;
			}
		}

		if(fs_makedir_recursive(m_aDestAbsoluteTmp) < 0)
		{
			dbg_msg("http", "i/o error, cannot create folder for: %s", m_aDest);
			return false;
		}

		m_File = io_open(m_aDestAbsoluteTmp, IOFLAG_WRITE);
		if(!m_File)
		{
			dbg_msg("http", "i/o error, cannot open file: %s", m_aDest);
			return false;
		}
	}
	return true;
}

bool CHttpRequest::ConfigureHandle(void *pHandle)
{
	CURL *pH = (CURL *) pHandle;
	if(!BeforeInit())
	{
		return false;
	}

	if(Config()->m_DbgCurl)
	{
		curl_easy_setopt(pH, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(pH, CURLOPT_DEBUGFUNCTION, CurlDebug);
	}
	long Protocols = CURLPROTO_HTTPS;
	if(Config()->m_HttpAllowInsecure)
	{
		Protocols |= CURLPROTO_HTTP;
	}

	curl_easy_setopt(pH, CURLOPT_ERRORBUFFER, m_aErr);

	curl_easy_setopt(pH, CURLOPT_CONNECTTIMEOUT_MS, m_Timeout.ConnectTimeoutMs);
	curl_easy_setopt(pH, CURLOPT_TIMEOUT_MS, m_Timeout.TimeoutMs);
	curl_easy_setopt(pH, CURLOPT_LOW_SPEED_LIMIT, m_Timeout.LowSpeedLimit);
	curl_easy_setopt(pH, CURLOPT_LOW_SPEED_TIME, m_Timeout.LowSpeedTime);
	if(m_MaxResponseSize >= 0)
	{
		curl_easy_setopt(pH, CURLOPT_MAXFILESIZE_LARGE, (curl_off_t) m_MaxResponseSize);
	}
	if(m_IfModifiedSince >= 0)
	{
		curl_easy_setopt(pH, CURLOPT_TIMEVALUE_LARGE, (curl_off_t) m_IfModifiedSince);
		curl_easy_setopt(pH, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
	}

	// ‘CURLOPT_PROTOCOLS’ is deprecated: since 7.85.0. Use CURLOPT_PROTOCOLS_STR
	// Wait until all platforms have 7.85.0
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	curl_easy_setopt(pH, CURLOPT_PROTOCOLS, Protocols);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
	curl_easy_setopt(pH, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pH, CURLOPT_MAXREDIRS, 4L);
	if(m_FailOnErrorStatus)
	{
		curl_easy_setopt(pH, CURLOPT_FAILONERROR, 1L);
	}
	curl_easy_setopt(pH, CURLOPT_URL, m_aUrl);
	curl_easy_setopt(pH, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pH, CURLOPT_USERAGENT, GAME_NAME " " GAME_RELEASE_VERSION " " MOD_NAME " " MOD_VERSION " (" CONF_PLATFORM_STRING "; " CONF_ARCH_STRING ")");
	curl_easy_setopt(pH, CURLOPT_ACCEPT_ENCODING, ""); // Use any compression algorithm supported by libcurl.

	curl_easy_setopt(pH, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(pH, CURLOPT_HEADERFUNCTION, HeaderCallback);
	curl_easy_setopt(pH, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(pH, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(pH, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(pH, CURLOPT_PROGRESSDATA, this);
	// ‘CURLOPT_PROGRESSFUNCTION’ is deprecated: since 7.32.0. Use CURLOPT_XFERINFOFUNCTION
	// See problems with curl_off_t type in header file in https://github.com/ddnet/ddnet/pull/6185/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	curl_easy_setopt(pH, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
	curl_easy_setopt(pH, CURLOPT_IPRESOLVE, m_IpResolve == IPRESOLVE::V4 ? CURL_IPRESOLVE_V4 : m_IpResolve == IPRESOLVE::V6 ? CURL_IPRESOLVE_V6 :
																  CURL_IPRESOLVE_WHATEVER);
	if(Config()->m_Bindaddr[0] != '\0')
	{
		curl_easy_setopt(pH, CURLOPT_INTERFACE, Config()->m_Bindaddr);
	}

	if(curl_version_info(CURLVERSION_NOW)->version_num < 0x074400)
	{
		// Causes crashes, see https://github.com/ddnet/ddnet/issues/4342.
		// No longer a problem in curl 7.68 and above, and 0x44 = 68.
		curl_easy_setopt(pH, CURLOPT_FORBID_REUSE, 1L);
	}

#ifdef CONF_PLATFORM_ANDROID
	curl_easy_setopt(pH, CURLOPT_CAINFO, "data/cacert.pem");
#endif

	switch(m_Type)
	{
	case REQUEST::GET:
		break;
	case REQUEST::HEAD:
		curl_easy_setopt(pH, CURLOPT_NOBODY, 1L);
		break;
	case REQUEST::POST:
	case REQUEST::POST_JSON:
		if(m_Type == REQUEST::POST_JSON)
		{
			Header("Content-Type: application/json");
		}
		else
		{
			Header("Content-Type:");
		}
		curl_easy_setopt(pH, CURLOPT_POSTFIELDS, m_pBody);
		curl_easy_setopt(pH, CURLOPT_POSTFIELDSIZE, m_BodyLength);
		break;
	}

	curl_easy_setopt(pH, CURLOPT_HTTPHEADER, m_pHeaders);

	return true;
}

size_t CHttpRequest::OnHeader(char *pHeader, size_t HeaderSize)
{
	// `pHeader` is NOT null-terminated.
	// `pHeader` has a trailing newline.

	if(HeaderSize <= 1)
	{
		m_HeadersEnded = true;
		return HeaderSize;
	}
	if(m_HeadersEnded)
	{
		// redirect, clear old headers
		m_HeadersEnded = false;
		m_HasDate = false;
		m_HasLastModified = false;
	}

	static const char DATE[] = "Date: ";
	static const char LAST_MODIFIED[] = "Last-Modified: ";

	// Trailing newline and null termination evens out.
	if(HeaderSize - 1 >= sizeof(DATE) - 1 && str_startswith_nocase(pHeader, DATE))
	{
		char aValue[128];
		str_truncate(aValue, sizeof(aValue), pHeader + (sizeof(DATE) - 1), HeaderSize - (sizeof(DATE) - 1) - 1);
		int64_t Value = curl_getdate(aValue, nullptr);
		if(Value != -1)
		{
			m_ResultDate = Value;
			m_HasDate = true;
		}
	}
	if(HeaderSize - 1 >= sizeof(LAST_MODIFIED) - 1 && str_startswith_nocase(pHeader, LAST_MODIFIED))
	{
		char aValue[128];
		str_truncate(aValue, sizeof(aValue), pHeader + (sizeof(LAST_MODIFIED) - 1), HeaderSize - (sizeof(LAST_MODIFIED) - 1) - 1);
		int64_t Value = curl_getdate(aValue, nullptr);
		if(Value != -1)
		{
			m_ResultLastModified = Value;
			m_HasLastModified = true;
		}
	}

	return HeaderSize;
}

size_t CHttpRequest::OnData(char *pData, size_t DataSize)
{
	// Need to check for the maximum response size here as curl can only
	// guarantee it if the server sets a Content-Length header.
	if(m_MaxResponseSize >= 0 && m_ResponseLength + DataSize > (uint64_t) m_MaxResponseSize)
	{
		return 0;
	}

	if(DataSize == 0)
	{
		return DataSize;
	}

	sha256_update(&m_ActualSha256Ctx, pData, DataSize);

	size_t Result = DataSize;

	if(m_WriteToMemory)
	{
		size_t NewBufferSize = maximum((size_t) 1024, m_BufferSize);
		while(m_ResponseLength + DataSize > NewBufferSize)
		{
			NewBufferSize *= 2;
		}
		if(NewBufferSize != m_BufferSize)
		{
			m_pBuffer = (unsigned char *) realloc(m_pBuffer, NewBufferSize);
			m_BufferSize = NewBufferSize;
		}
		mem_copy(m_pBuffer + m_ResponseLength, pData, DataSize);
	}
	if(m_WriteToFile)
	{
		Result = io_write(m_File, pData, DataSize);
	}
	m_ResponseLength += DataSize;
	return Result;
}

size_t CHttpRequest::HeaderCallback(char *pData, size_t Size, size_t Number, void *pUser)
{
	dbg_assert(Size == 1, "invalid size parameter passed to header callback");
	return ((CHttpRequest *) pUser)->OnHeader(pData, Number);
}

size_t CHttpRequest::WriteCallback(char *pData, size_t Size, size_t Number, void *pUser)
{
	return ((CHttpRequest *) pUser)->OnData(pData, Size * Number);
}

int CHttpRequest::ProgressCallback(void *pUser, double DlTotal, double DlCurr, double UlTotal, double UlCurr)
{
	CHttpRequest *pTask = (CHttpRequest *) pUser;
	pTask->m_Current = DlCurr;
	pTask->m_Size = DlTotal;
	pTask->m_Progress = DlTotal == 0.0 ? 0 : (100 * DlCurr) / DlTotal;
	pTask->OnProgress();
	return pTask->m_Abort ? -1 : 0;
}

void CHttpRequest::OnCompletionInternal(void *pHandle, unsigned int Result)
{
	if(pHandle)
	{
		CURL *pH = (CURL *) pHandle;
		long StatusCode;
		curl_easy_getinfo(pH, CURLINFO_RESPONSE_CODE, &StatusCode);
		m_StatusCode = StatusCode;
	}

	EHttpState State;
	const CURLcode Code = static_cast<CURLcode>(Result);
	if(Code != CURLE_OK)
	{
		if(Config()->m_DbgCurl || m_LogProgress >= HTTPLOG::FAILURE)
		{
			dbg_msg("http", "%s failed. libcurl error (%u): %s", m_aUrl, Code, m_aErr);
		}
		State = (Code == CURLE_ABORTED_BY_CALLBACK) ? EHttpState::ABORTED : EHttpState::ERROR;
	}
	else
	{
		if(Config()->m_DbgCurl || m_LogProgress >= HTTPLOG::ALL)
		{
			dbg_msg("http", "task done: %s", m_aUrl);
		}
		State = EHttpState::DONE;
	}

	if(State == EHttpState::DONE)
	{
		m_ActualSha256 = sha256_finish(&m_ActualSha256Ctx);
		if(m_ExpectedSha256 != SHA256_ZEROED && m_ActualSha256 != m_ExpectedSha256)
		{
			if(Config()->m_DbgCurl || m_LogProgress >= HTTPLOG::FAILURE)
			{
				char aActualSha256[SHA256_MAXSTRSIZE];
				sha256_str(m_ActualSha256, aActualSha256, sizeof(aActualSha256));
				char aExpectedSha256[SHA256_MAXSTRSIZE];
				sha256_str(m_ExpectedSha256, aExpectedSha256, sizeof(aExpectedSha256));
				dbg_msg("http", "SHA256 mismatch: got=%s, expected=%s, url=%s", aActualSha256, aExpectedSha256, m_aUrl);
			}
			State = EHttpState::ERROR;
		}
	}

	if(m_WriteToFile)
	{
		if(m_File && io_close(m_File) != 0)
		{
			dbg_msg("http", "i/o error, cannot close file: %s", m_aDest);
			State = EHttpState::ERROR;
		}
		m_File = nullptr;

		if(State == EHttpState::ERROR || State == EHttpState::ABORTED)
		{
			fs_remove(m_aDestAbsoluteTmp);
		}
		else if(m_IfModifiedSince >= 0 && m_StatusCode == 304) // 304 Not Modified
		{
			fs_remove(m_aDestAbsoluteTmp);
			if(m_WriteToMemory)
			{
				mem_free(m_pBuffer);
				m_pBuffer = nullptr;
				m_ResponseLength = 0;
				void *pBuffer;
				unsigned Length;
				IOHANDLE File = io_open(m_aDestAbsolute, IOFLAG_READ);
				bool Success = File;
				if(File)
				{
					io_read_all(File, &pBuffer, &Length);
					io_close(File);
				}
				if(Success)
				{
					m_pBuffer = (unsigned char *) pBuffer;
					m_ResponseLength = Length;
				}
				else
				{
					dbg_msg("http", "i/o error, cannot read existing file: %s", m_aDest);
					State = EHttpState::ERROR;
				}
			}
		}
		else if(!m_ValidateBeforeOverwrite)
		{
			if(fs_rename(m_aDestAbsoluteTmp, m_aDestAbsolute))
			{
				dbg_msg("http", "i/o error, cannot move file: %s", m_aDest);
				State = EHttpState::ERROR;
				fs_remove(m_aDestAbsoluteTmp);
			}
		}
	}

	// The globally visible state must be updated after OnCompletion has finished,
	// or other threads may try to access the result of a completed HTTP request,
	// before the result has been initialized/updated in OnCompletion.
	OnCompletion(State);
	m_pWaitLock->take();
	m_State = State;
	m_pWaitLock->release();
	m_pWaitSema->signal();
}

void CHttpRequest::OnValidation(bool Success)
{
	dbg_assert(m_ValidateBeforeOverwrite, "this function is illegal to call without having set ValidateBeforeOverwrite");
	m_ValidateBeforeOverwrite = false;
	if(Success)
	{
		if(m_IfModifiedSince >= 0 && m_StatusCode == 304) // 304 Not Modified
		{
			fs_remove(m_aDestAbsoluteTmp);
			return;
		}
		if(fs_rename(m_aDestAbsoluteTmp, m_aDestAbsolute))
		{
			dbg_msg("http", "i/o error, cannot move file: %s", m_aDest);
			m_State = EHttpState::ERROR;
			fs_remove(m_aDestAbsoluteTmp);
		}
	}
	else
	{
		m_State = EHttpState::ERROR;
		fs_remove(m_aDestAbsoluteTmp);
	}
}

void CHttpRequest::WriteToFile(IStorage *pStorage, const char *pDest, int StorageType)
{
	m_WriteToMemory = false;
	m_WriteToFile = true;
	str_copy(m_aDest, pDest, sizeof(m_aDest));
	m_StorageType = StorageType;
	if(StorageType == -2)
	{
		pStorage->GetBinaryPath(m_aDest, m_aDestAbsolute, sizeof(m_aDestAbsolute));
	}
	else
	{
		pStorage->GetCompletePath(StorageType, m_aDest, m_aDestAbsolute, sizeof(m_aDestAbsolute));
	}
	IStorage::FormatTmpPath(m_aDestAbsoluteTmp, sizeof(m_aDestAbsoluteTmp), m_aDestAbsolute);
}

void CHttpRequest::WriteToFileAndMemory(IStorage *pStorage, const char *pDest, int StorageType)
{
	WriteToFile(pStorage, pDest, StorageType);
	m_WriteToMemory = true;
}

void CHttpRequest::Header(const char *pNameColonValue)
{
	m_pHeaders = curl_slist_append((curl_slist *) m_pHeaders, pNameColonValue);
}

void CHttpRequest::Wait()
{
	m_pWaitLock->take();
	;
	while(m_State == EHttpState::QUEUED || m_State == EHttpState::RUNNING)
	{
		m_pWaitLock->release();
		m_pWaitSema->wait();
		m_pWaitLock->take();
		;
	}
	m_pWaitLock->release();
}

void CHttpRequest::Result(unsigned char **ppResult, size_t *pResultLength) const
{
	dbg_assert(State() == EHttpState::DONE, "Request not done");
	dbg_assert(m_WriteToMemory, "Result only usable when written to memory");
	*ppResult = m_pBuffer;
	*pResultLength = m_ResponseLength;
}

json_value *CHttpRequest::ResultJson() const
{
	unsigned char *pResult;
	size_t ResultLength;
	Result(&pResult, &ResultLength);
	return json_parse((char *) pResult, ResultLength);
}

const SHA256_DIGEST &CHttpRequest::ResultSha256() const
{
	dbg_assert(State() == EHttpState::DONE, "Request not done");
	return m_ActualSha256;
}

int CHttpRequest::StatusCode() const
{
	dbg_assert(State() == EHttpState::DONE, "Request not done");
	return m_StatusCode;
}

int64_t CHttpRequest::ResultAgeSeconds() const
{
	dbg_assert(State() == EHttpState::DONE, "Request not done");
	return m_ResultDate - m_ResultLastModified;
}

int64_t CHttpRequest::ResultLastModified() const
{
	dbg_assert(State() == EHttpState::DONE, "Request not done");
	return m_ResultLastModified;
}

bool CHttp::Init(int64_t ShutdownDelayMs, CConfig *pConfig)
{
	m_ShutdownDelayMs = ShutdownDelayMs;
	m_pConfig = pConfig;

#if !defined(CONF_FAMILY_WINDOWS)
	// As a multithreaded application we have to tell curl to not install signal
	// handlers and instead ignore SIGPIPE from OpenSSL ourselves.
	signal(SIGPIPE, SIG_IGN);
#endif
	m_Thread = thread_init(CHttp::ThreadMain, this);

	m_pLock = new lock();
	m_pSemaphore = new semaphore();

	m_pLock->take();
	while(m_State == EState::UNINITIALIZED)
	{
		m_pLock->release();
		// Simple busy wait - in real implementation you might want to use condition variables
		thread_sleep(1);
		m_pLock->take();
	}
	bool Result = m_State == EState::RUNNING;
	m_pLock->release();

	return Result;
}

void CHttp::ThreadMain(void *pUser)
{
	CHttp *pHttp = static_cast<CHttp *>(pUser);
	pHttp->RunLoop();
}

void CHttp::RunLoop()
{
	m_pLock->take();
	if(curl_global_init(CURL_GLOBAL_DEFAULT))
	{
		dbg_msg("http", "curl_global_init failed");
		m_State = EState::ERROR;
		m_pLock->release();
		m_pSemaphore->signal();
		return;
	}

	m_pMultiH = curl_multi_init();
	if(!m_pMultiH)
	{
		dbg_msg("http", "curl_multi_init failed");
		m_State = EState::ERROR;
		m_pLock->release();
		m_pSemaphore->signal();
		return;
	}

	// print curl version
	{
		curl_version_info_data *pVersion = curl_version_info(CURLVERSION_NOW);
		dbg_msg("http", "libcurl version %s (compiled = " LIBCURL_VERSION ")", pVersion->version);
	}

	m_State = EState::RUNNING;
	m_pLock->release();
	m_pSemaphore->signal();

	while(true)
	{
		static int s_NextTimeout = INT_MAX;
		int Events = 0;
		const CURLMcode PollCode = curl_multi_poll((CURLM *) m_pMultiH, nullptr, 0, s_NextTimeout, &Events);

		if(m_Shutdown)
		{
			int64_t Now = time_get();
			if(m_ShutdownTime == -1)
			{
				m_ShutdownTime = Now + (m_ShutdownDelayMs * time_freq()) / 1000;
				s_NextTimeout = m_ShutdownDelayMs;
			}
			else if(Now >= m_ShutdownTime || m_RunningRequests.size() == 0)
			{
				break;
			}
		}

		if(PollCode != CURLM_OK)
		{
			m_pLock->take();
			dbg_msg("http", "curl_multi_poll failed: %s", curl_multi_strerror(PollCode));
			m_State = EState::ERROR;
			break;
		}

		const CURLMcode PerformCode = curl_multi_perform((CURLM *) m_pMultiH, &Events);
		if(PerformCode != CURLM_OK)
		{
			m_pLock->take();
			dbg_msg("http", "curl_multi_perform failed: %s", curl_multi_strerror(PerformCode));
			m_State = EState::ERROR;
			break;
		}

		struct CURLMsg *pMsg;
		while((pMsg = curl_multi_info_read((CURLM *) m_pMultiH, &Events)))
		{
			if(pMsg->msg == CURLMSG_DONE)
			{
				auto RequestIt = m_RunningRequests.find(pMsg->easy_handle);
				dbg_assert(RequestIt != m_RunningRequests.end(), "Running handle not added to map");
				auto pRequest = std::move(RequestIt->second);
				m_RunningRequests.erase(RequestIt);

				pRequest->OnCompletionInternal(pMsg->easy_handle, pMsg->data.result);
				curl_multi_remove_handle((CURLM *) m_pMultiH, pMsg->easy_handle);
				curl_easy_cleanup(pMsg->easy_handle);
			}
		}

		array<CHttpRequest *> apNewRequests;
		apNewRequests.clear();
		m_pLock->take();
		apNewRequests = std::move(m_PendingRequests);
		m_PendingRequests.clear();
		m_pLock->release();

		// Process pending requests
		while(apNewRequests.size() > 0)
		{
			auto &pRequest = *apNewRequests.begin();
			m_pLock->release();

			if(Config()->m_DbgCurl)
				dbg_msg("http", "task: %s %s", CHttpRequest::GetRequestType(pRequest->m_Type), pRequest->m_aUrl);

			if(pRequest->ShouldSkipRequest())
			{
				pRequest->OnCompletion(EHttpState::DONE);
				pRequest->m_pWaitLock->take();
				pRequest->m_State = EHttpState::DONE;
				pRequest->m_pWaitLock->release();
				pRequest->m_pWaitSema->signal();
				apNewRequests.remove_index(0);
				continue;
			}

			CURL *pEH = curl_easy_init();
			if(!pEH)
			{
				dbg_msg("http", "curl_easy_init failed");
				pRequest->m_pWaitLock->take();
				m_State = EState::ERROR;
				break;
			}
			if(!pRequest->ConfigureHandle(pEH))
			{
				curl_easy_cleanup(pEH);
				str_copy(pRequest->m_aErr, "Failed to initialize request", sizeof(pRequest->m_aErr));
				pRequest->OnCompletionInternal(nullptr, CURLE_ABORTED_BY_CALLBACK);
				apNewRequests.remove_index(0);
				continue;
			}
			if(curl_multi_add_handle((CURLM *) m_pMultiH, pEH) != CURLM_OK)
			{
				dbg_msg("http", "curl_multi_add_handle failed");
				curl_easy_cleanup(pEH);
			}

			{
				pRequest->m_pWaitLock->take();
				pRequest->m_State = EHttpState::RUNNING;
				pRequest->m_pWaitLock->release();
			}
			m_RunningRequests.emplace(pEH, std::move(pRequest));
			apNewRequests.remove_index(0);
			continue;
		}
		// Only happens if m_State == ERROR, thus we already hold the lock
		if(apNewRequests.size() > 0)
		{
			m_PendingRequests = apNewRequests;
			break;
		}
	}

	// Cleanup
	m_pLock->take();
	bool Cleanup = m_State != EState::ERROR;

	// Handle pending requests
	for(int i = 0; i < m_PendingRequests.size(); i++)
	{
		CHttpRequest *pRequest = m_PendingRequests[i];
		str_copy(pRequest->m_aErr, "Shutting down", sizeof(pRequest->m_aErr));
		pRequest->OnCompletionInternal(nullptr, CURLE_ABORTED_BY_CALLBACK);
	}
	m_PendingRequests.clear();

	for(auto &ReqPair : m_RunningRequests)
	{
		auto &[pHandle, pRequest] = ReqPair;

		str_copy(pRequest->m_aErr, "Shutting down", sizeof(pRequest->m_aErr));
		pRequest->OnCompletionInternal(pHandle, CURLE_ABORTED_BY_CALLBACK);

		if(Cleanup)
		{
			curl_multi_remove_handle((CURLM *) m_pMultiH, pHandle);
			curl_easy_cleanup((CURL *) pHandle);
		}
	}
	m_RunningRequests.clear();

	if(Cleanup)
	{
		curl_multi_cleanup((CURLM *) m_pMultiH);
		curl_global_cleanup();
	}
	m_pLock->release();
}

void CHttp::Run(IHttpRequest *pRequest)
{
	CHttpRequest *pImpl = static_cast<CHttpRequest *>(pRequest);

	m_pLock->take();
	if(m_Shutdown || m_State == EState::ERROR)
	{
		str_copy(pImpl->m_aErr, "Shutting down", sizeof(pImpl->m_aErr));
		pImpl->OnCompletionInternal(nullptr, CURLE_ABORTED_BY_CALLBACK);
		m_pLock->release();
		return;
	}
	m_PendingRequests.add(pImpl);

	if(m_pMultiH)
	{
		curl_multi_wakeup((CURLM *) m_pMultiH);
	}
	m_pLock->release();
}

void CHttp::Shutdown()
{
	m_pLock->take();
	if(m_Shutdown || m_State != EState::RUNNING)
	{
		m_pLock->release();
		return;
	}

	m_Shutdown = true;

	if(m_pMultiH)
	{
		curl_multi_wakeup((CURLM *) m_pMultiH);
	}
	m_pLock->release();
}

CHttp::~CHttp()
{
	if(!m_Thread)
		return;

	Shutdown();
	thread_wait(m_Thread);

	delete m_pLock;
	delete m_pSemaphore;
}
