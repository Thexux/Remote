#pragma once


// cwatchdlg 对话框

class cwatchdlg : public CDialog
{
	DECLARE_DYNAMIC(cwatchdlg)

public:
	cwatchdlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~cwatchdlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_WATCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
};
