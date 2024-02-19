
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "clientsocket.h"

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

int CRemoteClientDlg::sendcommandpacket(int ncmd, uchar* pdata, int nlen, bool bclose)
{
	UpdateData();
	//m_server_address;
	TRACE("%0X, %d\r\n", m_server_address, atoi(m_nport));
	int res = pclient->init(m_server_address, atoi(m_nport)); // todo:返回值处理
	if (res == 0)
	{
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	int f = pclient->sendate(cpacket(ncmd, pdata, nlen));

	int cmd = pclient->dealcommand();
	
	TRACE("ack:%d\r\n", cmd);
	if (bclose) pclient->closesock();
	return cmd;
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
	UpdateData();
	m_server_address = 0x7f000001;
	m_nport = _T("9527");
	UpdateData(0);

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
	sendcommandpacket(1981);
}

void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int res = sendcommandpacket(1);
	if (res == -1)
	{
		AfxMessageBox(_T("命令处理失败！！！"));
		return;
	}
	string strdata = pclient->getpacket().strbuf;
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
	int cmd = sendcommandpacket(2, (uchar*)strpath.c_str(), strpath.size(), 0);
	FILEINFO* pinfo = (FILEINFO*)pclient->getpacket().strbuf.c_str();

	while (pinfo->hasnext)
	{
		cout << pinfo->hasnext << ' ' << pinfo->isdirectory << ' ' << pinfo->filename << endl;
		if (pinfo->isdirectory)
		{
			if (string(pinfo->filename) == "." || string(pinfo->filename) == "..")
			{
				int cmd = pclient->dealcommand();
				TRACE("ack:&d\r\n", cmd);
				if (cmd < 0) break;
				pinfo = (FILEINFO*)pclient->getpacket().strbuf.c_str();
				continue;

			}
			HTREEITEM ht = m_tree.InsertItem(pinfo->filename, htree, TVI_LAST);
			m_tree.InsertItem("", ht, TVI_LAST);
		}
		else m_list.InsertItem(0, pinfo->filename);
		
		cmd = pclient->dealcommand();
		//cout << cmd << endl;
		if (cmd < 0) break;
		pinfo = (FILEINFO*)pclient->getpacket().strbuf.c_str();
	}
	pclient->closesock();
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
	CFileDialog dlg(FALSE, "*", m_list.GetItemText(nlistslt, 0),
		OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT, 0, this);

	if (dlg.DoModal() != IDOK) return;
	cout << dlg.GetPathName() << endl;

	FILE* pfile = fopen(dlg.GetPathName(), "wb+");
	if (pfile == 0)
	{
		AfxMessageBox("本地没有权限保存该文件，或这文件无法创建！！！");
		return;
	}

	HTREEITEM hslt = m_tree.GetSelectedItem();
	strfile = getpath(hslt) + strfile;
	cout << strfile << endl;
	int res = sendcommandpacket(4, (uchar*)strfile.c_str(), strfile.size(), 0);
	if (res != 4)
	{
		AfxMessageBox("执行下载命令失败！！");
		return;
	}

	int nst = pclient->getpacket().strbuf.size();
	while (nst)
	{
		fwrite(pclient->getpacket().strbuf.c_str(), 1, nst, pfile);
		us cmd = pclient->dealcommand();
		if (cmd != 4)
		{
			AfxMessageBox("传输失败！！");
			break;
		}
		nst = pclient->getpacket().strbuf.size();
	}
	


	/*ll len = *(ll*)pclient->getpacket().strbuf.c_str();

	for (int nst = 1; nst; )
	{
		us cmd = pclient->dealcommand();
		if (cmd != 4)
		{
			AfxMessageBox("传输失败！！");
			break;
		}
		nst = pclient->getpacket().strbuf.size();
		fwrite(pclient->getpacket().strbuf.c_str(), 1, nst, pfile);
	}*/
	
	fclose(pfile);
	pclient->closesock();
}


void CRemoteClientDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
}


void CRemoteClientDlg::OnRunFile()
{
	// TODO: 在此添加命令处理程序代码
}
