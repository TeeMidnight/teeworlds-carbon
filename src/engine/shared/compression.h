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
#ifndef ENGINE_SHARED_COMPRESSION_H
#define ENGINE_SHARED_COMPRESSION_H

// variable int packing
class CVariableInt
{
public:
	enum
	{
		MAX_BYTES_PACKED = 5, // maximum number of bytes in a packed int
	};

	static unsigned char *Pack(unsigned char *pDst, int i, int DstSize);
	static const unsigned char *Unpack(const unsigned char *pSrc, int *pInOut, int SrcSize);

	static long Compress(const void *pSrc, int SrcSize, void *pDst, int DstSize);
	static long Decompress(const void *pSrc, int SrcSize, void *pDst, int DstSize);
};

#endif
