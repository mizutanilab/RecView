#pragma once
#include "afxwin.h"

class CGazoView;

// CDlgCircleLasso ダイアログ

class CDlgCircleLasso : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCircleLasso)

public:
	CDlgCircleLasso(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~CDlgCircleLasso();
	void SetView(CGazoView* pView);
	void UpdateCurrentCircle();
	void MakeCircleList();
	CString sCircleList;
	//CBrush    m_cBkBrs;

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CIRCLELASSO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	CGazoView* pv;

	DECLARE_MESSAGE_MAP()
public:

	CString m_sCircleCurrent;
	afx_msg void OnBnClickedCircleAdd();
	CListBox m_lbCircleList;
	afx_msg void OnBnClickedCircleDelete();
	virtual BOOL OnInitDialog();
protected:
	virtual void OnCancel();
	virtual void OnOK();
	void CircleReset(int idx);
public:
	afx_msg void OnBnClickedCircleReset1();
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCircleReset2();
	afx_msg void OnBnClickedCircleReset3();
	afx_msg void OnBnClickedCircleReset4();
	afx_msg void OnBnClickedCircleReset5();
};
