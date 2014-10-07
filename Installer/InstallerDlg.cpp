
// InstallerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Installer.h"
#include "InstallerDlg.h"
#include "afxdialogex.h"

#include "Function.h"
#include "LzmaLib\LzmaLib.h"
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInstallerDlg �Ի���




CInstallerDlg::CInstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInstallerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strDestination = _T("");
	m_hThread=NULL;
	m_bExitTask=FALSE;
}

void CInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DESTINATION, m_strDestination);
	DDX_Control(pDX, IDC_PROGRESS, m_pcProgress);
	DDX_Text(pDX, IDC_STATIC_INFORMATION, m_strInformation);
	DDX_Control(pDX, IDC_STATIC_INFORMATION, m_static_information);

}

BEGIN_MESSAGE_MAP(CInstallerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OK, &CInstallerDlg::OnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_EXPLOREDES, &CInstallerDlg::OnClickedButtonExploredes)
	ON_BN_CLICKED(IDCANCEL, &CInstallerDlg::OnBnClickedCancel)
	ON_MESSAGE(THREAD_INSTAL_OVER,&CInstallerDlg::OnInstalTaskOver)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CInstallerDlg ��Ϣ�������

BOOL CInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CInstallerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CInstallerDlg::InitUIStatus(int nStatus)
{
	UINT uID[]={IDC_EDIT_DESTINATION,IDC_BUTTON_EXPLOREDES,IDC_OK};
	switch (nStatus)
	{
	case 0:
		{
			for (int i=0;i<sizeof(uID)/sizeof(uID[0]);i++)
			{
				GetDlgItem(uID[i])->EnableWindow(TRUE);
			}
		}
		break;
	case 1:
		{
			for (int i=0;i<sizeof(uID)/sizeof(uID[0]);i++)
			{
				GetDlgItem(uID[i])->EnableWindow(FALSE);
			}
		}
		break;
	default:
		break;
	}
}

void CInstallerDlg::OnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InitUIStatus(1);
	CString strMsg;
	int nDesType=CheckDestinationDirectory(m_strDestination);
	if (0 != nDesType)
	{
		strMsg=_T("��������ȷ�İ�װĿ¼");
		MessageBox(strMsg);
		InitUIStatus(0);
		return;
	}

	if (!InitThread(strMsg))
	{
		MessageBox(strMsg);
		InitUIStatus(0);
		return;
	}
}


void CInstallerDlg::OnClickedButtonExploredes()
{
	// TODO: Add your control notification handler code here
	BROWSEINFO binfo;
	memset(&binfo,0x00,sizeof(binfo));
	binfo.hwndOwner=GetSafeHwnd();
	TCHAR szDesPath[MAX_PATH]={0};
	binfo.lpszTitle=_T("��ѡ��װĿ¼");
	binfo.ulFlags=BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST lpDlist;
	lpDlist=SHBrowseForFolder(&binfo);
	if (NULL != lpDlist)
	{
		SHGetPathFromIDList(lpDlist,szDesPath);
		m_strDestination=szDesPath;
		UpdateData(FALSE);
	}

	
}


void CInstallerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


//////////////////////////////////////////////////////////////////////////
//	ִ���߳�
//////////////////////////////////////////////////////////////////////////
void CInstallerDlg::CloseThreadHand()
{
	if (NULL!=m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread=NULL;
	}

	// 	if (NULL != m_hExitEvent)
	// 	{
	// 		CloseHandle(m_hExitEvent);
	// 		m_hExitEvent=NULL;
	// 	}
}

BOOL CInstallerDlg::InitThread(CString& strMsg)
{
	CloseThreadHand();
	m_bExitTask=FALSE;

	// ��ȡ��Դ��ַ
	HINSTANCE hInstance=AfxGetInstanceHandle();
	HRSRC hRsrc=FindResource(hInstance,MAKEINTRESOURCE(IDR_DATA_BINARY),_T("DATA"));
	if (NULL==hRsrc)
	{
		strMsg.Format("%s[%d]",_T("������Դ�ļ�ʧ��"),
			GetLastError());
		return FALSE;
	}
	HGLOBAL hGlobal=LoadResource(NULL,hRsrc);
	if (NULL==hGlobal)
	{
		strMsg.Format("%s-[%d]",_T("������Դ�ļ�ʧ��"),
			GetLastError());
		return FALSE;
	}
	m_pResourceBlock=(unsigned char*)LockResource(hGlobal);//pvָ���ڴ��ĵ�һ���ֽڣ�Ϊ���ֽ�ָ�� 
	if (NULL==m_pResourceBlock)
	{
		strMsg.Format("%s--[%d]",_T("������Դ�ļ�ʧ��"),
			GetLastError());
		return FALSE;
	}

	m_dwTotalSize=SizeofResource(hInstance,hRsrc);
	if (0==m_dwTotalSize)
	{
		strMsg=_T("������Դ�ļ�����Сʧ��");
		return FALSE;
	}
	m_dwDoneSize=0;
	m_pcProgress.SetRange(0,m_dwTotalSize/1024);
	m_pcProgress.SetPos(0);
	m_csInfoQueue.Lock();
	while(0 != m_infoQueue.size())
	{
		m_infoQueue.pop();
	}
	m_csInfoQueue.Unlock();

	SetTimer(0x01,100,NULL);

	m_hThread=(HANDLE)_beginthreadex(NULL,0,ThreadProc,(void* )this,1,NULL);
	if (NULL==m_hThread)
	{
		m_pResourceBlock=NULL;
		KillTimer(0x01);
		strMsg=_T("���������߳�ʧ��");
		CloseThreadHand();
		return FALSE;
	}
	return TRUE;
}

int CInstallerDlg::DoJob()
{
	static char szMsg[1024]={0};

	memset(szMsg,0x00,sizeof(szMsg));

	// ����ָ������
	memset(szMsg,0x00,sizeof(szMsg));
	VERIFY(m_dwTotalSize>0);
	CString strDestination=m_strDestination;
	strDestination.Replace("//","\\");
	strDestination.Replace("/","\\");
	strDestination.Replace("\\\\","\\");
	if ('\\' != strDestination.GetAt(strDestination.GetLength()-1))
	{
		strDestination+="\\";
	}
	int nReturn=UnPackageFile(m_pResourceBlock,m_dwTotalSize,strDestination,szMsg,sizeof(szMsg));
	if (0 != nReturn)
	{
		::PostMessage(GetSafeHwnd(),THREAD_INSTAL_OVER,(WPARAM)INSTAL_STATUS_FAIL,(LPARAM)szMsg);
	}
	else
	{
		::PostMessage(GetSafeHwnd(),THREAD_INSTAL_OVER,(WPARAM)INSTAL_STATUS_SUCCESS,(LPARAM)szMsg);
	}

	return nReturn;

}

void CInstallerDlg::ExitThread()
{

	m_bExitTask=TRUE;
	WaitForSingleObject(m_hThread,INFINITE);
	KillTimer(0x01);
	m_pcProgress.SetPos(0);
	CloseThreadHand();
	m_bExitTask=FALSE;
	m_pResourceBlock=NULL;
}

unsigned WINAPI CInstallerDlg::ThreadProc(void* lpParam)
{
	CInstallerDlg* pThread=(CInstallerDlg* )lpParam;
	VERIFY(NULL != lpParam);
	return pThread->DoJob();
}

LRESULT CInstallerDlg::OnInstalTaskOver(WPARAM wParam,LPARAM lParam)
{
	int nResult=(int)wParam;
	char* pMsg=(char*)lParam;

	if (INSTAL_STATUS_FAIL==nResult)
	{
		MessageBox(pMsg);
	}
	WaitForSingleObject(m_hThread,INFINITE);
	KillTimer(0x01);
	CloseThreadHand();
	m_bExitTask=FALSE;
	m_pResourceBlock=NULL;
	while(0 != m_infoQueue.size())
	{
		m_infoQueue.pop();
	}
	m_strInformation=_T("");
	UpdateData(FALSE);
	m_static_information.Invalidate();
	m_pcProgress.SetPos(0);
	InitUIStatus(0);

	if (INSTAL_STATUS_SUCCESS == nResult)
	{
		CDialogEx::OnOK();
	}
	return 0;
}

void CInstallerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (0x01 == nIDEvent)
	{
		KillTimer(0x01);
		UpdateProgress();
		SetTimer(0x01,100,NULL);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CInstallerDlg::UpdateProgress()
{
	m_csInfoQueue.Lock();
	if (!m_infoQueue.empty())
	{
		string strInfo=m_infoQueue.front();
		m_infoQueue.pop();

		m_strInformation=strInfo.c_str();
	}
	else
	{
		UpdateData(TRUE);
		if (!m_strInformation.IsEmpty())
		{
			if ("..." == m_strInformation.Right(3))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-3);
				m_strInformation+=".";
			}
			else if (".."==m_strInformation.Right(2))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-2);
				m_strInformation+="...";
			}
			else if ("."==m_strInformation.Right(1))
			{
				m_strInformation=m_strInformation.Left(m_strInformation.GetLength()-1);
				m_strInformation+="..";
			}
			else
			{
				m_strInformation+=".";
			}
		}
	}
	m_csInfoQueue.Unlock();
	UpdateData(FALSE);
	m_static_information.Invalidate();
	TRACE(m_strInformation);
	TRACE("\r\n");
}

void CInstallerDlg::ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength)
{
	// 	m_strInformation.Format("File=%s,t=%d,d=%d",pszFile,dwTotalLen,dwDealLength);
	// 	UpdateData(FALSE);
	// 	m_static_information.Invalidate();
	// 	Sleep(100);
	string strInformation=pszFile;
	m_csInfoQueue.Lock();
	m_infoQueue.push(strInformation);
	m_dwDoneSize+=dwDealLength;
	m_pcProgress.SetPos(m_dwDoneSize/1024);
	m_csInfoQueue.Unlock();
}

void CInstallerDlg::UpdateProgressPos(size_t dwDone)
{
	m_dwDoneSize+=dwDone;
	m_pcProgress.SetPos(m_dwDoneSize/1024);
}

int CInstallerDlg::UnPackageFile(unsigned char* pszSrcFile, DWORD dwFileLen, const char* pszDesPath, char* szMsg, int nMsgLen)
{
	// ����ļ�����

	DWORD dwPosition=0;
	size_t srcLen = 0;
	size_t destLen = 0;
	unsigned char* psrcRead = NULL; //ԭʼ�ļ�����
	unsigned char* pDecomress = NULL; //��Ž�ѹ������

	unsigned char prop[5] = {0};
	size_t sizeProp = 5;

	CString strFileDir;
	while( dwFileLen > dwPosition )
	{
		// �ļ�ͷ��Ϣ����
		BINARYINSTALLRESOURCEHEAD	FileHead;
		memset(&FileHead,0x00,sizeof(FileHead));
		memcpy(&FileHead,pszSrcFile+dwPosition,sizeof(FileHead));
		dwPosition+=sizeof(FileHead);
		VERIFY(dwPosition<=dwFileLen);

		if (FILE_ATTRIBUTE_DIRECTORY == FileHead.dwFileAttributes)
		{
			ShowProgress(FileHead.szFileFullPath,0,0);

			// ����Ŀ¼
			strFileDir.Empty();
			strFileDir.Format("%s%s",pszDesPath,FileHead.szFileFullPath);
			CFileStatus FileStatus;
			if (!CFile::GetStatus(strFileDir, FileStatus))
			{
				if (0 == MakeSureDirectoryPathExists(strFileDir))
				{
					sprintf(szMsg,"����Ŀ¼[%s]ʧ��",FileHead.szFileFullPath);
					return -1;
				}
			}

			UpdateProgressPos(sizeof(FileHead));
			continue;
		}

		// дĿ���ļ�
		ShowProgress(FileHead.szFileFullPath,0,0);
		CString strFile;
		strFile.Format("%s%s",pszDesPath,FileHead.szFileFullPath);

		// ����Ŀ¼
		int nPos=strFile.ReverseFind('\\');
		if (-1 == nPos)
		{
			sprintf(szMsg,"��Դ�ļ�[%s]��ʽ����",
				FileHead.szFileFullPath);
			return -1;
		}

		strFileDir=strFile.Left(nPos);
		CFileStatus FileStatus;
		if (!CFile::GetStatus(strFileDir, FileStatus))
		{
			if (0 == MakeSureDirectoryPathExists(strFileDir))
			{
				sprintf(szMsg,"����Ŀ¼[%s]ʧ��",FileHead.szFileFullPath);
				return -1;
			}
		}

		FILE* pDecompressFile = _tfopen(strFile, _T("ab")); 
		//д���ѹ������
		if (pDecompressFile == NULL)
		{
			sprintf(szMsg,"д���ļ�[%s]ʧ��",FileHead.szFileFullPath);
			return -1;
		}

		int nWriteDoneLength=0;
		do
		{
			// ���ָ����Ϣ
			COMPRESSEDPART	compressedPart;
			memset(&compressedPart,0x00,sizeof(compressedPart));
			memcpy(&compressedPart,pszSrcFile+dwPosition,sizeof(compressedPart));
			dwPosition+=sizeof(compressedPart);
			VERIFY(dwPosition<=dwFileLen);
			UpdateProgressPos(sizeof(compressedPart));

			srcLen=compressedPart.dwCompressedFileSize;
			destLen=compressedPart.dwPartSize;

			psrcRead = new unsigned char[srcLen+1]; //ԭʼ�ļ�����
			pDecomress = new unsigned char[destLen*2]; //��Ž�ѹ������
			memset(psrcRead,0x00,srcLen+1);
			memset(pDecomress,0x00,destLen*2);
			memcpy(psrcRead,pszSrcFile+dwPosition,srcLen);
			dwPosition+=srcLen;
			VERIFY(dwPosition<=dwFileLen);

			//ע�⣺��ѹ��ʱprops����Ҫʹ��ѹ��ʱ���ɵ�outProps����������������ѹ��
			if (SZ_OK != LzmaUncompress(pDecomress, &destLen, psrcRead, &srcLen, compressedPart.szCompressProp, 5))
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pDecompressFile);
				sprintf(szMsg,"�ļ�[%s]��ѹʧ��",FileHead.szFileFullPath);
				return -1;
			}

			if (compressedPart.dwPartSize != destLen)
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pDecompressFile);
				sprintf(szMsg,"�ļ�[%s]��",FileHead.szFileFullPath);
				return -1;
			}

			fwrite(pDecomress, sizeof(char), destLen, pDecompressFile);
			nWriteDoneLength+=destLen;

			UpdateProgressPos(srcLen);

			VERIFY(nWriteDoneLength<=FileHead.dwFileSize);
			delete [] psrcRead;
			delete [] pDecomress;
		}
		while (nWriteDoneLength < FileHead.dwFileSize);

		fclose(pDecompressFile);

	}
	return 0;
}