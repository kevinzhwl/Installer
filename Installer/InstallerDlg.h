
// InstallerDlg.h : 头文件
//
#include "Function.h"
#pragma once


// CInstallerDlg 对话框
class CInstallerDlg : public CDialogEx
{
// 构造
public:
	CInstallerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_INSTALLER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strDestination;
	CProgressCtrl m_pcProgress;
	afx_msg void OnClickedOk();
	afx_msg void OnClickedButtonExploredes();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnInstalTaskOver(WPARAM wParam,LPARAM lParam);

private:
	//	0-可安装状态，1-正在安装状态
	void	InitUIStatus(int nStatus);


//////////////////////////////////////////////////////////////////////////
// 工作线程
//////////////////////////////////////////////////////////////////////////
private:
	int		InitThread(CString& strMsg);
	static	unsigned WINAPI ThreadProc(void* lpParam);
	int		DoJob();
	void	ExitThread();
	void	CloseThreadHand();

	void	ShowProgress(const char* pszFile,DWORD dwTotalLen,DWORD dwDealLength);
	void	UpdateProgress();
	void	UpdateProgressPos(size_t dwDone);
	int		UnPackageFile(unsigned char* pszSrcFile, DWORD dwFileLen, const char* pszDesPath, char* szMsg, int nMsgLen);
private:
	HANDLE	m_hThread;
	BOOL	m_bExitTask;

	// 进度显示
	PROGRESSINFORMATIONQUEUE	m_infoQueue;
	CCriticalSection			m_csInfoQueue;	// 当前处理文件
	DWORD						m_dwTotalSize;
	DWORD						m_dwDoneSize;
	unsigned char*				m_pResourceBlock;
public:
	CStatic m_static_information;
	CString	m_strInformation;

};
