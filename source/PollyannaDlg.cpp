// PollyannaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Pollyanna.h"
#include "PollyannaDlg.h"
#include "DPerson.h"
#include "CSmtp.h"
#include "base64.h"
#include "Utils.h"

#include <vector>
#include <map>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SAVEFILE      _T("pollyanna.dat")
#define PREVPICKSFILE _T("pollyanna.prevpicks.txt")

#define PROGUPDATETIMER_ID          101
#define PROGUPDATETIMER_INTERVAL_MS 1000

// if enabled, when a pick is done, the target person is NOT shown
// in the list to avoid the person running it from seeing the picks
#define HIDE_TARGET_PERSON_IN_LIST

// Email config
#define EMAIL_CONFIG_SMTP_SERVER   _T("smtp.server.here")
#define EMAIL_CONFIG_SMTP_PORT     25
#define EMAIL_CONFIG_SENDER_NAME   _T("from")
#define EMAIL_CONFIG_SENDER_EMAIL  _T("from@domain.com")
#define EMAIL_CONFIG_REPLYTO_EMAIL _T("replyto@domain.com")

Person::Person()
: TargetPerson(NULL)
{
}

Person::Person(const Person& rhs)
{
   *this = rhs;
}

Person::~Person()
{
}

Person& Person::operator=(const Person& rhs)
{
   if (this == &rhs)
      return *this;

   Name = rhs.Name;
   Family = rhs.Family;
   TargetPerson = rhs.TargetPerson;
   Email = rhs.Email;

   return *this;
}

CString Person::Format(FormatType ft)
{
	CString result;

	if (ft == FT_DISPLAY_TO_USER)
	{
		CString emailstr;
		if (!Email.IsEmpty())
			emailstr.Format(_T(", %s"), Email);
		else
			emailstr = _T(", <no email>");

		CString tpstr;
		if (TargetPerson != NULL)
		{
#ifdef HIDE_TARGET_PERSON_IN_LIST
			tpstr = _T(" => <HIDDEN>");
#else
			tpstr.Format(_T(" => %s (%s)"), TargetPerson->Name, TargetPerson->Family);
#endif
		}

		result.Format(_T("%s (%s%s)%s"), Name, Family, emailstr, tpstr);
	}
	else if (ft == FT_COMPARE_WITH_PREV_PICKS)
	{
		CString tpstr;
		if (TargetPerson != NULL)
		{
			tpstr.Format(_T(" => %s (%s)"), TargetPerson->Name, TargetPerson->Family);
		}

		result.Format(_T("%s (%s)%s"), Name, Family, tpstr);
	}

	return result;
}

IMPLEMENT_SERIAL(Person,CObject,0);

void Person::Serialize(CArchive& ar)
{
   BYTE Schema = 0;
   Schema = 1; // added Email
   BYTE LoadSchema;

   if (ar.IsStoring())
   {
      ar << Schema;
      ar << Name;
      ar << Family;
      ar << TargetPerson;
	  ar << Email;
   }
   else
   {
      ar >> LoadSchema;
      ar >> Name;
      ar >> Family;
      ar >> TargetPerson;

	  if (LoadSchema >= 1)
		  ar >> Email;
   }
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPollyannaDlg dialog




CPollyannaDlg::CPollyannaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPollyannaDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPollyannaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PEOPLE, PeopleList);
	DDX_Control(pDX, IDC_MSG, MsgWnd);
	DDX_Control(pDX, IDC_ADD, AddBtn);
	DDX_Control(pDX, IDC_DRAWNAMES, DrawNamesBtn);
}

BEGIN_MESSAGE_MAP(CPollyannaDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_ADD, &CPollyannaDlg::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_DRAWNAMES, &CPollyannaDlg::OnBnClickedDrawnames)
   ON_WM_TIMER()
   ON_WM_CLOSE()
   ON_BN_CLICKED(IDC_EDIT_BTN, &CPollyannaDlg::OnBnClickedEditBtn)
   ON_BN_CLICKED(IDC_DELETE_BTN, &CPollyannaDlg::OnBnClickedDeleteBtn)
   ON_LBN_DBLCLK(IDC_PEOPLE, &CPollyannaDlg::OnDblclkPeople)
   ON_BN_CLICKED(IDC_WRITE_PICKS_TO_FILE_BTN, &CPollyannaDlg::OnBnClickedWritePicksToFileBtn)
   ON_BN_CLICKED(IDC_EMAIL_PICKS_BTN, &CPollyannaDlg::OnBnClickedEmailPicksBtn)
   ON_BN_CLICKED(IDC_DECODE_PICKS_FILE_BTN, &CPollyannaDlg::OnBnClickedDecodePicksFileBtn)
END_MESSAGE_MAP()


// CPollyannaDlg message handlers

BOOL CPollyannaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
   srand((unsigned int)time(NULL));
   Load();
   MsgWnd.SetWindowText(_T(""));

   NumTries = 0;
   NumDraws = 0;
   NumRejected = 0;

   return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPollyannaDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPollyannaDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPollyannaDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPollyannaDlg::OnBnClickedAdd()
{
   DPerson dlg;
   if (dlg.DoModal() != IDOK)
      return;

   Person & p = *new Person;
   p.Name = dlg.Name;
   p.Family = dlg.Family;
   p.Email = dlg.Email;

   int idx = PeopleList.AddString(p.Format(Person::FT_DISPLAY_TO_USER));
   PeopleList.SetItemData(idx,(DWORD_PTR)(&p));

   Save();
}

void CPollyannaDlg::OnBnClickedEditBtn()
{
	int sel = PeopleList.GetCurSel();
	if (sel < 0)
		return;

	Person& p = *((Person*)PeopleList.GetItemData(sel));

	DPerson dlg;
	dlg.Name = p.Name;
	dlg.Family = p.Family;
	dlg.Email = p.Email;
	if (dlg.DoModal() != IDOK)
		return;

	p.Name = dlg.Name;
	p.Family = dlg.Family;
	p.Email = dlg.Email;

	vector<Person*> people;
	for (int i = 0; i < PeopleList.GetCount(); i++)
	{
		people.push_back((Person*)PeopleList.GetItemData(i));
	}

	PeopleList.LockWindowUpdate();
	PeopleList.ResetContent();
	for (auto i = people.begin(); i != people.end(); i++)
	{
		Person& p = *(*i);
		int idx = PeopleList.AddString(p.Format(Person::FT_DISPLAY_TO_USER));
		PeopleList.SetItemData(idx,(DWORD_PTR)&p);
	}
	PeopleList.UnlockWindowUpdate();

	Save();
}


void CPollyannaDlg::OnBnClickedDeleteBtn()
{
	int sel = PeopleList.GetCurSel();
	if (sel < 0)
		return;

	Person& p = *((Person*)PeopleList.GetItemData(sel));
	delete &p;

	PeopleList.DeleteString(sel);

	Save();
}

void ProcessWaitingMessages()
{
	MSG m;
	while (PeekMessage(&m,NULL,0,0,PM_REMOVE))
	{
		TranslateMessage(&m);
		DispatchMessage(&m);
	}
}

void CPollyannaDlg::OnBnClickedDrawnames()
{
	CWaitCursor wc;

	MsgWnd.SetWindowText(_T(""));

	AddBtn.EnableWindow(FALSE);
	DrawNamesBtn.EnableWindow(FALSE);

	KeepRunning = TRUE;

	StartTickCount = GetTickCount();
	SetTimer(PROGUPDATETIMER_ID,PROGUPDATETIMER_INTERVAL_MS,NULL);

	CStringArray PrevPicks;
	CStdioFile ppf;
	if (ppf.Open(PREVPICKSFILE,CFile::modeRead|CFile::typeText))
	{
		CString l;
		while (ppf.ReadString(l))
		{
			l.Trim();
			if (l.GetLength() == 0)
				continue;
			if (l[0] == _T(';'))
				continue;
			PrevPicks.Add(l);
		}
		ppf.Close();
	}

	NumTries = 0;
	NumDraws = 0;
	NumRejected = 0;

	BOOL KeepTrying = TRUE;
	while (KeepTrying && KeepRunning)
	{
		ProcessWaitingMessages();

		NumTries++;
		KeepTrying = FALSE;

		CArray<Person*> People;
		for (int i = 0; i < PeopleList.GetCount(); i++)
		{
			Person& p = *((Person*)PeopleList.GetItemData(i));
			p.TargetPerson = NULL;
			People.Add(&p);
		}

		CArray<Person*> PeopleNeedingAssignments;
		PeopleNeedingAssignments.Append(People);

		CArray<Person*> RemainingRecipients;
		RemainingRecipients.Append(People);

		int r;
		double per;
		int ridx;

		while ((PeopleNeedingAssignments.GetSize() > 0) && KeepRunning)
		{
			ProcessWaitingMessages();

			int PersonSelectedForAssignmentIdx = -1;
			Person* pPersonSelectedForAssignment = NULL;
			if (PeopleNeedingAssignments.GetSize() == 1)
			{
				pPersonSelectedForAssignment = PeopleNeedingAssignments[0];
				PersonSelectedForAssignmentIdx = 0;
			}
			else
			{
				r = rand();
				per = (double)r / (double)RAND_MAX;
				ridx = (int)(per * (double)(PeopleNeedingAssignments.GetSize()-1));
				pPersonSelectedForAssignment = PeopleNeedingAssignments[ridx];
				PersonSelectedForAssignmentIdx = ridx;
			}
			Person& p = *pPersonSelectedForAssignment;

			BOOL NonFamilyLeft = FALSE;
			for (INT_PTR j = 0; (j < RemainingRecipients.GetSize()) && !NonFamilyLeft; j++)
			{
				Person& op = *RemainingRecipients[j];
				if (op.Family.CompareNoCase(p.Family) != 0)
					NonFamilyLeft = TRUE;
			}

			if (!NonFamilyLeft)
			{
				KeepTrying = TRUE;
				break;
			}

			BOOL GotRecipient = FALSE;
			while (!GotRecipient && KeepRunning)
			{
				NumDraws++;

				r = rand();
				per = (double)r / (double)RAND_MAX;
				ridx = (int)(per * (double)(RemainingRecipients.GetSize()-1));

				Person& op = *RemainingRecipients[ridx];
				if (op.Family.CompareNoCase(p.Family) != 0)
				{
					GotRecipient = TRUE;
					p.TargetPerson = &op;
					PeopleNeedingAssignments.RemoveAt(PersonSelectedForAssignmentIdx);
					RemainingRecipients.RemoveAt(ridx);
				}
			}
		}

		//
		// make sure noone got the same person as in a prev pick
		//
		if (!KeepTrying)
		{
			for (INT_PTR i = 0; i < People.GetSize(); i++)
			{
				Person& p = *People[i];
				CString curPick = p.Format(Person::FT_COMPARE_WITH_PREV_PICKS);

				BOOL WasPrevPick = FALSE;
				for (INT_PTR j = 0; (j < PrevPicks.GetSize()) && !WasPrevPick; j++)
				{
					CString prevPick = PrevPicks[j];

					curPick.Replace(_T(" "),_T(""));
					prevPick.Replace(_T(" "),_T(""));
					if (curPick.CompareNoCase(prevPick) == 0)
						WasPrevPick = TRUE;
				}

				if (WasPrevPick)
				{
					KeepTrying = TRUE;
					break;
				}
			}

			if (KeepTrying)
				NumRejected++;
		}

		//
		// make sure no family got only 1 other family
		//
		//if (!KeepTrying)
		//{
		//	CMapStringToOb FamilyAssignments;

		//	for (INT_PTR i = 0; i < People.GetSize(); i++)
		//	{
		//		Person& p = *People[i];

		//		CStringArray* _AssignmentArr;
		//		if (!FamilyAssignments.Lookup(p.Family,(CObject*&)_AssignmentArr))
		//		{
		//			_AssignmentArr = new CStringArray;
		//			FamilyAssignments.SetAt(p.Family,(CObject*&)_AssignmentArr);
		//		}
		//		CStringArray& AssignmentArr = *_AssignmentArr;

		//		Person& tp = *p.TargetPerson;

		//		BOOL TargetFamilyInArr = FALSE;
		//		for (INT_PTR j = 0; (j < AssignmentArr.GetSize()) && !TargetFamilyInArr; j++)
		//		{
		//			if (tp.Family.CompareNoCase(AssignmentArr[j]) == 0)
		//				TargetFamilyInArr = TRUE;
		//		}

		//		if (!TargetFamilyInArr)
		//			AssignmentArr.Add(tp.Family);
		//	}

		//	POSITION pos = FamilyAssignments.GetStartPosition();
		//	while (pos != NULL)
		//	{
		//		CString FamilyName;
		//		CStringArray* AssignmentArr;
		//		FamilyAssignments.GetNextAssoc(pos,FamilyName,(CObject*&)AssignmentArr);
		//		if (AssignmentArr->GetSize() < 2)
		//		{
		//			KeepTrying = TRUE;
		//		}
		//		delete AssignmentArr;
		//	}

		//	if (KeepTrying)
		//		NumRejected++;
		//}

		if (!KeepTrying)
		{
			PeopleList.LockWindowUpdate();
			PeopleList.ResetContent();
			for (INT_PTR i = 0; i < People.GetSize(); i++)
			{
				Person& p = *People[i];
				int idx = PeopleList.AddString(p.Format(Person::FT_DISPLAY_TO_USER));
				PeopleList.SetItemData(idx,(DWORD_PTR)(&p));
			}
			PeopleList.UnlockWindowUpdate();
		}
	}

	KillTimer(PROGUPDATETIMER_ID);

	AddBtn.EnableWindow(TRUE);
	DrawNamesBtn.EnableWindow(TRUE);

	CString msg;
	msg.Format(_T("Got it in %d tries, %d draws, %d rejected solutions"), NumTries, NumDraws, NumRejected);
	MsgWnd.SetWindowText(msg);
}

void CPollyannaDlg::Save()
{
   CFile f;
   if (f.Open(SAVEFILE,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary))
   {
      CArchive ar(&f,CArchive::store);

      Serialize(ar);

      ar.Flush();
      ar.Close();
   }
}

void CPollyannaDlg::Load()
{
   CFile f;
   if (f.Open(SAVEFILE,CFile::modeRead|CFile::typeBinary))
   {
      CArchive ar(&f,CArchive::load);

      Serialize(ar);

      ar.Flush();
      ar.Close();
   }
}

void CPollyannaDlg::Serialize(CArchive& ar)
{
   BYTE Schema = 0;
   BYTE LoadSchema;

   if (ar.IsStoring())
   {
      ar << Schema;

      int sz = PeopleList.GetCount();
      ar << sz;

      for (int i = 0; i < PeopleList.GetCount(); i++)
      {
         Person& p = *((Person*)PeopleList.GetItemData(i));
         p.Serialize(ar);
      }
   }
   else
   {
      ar >> LoadSchema;

      int sz;
      ar >> sz;

      PeopleList.ResetContent();
      for (int i = 0; i < sz; i++)
      {
         Person& p = *new Person;
         p.Serialize(ar);
         p.TargetPerson = NULL;

		 int idx = PeopleList.AddString(p.Format(Person::FT_DISPLAY_TO_USER));
         PeopleList.SetItemData(idx,(DWORD_PTR)(&p));
      }
   }
}

void CPollyannaDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == PROGUPDATETIMER_ID)
	{
		DWORD_PTR elapsedTickCount = GetTickCount() - StartTickCount;
		double elapsedHrs = (double)elapsedTickCount / 1000.0 / 60.0 / 60.0;

		CString s;
		s.Format(_T("%0.4f hrs - %d t %d d %d r"), elapsedHrs, NumTries, NumDraws, NumRejected);
		SetWindowText(s);
	}

	CDialog::OnTimer(nIDEvent);
}

void CPollyannaDlg::OnClose()
{
	KeepRunning = FALSE;
	PostQuitMessage(0);

	CDialog::OnClose();
}



void CPollyannaDlg::OnDblclkPeople()
{
	OnBnClickedEditBtn();
}


void CPollyannaDlg::OnBnClickedWritePicksToFileBtn()
{
	for (int i = 0; i < PeopleList.GetCount(); i++)
	{
		Person& p = *((Person*)PeopleList.GetItemData(i));
		if (p.TargetPerson == NULL)
		{
			AfxMessageBox(_T("Draw Names first!"));
			return;
		}
	}

	CTime now = CTime::GetCurrentTime();

	CString fileName;
	fileName.Format(_T("%spollyanna.prevpicks-%s-base64.txt"),
		theApp.appDir,
		now.Format(_T("%Y")));

	CStdioFile f;
	if (!f.Open(fileName,CFile::modeCreate|CFile::modeWrite|CFile::typeText))
	{
		AfxMessageBox(_T("File open error"));
		return;
	}

	for (int i = 0; i < PeopleList.GetCount(); i++)
	{
		Person& p = *((Person*)PeopleList.GetItemData(i));

		CString s;
		s.Format(_T("%s (%s) => %s (%s)"),
			p.Name,
			p.Family,
			p.TargetPerson->Name,
			p.TargetPerson->Family);

		CString es = STR_A2T(base64_encode((unsigned char*)(LPCSTR)(STR_T2A(s)),s.GetLength()).c_str());

		f.WriteString(es + _T("\n"));
	}

	f.Flush();
	f.Close();

	AfxMessageBox(_T("File Written"));
}

BOOL CPollyannaDlg::SendEmail(CStringArray& recipients, LPCTSTR subject, CStringArray& msgLines, CString& errDesc)
{
	CWaitCursor wc;

	try
	{
		CSmtp mail;

		mail.SetSMTPServer(STR_T2A(EMAIL_CONFIG_SMTP_SERVER),EMAIL_CONFIG_SMTP_PORT);
		mail.SetSenderName(STR_T2A(EMAIL_CONFIG_SENDER_NAME));
		mail.SetSenderMail(STR_T2A(EMAIL_CONFIG_SENDER_EMAIL));
		mail.SetReplyTo(STR_T2A(EMAIL_CONFIG_REPLYTO_EMAIL));

		mail.SetXPriority(XPRIORITY_NORMAL);
		mail.SetXMailer("CSmtp mailer");

		for (INT_PTR i = 0; i < recipients.GetSize(); i++)
			mail.AddRecipient(STR_T2A(recipients[i]));

		mail.SetSubject(STR_T2A(subject));

		for (INT_PTR i = 0; i < msgLines.GetSize(); i++)
			mail.AddMsgLine(STR_T2A(msgLines[i]));

		mail.Send();
	}
	catch(ECSmtp e)
	{
		errDesc = STR_A2T(e.GetErrorText().c_str());
		return FALSE;
	}

	return TRUE;
}

void CPollyannaDlg::OnBnClickedEmailPicksBtn()
{
	for (int i = 0; i < PeopleList.GetCount(); i++)
	{
		Person& p = *((Person*)PeopleList.GetItemData(i));
		if (p.TargetPerson == NULL)
		{
			AfxMessageBox(_T("Draw Names first!"));
			return;
		}
	}

	map<CString,vector<CString>> msgMap; // recip => msg lines

	for (int i = 0; i < PeopleList.GetCount(); i++)
	{
		Person& p = *((Person*)PeopleList.GetItemData(i));

		vector<CString> msgLines;
		auto j = msgMap.find(p.Email);
		if (j != msgMap.end())
		{
			msgLines = j->second;
		}
		else
		{
			msgLines.push_back(_T("This message contains the results of the gift drawing."));
			msgLines.push_back(_T("In the below, Joe => Dad means that Joe will get a gift for Dad."));
			msgLines.push_back(_T(""));
		}

		CString s;
		s.Format(_T("%s (%s) => %s (%s)"),
			p.Name,
			p.Family,
			p.TargetPerson->Name,
			p.TargetPerson->Family);
		msgLines.push_back(s);

		msgMap[p.Email] = msgLines;
	}

	CString errDesc;
	DWORD_PTR numErrors = 0;
	for (auto j = msgMap.begin(); j != msgMap.end(); j++)
	{
		CString recipient = j->first;
		vector<CString> msgLines = j->second;

		CStringArray recipients;
		recipients.Add(recipient);

		CStringArray msgLinesCSA;
		for (auto k = msgLines.begin(); k != msgLines.end(); k++)
		{
			msgLinesCSA.Add(*k);
		}

		if (!SendEmail(recipients,_T("Gift Drawing Results"),msgLinesCSA,errDesc))
		{
			TRACE1("Email Error: %s\n", errDesc);
			numErrors++;
		}
	}

	if (numErrors > 0)
		AfxMessageBox(_T("Finished with errors."));
	else
		AfxMessageBox(_T("Finished - no errors!"));
}


void CPollyannaDlg::OnBnClickedDecodePicksFileBtn()
{
	CFileDialog dlg(TRUE,_T("*.txt"),_T("prevpicks-base64.txt"),0,_T("Text Files (*.txt)|*.txt||"));
	if (dlg.DoModal() != IDOK)
		return;

	CStdioFile fin;
	if (!fin.Open(dlg.GetPathName(),CFile::modeRead|CFile::shareDenyWrite|CFile::typeText))
	{
		AfxMessageBox(_T("Unable to open input file!"));
		return;
	}

	CStdioFile fout;
	if (!fout.Open(dlg.GetPathName() + _T("-decoded.txt"),CFile::modeCreate|CFile::modeWrite|CFile::typeText))
	{
		AfxMessageBox(_T("Unable to open output file!"));
		return;
	}

	CString lineEnc;
	while (fin.ReadString(lineEnc))
	{
		CString line = STR_A2T(base64_decode(string(STR_T2A(lineEnc))).c_str());
		fout.WriteString(line + _T("\n"));
	}

	fout.Flush();
	fout.Close();

	fin.Close();
}
