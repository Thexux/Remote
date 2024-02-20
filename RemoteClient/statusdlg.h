#pragma once


// cstatusdlg 对话框

class cstatusdlg : public CDialog
{
	DECLARE_DYNAMIC(cstatusdlg)

public:
	cstatusdlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~cstatusdlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_info;
};
