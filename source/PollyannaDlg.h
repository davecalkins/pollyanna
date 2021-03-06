// PollyannaDlg.h : header file
//

#pragma once
#include "afxwin.h"

class Person : public CObject
{
public:
   Person();
   Person(const Person& rhs);
   virtual ~Person();

   Person& operator=(const Person& rhs);

   typedef enum
   {
	   FT_DISPLAY_TO_USER,
	   FT_COMPARE_WITH_PREV_PICKS
   } FormatType;

   CString Format(FormatType ft);

   CString Name;
   CString Family;
   CString Email;

   Person * TargetPerson;

   DECLARE_SERIAL(Person);
   virtual void Serialize(CArchive& ar);
};

// CPollyannaDlg dialog
class CPollyannaDlg : public CDialog
{
// Construction
public:
	CPollyannaDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_POLLYANNA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

   void Save();
   void Load();

   void Serialize(CArchive& ar);

   DWORD_PTR StartTickCount;
   DWORD_PTR NumTries;
   DWORD_PTR NumDraws;
   DWORD_PTR NumRejected;

   BOOL KeepRunning;

   BOOL SendEmail(CStringArray& recipients, LPCTSTR subject, CStringArray& msgLines, CString& errDesc);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
   CListBox PeopleList;
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedDrawnames();
   CStatic MsgWnd;
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   CButton AddBtn;
   CButton DrawNamesBtn;
   afx_msg void OnClose();
   afx_msg void OnBnClickedEditBtn();
   afx_msg void OnBnClickedDeleteBtn();
   afx_msg void OnDblclkPeople();
   afx_msg void OnBnClickedWritePicksToFileBtn();
   afx_msg void OnBnClickedEmailPicksBtn();
   afx_msg void OnBnClickedDecodePicksFileBtn();
};
