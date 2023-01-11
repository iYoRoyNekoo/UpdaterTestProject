
// UpdaterDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Updater.h"
#include "UpdaterDlg.h"
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
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdaterDlg 对话框



CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UPDATER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCurrent);
	DDX_Control(pDX, IDC_PROGRESS2, m_ProgressTotal);
}

enum USER_MESSAGES {
	UM_FINISH_INIT_DLG_UPDATE = WM_USER + 101
};

BEGIN_MESSAGE_MAP(CUpdaterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(UM_FINISH_INIT_DLG_UPDATE, &CUpdaterDlg::OnFinishInitUpdateDlg)
	ON_BN_CLICKED(IDOK, &CUpdaterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUpdaterDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CUpdaterDlg 消息处理程序

struct FILES_TO_DOWNLOAD {
	std::string strUrl;
	std::string strLocal;
};

CString strCurVer, strFilePath;
int mCurBuild, mTargetBuild = 0;
int totalDownload;
bool bWorking = false;
CMutex mLock;
FILES_TO_DOWNLOAD* mFiles;
CUpdaterDlg* Gp_Dlg;

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
int curl_showprogress_callback(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded) {
	double fractiondownloaded;
	if (TotalToDownload == 0.0)
		fractiondownloaded = 0;
	else 
		fractiondownloaded = NowDownloaded / TotalToDownload;
	int pointer = round(fractiondownloaded * 100);
	Gp_Dlg->m_ProgressCurrent.SetPos(pointer);
	return 0;
}

DWORD WINAPI OnCheckNewVer(LPVOID lpParam) {
	DNS_STATUS status;
	PDNS_RECORD pDnsRecord;
	CString strLQueryResult;
	std::string strQueryResult;
	Json::Reader jReader;
	Json::Value jRoot, jVersionInfo;

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
	int mLatestBuild = mCurBuild;
	std::string strTargetServerUrl, strTargetVersion;
	CURL* mCurl = curl_easy_init();
	CURLcode mCode;
	std::string mUA = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1 TestProject/1.0.0";
	std::string szbuffer;
	std::string szheader_buffer;
	double val;
	bool available = false;

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

		available = true;

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
			strTargetVersion = jVersionInfo["release"].asString();
		}
	}

	if (!available) {
		curl_easy_cleanup(mCurl);
		curl_global_cleanup();
		MessageBox(Gp_Dlg->m_hWnd, L"服务器连接失败！", L"Error", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
		return 0;
	}
	
	SetDlgItemTextA(Gp_Dlg->m_hWnd, IDC_VERSION_TAR, strTargetVersion.c_str());

	char szConfigUrl[512];

	sprintf_s(szConfigUrl, "%sgenerate-config.php?version=%d", strTargetServerUrl.c_str(), mCurBuild);

	curl_easy_setopt(mCurl, CURLOPT_URL, szConfigUrl);	//设置URL

	szbuffer = "";
	szheader_buffer = "";

	mCode = curl_easy_perform(mCurl);
	if (mCode != CURLE_OK) {//下载失败，中断
		curl_easy_cleanup(mCurl);
		curl_global_cleanup();
		MessageBox(Gp_Dlg->m_hWnd, L"配置文件下载失败！", L"Error", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
		return 0;
	}

	if (!jReader.parse(szbuffer, jRoot)) {//解析失败，服务端配置文件不合法
		curl_easy_cleanup(mCurl);
		curl_global_cleanup();
		MessageBox(Gp_Dlg->m_hWnd, L"配置文件解析失败！", L"Error", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
		return 0;
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

	TRACE(L"Get Info Finish.");
	curl_easy_cleanup(mCurl);
	curl_global_cleanup();

	mFiles = new FILES_TO_DOWNLOAD[jRoot.size()];
	for (int i = 0; i < jRoot.size(); i++) {
		mFiles[i].strUrl = strTargetServerUrl + jRoot[i]["remote"].asString();
		mFiles[i].strLocal = jRoot[i]["local"].asString();
	}

	totalDownload = jRoot.size();

	PostMessage(Gp_Dlg->m_hWnd, UM_FINISH_INIT_DLG_UPDATE, 0, 0);

	return 0;
}

BOOL CUpdaterDlg::OnInitDialog()
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
	Gp_Dlg = this;

#ifdef DEBUG
	strCurVer = "v1.0.0 Build001";
	mCurBuild = 1;
	strFilePath = ".\\";
#else
	if (__argc != 7) {
		MessageBox(L"不支持的调用方式！", L"Error", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
	}

	for (int i = 1; i < __argc; i++) {
		if (wcsstr(__wargv[i], L"--CurrentVersion"))
			strCurVer = __wargv[++i];
		if (wcsstr(__wargv[i], L"--CurrentBuild"))
			mCurBuild = _ttoi(__wargv[++i]);
		if (wcsstr(__wargv[i], L"--FilePath"))
			strFilePath = __wargv[++i];
	}
#endif // DEBUG

	SetDlgItemText(IDC_STATUS, L"正在获取版本信息...");
	SetDlgItemText(IDC_VERSION_CUR, strCurVer);
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);

	CreateThread(
		NULL,
		0,
		OnCheckNewVer,
		(LPVOID)this,
		0,
		NULL
	);  

	m_ProgressCurrent.SetRange(0, 100);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUpdaterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CUpdaterDlg::OnPaint()
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
HCURSOR CUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CUpdaterDlg::OnFinishInitUpdateDlg(WPARAM wParam, LPARAM lParam)
{
	((CButton*)GetDlgItem(IDOK))->EnableWindow(TRUE);
	SetDlgItemInt(IDC_TOTAL_DOWNLOAD, totalDownload, TRUE);
	SetDlgItemText(IDC_STATUS, L"就绪");
	m_ProgressTotal.SetRange(0, totalDownload);
	return LRESULT();
}

/*
class CBindCallback :public IBindStatusCallback
{
public:
	CBindCallback();
	virtual~CBindCallback();

	//接受显示进度窗口的句柄
	CUpdaterDlg* m_pdlg;

	//IBindStatusCallback的方法。除了OnProgress     外的其他方法都返回E_NOTIMPL

	STDMETHOD(OnStartBinding)
		(DWORD dwReserved,
			IBinding __RPC_FAR* pib)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetPriority)
		(LONG __RPC_FAR* pnPriority)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnLowResource)
		(DWORD reserved)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnProgress)
		(ULONG ulProgress,
			ULONG ulProgressMax,
			ULONG ulStatusCode,
			LPCWSTR wszStatusText);

	STDMETHOD(OnStopBinding)
		(HRESULT hresult,
			LPCWSTR szError)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetBindInfo)
		(DWORD __RPC_FAR* grfBINDF,
			BINDINFO __RPC_FAR* pbindinfo)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnDataAvailable)
		(DWORD grfBSCF,
			DWORD dwSize,
			FORMATETC __RPC_FAR* pformatetc,
			STGMEDIUM __RPC_FAR* pstgmed)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnObjectAvailable)
		(REFIID riid,
			IUnknown __RPC_FAR* punk)
	{
		return E_NOTIMPL;
	}

	// IUnknown方法.IE 不会调用这些方法的

	STDMETHOD_(ULONG, AddRef)()
	{
		return 0;
	}

	STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}

	STDMETHOD(QueryInterface)
		(REFIID riid,
			void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		return E_NOTIMPL;
	}
};
CBindCallback::CBindCallback()
{

}
CBindCallback::~CBindCallback()
{

}
LRESULT CBindCallback::OnProgress(ULONG ulProgress,
	ULONG ulProgressMax,
	ULONG ulSatusCode,
	LPCWSTR szStatusText)
{
	
	m_pdlg->m_ProgressCurrent.SetRange32(0, ulProgressMax);
	m_pdlg->m_ProgressCurrent.SetPos(ulProgress);

	CString szText;
	szText.Format(L"已下载%d%%", (int)(ulProgress * 100.0 / ulProgressMax));
	(m_pdlg->GetDlgItem(IDC_CURR_PROG))->SetWindowText(szText);

	return S_OK;
}
*/

DWORD WINAPI OnUpdate(LPVOID lpParam) {

	CURL* mCurl = curl_easy_init();
	CURLcode mCode;
	std::string mUA = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1 TestProject/1.0.0";
	std::string szheader_buffer;
	double val;

	//curl init
	curl_global_init(CURL_GLOBAL_ALL);
	curl_easy_setopt(mCurl, CURLOPT_USERAGENT, mUA.c_str());	//设置UserAgent
	curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, 0L);		//设置SSL验证级别（0-2，宽松-严格）
	curl_easy_setopt(mCurl, CURLOPT_CAINFO, "cacert.pem");		//根证书信息
	curl_easy_setopt(mCurl, CURLOPT_MAXREDIRS, 5);				//设置最大重定向次数
	curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1);			//设置301、302跳转跟随location
	curl_easy_setopt(mCurl, CURLOPT_TIMEOUT, 600L);				//超时设置
	curl_easy_setopt(mCurl, CURLOPT_CONNECTTIMEOUT, 10L);		//连接超时设置
	curl_easy_setopt(mCurl, CURLOPT_FAILONERROR, 1L);			//服务端返回40x代码时返回错误而不是下载错误页
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, curl_writefile_callback);//设置回调函数（不同于写入字符串）
	curl_easy_setopt(mCurl, CURLOPT_PROGRESSFUNCTION, curl_showprogress_callback);//设置进度显示
	curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, 0L);//设置进度
	//抓取头信息，回调函数  
	curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, curl_default_callback);
	curl_easy_setopt(mCurl, CURLOPT_HEADERDATA, &szheader_buffer);


	for (int i = 0; i < totalDownload; i++) {
		FILE* pFile;
		fopen_s(&pFile, mFiles[i].strLocal.c_str(), "wb");
		curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, pFile);
		curl_easy_setopt(mCurl, CURLOPT_URL, mFiles[i].strUrl.c_str());
		mCode = curl_easy_perform(mCurl);

		if (mCode != CURLE_OK) {//下载失败，中断
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
		//HRESULT ret=URLDownloadToFileA(NULL, strFilesToDownload[i].c_str(), UnicodeStringToSTLString(strFilePath).c_str(), 0, &cbc);
		fclose(pFile);

		Gp_Dlg->m_ProgressTotal.SetPos(i + 1);
	}

	bWorking = false;
	return 0;
}

void CUpdaterDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	bWorking = true;
	CreateThread(
		NULL,
		0,
		OnUpdate,
		NULL,
		0,
		NULL
	);
}

void CUpdaterDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (bWorking && MessageBox(L"正在进行更新，若中断容易出现未知的故障，是否中断更新？", L"Warning", MB_OKCANCEL | MB_ICONWARNING) == IDOK) {
		PostQuitMessage(-1);
	}
	CDialogEx::OnCancel();
}
