#if !defined(AFX_DLGHISTOGRAM_H__16463A4F_1E7E_488D_948A_5DAF232FA8E6__INCLUDED_)
#define AFX_DLGHISTOGRAM_H__16463A4F_1E7E_488D_948A_5DAF232FA8E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgHistogram.h : ヘッダー ファイル
//
#include "general.h"

#define CDLGHISTG_BITMAP_HEIGHT 150
#define CDLGHISTG_BITMAP_WIDTH 300

//#define CDLGHISTG_BITMAP_HEIGHT 190
//#define CDLGHISTG_BITMAP_WIDTH 400
#define CDLGHISTG_BITMAP_NEAR 5
//bitmap width must be a multiple of 4

#define CDLGHIST_IDLE 0
#define CDLGHIST_BUSY 1

class CGazoDoc;

/////////////////////////////////////////////////////////////////////////////
// CDlgHistogram ダイアログ

class CDlgHistogram : public CDialog
{
// コンストラクション
public:
	CDlgHistogram(CWnd* pParent = NULL);   // 標準のコンストラクタ
	~CDlgHistogram();
	void SetDoc(CGazoDoc* pDoc);
	void ParamCopyFrom(const CDlgHistogram& a);
	void UpdateView();
	void UpdateParam();
	void RedrawHistogram();//230613

	CString filePath;
	TCHAR* fileList;
	int nFiles;
	//CFileDialog fileDlg(TRUE, defaultExt, NULL, 
	//	OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
	//int iLowCursor;
	//int iHighCursor;
	int iStatus;

// ダイアログ データ
	//{{AFX_DATA(CDlgHistogram)
	enum { IDD = IDD_HISTOGRAM };
	CProgressCtrl	m_Progress;
	CStatic	m_Bitmap;
	CString	m_HstHigh;
	CString	m_HstLow;
	CString	m_HstMax;
	CString	m_HstUnit;
	CString	m_CursorHigh;
	CString	m_CursorLow;
	BOOL	m_Preview;
	int		m_TrmCentX;
	int		m_TrmCentY;
	int		m_TrmSizeX;
	int		m_TrmSizeY;
	int		m_TrmAngle;
	BOOL	m_EnableTrm;
	CString	m_FileMsg;
	//}}AFX_DATA

	CString	m_Prefix;
	BOOL	m_16bit;
	int		m_TrmAvrg;
	int		m_RemoveDepth;
	double	m_RemoveLAC;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDlgHistogram)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	void UpdateHistogram();
	void CopyHistogram();
	void UpdateBitmap();
	void SetCursor();
	TErr AllocBitmap();
	int GetCursorPnt(CString arg);
	CString GetIntensityFromCursor(int ipos);
	void EnableCtrl();
	void UpdateBoxMsg();
	CGazoDoc* pd;
	int* pHistgPix;
	int iHstDispMax;
	bool bLButtonDownHigh;
	bool bLButtonDownLow;
	int iXMouse;
	int iYMouse;
	double fLow, fHigh;
	bool bInvalidate;

	HBITMAP hBitmap;
	LPBITMAPINFO lpBmpInfo;
	BYTE* pBitmapPix;

	//230613
	int m_iCDLGHISTG_BITMAP_HEIGHT;
	int m_iCDLGHISTG_BITMAP_WIDTH;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CDlgHistogram)
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposHistgMag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillfocusHistgCurshigh();
	afx_msg void OnKillfocusHistgCurslow();
	virtual void OnOK();
	afx_msg void OnHistgPreview();
	afx_msg void OnHistgGetpath();
	afx_msg void OnHistgEntrim();
	afx_msg void OnKillfocusHistgTrmcentx();
	afx_msg void OnKillfocusHistgTrmcenty();
	afx_msg void OnKillfocusHistgTrmsizex();
	afx_msg void OnKillfocusHistgTrmsizey();
	afx_msg void OnKillfocusHistgTrmangle();
	afx_msg void OnHistgQueue();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedHistgOpt();
	//CString m_StatMsg;
	BOOL m_bEnablePolygon;
	afx_msg void OnBnClickedHistgEnpolygon();
	BOOL m_bHistLog;
	BOOL m_bUpdateHistg;
	afx_msg void OnBnClickedHistgUpdatehistg();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DLGHISTOGRAM_H__16463A4F_1E7E_488D_948A_5DAF232FA8E6__INCLUDED_)
