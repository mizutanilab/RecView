#pragma once
#include "afxwin.h"

class CGazoView;

// CDlgPolygon ダイアログ

class CDlgPolygon : public CDialog
{
	DECLARE_DYNAMIC(CDlgPolygon)

public:
	CDlgPolygon(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgPolygon();
	void SetView(CGazoView* pView);
	void UpdateCurrentPolygon();
	void MakePolygonList();
	CString sPolygonList;

// ダイアログ データ
	enum { IDD = IDD_POLYGON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	CGazoView* pv;

	DECLARE_MESSAGE_MAP()
public:

	CString m_sPolygonCurrent;
	afx_msg void OnBnClickedPolygonAdd();
	CListBox m_lbPolygonList;
	afx_msg void OnBnClickedPolygonDelete();
	virtual BOOL OnInitDialog();
protected:
	virtual void OnCancel();
	virtual void OnOK();
};
