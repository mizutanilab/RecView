#pragma once
#include "afxwin.h"
#include "gazoDoc.h"

#define CDLGRESOLNPLOT_BITMAP_HEIGHT 200
#define CDLGRESOLNPLOT_BITMAP_WIDTH 300
//#define CDLGRESOLNPLOT_BITMAP_HEIGHT 240
//#define CDLGRESOLNPLOT_BITMAP_WIDTH 340
#define CDLGRESOLNPLOT_BITMAP_NEAR 5

// CDlgResolnPlot ダイアログ

class CDlgResolnPlot : public CDialog
{
	DECLARE_DYNAMIC(CDlgResolnPlot)

public:
	CDlgResolnPlot(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgResolnPlot();

// ダイアログ データ
	enum { IDD = IDD_RESOLNPLOT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

protected:
	void UpdateHistogram();
	void UpdateBitmap();
	void UpdateCursorAndRegression();
	void EnableCtrl();
	void UpdateBoxMsg();
	void UpdateReport();
	void GetSlope();

	TErr AllocBitmap();
	void CopyHistogram();
	int GetCoordX(double dx);
	int GetCoordY(double dy);
	CString GetIntensityFromCursor(int ipos);

	HBITMAP hBitmap;
	LPBITMAPINFO lpBmpInfo;
	BYTE* pBitmapPix;

	CStatic m_Bitmap;
	int* pPlotPix;
	double m_dLow, m_dHigh,  m_dLeft, m_dRight;
	CString	m_CursorRight;
	CString	m_CursorLeft;
	bool bLButtonDownHigh;
	bool bLButtonDownLow;
	int iXMouse;
	int iYMouse;
	bool bInvalidate;
	double m_dSlope, m_dYSection;
	CString m_sReport;
	BOOL m_bPlotR;
	BOOL m_bPlotG;
	BOOL m_bPlotB;
	CString m_sYLow;
	CString m_sYHigh;

public:
	struct CGZD_RESOLN_LIST* m_psResolnList;
	int m_iMaxResolnList;
	bool m_bColor;
	double m_dPixelWidth;
	CString m_sFileName;
	int m_piDim[4];

	afx_msg void OnDeltaposResolnplotMag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedResolnplotPlotR();
	afx_msg void OnBnClickedResolnplotPlotG();
	afx_msg void OnBnClickedResolnplotPlotB();
	afx_msg void OnBnClickedResolnplotUpdate();
	afx_msg void OnBnClickedResolnplotCsv();
};
