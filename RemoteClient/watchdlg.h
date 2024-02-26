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

public:
	bool isfull() const
	{
		return m_isfull;
	}
	void setimagestatus(bool isfull = 0)
	{
		m_isfull = isfull;
	}
	CImage& getimage()
	{
		return m_image;
	}

protected:
	int m_obj_width;
	int m_obj_height;

	bool m_isfull; // 缓存是否有数据 1为有 0为无
	bool m_isclose; // 监控线程状态
	CImage m_image; // 缓存

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CPoint localtoremotepoint(CPoint& point);

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	virtual void OnOK();
};
