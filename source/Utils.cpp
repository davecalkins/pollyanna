#include "stdafx.h"
#include "Utils.h"

CStringA STR_T2A(LPCTSTR in)
{
	CStringA result;

	ASSERT(in != NULL);
	if (in == NULL)
		return result;

	int bytesNeeded = WideCharToMultiByte(CP_UTF8,0,in,-1,NULL,0,NULL,NULL);
	if (bytesNeeded == 0)
		return result;

	LPSTR buf = result.GetBuffer(bytesNeeded);
	int res = WideCharToMultiByte(CP_UTF8,0,in,-1,buf,bytesNeeded,NULL,NULL);
	result.ReleaseBuffer();
	if (res == 0)
	{
		result.Empty();
		return result;
	}

	return result;
}

CString STR_A2T(LPCSTR in)
{
	CString result;

	ASSERT(in != NULL);
	if (in == NULL)
		return result;

	int numCharsNeeded = MultiByteToWideChar(CP_UTF8,0,in,-1,NULL,0);
	if (numCharsNeeded == 0)
		return result;

	LPTSTR buf = result.GetBuffer(numCharsNeeded);
	int res = MultiByteToWideChar(CP_UTF8,0,in,-1,buf,numCharsNeeded);
	result.ReleaseBuffer();
	if (res == 0)
	{
		result.Empty();
		return result;
	}

	return result;
}
