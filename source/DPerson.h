#pragma once


// DPerson dialog

class DPerson : public CDialog
{
	DECLARE_DYNAMIC(DPerson)

public:
	DPerson(CWnd* pParent = NULL);   // standard constructor
	virtual ~DPerson();

// Dialog Data
	enum { IDD = IDD_PERSON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString Name;
   CString Family;
};
