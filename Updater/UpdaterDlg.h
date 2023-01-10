
// UpdaterDlg.h: 头文件
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


// CUpdaterDlg 对话框
class CUpdaterDlg : public CDialogEx
{
// 构造
public:
	CUpdaterDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnFinishInitUpdateDlg(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CProgressCtrl m_ProgressCurrent;
	CProgressCtrl m_ProgressTotal;
	afx_msg void OnBnClickedCancel();
};
