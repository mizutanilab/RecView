#if !defined(AFX_DLGMESSAGE_H__0796B2D1_B700_4B38_A7F2_973027D16FF9__INCLUDED_)
#define AFX_DLGMESSAGE_H__0796B2D1_B700_4B38_A7F2_973027D16FF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define CDLGMESSAGE_FUNCMODE_NORMAL 0
//#define CDLGMESSAGE_FUNCMODE_CONVBAT 1

// DlgMessage.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CDlgMessage ダイアログ

class CDlgMessage : public CDialog
{
// コンストラクション
public:
	CDlgMessage(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CDlgMessage)
	enum { IDD = IDD_MESSAGE };
	CString	m_Msg;
	//}}AFX_DATA

public:
	//int iFuncMode;
// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDlgMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CDlgMessage)
		// メモ: ClassWizard はこの位置にメンバ関数を追加します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeList1();
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//	virtual BOOL OnInitDialog();
//	afx_msg void OnBnClickedMsgConvbat();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DLGMESSAGE_H__0796B2D1_B700_4B38_A7F2_973027D16FF9__INCLUDED_)
