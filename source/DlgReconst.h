#if !defined(AFX_DLGRECONST_H__FCBC4AB4_2062_46ED_AF5E_6427F95F4FF3__INCLUDED_)
#define AFX_DLGRECONST_H__FCBC4AB4_2062_46ED_AF5E_6427F95F4FF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgReconst.h : ヘッダー ファイル
//

#include "resource.h"//because of including DlgReconst.h in StdAfx.cpp
#include "afxwin.h"

//#define CDLGRECONST_FILT_HAN 0
//#define CDLGRECONST_FILT_HAM 1
//#define CDLGRECONST_FILT_RAMP 2
//#define CDLGRECONST_FILT_PARZN 3
//#define CDLGRECONST_FILT_BUTER 4

#define CDLGRECONST_IDLE 0
#define CDLGRECONST_BUSY 1
#define CDLGRECONST_STOP 2
#define CDLGRECONST_PAUSE 4
#define CDLGRECONST_SHOWIMAGE 8

#define CDLGRECONST_FRAME_ALL 0
#define CDLGRECONST_FRAME_ODD 1
#define CDLGRECONST_FRAME_EVEN 2

class CGazoDoc;

/////////////////////////////////////////////////////////////////////////////
// CDlgReconst ダイアログ

class CDlgReconst : public CDialog
{
// コンストラクション
public:
	CDlgReconst(CWnd* pParent = NULL);   // 標準のコンストラクタ
	void Init(int iMax, int iWidth, double dCen1 = -1, double dCen2 = -1);
	//void SetLayer(int iSlice1 = -1, int iSlice2 = -1);
	//void SetCenter(double dCent1 = -1, double dCent2 = -1);
	//void GetLayer(int* iSlice1, int* iSlice2 = NULL);
	//void GetCenter(double* dCent1, double* dCent2 = NULL);
	//double GetScaleFactor();
	void SetDoc(CGazoDoc* pDoc);
	//static DWORD WINAPI BatchReconstThread(CGazoDoc* dlg);
	int GetOffsetCTaxis();
	void ParamCopyFrom(const CDlgReconst& a);
	void AdjustSliceWithBinning();

// ダイアログ データ
	//{{AFX_DATA(CDlgReconst)
	enum { IDD = IDD_RECONST };
	CProgressCtrl	m_Progress;
	int		m_Slice1;
	int		m_Slice2;
	double	m_PixelWidth;
	int		m_Filter;
	//CString	m_Suffix;
	double	m_Cutoff;
	double	m_Order;
	CString	m_Center1;
	CString	m_Center2;
	int		m_Interpolation;
	//}}AFX_DATA
	int iSliceMax;
	int iImageWidth;
	int iStatus;
	bool m_AngularIntp;
	bool m_Zernike;
	float m_TiltAngle;
	CString m_Outpath;
	int m_drStart;
	int m_drEnd;
	double m_drX;
	double m_drY;
	BOOL m_drOmit;
	int m_iDatasetSel;
	int m_nDataset;
	//int m_iDatasetSize;
	CString m_sDriftListPath;
	bool m_bDriftList;
	bool m_bDriftParams;

	int m_FrameUsage;
	bool bOptionUpdated;
	int m_iDataseForCenter1, m_iDataseForCenter2;
	bool m_bSkipInitialFlatsInHDF5;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDlgReconst)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	void EnableCtrl();
	bool CheckParams(int idx = 3);
	CGazoDoc* pd;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CDlgReconst)
	afx_msg void OnReconstQueue();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnReconstAuto1();
	afx_msg void OnReconstAuto2();
	afx_msg void OnReconstShow1();
	afx_msg void OnReconstShow2();
//	afx_msg void OnSelchangeReconstFilter();
	virtual void OnCancel();
	afx_msg void OnReconstStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bOffsetCT;
	int m_OffsetAxis;
	afx_msg void OnBnClickedReconstOffct();
	int m_Trim;
	afx_msg void OnBnClickedReconstOptions();
	afx_msg void OnEnKillfocusReconstCent1();
	afx_msg void OnEnKillfocusReconstCent2();
	afx_msg void OnBnClickedReconstSino1();
	CComboBox m_HisDataset;
	afx_msg void OnCbnSelchangeReconstHisdataset();
	CString m_Suffix;
	afx_msg void OnEnSetfocusReconstCent1();
	afx_msg void OnEnSetfocusReconstCent2();
	afx_msg void OnEnSetfocusReconstSlice1();
	afx_msg void OnEnSetfocusReconstSlice2();
	afx_msg void OnBnClickedReconstResoln();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DLGRECONST_H__FCBC4AB4_2062_46ED_AF5E_6427F95F4FF3__INCLUDED_)
