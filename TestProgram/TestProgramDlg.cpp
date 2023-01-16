
// TestProgramDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TestProgram.h"
#include "TestProgramDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTestProgramDlg 对话框


IMPLEMENT_DYNAMIC(CTestProgramDlg, CDialogEx);

CTestProgramDlg::CTestProgramDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTPROGRAM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = nullptr;
}

CTestProgramDlg::~CTestProgramDlg()
{
	// 如果该对话框有自动化代理，则
	//  此对话框的返回指针为 null，所以它知道
	//  此代理知道该对话框已被删除。
	if (m_pAutoProxy != nullptr)
		m_pAutoProxy->m_pDialog = nullptr;
}

void CTestProgramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

enum USER_MESSAGES {
	UM_UPDATEINFO = WM_USER + 101
};
BEGIN_MESSAGE_MAP(CTestProgramDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(UM_UPDATEINFO , &CTestProgramDlg::OnReceiveUpdateInfo)
END_MESSAGE_MAP()


// CTestProgramDlg 消息处理程序
HANDLE hThread = NULL;
CMutex mLock;
CString strVersion = L"v1.0.0-alpha Build001";
int mBuild = 1;

std::string UnicodeStringToSTLString(CString strUTF) {
	int n = strUTF.GetLength();
	int len = WideCharToMultiByte(CP_ACP, 0, strUTF, strUTF.GetLength(), NULL, 0, NULL, NULL);
	char* pStrSTL = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, strUTF, strUTF.GetLength(), pStrSTL, len, NULL, NULL);
	pStrSTL[len + 1] = '/0';
	return pStrSTL;
}

//HTTP回调函数（header和content通用）
size_t curl_default_callback(const char* ptr, size_t size, size_t nmemb, std::string* stream) {
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}
//文件下载回调函数
size_t curl_writefile_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

DWORD WINAPI OnCheckUpdate(LPVOID lpParam) {
	//线程Init	

	CTestProgramDlg* pDlg = (CTestProgramDlg*)lpParam;
	DNS_STATUS status;
	PDNS_RECORD pDnsRecord;
	CString strLQueryResult;
	std::string strQueryResult;
	Json::Reader jReader;
	Json::Value jRoot,jVersionInfo;

	mLock.Unlock();//线程互斥

	//线程Init结束


	//解析节点
	//		参考：https://learn.microsoft.com/zh-cn/troubleshoot/windows/win32/use-dnsquery-resolve-host-names
	status = DnsQuery(
		L"nodes.update.api.iyoroy.cn",	//TXT记录的主机名
		DNS_TYPE_TEXT,					//查询类型TXT
		DNS_QUERY_STANDARD,				//标准查询方式
		NULL,
		&pDnsRecord,
		NULL
	);
	if (status)return 0;//网络异常，无法查询DNS记录
	strLQueryResult = *(pDnsRecord->Data.TXT.pStringArray);
	strQueryResult = UnicodeStringToSTLString(strLQueryResult);
	//AfxMessageBox(strQueryResult);		//Test

	if (!jReader.parse(strQueryResult, jRoot))return 0;//无法解析JSON，终止

	double mMinTime = 60;
	int mLatestBuild = mBuild;
	std::string strTargetServerUrl;
	CURL* mCurl = curl_easy_init();
	CURLcode mCode;
	std::string mUA = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1 TestProject/1.0.0";
	std::string szbuffer;
	std::string szheader_buffer;
	double val;
	//curl init
	curl_global_init(CURL_GLOBAL_ALL);
	curl_easy_setopt(mCurl, CURLOPT_USERAGENT, mUA.c_str());	//设置UserAgent
	curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, 0L);		//设置SSL验证级别（0-2，宽松-严格）
	curl_easy_setopt(mCurl, CURLOPT_CAINFO, "cacert.pem");		//根证书信息
	curl_easy_setopt(mCurl, CURLOPT_MAXREDIRS, 5);				//设置最大重定向次数
	curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1);			//设置301、302跳转跟随location
	curl_easy_setopt(mCurl, CURLOPT_TIMEOUT, 30L);				//超时设置
	curl_easy_setopt(mCurl, CURLOPT_CONNECTTIMEOUT, 10L);		//连接超时设置
	curl_easy_setopt(mCurl, CURLOPT_FAILONERROR, 1L);			//服务端返回40x代码时返回错误而不是下载错误页
	//抓取内容后，回调函数  
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, curl_default_callback);
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, &szbuffer);
	//抓取头信息，回调函数  
	curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, curl_default_callback);
	curl_easy_setopt(mCurl, CURLOPT_HEADERDATA, &szheader_buffer);
	//curl init done

	for (int i = 0; i < jRoot.size(); i++) {
		
		szbuffer = "";
		szheader_buffer = "";

		std::string mServerUrl = jRoot[i].asString();
		std::string mVersionUrl = mServerUrl + "Release.json";

		curl_easy_setopt(mCurl, CURLOPT_URL, mVersionUrl.c_str());	//设置URL


		mCode = curl_easy_perform(mCurl);
		if (mCode != CURLE_OK) {//下载失败，中断
			//curl_easy_cleanup(mCurl);
			continue;
		}

		if (!jReader.parse(szbuffer, jVersionInfo)) {//解析失败，服务端配置文件不合法
			//curl_easy_cleanup(mCurl);
			continue;
		}

		mCode = curl_easy_getinfo(mCurl, CURLINFO_SIZE_DOWNLOAD, &val);
		if ((CURLE_OK == mCode) && (val > 0))
			TRACE(L"Data downloaded: %0.0f bytes.\n", val);
		mCode = curl_easy_getinfo(mCurl, CURLINFO_SPEED_DOWNLOAD, &val);
		if ((CURLE_OK == mCode) && (val > 0))
			TRACE(L"Average download speed: %0.3f kbyte/sec.\n", val / 1024);
		mCode = curl_easy_getinfo(mCurl, CURLINFO_TOTAL_TIME, &val);
		if ((CURLE_OK == mCode) && (val > 0))
			TRACE(L"Total download time: %0.3f sec.\n", val);

		TRACE(L"Get ReleaseVer Finish.");
		//curl_easy_cleanup(mCurl);

		if (mLatestBuild < jVersionInfo["build"].asInt() && val <= mMinTime) {
			mLatestBuild = jVersionInfo["build"].asInt();
			mMinTime = val;
			strTargetServerUrl = mServerUrl;
		}

	}

	if (mLatestBuild == mBuild) {			//最新版本build代号和当前相同则终止
		TRACE(L"No New Release found.");
		return 0;
	}

	TRACE(L"Select Node finish!");

//#ifndef DEBUG
	FILE* pUpdaterFile;
	std::string mUpdaterUrl = strTargetServerUrl + "Updater.exe";

	if (fopen_s(&pUpdaterFile, "Updater.exe", "wb")) {//无法创建更新器文件
		TRACE(L"Failed to open file!");
		return 0; 
	}

	curl_easy_setopt(mCurl, CURLOPT_URL, mUpdaterUrl.c_str());				//设置更新器URL
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, curl_writefile_callback);//设置回调函数（不同于写入字符串）
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, pUpdaterFile);				//将内容写至文件句柄
	curl_easy_setopt(mCurl, CURLOPT_TIMEOUT, 60L);							//超时设置

	mCode = curl_easy_perform(mCurl);

	fclose(pUpdaterFile);

	if (mCode != CURLE_OK)
		return 0;

	mCode = curl_easy_getinfo(mCurl, CURLINFO_SIZE_DOWNLOAD, &val);
	if ((CURLE_OK == mCode) && (val > 0))
		TRACE(L"Data downloaded: %0.0f bytes.\n", val);
	mCode = curl_easy_getinfo(mCurl, CURLINFO_SPEED_DOWNLOAD, &val);
	if ((CURLE_OK == mCode) && (val > 0))
		TRACE(L"Average download speed: %0.3f kbyte/sec.\n", val / 1024);
	mCode = curl_easy_getinfo(mCurl, CURLINFO_TOTAL_TIME, &val);
	if ((CURLE_OK == mCode) && (val > 0))
		TRACE(L"Total download time: %0.3f sec.\n", val);

	TRACE(L"Download Updater Finish.");

//#endif // DEBUG

	curl_easy_cleanup(mCurl);
	curl_global_cleanup();

	
	SendMessage(pDlg->m_hWnd, UM_UPDATEINFO, 0, 0);

	return 0;
}

BOOL CTestProgramDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	std::ifstream readdata;
	Json::Reader jReader;
	Json::Value jRoot;
	readdata.open("TestData1.json");
	jReader.parse(readdata, jRoot);
	SetDlgItemTextA(m_hWnd, IDC_DATA1_VER, jRoot["version"].asCString());
	readdata.close();
	readdata.open("TestData2.json");
	jReader.parse(readdata, jRoot);
	SetDlgItemTextA(m_hWnd, IDC_DATA2_VER, jRoot["version"].asCString());
	readdata.close();
	readdata.open("TestData3.json");
	jReader.parse(readdata, jRoot);
	SetDlgItemTextA(m_hWnd, IDC_DATA3_VER, jRoot["version"].asCString());
	readdata.close();
	SetDlgItemText(IDC_STATIC_MPV, strVersion);

	mLock.Lock();
	hThread = CreateThread(
		NULL,
		0,
		OnCheckUpdate,
		(LPVOID)this,
		0,
		NULL
	);
	mLock.Lock();
	mLock.Unlock();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestProgramDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestProgramDlg::OnPaint()
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
HCURSOR CTestProgramDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 当用户关闭 UI 时，如果控制器仍保持着它的某个
//  对象，则自动化服务器不应退出。  这些
//  消息处理程序确保如下情形: 如果代理仍在使用，
//  则将隐藏 UI；但是在关闭对话框时，
//  对话框仍然会保留在那里。

void CTestProgramDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

LRESULT CTestProgramDlg::OnReceiveUpdateInfo(WPARAM wParam, LPARAM lParam)
{
	if (MessageBox(L"检测到新版本，是否打开更新器？", L"Info", MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
		CString strPath;
		TCHAR pszCmdLine[512],pszProcessFileName[MAX_PATH];
		GetModuleFileName(NULL, pszProcessFileName, MAX_PATH);
		strPath.Format(L"%s", pszProcessFileName);
		int nPos = strPath.ReverseFind(L'\\');
		if (nPos > 0)
			strPath = strPath.Left(nPos);

		wsprintf(pszCmdLine, L"--CurrentBuild %d --CurrentVersion \"%s\"", mBuild, strVersion);
		ShellExecute(this->m_hWnd, L"runas", L"Updater.exe", pszCmdLine, strPath, 0);

		PostQuitMessage(0);
	}
	return LRESULT();
}

void CTestProgramDlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CTestProgramDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CTestProgramDlg::CanExit()
{
	// 如果代理对象仍保留在那里，则自动化
	//  控制器仍会保持此应用程序。
	//  使对话框保留在那里，但将其 UI 隐藏起来。
	if (m_pAutoProxy != nullptr)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}



//void CAboutDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CDialogEx::OnTimer(nIDEvent);
//}
