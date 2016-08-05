#pragma once


// CDlgHorizcenter ダイアログ

class CDlgHorizcenter : public CDialog
{
	DECLARE_DYNAMIC(CDlgHorizcenter)

public:
	CDlgHorizcenter(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgHorizcenter();
	void SetDoc(CGazoDoc* pDoc);

// ダイアログ データ
	enum { IDD = IDD_HORIZCENTER };

protected:
	CGazoDoc* pd;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedHorizcenGetlist();

	int mBack;
	CString mOutpath;
private:
	CString mList;
public:
	afx_msg void OnBnClickedHorizcenSave();
	int mAvg;
	int mRelative;
};
