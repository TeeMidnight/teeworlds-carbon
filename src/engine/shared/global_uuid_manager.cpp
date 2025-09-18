#include "protocol_ex.h"
#include "uuid_manager.h"

void RegisterGameUuids(CUuidManager *pManager);

static CUuidManager CreateGlobalUuidManager()
{
	CUuidManager Manager;
	RegisterUuids(&Manager);
	RegisterGameUuids(&Manager);
	return Manager;
}

CUuidManager g_UuidManager = CreateGlobalUuidManager();
