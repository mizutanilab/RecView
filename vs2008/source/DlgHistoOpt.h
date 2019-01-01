#pragma once


// CDlgHistoOpt ダイアログ

class CDlgHistoOpt : public CDialog
{
	DECLARE_DYNAMIC(CDlgHistoOpt)

public:
	CDlgHistoOpt(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgHistoOpt();

// ダイアログ データ
	enum { IDD = IDD_HISTO_OPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CString m_Prefix;
	int m_TrimAvrg;
	BOOL m_16bit;
	int m_RemoveDepth;
	double m_RemoveLAC;
};
