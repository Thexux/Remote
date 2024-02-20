// watchdlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "watchdlg.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// cwatchdlg 对话框

IMPLEMENT_DYNAMIC(cwatchdlg, CDialog)

cwatchdlg::cwatchdlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

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
END_MESSAGE_MAP()


// cwatchdlg 消息处理程序


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
		cout << "Time now is full: " << pparent->isfull() << endl;
		if (pparent->isfull())
		{		
			CRect rect;
			m_picture.GetWindowRect(rect);
			pparent->getimage().Save(_T("b.jpeg"), Gdiplus::ImageFormatJPEG);
			//pparent->getimage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			pparent->getimage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),
				0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(0);
			pparent->getimage().Destroy();
			pparent->setimagestatus();
		}

	}
	
	CDialog::OnTimer(nIDEvent);
}
