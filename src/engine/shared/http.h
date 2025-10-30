/*
 * This file is part of Carbon, a modified version of Teeworlds.
 * This file contains code derived from DDNet (ddnet.org), a race mod of Teeworlds.
 *
 * Copyright (C) 2018-2025 Dennis Felsing
 * Copyright (C) 2025 TeeMidnight
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/TeeMidnight/teeworlds-carbon
 */
#ifndef ENGINE_SHARED_HTTP_H
#define ENGINE_SHARED_HTTP_H

#include <base/hash_ctxt.h>
#include <base/math.h>
#include <base/tl/array.h>
// #include <base/tl/threading.h>
#include <engine/shared/config.h>

#include <engine/http.h>

#include <unordered_map>

typedef struct _json_value json_value;
class IStorage;

enum class EHttpState
{
	ERROR = -1,
	QUEUED,
	RUNNING,
	DONE,
	ABORTED,
};

enum class HTTPLOG
{
	NONE,
	FAILURE,
	ALL,
};

enum class IPRESOLVE
{
	WHATEVER,
	V4,
	V6,
};

struct CTimeout
{
	long ConnectTimeoutMs;
	long TimeoutMs;
	long LowSpeedLimit;
	long LowSpeedTime;
};

class CHttpRequest : public IHttpRequest
{
public:
	enum class REQUEST
	{
		GET = 0,
		HEAD,
		POST,
		POST_JSON,
	};

	static constexpr const char *GetRequestType(REQUEST Type)
	{
		switch(Type)
		{
		case REQUEST::GET:
			return "GET";
		case REQUEST::HEAD:
			return "HEAD";
		case REQUEST::POST:
		case REQUEST::POST_JSON:
			return "POST";
		}
		return "UNKNOWN";
	}

private:
	friend class CHttp;

	class CConfig *m_pConfig;

	char m_aUrl[256] = {0};

	void *m_pHeaders = nullptr;
	unsigned char *m_pBody = nullptr;
	size_t m_BodyLength = 0;

	bool m_ValidateBeforeOverwrite = false;
	bool m_SkipByFileTime = true;

	CTimeout m_Timeout = CTimeout{0, 0, 0, 0};
	int64_t m_MaxResponseSize = -1;
	int64_t m_IfModifiedSince = -1;
	REQUEST m_Type = REQUEST::GET;

	SHA256_DIGEST m_ActualSha256 = SHA256_ZEROED;
	SHA256_CTX m_ActualSha256Ctx;
	SHA256_DIGEST m_ExpectedSha256 = SHA256_ZEROED;

	bool m_WriteToMemory = true;
	bool m_WriteToFile = false;

	uint64_t m_ResponseLength = 0;

	size_t m_BufferSize = 0;
	unsigned char *m_pBuffer = nullptr;

	IOHANDLE m_File = nullptr;
	int m_StorageType = 0xdeadbeef;
	char m_aDestAbsoluteTmp[IO_MAX_PATH_LENGTH] = {0};
	char m_aDestAbsolute[IO_MAX_PATH_LENGTH] = {0};
	char m_aDest[IO_MAX_PATH_LENGTH] = {0};

	double m_Size = 0.0;
	double m_Current = 0.0;
	int m_Progress = 0;
	HTTPLOG m_LogProgress = HTTPLOG::ALL;
	IPRESOLVE m_IpResolve = IPRESOLVE::WHATEVER;

	bool m_FailOnErrorStatus = true;

	char m_aErr[256];
	EHttpState m_State = EHttpState::QUEUED;
	bool m_Abort = false;
	class lock *m_pWaitLock;
	class semaphore *m_pWaitSema;

	int m_StatusCode = 0;
	bool m_HeadersEnded = false;

	bool m_HasDate = false;
	int64_t m_ResultDate = 0;

	bool m_HasLastModified = false;
	int64_t m_ResultLastModified = 0;

	bool ShouldSkipRequest();
	bool BeforeInit();
	bool ConfigureHandle(void *pHandle);
	void OnCompletionInternal(void *pHandle, unsigned int Result);

	size_t OnHeader(char *pHeader, size_t HeaderSize);
	size_t OnData(char *pData, size_t DataSize);

	static int ProgressCallback(void *pUser, double DlTotal, double DlCurr, double UlTotal, double UlCurr);
	static size_t HeaderCallback(char *pData, size_t Size, size_t Number, void *pUser);
	static size_t WriteCallback(char *pData, size_t Size, size_t Number, void *pUser);

protected:
	virtual void OnProgress() {}
	virtual void OnCompletion(EHttpState State) {}

public:
	CConfig *Config() { return m_pConfig; }

	CHttpRequest(const char *pUrl, CConfig *pConfig);
	virtual ~CHttpRequest();

	void Timeout(CTimeout Timeout) { m_Timeout = Timeout; } // Skip the download if the local file is newer or as new as the remote file.
	void SkipByFileTime(bool SkipByFileTime) { m_SkipByFileTime = SkipByFileTime; }
	void MaxResponseSize(int64_t MaxResponseSize) { m_MaxResponseSize = MaxResponseSize; }
	void LogProgress(HTTPLOG LogProgress) { m_LogProgress = LogProgress; }
	void IpResolve(IPRESOLVE IpResolve) { m_IpResolve = IpResolve; }
	void FailOnErrorStatus(bool FailOnErrorStatus) { m_FailOnErrorStatus = FailOnErrorStatus; }
	// Download to memory only. Get the result via `Result*`.
	void WriteToMemory()
	{
		m_WriteToMemory = true;
		m_WriteToFile = false;
	}
	// Download to filesystem and memory.
	void WriteToFileAndMemory(IStorage *pStorage, const char *pDest, int StorageType);
	// Download to the filesystem only.
	void WriteToFile(IStorage *pStorage, const char *pDest, int StorageType);
	// Don't place the file in the specified location until
	// `OnValidation(true)` has been called.
	void ValidateBeforeOverwrite(bool ValidateBeforeOverwrite) { m_ValidateBeforeOverwrite = ValidateBeforeOverwrite; }
	void ExpectSha256(const SHA256_DIGEST &Sha256) { m_ExpectedSha256 = Sha256; }
	void Head() { m_Type = REQUEST::HEAD; }

	void Post(const unsigned char *pData, size_t DataLength)
	{
		m_Type = REQUEST::POST;
		m_BodyLength = DataLength;
		m_pBody = (unsigned char *) mem_alloc(maximum((size_t) 1, DataLength));
		mem_copy(m_pBody, pData, DataLength);
	}

	void PostJson(const char *pJson)
	{
		m_Type = REQUEST::POST_JSON;
		m_BodyLength = str_length(pJson);
		m_pBody = (unsigned char *) mem_alloc(m_BodyLength);
		mem_copy(m_pBody, pJson, m_BodyLength);
	}

	void Header(const char *pNameColonValue);
	void HeaderString(const char *pName, const char *pValue)
	{
		char aHeader[256];
		str_format(aHeader, sizeof(aHeader), "%s: %s", pName, pValue);
		Header(aHeader);
	}

	void HeaderInt(const char *pName, int Value)
	{
		char aHeader[256];
		str_format(aHeader, sizeof(aHeader), "%s: %d", pName, Value);
		Header(aHeader);
	}

	const char *Dest()
	{
		if(m_WriteToFile)
		{
			return m_aDest;
		}
		else
		{
			return nullptr;
		}
	}

	double Current() const { return m_Current; }
	double Size() const { return m_Size; }
	int Progress() const { return m_Progress; }
	EHttpState State() const { return m_State; }
	bool Done() const
	{
		return m_State != EHttpState::QUEUED && m_State != EHttpState::RUNNING;
	}
	void Abort() { m_Abort = true; }
	void OnValidation(bool Success);
	void Wait();

	void Result(unsigned char **ppResult, size_t *pResultLength) const;
	json_value *ResultJson() const;
	const SHA256_DIGEST &ResultSha256() const;

	int StatusCode() const;
	bool HasResultAgeSeconds() const { return m_HasDate; }
	int64_t ResultAgeSeconds() const;
	bool HasResultLastModified() const { return m_HasLastModified; }
	int64_t ResultLastModified() const;
};

// Helper functions
inline CHttpRequest *HttpHead(const char *pUrl, CConfig *pConfig)
{
	CHttpRequest *pResult = new CHttpRequest(pUrl, pConfig);
	pResult->Head();
	return pResult;
}

inline CHttpRequest *HttpGet(const char *pUrl, CConfig *pConfig)
{
	return new CHttpRequest(pUrl, pConfig);
}

inline CHttpRequest *HttpGetFile(const char *pUrl, CConfig *pConfig, IStorage *pStorage, const char *pOutputFile, int StorageType)
{
	CHttpRequest *pResult = HttpGet(pUrl, pConfig);
	pResult->WriteToFile(pStorage, pOutputFile, StorageType);
	pResult->Timeout(CTimeout{4000, 0, 500, 5});
	return pResult;
}

inline CHttpRequest *HttpPost(const char *pUrl, CConfig *pConfig, const unsigned char *pData, size_t DataLength)
{
	CHttpRequest *pResult = new CHttpRequest(pUrl, pConfig);
	pResult->Post(pData, DataLength);
	pResult->Timeout(CTimeout{4000, 15000, 500, 5});
	return pResult;
}

inline CHttpRequest *HttpPostJson(const char *pUrl, CConfig *pConfig, const char *pJson)
{
	CHttpRequest *pResult = new CHttpRequest(pUrl, pConfig);
	pResult->PostJson(pJson);
	pResult->Timeout(CTimeout{4000, 15000, 500, 5});
	return pResult;
}

void EscapeUrl(char *pBuf, int Size, const char *pStr);
bool HttpHasIpresolveBug();

// In an ideal world this would be a kernel interface
class CHttp : public IHttp
{
public:
	enum class EState
	{
		UNINITIALIZED,
		RUNNING,
		ERROR,
	};

private:
	void *m_Thread = nullptr;

	class lock *m_pLock;
	class semaphore *m_pSemaphore;
	EState m_State = EState::UNINITIALIZED;

	array<CHttpRequest *> m_PendingRequests;
	std::unordered_map<void *, CHttpRequest *> m_RunningRequests{};

	bool m_Shutdown = false;

	void *m_pMultiH = nullptr;

	static void ThreadMain(void *pUser);
	void RunLoop();

	class CConfig *m_pConfig;

public:
	CConfig *Config() { return m_pConfig; }
	bool Init(CConfig *pConfig);

	void Run(IHttpRequest *pRequest) override;
	void Shutdown() override;
	virtual ~CHttp();
};

#endif // ENGINE_SHARED_HTTP_H
