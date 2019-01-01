#pragma once


#define CDLGREFRAC_IDLE 0
#define CDLGREFRAC_BUSY 1
#define CDLGREFRAC_STOP 2
#define CDLGREFRAC_PAUSE 4
#define CDLGREFRAC_SHOWIMAGE 8

// CDlgRefraction ダイアログ

class CDlgRefraction : public CDialog
{
	DECLARE_DYNAMIC(CDlgRefraction)

public:
	CDlgRefraction(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgRefraction();

// ダイアログ データ
	enum { IDD = IDD_REFRACTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	int iStatus;
	void SetDoc(CGazoDoc* pDoc);

	//int m_AtomZ;
	//int m_AtomA;
	//double m_Density;
	//double m_LAC;
	//double m_Lambda;
	//double m_S2DDist;
	//double m_PxSize;

protected:
	void EnableCtrl();
	CGazoDoc* pd;

	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedRefrShow();
	afx_msg void OnBnClickedRefrStop();
	virtual BOOL OnInitDialog();
	int m_AtomZ;
	int m_AtomA;
	double m_Density;
	double m_LAC;
	double m_Xenergy;
	double m_S2Ddist;
	double m_PxSize;
	CString m_Prefix;
};
