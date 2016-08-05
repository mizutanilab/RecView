#pragma once


// CDlgGeneral ダイアログ

class CDlgGeneral : public CDialog
{
	DECLARE_DYNAMIC(CDlgGeneral)

public:
	CDlgGeneral(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgGeneral();

// ダイアログ データ
	enum { IDD = IDD_GENERAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CString m_sCaption1;
	CString m_sInput1;
	CString m_sCaption2;
	CString m_sInput2;
	CString m_sCaption3;
	CString m_sInput3;
	virtual BOOL OnInitDialog();
	CString m_sTitle;
};
