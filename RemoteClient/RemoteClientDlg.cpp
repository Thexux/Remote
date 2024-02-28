
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "clientsocket.h"
#include "clientcontroller.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

csocket* pclient = csocket::getsocket();
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


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nport(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDR_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nport);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_list);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)

	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDR_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddrServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPackAck)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
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
	inituidata();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRemoteClientDlg::OnPaint()
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
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRemoteClientDlg::OnBnClickedBtnTest()
{
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->sendcommandpacket(GetSafeHwnd(), 505);
}

void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	bool ok = pclientctl->sendcommandpacket(GetSafeHwnd(), 1, 0, 0);
	if (ok == 0)
	{
		AfxMessageBox(_T("命令处理失败！！！"));
		return;
	}
}

string CRemoteClientDlg::getpath(HTREEITEM htree)
{
	string stres, strt;
	do
	{
		strt = m_tree.GetItemText(htree);
		stres = strt + '\\' + stres;
		htree = m_tree.GetParentItem(htree);
	} while (htree);
	return stres;
}

void CRemoteClientDlg::deletetreechilditem(HTREEITEM htree)
{
	HTREEITEM hsub = 0;
	do
	{
		hsub = m_tree.GetChildItem(htree);
		if (hsub) m_tree.DeleteItem(hsub);
	} while (hsub);
}

void CRemoteClientDlg::loadfilecurrent()
{
	HTREEITEM htree = m_tree.GetSelectedItem();
	string strpath = getpath(htree);
	m_list.DeleteAllItems();
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	bool ok = pclientctl->sendcommandpacket(GetSafeHwnd(), 2, (uchar*)strpath.c_str(), strpath.size(), (WPARAM)htree);
	if (ok == 0) cout << "loadfileinfo is error"; // TODO：错误处理
}

void CRemoteClientDlg::loadfileinfo()
{
	CPoint ptmouse;
	GetCursorPos(&ptmouse);
	m_tree.ScreenToClient(&ptmouse);
	HTREEITEM htree = m_tree.HitTest(ptmouse, 0);
	if (htree == 0) return;
	if (m_tree.GetChildItem(htree) == 0) return;
	deletetreechilditem(htree);
	m_list.DeleteAllItems();
	string strpath = getpath(htree);
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	bool ok = pclientctl->sendcommandpacket(GetSafeHwnd(), 2, (uchar*)strpath.c_str(), strpath.size(), (WPARAM)htree);
	if (ok == 0) cout << "loadfileinfo is error"; // TODO：错误处理
}

void CRemoteClientDlg::inituidata()
{
	UpdateData();
	m_server_address = 0x7f000001;
	m_nport = _T("9527");
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->updateaddr(m_server_address, atoi(m_nport));

	UpdateData(0);
	m_dlgstatus.Create(IDD_DLG_STATUS, this);
	m_dlgstatus.ShowWindow(SW_HIDE);
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	loadfileinfo();
}

void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	loadfileinfo();
}

void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptmouse, ptlist;
	GetCursorPos(&ptmouse);
	ptlist = ptmouse;
	m_list.ScreenToClient(&ptlist);
	int listslt = m_list.HitTest(ptlist);
	if (listslt < 0) return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* ppop = menu.GetSubMenu(0);
	if (ppop) ppop->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptmouse.x, ptmouse.y, this);
}

void CRemoteClientDlg::OnDownloadFile()
{
	int nlistslt = m_list.GetSelectionMark();
	string strfile = m_list.GetItemText(nlistslt, 0);

	HTREEITEM hslt = m_tree.GetSelectedItem();
	strfile = getpath(hslt) + strfile;

	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->downflie(strfile);
}


void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hslt = m_tree.GetSelectedItem();
	string strpath = getpath(hslt);
	int nslt = m_list.GetSelectionMark();
	string strfile = m_list.GetItemText(nslt, 0);
	strfile = strpath + strfile;
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	bool ok = pclientctl->sendcommandpacket(GetSafeHwnd(), 5, (uchar*)strfile.c_str(), strfile.size());
	if (ok == 0) AfxMessageBox("删除文件命令执行失败");
	loadfilecurrent();
}


void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hslt = m_tree.GetSelectedItem();
	string strpath = getpath(hslt);
	int nslt = m_list.GetSelectionMark();
	string strfile = m_list.GetItemText(nslt, 0);
	strfile = strpath + strfile;
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	bool ok = pclientctl->sendcommandpacket(GetSafeHwnd(), 3, (uchar*)strfile.c_str(), strfile.size());
	if (ok == 0) AfxMessageBox("打开文件命令执行失败");

}

void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->getinstance()->startwatchscreen();
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddrServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	*pResult = 0;

	UpdateData();
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->updateaddr(m_server_address, atoi(m_nport));
}


void CRemoteClientDlg::OnEnChangeEditPort()
{
	UpdateData();
	cclientcontroller* pclientctl = cclientcontroller::getinstance();
	pclientctl->updateaddr(m_server_address, atoi(m_nport));
}

void CRemoteClientDlg::strtotree(const string& strdata, CTreeCtrl& m_tree)
{
	string strt;
	m_tree.DeleteAllItems();
	for (int i = 0; i < strdata.size(); i++)
	{
		if (strdata[i] == ',')
		{
			strt += ':';
			HTREEITEM ht = m_tree.InsertItem(strt.c_str(), TVI_ROOT, TVI_LAST);
			m_tree.InsertItem(0, ht, TVI_LAST);
			strt = "";
		}
		else strt += strdata[i];
	}
}
void CRemoteClientDlg::updatefileinfo(const FILEINFO& finfo, HTREEITEM hparent)
{
	cout << finfo.hasnext << ' ' << finfo.isdirectory << ' ' << finfo.filename << endl;
	if (finfo.hasnext == 0) return;
	if (finfo.isdirectory)
	{
		if (string(finfo.filename) == "." || string(finfo.filename) == "..") return;

		HTREEITEM ht = m_tree.InsertItem(finfo.filename, hparent, TVI_LAST);
		m_tree.InsertItem("", ht, TVI_LAST);
		m_tree.Expand(hparent, TVE_EXPAND);
	}
	else m_list.InsertItem(0, finfo.filename);
}

void CRemoteClientDlg::updatedownloadfile(const string& strdata, FILE* pfile)
{
	static ll llen = 0;
	if (llen == 0) llen = *(ll*)strdata.c_str();
	if (strdata.size() == 0)
	{
		llen = 0, fclose(pfile);
		cclientcontroller::getinstance()->downloadend();
		return;
	}
	fwrite(strdata.c_str(), 1, strdata.size(), pfile);
}

void CRemoteClientDlg::dealcommand(us ncmd, const string& strdata, LPARAM lParam)
{
	switch (ncmd)
	{
	case 505:
		TRACE("connection success!\r\n");
		break;
	case 1: // 获取驱动信息
		strtotree(strdata, m_tree);
		break;
	case 2: // 获取文件信息
		updatefileinfo(*(FILEINFO*)strdata.c_str(), (HTREEITEM)lParam);
		break;
	case 3:
		TRACE("run file done!\r\n");
		break;
	case 4:
		updatedownloadfile(strdata, (FILE*)lParam);
		break;
	case 5:
		TRACE("delete file done!\r\n");
		break;
	default:
		TRACE("unknow data received %d!\r\n", ncmd);
		break;
	}
}

LRESULT CRemoteClientDlg::OnSendPackAck(WPARAM wParm, LPARAM lParam)
{
	if (lParam < 0) TRACE("socket is error %d!\r\n", lParam);
	else if (lParam == 1) TRACE("the other side socket is close!\r\n");
	else
	{
		if (wParm == 0) return 0;
		cpacket pack = *(cpacket*)wParm; delete (cpacket*)wParm;
		dealcommand(pack.scmd, pack.strbuf, lParam);
	}


	return 0;
}

