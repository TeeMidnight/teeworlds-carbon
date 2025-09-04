#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	IOFLAG_READ = 1,
	IOFLAG_WRITE = 2,
	IOFLAG_APPEND = 4,
	IOFLAG_SKIP_BOM = 8,

	IOSEEK_START = 0,
	IOSEEK_CUR = 1,
	IOSEEK_END = 2,

	IO_MAX_PATH_LENGTH = 512,
};

typedef struct IOINTERNAL *IOHANDLE;

typedef void *LOCK;

#if defined(CONF_FAMILY_UNIX) && !defined(CONF_PLATFORM_MACOS)
#include <semaphore.h>
typedef sem_t SEMAPHORE;
#elif defined(CONF_FAMILY_WINDOWS)
typedef void *SEMAPHORE;
#else
typedef struct SEMINTERNAL *SEMAPHORE;
#endif

enum
{
	SEASON_SPRING = 0,
	SEASON_SUMMER,
	SEASON_AUTUMN,
	SEASON_WINTER,
	SEASON_NEWYEAR
};

typedef struct
{
	int type;
	int ipv4sock;
	int ipv6sock;
} NETSOCKET;

enum
{
	NETADDR_MAXSTRSIZE = 1 + (8 * 4 + 7) + 1 + 1 + 5 + 1, // [XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX]:XXXXX

	NETADDR_SIZE_IPV4 = 4,
	NETADDR_SIZE_IPV6 = 16,

	NETTYPE_INVALID = 0,
	NETTYPE_IPV4 = 1,
	NETTYPE_IPV6 = 2,
	NETTYPE_LINK_BROADCAST = 4,
	NETTYPE_ALL = NETTYPE_IPV4 | NETTYPE_IPV6
};

typedef struct
{
	unsigned int type;
	unsigned char ip[NETADDR_SIZE_IPV6];
	unsigned short port;
	unsigned short reserved;
} NETADDR;

typedef struct
{
	int sent_packets;
	int sent_bytes;
	int recv_packets;
	int recv_bytes;
} NETSTATS;

enum
{
	UTF8_BYTE_LENGTH = 4
};

#ifdef __cplusplus
}
#endif

#endif // BASE_TYPE_H
