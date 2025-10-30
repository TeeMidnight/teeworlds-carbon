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
#ifndef TEST_TEST_H
#define TEST_TEST_H
class CTestInfo
{
public:
	CTestInfo();
	void Filename(char *pBuffer, int BufferLength, const char *pSuffix);
	char m_aFilenamePrefix[64];
	char m_aFilename[64];
};
#endif // TEST_TEST_H
