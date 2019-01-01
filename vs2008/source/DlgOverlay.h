#pragma once
#include "afxwin.h"


// CDlgOverlay ダイアログ

class CDlgOverlay : public CDialog
{
	DECLARE_DYNAMIC(CDlgOverlay)

public:
	CDlgOverlay(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgOverlay();

	void SetDoc(CGazoDoc* pDoc);
	void UpdateGazoview();

// ダイアログ データ
	enum { IDD = IDD_OVERLAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	CGazoDoc* pd;
	void Rotation(double dDeg);
	void Magnify(double dScale);
	void Zshift(int iDelta);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOvlyup();
	double m_rx1;
	double m_ry1;
	double m_rx2;
	double m_ry2;
	double m_rx3;
	double m_ry3;
	double m_rx4;
	double m_ry4;
	double m_ru1;
	double m_rv1;
	double m_ru2;
	double m_rv2;
	double m_ru3;
	double m_rv3;
	double m_ru4;
	double m_rv4;
	afx_msg void OnBnClickedOvlyApply();
	afx_msg void OnBnClickedOvlydown();
	afx_msg void OnBnClickedOvlyleft();
	afx_msg void OnBnClickedOvlyright();
	afx_msg void OnBnClickedOvlyCW();
	afx_msg void OnBnClickedOvlyCCW();
	afx_msg void OnBnClickedOvlylarge();
	afx_msg void OnBnClickedOvlysmall();
	BOOL m_bMove1;
	BOOL m_bMove2;
	BOOL m_bMove3;
	BOOL m_bMove4;
	double m_rw1;
	double m_rw2;
	double m_rw3;
	double m_rw4;
	BOOL m_bMovew1;
	BOOL m_bMovew2;
	BOOL m_bMovew3;
	BOOL m_bMovew4;
	afx_msg void OnBnClickedOvlyWup();
	afx_msg void OnBnClickedOvlyWdown();
};
