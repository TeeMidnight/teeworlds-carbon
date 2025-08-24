/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <engine/shared/jobs.h>
#include "kernel.h"

class IEngine : public IInterface
{
	MACRO_INTERFACE("engine", 0)

protected:
	class CJobPool m_JobPool;

public:
	virtual void Init() = 0;
	virtual void ShutdownJobs() = 0;
	virtual void InitLogfile() = 0;
	virtual void QueryNetLogHandles(IOHANDLE *pHDLSend, IOHANDLE *pHDLRecv) = 0;
	virtual void AddJob(std::shared_ptr<IJob> pJob) = 0;
};

extern IEngine *CreateEngine(const char *pAppname);

#endif
