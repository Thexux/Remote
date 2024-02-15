#pragma once


// clockdialog 对话框

class clockdialog : public CDialog
{
	DECLARE_DYNAMIC(clockdialog)

public:
	clockdialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~clockdialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
