// gazo.h : GAZO アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_GAZO_H__5986D87A_9E81_43FB_B968_779630EA7268__INCLUDED_)
#define AFX_GAZO_H__5986D87A_9E81_43FB_B968_779630EA7268__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル
#include "general.h"
//class CGazoDoc;
#include "DlgQueue.h"
#include "DlgProperty.h"
#include "DlgReconst.h"
#include "DlgHistogram.h"
#include "DlgLsqfit.h"
#include "DlgRenumFiles.h"
#include <afxmt.h>//131019 CMutex
#include "cxyz.h"//181214
//#include "DlgDialbox.h"

/////////////////////////////////////////////////////////////////////////////
// CGazoApp:
// このクラスの動作の定義に関しては gazo.cpp ファイルを参照してください。
//

#define CGAZOAPP_CMD_NONE 0
#define CGAZOAPP_CMD_REN 1
#define CGAZOAPP_CMD_COPY 2
#define CGAZOAPP_CMD_AVG 3
#define CGAZOAPP_CMD_MAXTOKEN 255

class CGazoApp : public CWinApp
{
public:
	CGazoApp();
	~CGazoApp();
	void OnFileOpen();
	CView* RequestNew();
	void SetBusy();
	void SetIdle();
	bool IsBusy();
	CString Lsqfit(LSQFIT_QUEUE* lq, CDlgLsqfit* dlg = NULL, CDlgQueue* dqueue = NULL);
	CString LsqfitMin(LSQFIT_QUEUE* lq, CDlgLsqfit* dlg = NULL, CDlgQueue* dqueue = NULL);

	int iAvailableCPU;
	//150101 HCURSOR hCursorRot;
	CString sProgVersion;
	bool bShowBoxAxis, bDragScroll, bWheelToGo;

	CDlgQueue dlgQueue;
	CDlgProperty dlgProperty;
	//TReal prevPixelWidth;
	CDlgReconst prevDlgReconst;
	CDlgHistogram prevDlgHistogram;

	CMutex m_mutex;//131019

//	CDlgDialbox dlgDialbox;

private:
	TErr CalcAvgImage(CString path, CString* files, int nfiles);
	double GetImageDiff(short* psBinRefPixel, short* psBinQryPixel, int ibin, CXyz cDelta, CXyz cDisp, 
							   int ibxref, int ibyref, int ibzref, int ibxqry, int ibyqry, int ibzqry);

protected:
	int iStatus;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CGazoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション
	//{{AFX_MSG(CGazoApp)
	afx_msg void OnAppAbout();
	afx_msg void OnUpdateAppExit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTomoQueue(CCmdUI* pCmdUI);
	afx_msg void OnTomoQueue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTomoProperty();
	afx_msg void OnUpdateTomoProperty(CCmdUI *pCmdUI);
	afx_msg void OnUpdateWindowNew(CCmdUI *pCmdUI);
	afx_msg void OnFilePrepfiles();
	afx_msg void OnTomoLsqfit();
	afx_msg void OnFileCloseall();
	afx_msg void OnTomoRenumfiles();
	afx_msg void OnViewError();
	afx_msg void OnFileSavequeue();
	afx_msg void OnFileLoadqueue();
	afx_msg void OnAppExit();
	virtual BOOL SaveAllModified();
	afx_msg void OnUpdateViewBoxaxislabel(CCmdUI *pCmdUI);
	//afx_msg void OnViewBoxaxislabel();
	afx_msg void OnViewDragscroll();
	afx_msg void OnUpdateViewDragscroll(CCmdUI *pCmdUI);
	//afx_msg void OnFileDialbox();
	//afx_msg LRESULT OnDialbox(WPARAM wParam, LPARAM lParam);//161210
	afx_msg void OnViewWheeltogo();
	afx_msg void OnUpdateViewWheeltogo(CCmdUI *pCmdUI);
};

unsigned __stdcall GetImageDiffThread(void* pArg);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GAZO_H__5986D87A_9E81_43FB_B968_779630EA7268__INCLUDED_)
