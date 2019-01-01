// DlgRefraction.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgRefraction.h"
#include "gazoDoc.h"
#include "DlgQueue.h"


// CDlgRefraction ダイアログ

IMPLEMENT_DYNAMIC(CDlgRefraction, CDialog)

CDlgRefraction::CDlgRefraction(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRefraction::IDD, pParent)
	, m_AtomZ(6)
	, m_AtomA(12)
	, m_Density(2.7)
	, m_LAC(41.3)
	, m_Xenergy(12.0)
	, m_S2Ddist(2.0)
	, m_PxSize(0.5)
	, m_Prefix(_T("m"))
{
	iStatus = CDLGREFRAC_IDLE;
	pd = NULL;
}

CDlgRefraction::~CDlgRefraction()
{
}

void CDlgRefraction::SetDoc(CGazoDoc* pDoc) {
	pd = pDoc;
}

void CDlgRefraction::EnableCtrl() {
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_SHOW)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_ATOMZ)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_ATOMA)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_DENSITY)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_LAC)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_XENERGY)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_S2DDIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_PXSIZE)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFR_PREFIX)->EnableWindow(FALSE);
	//
	GetDlgItem(IDC_RECONST_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	if (iStatus & CDLGREFRAC_SHOWIMAGE) return;
	//
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDOK)->SetWindowText("Pause");
	if (iStatus & CDLGREFRAC_BUSY) return;
	//
	GetDlgItem(IDOK)->SetWindowText("Batch");
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_SHOW)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_ATOMZ)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_ATOMA)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_DENSITY)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_LAC)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_XENERGY)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_S2DDIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_PXSIZE)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFR_PREFIX)->EnableWindow(TRUE);
	//
	GetDlgItem(IDC_REFR_STATUS)->SetWindowText("Ready");
}

void CDlgRefraction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_REFR_ATOMZ, m_AtomZ);
	DDX_Text(pDX, IDC_REFR_ATOMA, m_AtomA);
	DDX_Text(pDX, IDC_REFR_DENSITY, m_Density);
	DDX_Text(pDX, IDC_REFR_LAC, m_LAC);
	DDX_Text(pDX, IDC_REFR_XENERGY, m_Xenergy);
	DDX_Text(pDX, IDC_REFR_S2DDIST, m_S2Ddist);
	DDX_Text(pDX, IDC_REFR_PXSIZE, m_PxSize);
	DDX_Text(pDX, IDC_REFR_PREFIX, m_Prefix);
}


BEGIN_MESSAGE_MAP(CDlgRefraction, CDialog)
	ON_BN_CLICKED(IDC_REFR_SHOW, &CDlgRefraction::OnBnClickedRefrShow)
	ON_BN_CLICKED(IDC_REFR_STOP, &CDlgRefraction::OnBnClickedRefrStop)
END_MESSAGE_MAP()


// CDlgRefraction メッセージ ハンドラ

void CDlgRefraction::OnBnClickedRefrShow()
{
	UpdateData();
	//if (CheckParams(1)) {AfxMessageBox("Parameter error"); return;}
	iStatus = CDLGREFRAC_SHOWIMAGE;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	EnableCtrl();
	if (pd) {
		pd->EnableSystemMenu(false);
		//
		REFRAC_QUEUE refq;
		const double z0 = m_S2Ddist * 0.1;
		const double lambda = m_Xenergy * 1000. / 12398.;
		refq.ndz0 = z0 * 0.00000270 * m_AtomZ / m_AtomA * m_Density * lambda * lambda;
		refq.dLAC = m_LAC;
		refq.dPixelWidth = m_PxSize;//um
		refq.dataPath = pd->GetDataPath();
		refq.itexFilePrefix = pd->GetDataPrefix();
		refq.outFilePrefix = m_Prefix;
		pd->GetDimension(&(refq.iXdim), &(refq.iYdim));
		//
		pd->ShowRefracCorr(&refq);
		//
		pd->EnableSystemMenu(true);
	}
	pApp->SetIdle();
	iStatus = CDLGREFRAC_IDLE;
	EnableCtrl();
}

void CDlgRefraction::OnOK()
{
	if (iStatus & CDLGREFRAC_PAUSE) {
		iStatus &= ~CDLGREFRAC_PAUSE;
		return;
	}
	if (iStatus & CDLGREFRAC_BUSY) {
		GetDlgItem(IDOK)->SetWindowText("Resume");
		iStatus |= CDLGREFRAC_PAUSE;
		while (iStatus & CDLGREFRAC_BUSY) {
			::ProcessMessage();
			if (!(iStatus & CDLGREFRAC_PAUSE)) {
				GetDlgItem(IDOK)->SetWindowText("Pause");
				break;
			}
		}
		return;
	}
	UpdateData();
	//if (CheckParams()) {AfxMessageBox("Parameter error"); return;}
	iStatus = CDLGREFRAC_BUSY;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	//
	EnableCtrl();
	if (pd) {
		pd->EnableSystemMenu(false);
		//
		REFRAC_QUEUE refq;
		const double z0 = m_S2Ddist * 0.1;
		const double lambda = m_Xenergy * 1000. / 12398.;
		refq.ndz0 = z0 * 0.00000270 * m_AtomZ / m_AtomA * m_Density * lambda * lambda;
		refq.dLAC = m_LAC;
		refq.dPixelWidth = m_PxSize;//um
		refq.dataPath = pd->GetDataPath();
		refq.itexFilePrefix = pd->GetDataPrefix();
		refq.outFilePrefix = m_Prefix;
		pd->GetDimension(&(refq.iXdim), &(refq.iYdim));
		//
		pd->BatchRefracCorr(&refq);
		//
		pd->EnableSystemMenu(true);
	}
	//
	pApp->SetIdle();
	iStatus = CDLGREFRAC_IDLE;
	EnableCtrl();

	//CDialog::OnOK();
}

void CDlgRefraction::OnCancel()
{
	UpdateData();//090212
	ShowWindow(SW_HIDE);
	DestroyWindow();
	iStatus = CDLGREFRAC_IDLE;
	//CDialog::OnCancel();
}

void CDlgRefraction::OnBnClickedRefrStop()
{
	iStatus = CDLGREFRAC_STOP;
}

BOOL CDlgRefraction::OnInitDialog()
{
	CDialog::OnInitDialog();

	EnableCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
