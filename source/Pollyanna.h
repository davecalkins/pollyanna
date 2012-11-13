// Pollyanna.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPollyannaApp:
// See Pollyanna.cpp for the implementation of this class
//

class CPollyannaApp : public CWinApp
{
public:
	CPollyannaApp();

// Overrides
	public:
	virtual BOOL InitInstance();

	CString appDir;

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPollyannaApp theApp;