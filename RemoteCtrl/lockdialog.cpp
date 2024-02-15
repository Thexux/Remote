// lockdialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "lockdialog.h"
#include "afxdialogex.h"


// clockdialog 对话框

IMPLEMENT_DYNAMIC(clockdialog, CDialog)

clockdialog::clockdialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

clockdialog::~clockdialog()
{
}

void clockdialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(clockdialog, CDialog)
END_MESSAGE_MAP()


// clockdialog 消息处理程序
