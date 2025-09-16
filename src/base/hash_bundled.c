/*
 * This file is part of Carbon, a modified version of Teeworlds.
 *
 * Copyright (C) 2007-2025 Magnus Auvinen
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#if !defined(CONF_OPENSSL)

#include "hash_ctxt.h"

#include <engine/external/md5/md5.h>

void md5_update(MD5_CTX *ctxt, const void *data, size_t data_len)
{
	md5_append(ctxt, data, data_len);
}

MD5_DIGEST md5_finish(MD5_CTX *ctxt)
{
	MD5_DIGEST result;
	md5_finish_(ctxt, result.data);
	return result;
}

#endif
