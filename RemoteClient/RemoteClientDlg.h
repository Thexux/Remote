﻿
// RemoteClientDlg.h: 头文件
//
#include "common.h"
#include "statusdlg.h"
#include "watchdlg.h"
#include "clientsocket.h"
#pragma once

#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER + 2) 
#endif

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:

	string getpath(HTREEITEM htree);
	void deletetreechilditem(HTREEITEM htree);
	void loadfilecurrent();
	void loadfileinfo();
	void inituidata();
	void strtotree(const string& driver, CTreeCtrl& tree);
	void updatefileinfo(const FILEINFO& finfo, HTREEITEM hparent);
	void updatedownloadfile(const string& strdata, FILE* pfile);
	void dealcommand(us ncmd, const string& strdata, LPARAM lParam);

// 实现
protected:
	HICON m_hIcon;
	cstatusdlg m_dlgstatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nport;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult); 
	// 显示文件
	CListCtrl m_list;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIpnFieldchangedIpaddrServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
	afx_msg LRESULT OnSendPackAck(WPARAM wParm, LPARAM lParam);
};
