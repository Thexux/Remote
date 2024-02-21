// watchdlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "watchdlg.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"
#include "clientsocket.h"


// cwatchdlg 对话框

IMPLEMENT_DYNAMIC(cwatchdlg, CDialog)

cwatchdlg::cwatchdlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_obj_width = -1 , m_obj_height = -1;
}

cwatchdlg::~cwatchdlg()
{
}

void cwatchdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(cwatchdlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &cwatchdlg::OnStnClickedWatch)
END_MESSAGE_MAP()


// cwatchdlg 消息处理程序


CPoint cwatchdlg::localtoremotepoint(CPoint& point)
{
	CRect clientrect;
	m_picture.GetWindowRect(&clientrect);
	return CPoint(m_obj_width * point.x / clientrect.Width(), m_obj_height * point.y / clientrect.Height());
}

BOOL cwatchdlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 45, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void cwatchdlg::OnTimer(UINT_PTR nIDEvent)
{ 
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
		//cout << "Time now is full: " << pparent->isfull() << endl;
		if (pparent->isfull())
		{		
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pparent->getimage().Save(_T("b.jpeg"), Gdiplus::ImageFormatJPEG);
			//pparent->getimage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			if (m_obj_width == -1) m_obj_width = pparent->getimage().GetWidth();
			if (m_obj_height == -1) m_obj_height = pparent->getimage().GetHeight();
			pparent->getimage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),
				0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pparent->getimage().Destroy();
			pparent->setimagestatus();
		}

	}
	
	CDialog::OnTimer(nIDEvent);
}

void cwatchdlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 1, mouse.sbtn = 0, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void cwatchdlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 2, mouse.sbtn = 0, mouse.pt =localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);
	CDialog::OnLButtonDown(nFlags, point);
}


void cwatchdlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 3, mouse.sbtn = 0, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);
	CDialog::OnLButtonUp(nFlags, point);
}



void cwatchdlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 1, mouse.sbtn = 1, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);

	CDialog::OnRButtonDblClk(nFlags, point);
}


void cwatchdlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 2, mouse.sbtn = 1, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);

	CDialog::OnRButtonDown(nFlags, point);
}


void cwatchdlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 3, mouse.sbtn = 1, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);

	CDialog::OnRButtonUp(nFlags, point);
}



void cwatchdlg::OnMouseMove(UINT nFlags, CPoint point)
{
	MOUSEV mouse;
	mouse.sact = 0, mouse.sbtn = 3, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);

	CDialog::OnMouseMove(nFlags, point);
}


void cwatchdlg::OnStnClickedWatch()
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	MOUSEV mouse;
	mouse.sact = 0, mouse.sbtn = 0, mouse.pt = localtoremotepoint(point);
	CRemoteClientDlg* pparent = (CRemoteClientDlg*)GetParent();
	pparent->SendMessage(WM_SEND_PACKET, 7 << 1, (LPARAM)&mouse);

}


void cwatchdlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}
