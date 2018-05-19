#pragma once


// CDlgRenumFiles ダイアログ

class CDlgRenumFiles : public CDialog
{
	DECLARE_DYNAMIC(CDlgRenumFiles)

public:
	CDlgRenumFiles(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgRenumFiles();

// ダイアログ データ
	enum { IDD = IDD_RENUMFILES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	CString m_FileList;
	CString m_FileListMsg;
	afx_msg void OnBnClickedRenumSetfile();
	int nFiles;
	CString m_Prefix;
	int m_StartIndex;
	CString m_OutPath;
	afx_msg void OnBnClickedRenumSetpath();
	int m_ResliceOrgZ;
	int m_ResliceOrgY;
	int m_ResliceOrgX;
	int m_ResliceEndX;
	int m_ResliceEndY;
	int m_ResliceEndZ;
};
