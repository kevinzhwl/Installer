
// InstallerDlg.h : ͷ�ļ�
//
#include "Function.h"
#pragma once


// CInstallerDlg �Ի���
class CInstallerDlg : public CDialogEx
{
// ����
public:
	CInstallerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_INSTALLER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	//	0-�ɰ�װ״̬��1-���ڰ�װ״̬
	void	InitUIStatus(int nStatus);


//////////////////////////////////////////////////////////////////////////
// �����߳�
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

	// ������ʾ
	PROGRESSINFORMATIONQUEUE	m_infoQueue;
	CCriticalSection			m_csInfoQueue;	// ��ǰ�����ļ�
	DWORD						m_dwTotalSize;
	DWORD						m_dwDoneSize;
	unsigned char*				m_pResourceBlock;
public:
	CStatic m_static_information;
	CString	m_strInformation;

};
