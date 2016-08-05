#pragma once
#include "afxwin.h"

#define CDLGRECONST_FILT_HAN 0
#define CDLGRECONST_FILT_HAM 1
#define CDLGRECONST_FILT_RAMP 2
#define CDLGRECONST_FILT_PARZN 3
#define CDLGRECONST_FILT_BUTER 4

// CDlgReconOpt ダイアログ

class CDlgReconOpt : public CDialog
{
	DECLARE_DYNAMIC(CDlgReconOpt)

public:
	CDlgReconOpt(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgReconOpt();

// ダイアログ データ
	enum { IDD = IDD_RECON_OPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	void EnableCtrl();


	DECLARE_MESSAGE_MAP()
public:
	int nDataset;
	int iDatasetSel;

	int m_Filter;
	double m_Cutoff;
	double m_Order;
	int m_Interpolation;
	CString m_Suffix;
	afx_msg void OnCbnSelchangeReconstFilter();
	BOOL m_AngularIntp;
	//float m_TiltAngle;
	int m_Trim;
	CString m_Outpath;
	afx_msg void OnBnClickedReconstSetpath();
	int m_drStart;
	int m_drEnd;
	double m_drX;
	double m_drY;
	BOOL m_drOmit;
	BOOL m_Zernike;
	afx_msg void OnBnClickedReconstOmit();
	CComboBox m_HisDataset;
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeReconstHisdataset();
	BOOL m_bDriftParams;
	afx_msg void OnBnClickedReconstDriftparams();
	BOOL m_bDriftList;
	afx_msg void OnBnClickedReconstDriftlist();
	CString m_sDriftListPath;
	afx_msg void OnBnClickedReconstSetdriftlist();
	int m_FrameUsage;
	BOOL m_bSkipInitialFlatsInHDF5;
};
