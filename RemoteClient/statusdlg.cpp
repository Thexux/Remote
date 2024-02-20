// statusdlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "statusdlg.h"
#include "afxdialogex.h"


// cstatusdlg 对话框

IMPLEMENT_DYNAMIC(cstatusdlg, CDialog)

cstatusdlg::cstatusdlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_STATUS, pParent)
{

}

cstatusdlg::~cstatusdlg()
{
}

void cstatusdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFO, m_info);
}


BEGIN_MESSAGE_MAP(cstatusdlg, CDialog)
END_MESSAGE_MAP()


// cstatusdlg 消息处理程序
