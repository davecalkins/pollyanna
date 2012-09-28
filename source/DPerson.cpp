// DPerson.cpp : implementation file
//

#include "stdafx.h"
#include "Pollyanna.h"
#include "DPerson.h"


// DPerson dialog

IMPLEMENT_DYNAMIC(DPerson, CDialog)

DPerson::DPerson(CWnd* pParent /*=NULL*/)
	: CDialog(DPerson::IDD, pParent)
   , Name(_T(""))
   , Family(_T(""))
{

}

DPerson::~DPerson()
{
}

void DPerson::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_NAME, Name);
   DDX_Text(pDX, IDC_FAMILY, Family);
}


BEGIN_MESSAGE_MAP(DPerson, CDialog)
END_MESSAGE_MAP()


// DPerson message handlers
