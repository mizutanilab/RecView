// DlgReconst.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgReconst.h"
#include "gazoDoc.h"
#include "gazoView.h"
#include "DlgReconOpt.h"
#include "DlgMessage.h"
#include "DlgFrameList.h"
//CUDA declaration
#include "cudaReconst.h"
//OpenCL
#include "clReconst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgReconst ダイアログ


CDlgReconst::CDlgReconst(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReconst::IDD, pParent)
	, m_bOffsetCT(FALSE)
	, m_OffsetAxis(0)
	, m_Trim(0)
	, m_Suffix(_T("rec"))
{
	//{{AFX_DATA_INIT(CDlgReconst)
	m_Slice1 = 0;
	m_Slice2 = 0;
	m_PixelWidth = 0.5;
	m_Filter = CDLGRECONST_FILT_HAN;
	//m_Suffix = _T("rec");
	m_Cutoff = 0.5;
	m_Order = 6.0;
	m_Center1 = _T("0.00");
	m_Center2 = _T("0.00");
	m_Interpolation = 2;
	m_AngularIntp = false;
	m_Zernike = false;
	m_FrameUsage = CDLGRECONST_FRAME_ALL;
	m_dAxisInc = 0.5;

	m_TiltAngle = 0;
	m_Outpath = _T("");
	//}}AFX_DATA_INIT
	iSliceMax = 0;
	iImageWidth = 0;
	pd = NULL;
	iStatus = CDLGRECONST_IDLE;
	iContext = CDLGRECONST_CONTEXT_NONE;
	m_drStart = 0;
	m_drEnd = 0;
	m_drX = 0;
	m_drY = 0;
	m_drOmit = FALSE;
	//111108
	m_iDatasetSel = 0;
	m_nDataset = 1;
	//m_iDatasetSize = 1;
	m_sDriftListPath = _T("");
	m_bDriftList = false;
	m_bDriftParams = false;

	bOptionUpdated = true;
	m_iDataseForCenter1 = -1;
	m_iDataseForCenter2 = -1;

	m_bSkipInitialFlatsInHDF5 = false;

	m_iDlgFL_SampleFrameStart = 0;
	m_iDlgFL_SampleFrameEnd = 0;
}

void CDlgReconst::SetDoc(CGazoDoc* pDoc) {if (pDoc) pd = pDoc;}

void CDlgReconst::Init(int iMax, int iWidth, double dCen1, double dCen2) {
	iSliceMax = iMax;
	iImageWidth = iWidth;
	//110913
	m_Slice1 = 0;
	m_Slice2 = iMax - 1;
	//
	if (dCen1 >= 0) m_Center1.Format("%.2f", dCen1);
	if (dCen2 >= 0) m_Center2.Format("%.2f", dCen2);
	m_OffsetAxis = iWidth - 100;
	m_Trim = 0;
}

void CDlgReconst::ParamCopyFrom(const CDlgReconst& a) {
	m_PixelWidth = a.m_PixelWidth;
	m_Filter = a.m_Filter;
	//111108 m_Suffix = a.m_Suffix;
	m_Cutoff = a.m_Cutoff;
	m_Order = a.m_Order;
	m_Interpolation = a.m_Interpolation;
	m_AngularIntp = a.m_AngularIntp;
	m_Zernike = a.m_Zernike;
	m_TiltAngle = a.m_TiltAngle;
	m_bOffsetCT = a.m_bOffsetCT;
	m_OffsetAxis = a.m_OffsetAxis;
	m_Trim = a.m_Trim;
	m_drStart = a.m_drStart;
	m_drEnd = a.m_drEnd;
	m_drX = a.m_drX;
	m_drY = a.m_drY;
	m_drOmit = a.m_drOmit;
	m_sDriftListPath = a.m_sDriftListPath;
	m_bDriftList = a.m_bDriftList;
	m_bDriftParams = a.m_bDriftParams;
	m_FrameUsage = a.m_FrameUsage;
	m_bSkipInitialFlatsInHDF5 = a.m_bSkipInitialFlatsInHDF5;
}

void CDlgReconst::EnableCtrl() {
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_AUTO1)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_AUTO2)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SHOW1)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SHOW2)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SINO1)->EnableWindow(FALSE);
	//GetDlgItem(IDC_RECONST_RESOLN)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_QUEUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SLICE1)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SLICE2)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_CENT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_CENT2)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_PIXEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_TILT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_OPTIONS)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_HISDATASET2)->EnableWindow(FALSE);
	//GetDlgItem(IDC_RECONST_INTP)->EnableWindow(FALSE);
	//GetDlgItem(IDC_RECONST_FILTER)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SUFFIX3)->EnableWindow(FALSE);
	//GetDlgItem(IDC_RECONST_CUTOFF)->EnableWindow(FALSE);
	//GetDlgItem(IDC_RECONST_ORDER)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_OFFCT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_OFFAXIS)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SELECT)->EnableWindow(FALSE);//select frame
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	if (iStatus & CDLGRECONST_SHOWIMAGE) return;
	//
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDOK)->SetWindowText("Pause");
	if (iStatus & CDLGRECONST_BUSY) return;
	//
	GetDlgItem(IDOK)->SetWindowText("Batch");
	GetDlgItem(IDC_RECONST_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_AUTO1)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_AUTO2)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SHOW1)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SHOW2)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SINO1)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RECONST_RESOLN)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SELECT)->EnableWindow(TRUE);//select frame
	GetDlgItem(IDC_RECONST_QUEUE)->EnableWindow(TRUE);//////////////
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	//
	GetDlgItem(IDC_RECONST_SLICE1)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SLICE2)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_CENT1)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_CENT2)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_PIXEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_TILT)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_OPTIONS)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_HISDATASET2)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RECONST_INTP)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RECONST_FILTER)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECONST_SUFFIX3)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RECONST_CUTOFF)->EnableWindow(TRUE);
	//
	//GetDlgItem(IDC_RECONST_ORDER)->EnableWindow(FALSE);
	//if (m_Filter == CDLGRECONST_FILT_BUTER) {
	//	GetDlgItem(IDC_RECONST_ORDER)->EnableWindow(TRUE);
	//}
//080704 #ifdef _DEBUG
	GetDlgItem(IDC_RECONST_OFFCT)->EnableWindow(TRUE);
	if (m_bOffsetCT) GetDlgItem(IDC_RECONST_OFFAXIS)->EnableWindow(TRUE);
//080704 #endif
}

void CDlgReconst::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReconst)
	DDX_Control(pDX, IDC_RECONST_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_RECONST_SLICE1, m_Slice1);
	DDX_Text(pDX, IDC_RECONST_SLICE2, m_Slice2);
	DDX_Text(pDX, IDC_RECONST_PIXEL, m_PixelWidth);
	DDV_MinMaxDouble(pDX, m_PixelWidth, 0., 1000.);
	//DDX_CBIndex(pDX, IDC_RECONST_FILTER, m_Filter);
	//DDX_Text(pDX, IDC_RECONST_SUFFIX, m_Suffix);
	//DDX_Text(pDX, IDC_RECONST_CUTOFF, m_Cutoff);
	//DDV_MinMaxDouble(pDX, m_Cutoff, 0., 1.);
	//DDX_Text(pDX, IDC_RECONST_ORDER, m_Order);
	DDX_Text(pDX, IDC_RECONST_CENT1, m_Center1);
	DDX_Text(pDX, IDC_RECONST_CENT2, m_Center2);
	//DDX_CBIndex(pDX, IDC_RECONST_INTP, m_Interpolation);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_RECONST_OFFCT, m_bOffsetCT);
	DDX_Text(pDX, IDC_RECONST_OFFAXIS, m_OffsetAxis);
	DDX_Text(pDX, IDC_RECONST_TILT, m_TiltAngle);//100320
	DDV_MinMaxFloat(pDX, m_TiltAngle, -180, 180);
	DDX_Control(pDX, IDC_RECONST_HISDATASET2, m_HisDataset);
	DDX_Text(pDX, IDC_RECONST_SUFFIX3, m_Suffix);
}


BEGIN_MESSAGE_MAP(CDlgReconst, CDialog)
	//{{AFX_MSG_MAP(CDlgReconst)
	ON_BN_CLICKED(IDC_RECONST_QUEUE, OnReconstQueue)
	ON_BN_CLICKED(IDC_RECONST_AUTO1, OnReconstAuto1)
	ON_BN_CLICKED(IDC_RECONST_AUTO2, OnReconstAuto2)
	ON_BN_CLICKED(IDC_RECONST_SHOW1, OnReconstShow1)
	ON_BN_CLICKED(IDC_RECONST_SHOW2, OnReconstShow2)
//	ON_CBN_SELCHANGE(IDC_RECONST_FILTER, OnSelchangeReconstFilter)
	ON_BN_CLICKED(IDC_RECONST_STOP, OnReconstStop)
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDC_RECONST_OPTIONS, OnReconstOptions)
	ON_BN_CLICKED(IDC_RECONST_OFFCT, &CDlgReconst::OnBnClickedReconstOffct)
	ON_BN_CLICKED(IDC_RECONST_OPTIONS, &CDlgReconst::OnBnClickedReconstOptions)
	ON_EN_KILLFOCUS(IDC_RECONST_CENT1, &CDlgReconst::OnEnKillfocusReconstCent1)
	ON_EN_KILLFOCUS(IDC_RECONST_CENT2, &CDlgReconst::OnEnKillfocusReconstCent2)
	ON_BN_CLICKED(IDC_RECONST_SINO1, &CDlgReconst::OnBnClickedReconstSino1)
	ON_CBN_SELCHANGE(IDC_RECONST_HISDATASET2, &CDlgReconst::OnCbnSelchangeReconstHisdataset)
	ON_EN_SETFOCUS(IDC_RECONST_CENT1, &CDlgReconst::OnEnSetfocusReconstCent1)
	ON_EN_SETFOCUS(IDC_RECONST_CENT2, &CDlgReconst::OnEnSetfocusReconstCent2)
	ON_EN_SETFOCUS(IDC_RECONST_SLICE1, &CDlgReconst::OnEnSetfocusReconstSlice1)
	ON_EN_SETFOCUS(IDC_RECONST_SLICE2, &CDlgReconst::OnEnSetfocusReconstSlice2)
//	ON_BN_CLICKED(IDC_RECONST_RESOLN, &CDlgReconst::OnBnClickedReconstResoln)
	ON_BN_CLICKED(IDC_RECONST_SELECT, &CDlgReconst::OnBnClickedReconstSelect)
	ON_EN_UPDATE(IDC_RECONST_CENT1, &CDlgReconst::OnEnUpdateReconstCent1)
	ON_EN_UPDATE(IDC_RECONST_CENT2, &CDlgReconst::OnEnUpdateReconstCent2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReconst メッセージ ハンドラ

void CDlgReconst::OnReconstQueue() 
{
	if (!pd) return;
	UpdateData();//181202
	if (CheckParams()) {AfxMessageBox("Parameter error"); return;}
	//181202 UpdateData();
	if (m_Slice1 == m_Slice2) {
		if (AfxMessageBox("This will generate axis-scan images.\r\nProceed?", MB_OKCANCEL) == IDCANCEL) return;
	}
	//
	RECONST_QUEUE rq;
	rq.iTrimWidth = m_Trim;
	rq.iLayer1 = m_Slice1;
	rq.iLayer2 = m_Slice2;
	rq.dCenter1 = atof(m_Center1) - rq.iTrimWidth;
	rq.dCenter2 = atof(m_Center2) - rq.iTrimWidth;
	rq.dPixelWidth = m_PixelWidth;
	rq.iFilter = m_Filter;
	rq.dCutoff = m_Cutoff;
	rq.dOrder = m_Order;
	rq.iInterpolation = m_Interpolation;
	rq.outFilePrefix = m_Suffix;
	rq.outFilePath = m_Outpath;
	//pd params
	rq.itexFilePrefix = pd->GetDataPrefix();
	rq.itexFileSuffix = pd->GetDataSuffix();
	pd->GetDimension(&(rq.iXdim), &(rq.iYdim));
	rq.iRawSinoXdim = rq.iXdim;
	rq.iSinoYdim = pd->GetSinogramYdim(m_bOffsetCT);
	if (m_bOffsetCT) {rq.iXdim *= 2; rq.bOffsetCT = true;}
	else rq.bOffsetCT = false;
	rq.iXdim -= rq.iTrimWidth * 2;
	rq.dReconFlags = 0;
	if (m_AngularIntp) rq.dReconFlags |= RQFLAGS_ANGINTP;// else rq.dReconFlags &= ~RQFLAGS_ANGINTP;
	if (m_Zernike) rq.dReconFlags |= RQFLAGS_ZERNIKE;// else rq.dReconFlags &= ~RQFLAGS_ZERNIKE;
	if (m_bSkipInitialFlatsInHDF5) rq.dReconFlags |= RQFLAGS_SKIPINITIALFLATSINHDF5;
	//140728
	if (m_FrameUsage == CDLGRECONST_FRAME_ODD) rq.dReconFlags |=  RQFLAGS_USEONLYODDFRAMES;
	else if (m_FrameUsage == CDLGRECONST_FRAME_EVEN) rq.dReconFlags |=  RQFLAGS_USEONLYEVENFRAMES;
	//110920 rq.bAngularIntp = m_AngularIntp;
	rq.fTiltAngle = m_TiltAngle;
	rq.logFileName = pd->GetLogPath();
	rq.dataPath = pd->GetDataPath();
	rq.drStart = m_drStart;
	rq.drEnd = m_drEnd;
	rq.drX = m_drX;
	rq.drY = m_drY;
	rq.drOmit = m_drOmit;
	rq.dAxisInc = m_dAxisInc;
	if (m_bDriftList) rq.dReconFlags |= RQFLAGS_DRIFTLIST;
	if (m_bDriftParams) rq.dReconFlags |= RQFLAGS_DRIFTPARAMS;
	rq.sDriftListPath = m_sDriftListPath;
	rq.sFramesToExclude = pd->m_sFramesToExclude;
	rq.iSampleFrameStart = pd->dlgReconst.m_iDlgFL_SampleFrameStart;
	rq.iSampleFrameEnd = pd->dlgReconst.m_iDlgFL_SampleFrameEnd;
	//rq.iDatasetSize = m_iDatasetSize;
	rq.iDatasetSel = m_iDatasetSel;
	rq.bReconOptionUpdated = true;
	rq.iLossFrameSet = pd->iLossFrameSet;//120715
	//rq.filePath = pd->GetPathName();
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->dlgQueue.AddRecQueue(&rq);
	//100315 pApp->prevPixelWidth = m_PixelWidth;
	pApp->prevDlgReconst.ParamCopyFrom(*this);
	pApp->prevDlgReconst.iStatus = CDLGRECONST_BUSY;
	//
	ShowWindow(SW_HIDE);
	DestroyWindow();
	iContext = CDLGRECONST_CONTEXT_NONE;
	iStatus = CDLGRECONST_IDLE;
	//delete GPU memory if any 190107
	pd->GPUMemFree();
	//if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
	//	int nCPU = (int)(pApp->dlgProperty.iCUDA);
	//	for (int i = 0; i < nCPU; i++) { CudaReconstMemFree(&(pd->ri[i])); }
	//}
	//else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
	//	int nCPU = (int)(pApp->dlgProperty.iATIstream);
	//	for (int i = 0; i < nCPU; i++) { CLReconstMemFree(&(pd->ri[i])); }
	//}
//AfxMessageBox(rq.dataPath + rq.itexFilePrefix + rq.itexFileSuffix);
}

bool CDlgReconst::CheckParams(int idx) {
	int ixd, iyd;
	pd->GetDimension(&ixd, &iyd);
	if (m_bOffsetCT) ixd *= 2;
	ixd -= m_Trim * 2;
	if (ixd < 1) return true;
	bool bErr = false;
	const int iBinning = (m_Interpolation == 0) ? 4 : ((m_Interpolation == 1) ? 2 : 1);
	if (idx & 1) {
		if (m_Slice1 < 0) bErr = true;
		if (m_Slice1 >= iSliceMax - (iBinning-1)) bErr = true;//160803
		if (atof(m_Center1) <= 0) bErr = true;
		if (atof(m_Center1) >= iImageWidth) bErr = true;
	}
	if (idx & 2) {
		if (m_Slice2 < 0) bErr = true;
		if (m_Slice2 >= iSliceMax - (iBinning-1)) bErr = true;//160803
		if (atof(m_Center2) <= 0) bErr = true;
		if (atof(m_Center2) >= iImageWidth) bErr = true;
	}
	//160803
	//if ((idx & 3) == 3) {
	//	if (m_Slice2 - m_Slice1 + 1 < iBinning) bErr = true;
	//}
	return bErr;
}

void CDlgReconst::OnOK() 
{
	if (iStatus & CDLGRECONST_PAUSE) {
		iStatus &= ~CDLGRECONST_PAUSE;
		return;
	}
	if (iStatus & CDLGRECONST_BUSY) {
		GetDlgItem(IDOK)->SetWindowText("Resume");
		iStatus |= CDLGRECONST_PAUSE;
		while (iStatus & CDLGRECONST_BUSY) {
			::ProcessMessage();
			if (!(iStatus & CDLGRECONST_PAUSE)) {
				GetDlgItem(IDOK)->SetWindowText("Pause");
				break;
			}
		}
		return;
	}
	UpdateData();
	if (CheckParams()) {AfxMessageBox("Parameter error"); return;}
	if (m_Slice1 == m_Slice2) {
		if (AfxMessageBox("This will generate axis-scan images.\r\nProceed?", MB_OKCANCEL) == IDCANCEL) return;
	}
	//mutex190628
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	CSingleLock slock(&(pApp->m_mutex), FALSE);
	if (pApp->dlgProperty.m_ProcessorType != CDLGPROPERTY_PROCTYPE_INTEL) {
		slock.Lock(10);
		if (!slock.IsLocked()) {
			AfxMessageBox("Error: another instance is running.");
			return;
		}
	}
	iStatus = CDLGRECONST_BUSY;
	//CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	//100315 pApp->prevPixelWidth = m_PixelWidth;
	pApp->prevDlgReconst.ParamCopyFrom(*this);
	pApp->prevDlgReconst.iStatus = CDLGRECONST_BUSY;
	EnableCtrl();
	//
	if (pd) {
		RECONST_QUEUE rq;
		rq.iTrimWidth = m_Trim;
		rq.iLayer1 = m_Slice1;
		rq.iLayer2 = m_Slice2;
		rq.dCenter1 = atof(m_Center1) - rq.iTrimWidth;
		rq.dCenter2 = atof(m_Center2) - rq.iTrimWidth;
		rq.dPixelWidth = m_PixelWidth;
		rq.iFilter = m_Filter;
		rq.dCutoff = m_Cutoff;
		rq.dOrder = m_Order;
		rq.iInterpolation = m_Interpolation;
		rq.outFilePrefix = m_Suffix;
		rq.outFilePath = m_Outpath;
		rq.itexFilePrefix = pd->GetDataPrefix();
		rq.itexFileSuffix = pd->GetDataSuffix();
		pd->GetDimension(&(rq.iXdim), &(rq.iYdim));
		rq.iRawSinoXdim = rq.iXdim;
		rq.iSinoYdim = pd->GetSinogramYdim(m_bOffsetCT);
		if (m_bOffsetCT) {rq.iXdim *= 2; rq.bOffsetCT = true;}
		else rq.bOffsetCT = false;
		rq.iXdim -= rq.iTrimWidth * 2;
		rq.dReconFlags = 0;
		if (m_AngularIntp) rq.dReconFlags |= RQFLAGS_ANGINTP;
		if (m_Zernike) rq.dReconFlags |= RQFLAGS_ZERNIKE;
		if (m_bSkipInitialFlatsInHDF5) rq.dReconFlags |= RQFLAGS_SKIPINITIALFLATSINHDF5;
		//110920 rq.bAngularIntp = m_AngularIntp;
		rq.fTiltAngle = m_TiltAngle;
		rq.logFileName = pd->GetLogPath();
		rq.dataPath = pd->GetDataPath();
		rq.drStart = m_drStart;
		rq.drEnd = m_drEnd;
		rq.drX = m_drX;
		rq.drY = m_drY;
		rq.drOmit = m_drOmit;
		rq.dAxisInc = m_dAxisInc;
		if (m_bDriftList) rq.dReconFlags |= RQFLAGS_DRIFTLIST;
		if (m_bDriftParams) rq.dReconFlags |= RQFLAGS_DRIFTPARAMS;
		rq.sDriftListPath = m_sDriftListPath;
		rq.sFramesToExclude = pd->m_sFramesToExclude;
		rq.iSampleFrameStart = pd->dlgReconst.m_iDlgFL_SampleFrameStart;
		rq.iSampleFrameEnd = pd->dlgReconst.m_iDlgFL_SampleFrameEnd;
		//rq.iDatasetSize = m_iDatasetSize;
		rq.iDatasetSel = m_iDatasetSel;
		rq.iLossFrameSet = pd->iLossFrameSet;//120715
		rq.bReconOptionUpdated = bOptionUpdated;
		//
		pd->EnableSystemMenu(false);
		pd->BatchReconst(&rq);
		pd->EnableSystemMenu(true);
		bOptionUpdated = false;
	}
	//
	if (slock.IsLocked()) slock.Unlock();//190628
	pApp->SetIdle();
	iStatus = CDLGRECONST_IDLE;
	EnableCtrl();
	//CDialog::OnOK();
}

void CDlgReconst::OnCancel() 
{
	UpdateData();//090212
	ShowWindow(SW_HIDE);
	DestroyWindow();
	iStatus = CDLGRECONST_IDLE;
	iContext = CDLGRECONST_CONTEXT_NONE;
	//delete GPU memory if any 190107
	pd->GPUMemFree();
	//CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	//if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
	//	int nCPU = (int)(pApp->dlgProperty.iCUDA);
	//	for (int i = 0; i < nCPU; i++) { CudaReconstMemFree(&(pd->ri[i])); }
	//}
	//else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
	//	int nCPU = (int)(pApp->dlgProperty.iATIstream);
	//	for (int i = 0; i < nCPU; i++) { CLReconstMemFree(&(pd->ri[i])); }
	//}
	//CDialog::OnCancel();
}

BOOL CDlgReconst::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_HisDataset.AddString("0");
	if (m_nDataset > 1) {
		for (int i=1; i<m_nDataset; i++) {
			CString line; line.Format("%d", i);
			m_HisDataset.AddString(line);
		}
	}
	m_iDatasetSel = (m_iDatasetSel < m_nDataset) ? m_iDatasetSel : 0;
	m_HisDataset.SetCurSel(m_iDatasetSel);
	if (m_nDataset > 1) OnCbnSelchangeReconstHisdataset();

	EnableCtrl();
	m_Progress.SetRange(0, PROGRESS_BAR_UNIT * 2);
	m_Progress.SetPos(0);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDlgReconst::AdjustSliceWithBinning() {
	//160803
	const int iBinning = (m_Interpolation == 0) ? 4 : ((m_Interpolation == 1) ? 2 : 1);
	m_Slice1 = (m_Slice1 >= iSliceMax - (iBinning-1)) ? (iSliceMax-1 - (iBinning-1)) : m_Slice1;
	m_Slice2 = (m_Slice2 >= iSliceMax - (iBinning-1)) ? (iSliceMax-1 - (iBinning-1)) : m_Slice2;
}

void CDlgReconst::OnReconstAuto1() 
{
	if (!pd) return;
	UpdateData();
	double dCenter, dGrad, center = -1;
	BeginWaitCursor();
	if ( !pd->GetAxis(m_Slice1, &dCenter, &dGrad, GetOffsetCTaxis(), m_iDatasetSel) ) {
		center = dCenter + m_Slice1 * dGrad;
	}
	//else center = pd->GetCenter(m_Slice1);
	EndWaitCursor();
	if (center >= 0) {
		m_Center1.Format("%.2f", center);
		//EnableCtrl();
		UpdateData(FALSE);
		m_iDataseForCenter1 = m_iDatasetSel;
	}
}

void CDlgReconst::OnReconstAuto2() 
{
	if (!pd) return;
	UpdateData();
	double dCenter, dGrad, center = -1;
	BeginWaitCursor();
	if ( !pd->GetAxis(m_Slice2, &dCenter, &dGrad, GetOffsetCTaxis(), m_iDatasetSel) ) {
		center = dCenter + m_Slice2 * dGrad;
	}
	//else center = pd->GetCenter(m_Slice2);
	EndWaitCursor();
	if (center >= 0) {
		m_Center2.Format("%.2f", center);
		//EnableCtrl();
		UpdateData(FALSE);
		m_iDataseForCenter2 = m_iDatasetSel;
	}
}

void CDlgReconst::IncDecTilt(int iStep) {
	UpdateData();
	m_TiltAngle += iStep;
	//double dCenter = atof(m_Center1);
	//dCenter += (iDirection > 0 ? 1 : -1) * m_dAxisInc;
	//m_Center1.Format("%.2f", dCenter);
	UpdateData(FALSE);
	//if (m_bOffsetCT && pd) pd->ResetSinogram();
	//m_iDataseForCenter1 = m_iDatasetSel;
}

void CDlgReconst::IncDecCenter(int iParams, int iDirection) {
	if (iParams == 1) {
		UpdateData();
		double dCenter = atof(m_Center1);
		dCenter += (iDirection > 0 ? 1 : -1) * m_dAxisInc;
		m_Center1.Format("%.2f", dCenter);
		UpdateData(FALSE);
		if (m_bOffsetCT && pd) pd->ResetSinogram();
		m_iDataseForCenter1 = m_iDatasetSel;
	} else if (iParams == 2) {
		UpdateData();
		double dCenter = atof(m_Center2);
		dCenter += (iDirection > 0 ? 1 : -1) * m_dAxisInc;
		m_Center2.Format("%.2f", dCenter);
		UpdateData(FALSE);
		if (m_bOffsetCT && pd) pd->ResetSinogram();
		m_iDataseForCenter2 = m_iDatasetSel;
	} else {
		return;
	}
}

void CDlgReconst::OnReconstShow1() {CalcTomogram(1);}
void CDlgReconst::OnReconstShow2() { CalcTomogram(2); }

void CDlgReconst::CalcTomogram(int iParams, CGazoDoc* pdTarget) {
	UpdateData();
	int iSlice = 0;
	int iRtnContext = CDLGRECONST_CONTEXT_NONE;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	if (!pd) {AfxMessageBox("Data not found"); return;}
	if (iParams == 1) {
		iSlice = m_Slice1;
		if ((m_Slice1 >= 0) && (m_Slice1 < iSliceMax)) {
			if (m_iDataseForCenter1 != m_iDatasetSel) OnReconstAuto1();
		}
		if (CheckParams(1)) { AfxMessageBox("Parameter error"); return; }
		iRtnContext = CDLGRECONST_CONTEXT_TOMO1;
	} else if (iParams == 2) {
		iSlice = m_Slice2;
		if ((m_Slice2 >= 0) && (m_Slice2 < iSliceMax)) {
			if (m_iDataseForCenter2 != m_iDatasetSel) OnReconstAuto2();
		}
		if (CheckParams(2)) { AfxMessageBox("Parameter error"); return; }
		iRtnContext = CDLGRECONST_CONTEXT_TOMO2;
	} else {
		AfxMessageBox("Unknown params");
		return;
	}
	iContext = iRtnContext;
	pApp->SetBusy();
	iStatus = CDLGRECONST_SHOWIMAGE;
	EnableCtrl();
	pd->EnableSystemMenu(false);
	m_Progress.SetRange32(0, PROGRESS_BAR_UNIT * 2);
	m_Progress.SetStep(1);
	do {
		iStatus = CDLGRECONST_SHOWIMAGE;
		m_Progress.SetPos(0);
		RECONST_QUEUE rq;
		rq.iTrimWidth = m_Trim;
		rq.iLayer1 = m_Slice1;
		rq.iLayer2 = m_Slice2;
		rq.dCenter1 = atof(m_Center1) - rq.iTrimWidth;
		rq.dCenter2 = atof(m_Center2) - rq.iTrimWidth;
		const double dCenter = (iParams == 1) ? rq.dCenter1 : rq.dCenter2;
		rq.dPixelWidth = m_PixelWidth;
		rq.iFilter = m_Filter;
		rq.dCutoff = m_Cutoff;
		rq.dOrder = m_Order;
		rq.iInterpolation = m_Interpolation;
		rq.outFilePrefix = m_Suffix;
		rq.outFilePath = m_Outpath;
		rq.itexFilePrefix = pd->GetDataPrefix();
		rq.itexFileSuffix = pd->GetDataSuffix();
		pd->GetDimension(&(rq.iXdim), &(rq.iYdim));
		rq.iRawSinoXdim = rq.iXdim;
		rq.iSinoYdim = pd->GetSinogramYdim(m_bOffsetCT);
		if (m_bOffsetCT) { rq.iXdim *= 2; rq.bOffsetCT = true; }
		else rq.bOffsetCT = false;
		rq.iXdim -= rq.iTrimWidth * 2;
		rq.dReconFlags = 0;
		if (m_AngularIntp) rq.dReconFlags |= RQFLAGS_ANGINTP;
		if (m_Zernike) rq.dReconFlags |= RQFLAGS_ZERNIKE;
		if (m_bSkipInitialFlatsInHDF5) rq.dReconFlags |= RQFLAGS_SKIPINITIALFLATSINHDF5;
		//140728
		if (m_FrameUsage == CDLGRECONST_FRAME_ODD) rq.dReconFlags |= RQFLAGS_USEONLYODDFRAMES;
		else if (m_FrameUsage == CDLGRECONST_FRAME_EVEN) rq.dReconFlags |= RQFLAGS_USEONLYEVENFRAMES;
		//110920 rq.bAngularIntp = m_AngularIntp;
		rq.fTiltAngle = m_TiltAngle;
		rq.logFileName = pd->GetLogPath();
		rq.dataPath = pd->GetDataPath();
		rq.drStart = m_drStart;
		rq.drEnd = m_drEnd;
		rq.drX = m_drX;
		rq.drY = m_drY;
		rq.drOmit = m_drOmit;
		if (m_bDriftList) rq.dReconFlags |= RQFLAGS_DRIFTLIST;
		if (m_bDriftParams) rq.dReconFlags |= RQFLAGS_DRIFTPARAMS;
		rq.sDriftListPath = m_sDriftListPath;
		rq.sFramesToExclude = pd->m_sFramesToExclude;
		rq.iSampleFrameStart = pd->dlgReconst.m_iDlgFL_SampleFrameStart;
		rq.iSampleFrameEnd = pd->dlgReconst.m_iDlgFL_SampleFrameEnd;
		//rq.iDatasetSize = m_iDatasetSize;
		rq.iDatasetSel = m_iDatasetSel;
		rq.iLossFrameSet = pd->iLossFrameSet;//120715
		//CString msg; msg.Format("%d", rq.iLossFrameSet); AfxMessageBox(msg);
		rq.bReconOptionUpdated = bOptionUpdated;
		//
		pd->ShowTomogram(&rq, iSlice, dCenter, pdTarget);//, m_Filter, m_Interpolation, m_Suffix);
	} while (iStatus & CDLGRECONST_WHEEL);
	bOptionUpdated = false;
	pd->EnableSystemMenu(true);
	m_Progress.SetPos(PROGRESS_BAR_UNIT * 2);
	pApp->SetIdle();
	iStatus = CDLGRECONST_IDLE;
	EnableCtrl();
}

/*190107
void CDlgReconst::OnReconstShow2() 
{
	UpdateData();
	if ((m_Slice2 >= 0)&&(m_Slice2 < iSliceMax)) {
		//151110 if (atof(m_Center2) <= 0) OnReconstAuto2();
		if (m_iDataseForCenter2 != m_iDatasetSel) OnReconstAuto2();
	}
	if (CheckParams(2)) {AfxMessageBox("Parameter error"); return;}
	//CDialog::OnCancel();
	iStatus = CDLGRECONST_SHOWIMAGE;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	EnableCtrl();
	m_Progress.SetRange32(0, PROGRESS_BAR_UNIT * 2);
	m_Progress.SetPos(0);
	m_Progress.SetStep(1);
	if (pd) {
		pd->EnableSystemMenu(false);
		RECONST_QUEUE rq;
		rq.iTrimWidth = m_Trim;
		rq.iLayer1 = m_Slice1;
		rq.iLayer2 = m_Slice2;
		rq.dCenter1 = atof(m_Center1) - rq.iTrimWidth;
		rq.dCenter2 = atof(m_Center2) - rq.iTrimWidth;
		rq.dPixelWidth = m_PixelWidth;
		rq.iFilter = m_Filter;
		rq.dCutoff = m_Cutoff;
		rq.dOrder = m_Order;
		rq.iInterpolation = m_Interpolation;
		rq.outFilePrefix = m_Suffix;
		rq.outFilePath = m_Outpath;
		rq.itexFilePrefix = pd->GetDataPrefix();
		rq.itexFileSuffix = pd->GetDataSuffix();
		pd->GetDimension(&(rq.iXdim), &(rq.iYdim));
		rq.iRawSinoXdim = rq.iXdim;
		rq.iSinoYdim = pd->GetSinogramYdim(m_bOffsetCT);
		if (m_bOffsetCT) {rq.iXdim *= 2; rq.bOffsetCT = true;}
		else rq.bOffsetCT = false;
		rq.iXdim -= rq.iTrimWidth * 2;
		rq.dReconFlags = 0;
		if (m_AngularIntp) rq.dReconFlags |= RQFLAGS_ANGINTP;
		if (m_Zernike) rq.dReconFlags |= RQFLAGS_ZERNIKE;
		if (m_bSkipInitialFlatsInHDF5) rq.dReconFlags |= RQFLAGS_SKIPINITIALFLATSINHDF5;
		//140728
		if (m_FrameUsage == CDLGRECONST_FRAME_ODD) rq.dReconFlags |=  RQFLAGS_USEONLYODDFRAMES;
		else if (m_FrameUsage == CDLGRECONST_FRAME_EVEN) rq.dReconFlags |=  RQFLAGS_USEONLYEVENFRAMES;
		//110920 rq.bAngularIntp = m_AngularIntp;
		rq.fTiltAngle = m_TiltAngle;
		rq.logFileName = pd->GetLogPath();
		rq.dataPath = pd->GetDataPath();
		rq.drStart = m_drStart;
		rq.drEnd = m_drEnd;
		rq.drX = m_drX;
		rq.drY = m_drY;
		rq.drOmit = m_drOmit;
		if (m_bDriftList) rq.dReconFlags |= RQFLAGS_DRIFTLIST;
		if (m_bDriftParams) rq.dReconFlags |= RQFLAGS_DRIFTPARAMS;
		rq.sDriftListPath = m_sDriftListPath;
		rq.sFramesToExclude = pd->m_sFramesToExclude;
		rq.iSampleFrameStart = pd->dlgReconst.m_iDlgFL_SampleFrameStart;
		rq.iSampleFrameEnd = pd->dlgReconst.m_iDlgFL_SampleFrameEnd;
		//rq.iDatasetSize = m_iDatasetSize;
		rq.iDatasetSel = m_iDatasetSel;
		rq.iLossFrameSet = pd->iLossFrameSet;//120715
		rq.bReconOptionUpdated = bOptionUpdated;
		pd->ShowTomogram(&rq, m_Slice2, rq.dCenter2);//, m_Filter, m_Interpolation, m_Suffix);
		bOptionUpdated = false;
		pd->EnableSystemMenu(true);
	}
	m_Progress.SetPos(PROGRESS_BAR_UNIT * 2);
	pApp->SetIdle();
	iStatus = CDLGRECONST_IDLE;
	iContext = CDLGRECONST_CONTEXT_TOMO2;
	EnableCtrl();
}*/

void CDlgReconst::OnBnClickedReconstSino1()
{
	//111019
	UpdateData();
	//if (CheckParams(1)) {AfxMessageBox("Parameter error"); return;}
	//CDialog::OnCancel();
	iStatus = CDLGRECONST_SHOWIMAGE;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	EnableCtrl();
	m_Progress.SetRange32(0, PROGRESS_BAR_UNIT);
	m_Progress.SetPos(0);
	m_Progress.SetStep(1);
	if (pd) {
		pd->EnableSystemMenu(false);
		RECONST_QUEUE rq;
		rq.iTrimWidth = m_Trim;
		rq.iLayer1 = m_Slice1;
		rq.iLayer2 = m_Slice2;
		rq.dCenter1 = atof(m_Center1) - rq.iTrimWidth;
		rq.dCenter2 = atof(m_Center2) - rq.iTrimWidth;
		rq.dPixelWidth = m_PixelWidth;
		rq.iFilter = m_Filter;
		rq.dCutoff = m_Cutoff;
		rq.dOrder = m_Order;
		rq.iInterpolation = m_Interpolation;
		rq.outFilePrefix = m_Suffix;
		rq.outFilePath = m_Outpath;
		rq.itexFilePrefix = pd->GetDataPrefix();
		rq.itexFileSuffix = pd->GetDataSuffix();
		pd->GetDimension(&(rq.iXdim), &(rq.iYdim));
		rq.iRawSinoXdim = rq.iXdim;
		rq.iSinoYdim = pd->GetSinogramYdim(m_bOffsetCT);
		if (m_bOffsetCT) {rq.iXdim *= 2; rq.bOffsetCT = true;}
		else rq.bOffsetCT = false;
		rq.iXdim -= rq.iTrimWidth * 2;
		rq.dReconFlags = 0;
		if (m_AngularIntp) rq.dReconFlags |= RQFLAGS_ANGINTP;
		if (m_Zernike) rq.dReconFlags |= RQFLAGS_ZERNIKE;
		if (m_bSkipInitialFlatsInHDF5) rq.dReconFlags |= RQFLAGS_SKIPINITIALFLATSINHDF5;
		//110920 rq.bAngularIntp = m_AngularIntp;
		rq.fTiltAngle = m_TiltAngle;
		rq.logFileName = pd->GetLogPath();
		rq.dataPath = pd->GetDataPath();
		rq.drStart = m_drStart;
		rq.drEnd = m_drEnd;
		rq.drX = m_drX;
		rq.drY = m_drY;
		rq.drOmit = m_drOmit;
		if (m_bDriftList) rq.dReconFlags |= RQFLAGS_DRIFTLIST;
		if (m_bDriftParams) rq.dReconFlags |= RQFLAGS_DRIFTPARAMS;
		rq.sDriftListPath = m_sDriftListPath;
		rq.sFramesToExclude = pd->m_sFramesToExclude;
		rq.iSampleFrameStart = pd->dlgReconst.m_iDlgFL_SampleFrameStart;
		rq.iSampleFrameEnd = pd->dlgReconst.m_iDlgFL_SampleFrameEnd;
		//rq.iDatasetSize = m_iDatasetSize;
		rq.iDatasetSel = m_iDatasetSel;
		rq.iLossFrameSet = pd->iLossFrameSet;//120715
		rq.bReconOptionUpdated = bOptionUpdated;
		//
		pd->ShowSinogram(&rq, m_Slice1, rq.dCenter1);
		bOptionUpdated = false;
		pd->EnableSystemMenu(true);
	}
	m_Progress.SetPos(PROGRESS_BAR_UNIT);
	pApp->SetIdle();
	iStatus = CDLGRECONST_IDLE;
	iContext = CDLGRECONST_CONTEXT_SINO;
	EnableCtrl();
}

void CDlgReconst::OnReconstStop() 
{
	iStatus = CDLGRECONST_STOP;	
}

void CDlgReconst::OnBnClickedReconstOffct()
{
	UpdateData();
	EnableCtrl();
	if (pd) {
		pd->ResetSinogram();
		pd->LoadLogFile(m_bOffsetCT);
	}
}

int CDlgReconst::GetOffsetCTaxis() {
	if (m_bOffsetCT) return m_OffsetAxis;
	else return 0;
}
void CDlgReconst::OnBnClickedReconstOptions()
{
	CDlgReconOpt dlg;
	dlg.m_Filter = m_Filter;
	dlg.m_Cutoff = m_Cutoff;
	dlg.m_Order = m_Order;
	dlg.m_Interpolation = m_Interpolation;
	dlg.m_Suffix = m_Suffix;
	dlg.m_AngularIntp = m_AngularIntp ? TRUE : FALSE;
	dlg.m_Zernike = m_Zernike ? TRUE : FALSE;
	dlg.m_bSkipInitialFlatsInHDF5 = m_bSkipInitialFlatsInHDF5 ? TRUE : FALSE;
	//dlg.m_TiltAngle = m_TiltAngle;
	dlg.m_Trim = m_Trim;
	dlg.m_Outpath = m_Outpath;
	dlg.m_drStart = m_drStart;
	dlg.m_drEnd = m_drEnd;
	dlg.m_drX = m_drX;
	dlg.m_drY = m_drY;
	dlg.m_drOmit = m_drOmit;
	dlg.iDatasetSel = m_iDatasetSel;
	dlg.m_sDriftListPath = m_sDriftListPath;
	dlg.m_bDriftList = m_bDriftList;
	dlg.m_bDriftParams = m_bDriftParams;
	dlg.m_FrameUsage = m_FrameUsage;
	dlg.m_dAxisInc = m_dAxisInc;
	//if (pd) dlg.nDataset = pd->maxHisFrame / pd->CountFrameFromConvBat(pd->GetDataPath());
	dlg.nDataset = m_nDataset;
	if (dlg.DoModal() == IDCANCEL) return;
	m_Filter = dlg.m_Filter;
	m_Cutoff = dlg.m_Cutoff;
	m_Order = dlg.m_Order;
	m_Interpolation = dlg.m_Interpolation;
	//m_Suffix = dlg.m_Suffix;
	m_AngularIntp = dlg.m_AngularIntp ? true : false;
	m_Zernike = dlg.m_Zernike ? true : false;
	m_bSkipInitialFlatsInHDF5 = dlg.m_bSkipInitialFlatsInHDF5 ? true : false;
	//m_TiltAngle = dlg.m_TiltAngle;
	m_Trim = dlg.m_Trim;
	m_Outpath = dlg.m_Outpath;
	m_drStart = dlg.m_drStart;
	m_drEnd = dlg.m_drEnd;
	m_drX = dlg.m_drX;
	m_drY = dlg.m_drY;
	m_drOmit = dlg.m_drOmit;
	//m_iDatasetSel = dlg.iDatasetSel;
	m_sDriftListPath = dlg.m_sDriftListPath;
	m_bDriftList = dlg.m_bDriftList ? true : false;
	m_bDriftParams = dlg.m_bDriftParams ? true : false;
	m_FrameUsage = dlg.m_FrameUsage;
	m_dAxisInc = dlg.m_dAxisInc;
	//140728
	//if (m_FrameUsage == CDLGRECONST_FRAME_ALL) AfxMessageBox("Frame all");
	//else if (m_FrameUsage == CDLGRECONST_FRAME_ODD) AfxMessageBox("Frame odd");
	//else if (m_FrameUsage == CDLGRECONST_FRAME_EVEN) AfxMessageBox("Frame even");

	bOptionUpdated = true;
}

void CDlgReconst::OnEnKillfocusReconstCent1()
{
	if (m_bOffsetCT && pd) pd->ResetSinogram();
	m_iDataseForCenter1 = m_iDatasetSel;
}

void CDlgReconst::OnEnKillfocusReconstCent2()
{
	if (m_bOffsetCT && pd) pd->ResetSinogram();
	m_iDataseForCenter2 = m_iDatasetSel;
}


void CDlgReconst::OnCbnSelchangeReconstHisdataset()
{
	if (m_nDataset <= 1) return;
	const int idigit = (int)log10((double)m_nDataset) + 1;
	CString fmt, fn;
	fmt.Format("%%0%dd", idigit);
	m_iDatasetSel = m_HisDataset.GetCurSel();
	fn.Format(fmt, m_iDatasetSel);
	UpdateData();
	m_Suffix = m_Suffix.SpanExcluding("01234567890") + fn;
	UpdateData(FALSE);
	bOptionUpdated = true;
	pd->dAxisCenter = 0;//130203 clear axis
}

void CDlgReconst::OnEnSetfocusReconstCent1()
{
	SetDefID(IDC_RECONST_SHOW1);
}

void CDlgReconst::OnEnSetfocusReconstCent2()
{
	SetDefID(IDC_RECONST_SHOW2);
}

void CDlgReconst::OnEnSetfocusReconstSlice1()
{
	SetDefID(IDC_RECONST_SHOW1);
}

void CDlgReconst::OnEnSetfocusReconstSlice2()
{
	SetDefID(IDC_RECONST_SHOW2);
}

//void CDlgReconst::OnBnClickedReconstResoln(){
//}

void CDlgReconst::OnBnClickedReconstSelect()
{
	CDlgFrameList dlg;
	dlg.m_sFramesToExclude = pd->m_sFramesToExclude;
	dlg.pd = this->pd;
	if (dlg.DoModal() == IDCANCEL) return;
	pd->m_sFramesToExclude = dlg.m_sFramesToExclude;
	bOptionUpdated = true;
}

void CDlgReconst::OnEnUpdateReconstCent1()
{
	// TODO:  これが RICHEDIT コントロールの場合、
	// まず、CDialog::OnInitDialog() 関数をオーバーライドして、OR 状態の ENM_CHANGE
	// OR 状態の ENM_SCROLL フラグを IParam マスクに入れて、
	// OR 状態の ENM_UPDATE フラグを lParam マスクに入れて、

	// TODO:  ここにコントロール通知ハンドラ コードを追加してください。
	m_iDataseForCenter1 = m_iDatasetSel;//170503
}

void CDlgReconst::OnEnUpdateReconstCent2()
{
	// TODO:  これが RICHEDIT コントロールの場合、
	// まず、CDialog::OnInitDialog() 関数をオーバーライドして、OR 状態の ENM_CHANGE
	// OR 状態の ENM_SCROLL フラグを IParam マスクに入れて、
	// OR 状態の ENM_UPDATE フラグを lParam マスクに入れて、

	// TODO:  ここにコントロール通知ハンドラ コードを追加してください。
	m_iDataseForCenter2 = m_iDatasetSel;//170503
}
