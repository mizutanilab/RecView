// DlgProperty.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgProperty.h"
#include "MainFrm.h"

// CDlgProperty ダイアログ

IMPLEMENT_DYNAMIC(CDlgProperty, CDialog)

CDlgProperty::CDlgProperty(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgProperty::IDD, pParent)
	, bEnableSIMD(FALSE)
	, m_ProcessorType(CDLGPROPERTY_PROCTYPE_INTEL)
	, m_EnReport(TRUE)
	, bUseCUDAFFT(FALSE)
	, m_EnFastSeek(TRUE)
	, m_bEnableAVX2(FALSE)
	, m_EnCUDAStream(TRUE) //190620
{
	Init(1, false, false, 0, CUDA_BLOCKSIZE, CUDA_WARPSIZE, 
					0, ATISTREAM_MAXWORK, ATISTREAM_UNITWORK);
}

CDlgProperty::~CDlgProperty()
{
}

void CDlgProperty::Init(int icpu, bool bsimd, bool bavx2, 
						int iCudaCount, int iCudaBlock, int iCudaWarp,
						int iATIcount, int iATImaxwork, int iATIunitwork, int iProcessorType) {
	//CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	//iCPU = pApp->iAvailableCPU;
	iCPU = icpu > 1 ? icpu : 1;
	bSIMD = bsimd;
	if (bSIMD) bEnableSIMD = TRUE;//160918 enabled again
	bAVX2 = bavx2;
	if (bAVX2) m_bEnableAVX2 = TRUE;
	maxCUDA = iCudaCount;
	maxCUDAThreadsPerBlock = iCudaBlock;
	iCUDAwarpsize = iCudaWarp;
	iCUDAnblock = CUDA_BLOCKSIZE;
	iCUDA = maxCUDA;
	//
	maxATIstream = iATIcount;
	maxATIstreamNwork = iATImaxwork;
	iATIstreamUnitwork = iATIunitwork;
	iATIstreamNwork = ATISTREAM_MAXWORK;
	iATIstream = maxATIstream;
	switch (iProcessorType) {
	case CDLGPROPERTY_PROCTYPE_CUDA: {m_ProcessorType = iProcessorType; break;}
	case CDLGPROPERTY_PROCTYPE_ATISTREAM: {m_ProcessorType = iProcessorType; break;}
	case CDLGPROPERTY_PROCTYPE_INTEL: {m_ProcessorType = iProcessorType; break;}
	default: {
		if (maxCUDA) m_ProcessorType = CDLGPROPERTY_PROCTYPE_CUDA;
		else if (maxATIstream) m_ProcessorType = CDLGPROPERTY_PROCTYPE_ATISTREAM;
		break;
		}
	}
	switch (m_ProcessorType) {
	case CDLGPROPERTY_PROCTYPE_CUDA: {m_EnReport = TRUE; break;}
	case CDLGPROPERTY_PROCTYPE_ATISTREAM: {m_EnReport = FALSE; break;}
	default: {m_EnReport = TRUE; break;}
	}
	//090724 iMemory = 20;
	//190729 iMemory = 80;
	iMemory = 40;
}

void CDlgProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROP_NCPU, m_CPU);
	DDX_Control(pDX, IDC_PROP_MEMORY, m_Memory);
	DDX_Check(pDX, IDC_PROP_SIMD, bEnableSIMD);
	DDX_Radio(pDX, IDC_PROP_INTELCPU, m_ProcessorType);
	DDX_Control(pDX, IDC_PROP_NGPU, m_CUDA);
	DDX_Control(pDX, IDC_PROP_CUDANBLOCK, m_CUDAnblock);
	DDX_Check(pDX, IDC_PROP_ENREPORT, m_EnReport);
	DDX_Check(pDX, IDC_PROP_CUDAFFT, bUseCUDAFFT);
	DDX_Control(pDX, IDC_PROP_NATISTREAM, m_ATIstream);
	DDX_Control(pDX, IDC_PROP_ATISTREAMNWORK, m_ATIstreamNwork);
	DDX_Check(pDX, IDC_PROP_ENFASTSEEK, m_EnFastSeek);
	DDX_Check(pDX, IDC_PROP_AVX2, m_bEnableAVX2);
	DDX_Check(pDX, IDC_PROP_CUDASTREAM, m_EnCUDAStream);
}


BEGIN_MESSAGE_MAP(CDlgProperty, CDialog)
ON_BN_CLICKED(IDC_PROP_INTELCPU, &CDlgProperty::OnBnClickedIntelcpu)
ON_BN_CLICKED(IDC_PROP_CUDAGPU, &CDlgProperty::OnBnClickedCudagpu)
ON_BN_CLICKED(IDC_PROP_ATISTREAM, &CDlgProperty::OnBnClickedPropAtistream)
ON_BN_CLICKED(IDC_PROP_SIMD, &CDlgProperty::OnBnClickedPropSimd)
ON_BN_CLICKED(IDC_PROP_INFO, &CDlgProperty::OnBnClickedPropInfo)
END_MESSAGE_MAP()


// CDlgProperty メッセージ ハンドラ

BOOL CDlgProperty::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ここに初期化を追加してください
	rMemory = iMemory;
	rCPU = iCPU;
	rCUDA = iCUDA;
	r_ProcessorType = m_ProcessorType;
	rEnableSIMD = bEnableSIMD;
	rEnableAVX2 = m_bEnableAVX2;
	rCUDAnblock = iCUDAnblock;
	r_EnReport = m_EnReport;
	r_EnFastSeek = m_EnFastSeek;
	r_UseCUDAFFT = bUseCUDAFFT;
	r_EnCUDAStream = m_EnCUDAStream;
	rATIstream = iATIstream;
	rATIstreamNwork = iATIstreamNwork;

	m_Memory.InsertString(0, "10%");
	m_Memory.InsertString(1, "20%");
	m_Memory.InsertString(2, "40%");
	m_Memory.InsertString(3, "60%");
	m_Memory.InsertString(4, "80%");
	m_Memory.InsertString(5, "100%");
	if (iMemory == 10) m_Memory.SetCurSel(0);
	else if (iMemory == 20) m_Memory.SetCurSel(1);
	else if (iMemory == 40) m_Memory.SetCurSel(2);
	else if (iMemory == 60) m_Memory.SetCurSel(3);
	else if (iMemory == 100) m_Memory.SetCurSel(5);
	else m_Memory.SetCurSel(4);

	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	int i;
	//INTEL CPU
	for (i=0; i<pApp->iAvailableCPU; i++) {
		CString caption;
		caption.Format("%3d", i+1);
		m_CPU.InsertString(i, caption);
	}
	i = pApp->iAvailableCPU;
	if (iCPU > i) {
		m_CPU.SetCurSel(i-1);
		iCPU = i;
	} else {
		m_CPU.SetCurSel(iCPU-1);
	}
	//CUDA GPU
	//CString msg; msg.Format("131011 maxCUDA %d", maxCUDA); AfxMessageBox(msg);
	for (i=0; i<maxCUDA; i++) {
		CString caption;
		caption.Format("%3d", i+1);
		m_CUDA.InsertString(i, caption);
	}
	i = maxCUDA;
	if (iCUDA > i) {
		m_CUDA.SetCurSel(i-1);
		iCUDA = i;
	} else {
		m_CUDA.SetCurSel(iCUDA-1);
	}
	//CUDA NBLOCK
	int item = 0;
	int iCurrSel = 0;
	CString caption;
	i = iCUDAwarpsize;
	{
		caption.Format("%4d", i);
		m_CUDAnblock.InsertString(item, caption);
		m_CUDAnblock.SetItemData(item, i);
		if (i == iCUDAnblock) iCurrSel = item;
		item++;
	}
	for (i=iCUDAwarpsize*2; i<=maxCUDAThreadsPerBlock; i+=iCUDAwarpsize*2) {
		caption.Format("%4d", i);
		m_CUDAnblock.InsertString(item, caption);
		m_CUDAnblock.SetItemData(item, i);
		if (i == iCUDAnblock) iCurrSel = item;
		item++;
	}
	m_CUDAnblock.SetCurSel(iCurrSel);
	//ATIstream GPU
	for (i=0; i<maxATIstream; i++) {
		CString caption;
		caption.Format("%3d", i+1);
		m_ATIstream.InsertString(i, caption);
	}
	i = maxATIstream;
	if (iATIstream > i) {
		m_ATIstream.SetCurSel(i-1);
		iATIstream = i;
	} else {
		m_ATIstream.SetCurSel(iATIstream-1);
	}
	//ATIstream Nwork
	item = 0;
	iCurrSel = 0;
	for (i=iATIstreamUnitwork; i<=maxATIstreamNwork; i+=iATIstreamUnitwork) {
		caption.Format("%4d", i);
		m_ATIstreamNwork.InsertString(item, caption);
		m_ATIstreamNwork.SetItemData(item, i);
		if (i == iATIstreamNwork) iCurrSel = item;
		item++;
	}
	m_ATIstreamNwork.SetCurSel(iCurrSel);
	//
	EnableCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgProperty::EnableCtrl() {
	GetDlgItem(IDC_PROP_CUDAGPU)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_ATISTREAM)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_INTELCPU)->EnableWindow(TRUE);
	//
	GetDlgItem(IDC_PROP_SIMD)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_AVX2)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_NCPU)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_NGPU)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_CUDANBLOCK)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_CUDAFFT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_CUDASTREAM)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_NATISTREAM)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROP_ATISTREAMNWORK)->EnableWindow(FALSE);
	//
	if (iCUDA) GetDlgItem(IDC_PROP_CUDAGPU)->EnableWindow(TRUE);
	if (iATIstream) GetDlgItem(IDC_PROP_ATISTREAM)->EnableWindow(TRUE);
	switch (m_ProcessorType) {
		case CDLGPROPERTY_PROCTYPE_INTEL: {
			if (bSIMD) {
				GetDlgItem(IDC_PROP_SIMD)->EnableWindow(TRUE);
				if (bAVX2 && bEnableSIMD) GetDlgItem(IDC_PROP_AVX2)->EnableWindow(TRUE);
			}
			GetDlgItem(IDC_PROP_NCPU)->EnableWindow(TRUE);
			break;}
		case CDLGPROPERTY_PROCTYPE_CUDA: {
			GetDlgItem(IDC_PROP_NGPU)->EnableWindow(TRUE);
			GetDlgItem(IDC_PROP_CUDANBLOCK)->EnableWindow(TRUE);
			GetDlgItem(IDC_PROP_CUDASTREAM)->EnableWindow(TRUE);
#ifdef CUDAFFT
			GetDlgItem(IDC_PROP_CUDAFFT)->EnableWindow(TRUE);
#endif
			break;}
		case CDLGPROPERTY_PROCTYPE_ATISTREAM: {
			GetDlgItem(IDC_PROP_NATISTREAM)->EnableWindow(TRUE);
			GetDlgItem(IDC_PROP_ATISTREAMNWORK)->EnableWindow(TRUE);
			break;}
		default: {
			break;}
	}
}

void CDlgProperty::OnOK()
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	UpdateData();
	iMemory = m_Memory.GetCurSel();
	switch (iMemory) {
		case 0: {iMemory = 10; break;}
		case 1: {iMemory = 20; break;}
		case 2: {iMemory = 40; break;}
		case 3: {iMemory = 60; break;}
		case 5: {iMemory = 100; break;}
		default: {iMemory = 40;}
	}
	iCPU = m_CPU.GetCurSel() + 1;
	//
	iCUDA = m_CUDA.GetCurSel() + 1;
	int item = m_CUDAnblock.GetCurSel();
	if (item >= 0) iCUDAnblock = (int)m_CUDAnblock.GetItemData(item);
	else iCUDAnblock = CUDA_BLOCKSIZE;
	//
	iATIstream = m_ATIstream.GetCurSel() + 1;
	item = m_ATIstreamNwork.GetCurSel();
	if (item >= 0) iATIstreamNwork = (int)m_ATIstreamNwork.GetItemData(item);
	else iATIstreamNwork = ATISTREAM_MAXWORK;

	CString sPane1 = "";
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	switch (m_ProcessorType) {
	case CDLGPROPERTY_PROCTYPE_INTEL: {sPane1 = pApp->sCPUname; break;}
	case CDLGPROPERTY_PROCTYPE_CUDA: {sPane1 = pApp->sCudaGPUname; break;}
	case CDLGPROPERTY_PROCTYPE_ATISTREAM: {sPane1 = "AMD GPU"; break;}
	default: {sPane1 = "Generic processor"; break;}
	}
	CMainFrame* pf = (CMainFrame*)AfxGetMainWnd();
	if (pf) pf->m_wndStatusBar.SetPaneText(1, sPane1);

	CDialog::OnOK();
}

void CDlgProperty::OnBnClickedIntelcpu()
{
	UpdateData();
	EnableCtrl();
}

void CDlgProperty::OnBnClickedCudagpu()
{
	UpdateData();
	EnableCtrl();
}

void CDlgProperty::OnBnClickedPropAtistream()
{
	UpdateData();
	EnableCtrl();
}

void CDlgProperty::OnCancel()
{
	iMemory = rMemory;
	m_ProcessorType = r_ProcessorType;
	iCPU = rCPU;
	bEnableSIMD = rEnableSIMD;
	m_bEnableAVX2 = rEnableAVX2;
	iCUDA = rCUDA;
	iCUDAnblock = rCUDAnblock;
	bUseCUDAFFT = r_UseCUDAFFT;
	iATIstream = rATIstream;
	iATIstreamNwork = rATIstreamNwork;
	m_EnReport = r_EnReport;
	m_EnFastSeek = r_EnFastSeek;
	m_EnCUDAStream = r_EnCUDAStream;

	CDialog::OnCancel();
}



void CDlgProperty::OnBnClickedPropSimd()
{
	UpdateData();
	EnableCtrl();
}


void CDlgProperty::OnBnClickedPropInfo()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->OnViewError();
}
