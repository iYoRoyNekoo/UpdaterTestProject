
// DlgProxy.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TestProgram.h"
#include "DlgProxy.h"
#include "TestProgramDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestProgramDlgAutoProxy

IMPLEMENT_DYNCREATE(CTestProgramDlgAutoProxy, CCmdTarget)

CTestProgramDlgAutoProxy::CTestProgramDlgAutoProxy()
{
	EnableAutomation();

	// 为使应用程序在自动化对象处于活动状态时一直保持
	//	运行，构造函数调用 AfxOleLockApp。
	AfxOleLockApp();

	// 通过应用程序的主窗口指针
	//  来访问对话框。  设置代理的内部指针
	//  指向对话框，并设置对话框的后向指针指向
	//  该代理。
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CTestProgramDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CTestProgramDlg)))
		{
			m_pDialog = reinterpret_cast<CTestProgramDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CTestProgramDlgAutoProxy::~CTestProgramDlgAutoProxy()
{
	// 为了在用 OLE 自动化创建所有对象后终止应用程序，
	//	析构函数调用 AfxOleUnlockApp。
	//  除了做其他事情外，这还将销毁主对话框
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CTestProgramDlgAutoProxy::OnFinalRelease()
{
	// 释放了对自动化对象的最后一个引用后，将调用
	// OnFinalRelease。  基类将自动
	// 删除该对象。  在调用该基类之前，请添加您的
	// 对象所需的附加清理代码。

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CTestProgramDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTestProgramDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// 注意: 我们添加了对 IID_ITestProgram 的支持来支持类型安全绑定
//  以支持来自 VBA 的类型安全绑定。  此 IID 必须同附加到 .IDL 文件中的
//  调度接口的 GUID 匹配。

// {cf5b88d5-762b-4791-af38-da244c08fd30}
static const IID IID_ITestProgram =
{0xcf5b88d5,0x762b,0x4791,{0xaf,0x38,0xda,0x24,0x4c,0x08,0xfd,0x30}};

BEGIN_INTERFACE_MAP(CTestProgramDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CTestProgramDlgAutoProxy, IID_ITestProgram, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 宏是在此项目的 pch.h 中定义的
// {3b346743-39b7-48c4-94a9-b8e2425cf3e9}
IMPLEMENT_OLECREATE2(CTestProgramDlgAutoProxy, "TestProgram.Application", 0x3b346743,0x39b7,0x48c4,0x94,0xa9,0xb8,0xe2,0x42,0x5c,0xf3,0xe9)


// CTestProgramDlgAutoProxy 消息处理程序
