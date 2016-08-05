#pragma once
#include "afxcmn.h"

class CGazoDoc;

// CDlgFrameList ダイアログ

class CDlgFrameList : public CDialog
{
	DECLARE_DYNAMIC(CDlgFrameList)

public:
	CDlgFrameList(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgFrameList();

// ダイアログ データ
	enum { IDD = IDD_FRAMELIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
private:
public:
	CTreeCtrl m_treeFrames;
	CGazoDoc* pd;
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};
