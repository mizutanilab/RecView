// gazoView.h : CGazoView クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAZOVIEW_H__31B6D46C_84D9_44AC_BC6B_1EFD1BEA14A8__INCLUDED_)
#define AFX_GAZOVIEW_H__31B6D46C_84D9_44AC_BC6B_1EFD1BEA14A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "StdAfx.h"
#include "DlgPolygon.h"

#define CGAZOVIEW_NPOLYGON 12

class CGazoView : public CView
{
friend class CMainFrame;

protected: // シリアライズ機能のみから作成します。
	CGazoView();
	DECLARE_DYNCREATE(CGazoView)

// アトリビュート
public:
	CGazoDoc* GetDocument();

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CGazoView)
	public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画する際にオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CGazoView();
	void SetPixels(int ixd, int iyd, int iBrightness, int iContrast);
	void SetOverlay(int** ppOverlay, int ixd, int iyd, int izd, int iBrightness, int iContrast);
	void GetBoxParams(int* xcent, int* ycent, int* xsize, int* ysize, int* angle, bool* enb);
	void SetBoxParams(int xcent, int ycent, int xsize, int ysize, int angle);
	void EnableBox(bool bEnable);
	bool GetLineParams(int* xstart = NULL, int* ystart = NULL, int* xend = NULL, int* yend = NULL);
	double pdOverlayCoeff[8];
	void InitPolygon(int xcent, int ycent, int xsize, int ysize);
	bool PointInPolygon(CPoint point);
	bool bPolygonEnabled;
	int iPolygonX[CGAZOVIEW_NPOLYGON];
	int iPolygonY[CGAZOVIEW_NPOLYGON];
	CDlgPolygon dlgPolygon;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void GetCoord(CPoint* pPnt);
	bool PointInBox(CPoint point);
	void UpdateBitmap(int ixdarg, int iBrightness, int iContrast);
	HBITMAP hBitmap;
	LPDWORD lpBmPixel;
	int iMagnify;
	LPBITMAPINFO lpBmpInfo;
//	BYTE* pPix;
//	int maxPix;
	BYTE* pbOverlay;
	int maxOverlay;
	int iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle;
	bool bBoxEnabled, bLButtonDown, bBoxMove;
	CPoint pntBox[5];
	CPoint pntLButton;
	int iPickedBoxPnt;
	bool bRButtonDown, bRedLine;
	CPoint pntRedFrom, pntRedTo;
	bool bDragScrollEnabled;
	int iScrollH, iScrollV;
	bool bPolygonMove;
	CPoint pntPolygon[CGAZOVIEW_NPOLYGON+1];
	int iPickedPolygonPnt;
private:
	int ixdim, iydim;
	HCURSOR hCursor;
// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CGazoView)
	afx_msg void OnToolbarMag();
	afx_msg void OnToolbarMin();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	//manually written
	//afx_msg void OnReconstExec1();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewBoxaxislabel();
	//manually removed
	//afx_msg void OnUpdateViewBoxaxislabel(CCmdUI *pCmdUI);
	afx_msg void OnAnalysisPolygonlasso();
	afx_msg void OnUpdateAnalysisPolygonlasso(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // gazoView.cpp ファイルがデバッグ環境の時使用されます。
inline CGazoDoc* CGazoView::GetDocument()
   { return (CGazoDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GAZOVIEW_H__31B6D46C_84D9_44AC_BC6B_1EFD1BEA14A8__INCLUDED_)
