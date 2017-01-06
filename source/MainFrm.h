// MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_)
#define AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DlgDialbox.h"

class CMainFrame : public CMDIFrameWnd
{
friend class CGazoView;
friend class CGazoDoc;
friend class CDlgHistogram;
//friend class CDlgQueue;

	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CDlgDialbox dlgDialbox;

private:
	HACCEL mfrAccel;//161225

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // コントロール バー用メンバ
	//CStatusBar  m_wndStatusBar;
	//CToolBar    m_wndToolBar;

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileDialbox();
	afx_msg LRESULT OnDialbox(WPARAM wParam, LPARAM lParam);//161210
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAcckeyA();
	afx_msg void OnAcckeyS();
	afx_msg void OnAcckeyZ();
	afx_msg void OnAcckeyX();
	afx_msg void OnAcckeyQ();
	afx_msg void OnAcckeyW();
	afx_msg void OnAcckeyE();
	afx_msg void OnAcckeyR();
	afx_msg void OnAcckeyD();
	afx_msg void OnAcckeyF();
	afx_msg void OnAcckeyC();
	afx_msg void OnAcckeyV();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnAcckeyT();
	afx_msg void OnAcckeyG();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_)
