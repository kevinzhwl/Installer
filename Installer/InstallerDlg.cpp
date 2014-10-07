
// InstallerDlg.cpp : 实现文件
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


// CInstallerDlg 对话框




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


// CInstallerDlg 消息处理程序

BOOL CInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CInstallerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
		strMsg=_T("请输入正确的安装目录");
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
	binfo.lpszTitle=_T("请选择安装目录");
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
//	执行线程
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

	// 获取资源地址
	HINSTANCE hInstance=AfxGetInstanceHandle();
	HRSRC hRsrc=FindResource(hInstance,MAKEINTRESOURCE(IDR_DATA_BINARY),_T("DATA"));
	if (NULL==hRsrc)
	{
		strMsg.Format("%s[%d]",_T("加载资源文件失败"),
			GetLastError());
		return FALSE;
	}
	HGLOBAL hGlobal=LoadResource(NULL,hRsrc);
	if (NULL==hGlobal)
	{
		strMsg.Format("%s-[%d]",_T("加载资源文件失败"),
			GetLastError());
		return FALSE;
	}
	m_pResourceBlock=(unsigned char*)LockResource(hGlobal);//pv指向内存块的第一个字节，为单字节指针 
	if (NULL==m_pResourceBlock)
	{
		strMsg.Format("%s--[%d]",_T("加载资源文件失败"),
			GetLastError());
		return FALSE;
	}

	m_dwTotalSize=SizeofResource(hInstance,hRsrc);
	if (0==m_dwTotalSize)
	{
		strMsg=_T("计算资源文件包大小失败");
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
		strMsg=_T("创建工作线程失败");
		CloseThreadHand();
		return FALSE;
	}
	return TRUE;
}

int CInstallerDlg::DoJob()
{
	static char szMsg[1024]={0};

	memset(szMsg,0x00,sizeof(szMsg));

	// 处理指定任务
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
	// 输出文件内容

	DWORD dwPosition=0;
	size_t srcLen = 0;
	size_t destLen = 0;
	unsigned char* psrcRead = NULL; //原始文件数据
	unsigned char* pDecomress = NULL; //存放解压缩数据

	unsigned char prop[5] = {0};
	size_t sizeProp = 5;

	CString strFileDir;
	while( dwFileLen > dwPosition )
	{
		// 文件头信息处理
		BINARYINSTALLRESOURCEHEAD	FileHead;
		memset(&FileHead,0x00,sizeof(FileHead));
		memcpy(&FileHead,pszSrcFile+dwPosition,sizeof(FileHead));
		dwPosition+=sizeof(FileHead);
		VERIFY(dwPosition<=dwFileLen);

		if (FILE_ATTRIBUTE_DIRECTORY == FileHead.dwFileAttributes)
		{
			ShowProgress(FileHead.szFileFullPath,0,0);

			// 创建目录
			strFileDir.Empty();
			strFileDir.Format("%s%s",pszDesPath,FileHead.szFileFullPath);
			CFileStatus FileStatus;
			if (!CFile::GetStatus(strFileDir, FileStatus))
			{
				if (0 == MakeSureDirectoryPathExists(strFileDir))
				{
					sprintf(szMsg,"创建目录[%s]失败",FileHead.szFileFullPath);
					return -1;
				}
			}

			UpdateProgressPos(sizeof(FileHead));
			continue;
		}

		// 写目标文件
		ShowProgress(FileHead.szFileFullPath,0,0);
		CString strFile;
		strFile.Format("%s%s",pszDesPath,FileHead.szFileFullPath);

		// 创建目录
		int nPos=strFile.ReverseFind('\\');
		if (-1 == nPos)
		{
			sprintf(szMsg,"资源文件[%s]格式错误",
				FileHead.szFileFullPath);
			return -1;
		}

		strFileDir=strFile.Left(nPos);
		CFileStatus FileStatus;
		if (!CFile::GetStatus(strFileDir, FileStatus))
		{
			if (0 == MakeSureDirectoryPathExists(strFileDir))
			{
				sprintf(szMsg,"创建目录[%s]失败",FileHead.szFileFullPath);
				return -1;
			}
		}

		FILE* pDecompressFile = _tfopen(strFile, _T("ab")); 
		//写入解压缩数据
		if (pDecompressFile == NULL)
		{
			sprintf(szMsg,"写入文件[%s]失败",FileHead.szFileFullPath);
			return -1;
		}

		int nWriteDoneLength=0;
		do
		{
			// 读分割段信息
			COMPRESSEDPART	compressedPart;
			memset(&compressedPart,0x00,sizeof(compressedPart));
			memcpy(&compressedPart,pszSrcFile+dwPosition,sizeof(compressedPart));
			dwPosition+=sizeof(compressedPart);
			VERIFY(dwPosition<=dwFileLen);
			UpdateProgressPos(sizeof(compressedPart));

			srcLen=compressedPart.dwCompressedFileSize;
			destLen=compressedPart.dwPartSize;

			psrcRead = new unsigned char[srcLen+1]; //原始文件数据
			pDecomress = new unsigned char[destLen*2]; //存放解压缩数据
			memset(psrcRead,0x00,srcLen+1);
			memset(pDecomress,0x00,destLen*2);
			memcpy(psrcRead,pszSrcFile+dwPosition,srcLen);
			dwPosition+=srcLen;
			VERIFY(dwPosition<=dwFileLen);

			//注意：解压缩时props参数要使用压缩时生成的outProps，这样才能正常解压缩
			if (SZ_OK != LzmaUncompress(pDecomress, &destLen, psrcRead, &srcLen, compressedPart.szCompressProp, 5))
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pDecompressFile);
				sprintf(szMsg,"文件[%s]解压失败",FileHead.szFileFullPath);
				return -1;
			}

			if (compressedPart.dwPartSize != destLen)
			{
				delete [] psrcRead;
				delete [] pDecomress;
				fclose(pDecompressFile);
				sprintf(szMsg,"文件[%s]损坏",FileHead.szFileFullPath);
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