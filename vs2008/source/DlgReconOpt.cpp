// DlgReconOpt.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgReconOpt.h"
#include <shlobj.h> //SHBrowseForFolder

// CDlgReconOpt ダイアログ

IMPLEMENT_DYNAMIC(CDlgReconOpt, CDialog)

CDlgReconOpt::CDlgReconOpt(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReconOpt::IDD, pParent)
	, m_Filter(CDLGRECONST_FILT_HAN)
	, m_Cutoff(0.5)
	, m_Order(6.0)
	, m_Interpolation(2)
	, m_Suffix(_T("rec"))
	, m_AngularIntp(FALSE)
	, m_Trim(0)
	, m_Outpath(_T(""))
	, m_drStart(0)
	, m_drEnd(0)
	, m_drX(0)
	, m_drY(0)
	, m_drOmit(FALSE)
	, m_Zernike(FALSE)
	, m_bDriftParams(FALSE)
	, m_bDriftList(FALSE)
	, m_sDriftListPath(_T(""))
	, m_FrameUsage(CDLGRECONST_FRAME_ALL)
	, m_bSkipInitialFlatsInHDF5(FALSE)
	, m_dAxisInc(0.5)
{
	nDataset = 1;
	iDatasetSel = 0;
}

CDlgReconOpt::~CDlgReconOpt()
{
}

void CDlgReconOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_RECONST_FILTER, m_Filter);
	DDX_Text(pDX, IDC_RECONST_CUTOFF, m_Cutoff);
	DDV_MinMaxDouble(pDX, m_Cutoff, 0., 1.);
	DDX_Text(pDX, IDC_RECONST_ORDER, m_Order);
	DDX_CBIndex(pDX, IDC_RECONST_INTP, m_Interpolation);
	DDX_Text(pDX, IDC_RECONST_SUFFIX, m_Suffix);
	DDX_Check(pDX, IDC_RECONST_ANGINTP, m_AngularIntp);
	DDX_Text(pDX, IDC_RECONST_TRIM, m_Trim);
	DDV_MinMaxInt(pDX, m_Trim, 0, 10000);
	DDX_Text(pDX, IDC_RECONST_OUTPATH, m_Outpath);
	DDX_Text(pDX, IDC_RECONST_DRSTART, m_drStart);
	DDX_Text(pDX, IDC_RECONST_DREND, m_drEnd);
	DDX_Text(pDX, IDC_RECONST_DRX, m_drX);
	DDX_Text(pDX, IDC_RECONST_DRY, m_drY);
	DDX_Check(pDX, IDC_RECONST_OMIT, m_drOmit);
	DDX_Check(pDX, IDC_RECONST_ZERNIKE, m_Zernike);
	DDX_Control(pDX, IDC_RECONST_HISDATASET, m_HisDataset);
	DDX_Check(pDX, IDC_RECONST_DRIFTPARAMS, m_bDriftParams);
	DDX_Check(pDX, IDC_RECONST_DRIFTLIST, m_bDriftList);
	DDX_Text(pDX, IDC_RECONST_DRIFTLISTPATH, m_sDriftListPath);
	DDX_Radio(pDX, IDC_RECONST_FRAMEALL, m_FrameUsage);
	DDX_Check(pDX, IDC_RECONST_SKIPINITIALFLATSINHDF5, m_bSkipInitialFlatsInHDF5);
	DDX_Text(pDX, IDC_RECONST_AXISINC, m_dAxisInc);
	DDV_MinMaxDouble(pDX, m_dAxisInc, 0, 10);
}


BEGIN_MESSAGE_MAP(CDlgReconOpt, CDialog)
	ON_CBN_SELCHANGE(IDC_RECONST_FILTER, &CDlgReconOpt::OnCbnSelchangeReconstFilter)
	ON_BN_CLICKED(IDC_RECONST_SETPATH, &CDlgReconOpt::OnBnClickedReconstSetpath)
	ON_BN_CLICKED(IDC_RECONST_OMIT, &CDlgReconOpt::OnBnClickedReconstOmit)
	ON_CBN_SELCHANGE(IDC_RECONST_HISDATASET, &CDlgReconOpt::OnCbnSelchangeReconstHisdataset)
	ON_BN_CLICKED(IDC_RECONST_DRIFTPARAMS, &CDlgReconOpt::OnBnClickedReconstDriftparams)
	ON_BN_CLICKED(IDC_RECONST_DRIFTLIST, &CDlgReconOpt::OnBnClickedReconstDriftlist)
	ON_BN_CLICKED(IDC_RECONST_SETDRIFTLIST, &CDlgReconOpt::OnBnClickedReconstSetdriftlist)
END_MESSAGE_MAP()


// CDlgReconOpt メッセージ ハンドラ

void CDlgReconOpt::OnCbnSelchangeReconstFilter()
{
	UpdateData();
	EnableCtrl();
}

void CDlgReconOpt::EnableCtrl() {
	GetDlgItem(IDC_RECONST_ORDER)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_DRSTART)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_DREND)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_OMIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_DRX)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_DRY)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_DRIFTLISTPATH)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECONST_SETDRIFTLIST)->EnableWindow(FALSE);
	if (m_Filter == CDLGRECONST_FILT_BUTER) {
		GetDlgItem(IDC_RECONST_ORDER)->EnableWindow(TRUE);
	}
	if (m_bDriftParams) {
		GetDlgItem(IDC_RECONST_DRSTART)->EnableWindow(TRUE);
		GetDlgItem(IDC_RECONST_DREND)->EnableWindow(TRUE);
		GetDlgItem(IDC_RECONST_OMIT)->EnableWindow(TRUE);
		if (!m_drOmit) {
			GetDlgItem(IDC_RECONST_DRX)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECONST_DRY)->EnableWindow(TRUE);
		}
	}
	if (m_bDriftList) {
		GetDlgItem(IDC_RECONST_DRIFTLISTPATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_RECONST_SETDRIFTLIST)->EnableWindow(TRUE);
	}
}

int __stdcall FolderBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	//SHBrowseForFolder callback function to initialize default folder
	if (uMsg == BFFM_INITIALIZED) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	return 0;
}

void CDlgReconOpt::OnBnClickedReconstSetpath()
{
	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;
	TCHAR szDisplayName[MAX_PATH];
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	_stprintf_s(path_buffer, _MAX_PATH, m_Outpath);

	bInfo.hwndOwner = AfxGetMainWnd()->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDisplayName;
	bInfo.lpszTitle = _T("Output folder"); 
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = FolderBrowseCallbackProc;
	bInfo.lParam = (LPARAM)path_buffer;

	pIDList = ::SHBrowseForFolder(&bInfo);
	if (pIDList == NULL) return;
	else {
		if (!::SHGetPathFromIDList(pIDList, szDisplayName)) return;
		m_Outpath = szDisplayName;
		if (m_Outpath.Right(1) != "\\") m_Outpath += "\\";
		//AfxMessageBox(m_Outpath);
		UpdateData(FALSE);
		::CoTaskMemFree( pIDList );
	}
	return;
}

void CDlgReconOpt::OnBnClickedReconstOmit()
{
	UpdateData();
	EnableCtrl();
}

BOOL CDlgReconOpt::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_HisDataset.AddString("0");
	if (nDataset > 1) {
		for (int i=1; i<nDataset; i++) {
			CString line; line.Format("%d", i);
			m_HisDataset.AddString(line);
		}
	}
	iDatasetSel = (iDatasetSel < nDataset) ? iDatasetSel : 0;
	m_HisDataset.SetCurSel(iDatasetSel);
	if (nDataset > 1) OnCbnSelchangeReconstHisdataset();

	EnableCtrl();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgReconOpt::OnCbnSelchangeReconstHisdataset()
{
	if (nDataset <= 1) return;
	const int idigit = (int)log10((double)nDataset) + 1;
	CString fmt, fn;
	fmt.Format("%%0%dd", idigit);
	iDatasetSel = m_HisDataset.GetCurSel();
	fn.Format(fmt, iDatasetSel);
	UpdateData();
	m_Suffix = m_Suffix.SpanExcluding("01234567890") + fn;
	UpdateData(FALSE);
}

void CDlgReconOpt::OnBnClickedReconstDriftparams()
{
	UpdateData();
	EnableCtrl();
}

void CDlgReconOpt::OnBnClickedReconstDriftlist()
{
	UpdateData();
	EnableCtrl();
}

void CDlgReconOpt::OnBnClickedReconstSetdriftlist()
{
	static char BASED_CODE szFilter[] = "text files (*.txt)|*.txt|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".txt";
	CFileDialog fileDlg(FALSE, defaultExt, m_sDriftListPath, OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) return;
	m_sDriftListPath = fileDlg.GetPathName();
	UpdateData(FALSE);
}
