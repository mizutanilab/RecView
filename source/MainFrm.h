// MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_)
#define AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__4164546B_640D_4239_9245_6ACEBBD57BFB__INCLUDED_)
