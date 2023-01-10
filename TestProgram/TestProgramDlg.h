
// TestProgramDlg.h: 头文件
//

#pragma once

#include <winsock2.h> //winsock
#include <windns.h> //DNS api's

#include <string.h>
#include <cassert>
#include <fstream>

#include <json/json.h>
#include <curl/curl.h>

#pragma comment(lib,"Dnsapi.lib")
#pragma comment(lib,"lib/jsoncpp.lib")
#pragma comment(lib,"lib/libcurl.lib")


class CTestProgramDlgAutoProxy;

// CTestProgramDlg 对话框
class CTestProgramDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTestProgramDlg);
	friend class CTestProgramDlgAutoProxy;

// 构造
public:
	CTestProgramDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CTestProgramDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTPROGRAM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	CTestProgramDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg LRESULT OnReceiveUpdateInfo(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};
