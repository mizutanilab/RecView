// DlgLsqfit.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgLsqfit.h"
#include "DlgQueue.h"
#include "cudaReconst.h"
#include <sys\timeb.h> //_timeb, _ftime
#include <process.h> //_beginthread

// CDlgLsqfit ダイアログ

IMPLEMENT_DYNAMIC(CDlgLsqfit, CDialog)

CDlgLsqfit::CDlgLsqfit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLsqfit::IDD, pParent)
	, m_RefList(_T(""))
	, m_RefMsg(_T(""))
	, m_Result(_T(""))
	, m_QryList(_T(""))
	, m_QryMsg(_T(""))
	, m_XLow(-10)
	, m_XHigh(10)
	, m_YLow(-10)
	, m_YHigh(10)
	, m_ZLow(-5)
	, m_ZHigh(5)
	, m_bMaxDiameter(TRUE)
{
	nRefFiles = 0;
	nQryFiles = 0;
	bStarted = false;
	//
	//refFilePath.Empty();
	//refFileList = new TCHAR[MAX_FILE_DIALOG_LIST];
	//_tcscpy_s(refFileList, MAX_FILE_DIALOG_LIST, "");
	UpdateNfiles();
}

CDlgLsqfit::~CDlgLsqfit()
{
	//if (refFileList) delete [] refFileList;
}

void CDlgLsqfit::UpdateNfiles() {
	m_RefMsg.Format("Reference image set: %d", nRefFiles);
	m_QryMsg.Format("Query image set: %d", nQryFiles);
}

void CDlgLsqfit::EnableCtrl() {
	GetDlgItem(IDC_LSQFIT_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_QUEUE)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_XLOW)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_YLOW)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_YHIGH)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_ZLOW)->EnableWindow(TRUE);
	GetDlgItem(IDC_LSQFIT_ZHIGH)->EnableWindow(TRUE);
	if (bStarted) {
		GetDlgItem(IDC_LSQFIT_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_QUEUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	if (m_bMaxDiameter) {
		GetDlgItem(IDC_LSQFIT_XLOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_YLOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_YHIGH)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_ZLOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_ZHIGH)->EnableWindow(FALSE);
	}
}

void CDlgLsqfit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LSQFIT_REFLIST, m_RefList);
	DDX_Text(pDX, IDC_LSQFIT_REFMSG, m_RefMsg);
	DDX_Text(pDX, IDC_LSQFIT_RESULT, m_Result);
	DDX_Text(pDX, IDC_LSQFIT_QRYLIST, m_QryList);
	DDX_Text(pDX, IDC_LSQFIT_QRYMSG, m_QryMsg);
	DDX_Text(pDX, IDC_LSQFIT_XLOW, m_XLow);
	DDV_MinMaxInt(pDX, m_XLow, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_XHIGH, m_XHigh);
	DDV_MinMaxInt(pDX, m_XHigh, -100, 10000);
	DDX_Text(pDX, IDC_LSQFIT_YLOW, m_YLow);
	DDV_MinMaxInt(pDX, m_YLow, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_YHIGH, m_YHigh);
	DDV_MinMaxInt(pDX, m_YHigh, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_ZLOW, m_ZLow);
	DDV_MinMaxInt(pDX, m_ZLow, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_ZHIGH, m_ZHigh);
	DDV_MinMaxInt(pDX, m_ZHigh, -100, 100);
	DDX_Check(pDX, IDC_LSQFIT_MAXDIA, m_bMaxDiameter);
}


BEGIN_MESSAGE_MAP(CDlgLsqfit, CDialog)
	ON_BN_CLICKED(IDC_LSQFIT_REFSET, &CDlgLsqfit::OnBnClickedLsqfitRefset)
	ON_BN_CLICKED(IDC_LSQFIT_START, &CDlgLsqfit::OnBnClickedLsqfitStart)
	ON_BN_CLICKED(IDC_LSQFIT_QRYSET, &CDlgLsqfit::OnBnClickedLsqfitQryset)
	ON_BN_CLICKED(IDC_LSQFIT_STOP, &CDlgLsqfit::OnBnClickedLsqfitStop)
	ON_BN_CLICKED(IDC_LSQFIT_QUEUE, &CDlgLsqfit::OnBnClickedLsqfitQueue)
	ON_BN_CLICKED(IDC_LSQFIT_MAXDIA, &CDlgLsqfit::OnBnClickedLsqfitMaxdia)
END_MESSAGE_MAP()


// CDlgLsqfit メッセージ ハンドラ

void CDlgLsqfit::OnBnClickedLsqfitRefset()
{
	UpdateData();
	TCHAR* fileList = new TCHAR[MAX_FILE_DIALOG_LIST];
	if (!fileList) return;
	CString filePath = "";//////////////
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, filePath + "rec.tif");
	static char BASED_CODE szFilter[] = "TIFF files (*.tif)|*.tif|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".tif";
	CFileDialog fileDlg(TRUE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
	fileDlg.m_ofn.nMaxFile = MAX_FILE_DIALOG_LIST;
	fileDlg.m_ofn.lpstrFile = (LPTSTR)fileList;
	if (fileDlg.DoModal() == IDCANCEL) return;
	POSITION pos = fileDlg.GetStartPosition();
	nRefFiles = 0;
	m_RefList.Empty();
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	//TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
		//181217
		if (!m_QryList.IsEmpty() && (nRefFiles == 0)) {
			TCHAR rdrive[_MAX_DRIVE]; TCHAR rdir[_MAX_DIR];
			_tsplitpath_s( path_buffer, rdrive, _MAX_DRIVE, rdir, _MAX_DIR, NULL, 0, NULL, 0 );
			TCHAR qpath_buffer[_MAX_PATH]; TCHAR qdrive[_MAX_DRIVE]; TCHAR qdir[_MAX_DIR];
			int iqpos = 0;
			_stprintf_s(qpath_buffer, _MAX_PATH, m_QryList.Tokenize(_T("\r\n"), iqpos));
			_tsplitpath_s( qpath_buffer, qdrive, _MAX_DRIVE, qdir, _MAX_DIR, NULL, 0, NULL, 0 );
			if ((_tcscmp(rdrive, qdrive) == 0)&&(_tcscmp(rdir, qdir) == 0)) {
				m_Result += "*****The folders of Ref and Qry sets are the same*****\r\n";
				UpdateData(FALSE);
			}
		}
		//
		m_RefList += path_buffer;
		m_RefList += "\r\n";
		nRefFiles++;
		//if (nRefFiles == 1) _tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	}
	//_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	//m_FileMsg.Format("%d files (%s)", nFiles, path_buffer);
	//EnableCtrl();
	if (fileList) delete [] fileList;
	UpdateNfiles();
	UpdateData(FALSE);
}

void CDlgLsqfit::OnBnClickedLsqfitQryset()
{
	UpdateData();
	TCHAR* fileList = new TCHAR[MAX_FILE_DIALOG_LIST];
	if (!fileList) return;
	CString filePath = "";//////////////
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, filePath + "rec.tif");
	static char BASED_CODE szFilter[] = "TIFF files (*.tif)|*.tif|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".tif";
	CFileDialog fileDlg(TRUE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
	fileDlg.m_ofn.nMaxFile = MAX_FILE_DIALOG_LIST;
	fileDlg.m_ofn.lpstrFile = (LPTSTR)fileList;
	if (fileDlg.DoModal() == IDCANCEL) return;
	POSITION pos = fileDlg.GetStartPosition();
	nQryFiles = 0;
	m_QryList.Empty();
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
		//181217
		if (!m_RefList.IsEmpty() && (nQryFiles == 0)) {
			TCHAR qdrive[_MAX_DRIVE]; TCHAR qdir[_MAX_DIR];
			_tsplitpath_s( path_buffer, qdrive, _MAX_DRIVE, qdir, _MAX_DIR, NULL, 0, NULL, 0 );
			TCHAR rpath_buffer[_MAX_PATH]; TCHAR rdrive[_MAX_DRIVE]; TCHAR rdir[_MAX_DIR];
			int irpos = 0;
			_stprintf_s(rpath_buffer, _MAX_PATH, m_RefList.Tokenize(_T("\r\n"), irpos));
			_tsplitpath_s( rpath_buffer, rdrive, _MAX_DRIVE, rdir, _MAX_DIR, NULL, 0, NULL, 0 );
			//CString msg;
			//msg.Format("%s %s %s\r\n%s %s %s", path_buffer, qdrive, qdir, rpath_buffer, rdrive, rdir);
			//AfxMessageBox(msg);
			if ((_tcscmp(rdrive, qdrive) == 0)&&(_tcscmp(rdir, qdir) == 0)) {
				m_Result += "*****The folders of Ref and Qry sets are the same*****\r\n";
				UpdateData(FALSE);
			}
		}
		//
		m_QryList += path_buffer;
		m_QryList += "\r\n";
		nQryFiles++;
	}
	if (fileList) delete [] fileList;
	UpdateNfiles();
	UpdateData(FALSE);
}

int LsqFitCompare( const void *arg1, const void *arg2 ) {
	CString str1 = ** ( CString** ) arg1;
	TReal r1 = atof(str1.SpanExcluding(" "));
	CString str2 = ** ( CString** ) arg2;
	TReal r2 = atof(str2.SpanExcluding(" "));
	if (r1 > r2) return 1;
	else if (r1 < r2) return -1;
	else return 0;
	//if ( ** ( CString** ) arg1 > ** ( CString** ) arg2 ) return 1;
	//else if ( ** ( CString** ) arg1 < ** ( CString** ) arg2 ) return -1;
	//else return 0;
}

void CDlgLsqfit::OnBnClickedLsqfitStart()
{
	UpdateData();
	m_Result.Empty();
	LSQFIT_QUEUE lq;
	lq.nRefFiles = this->nRefFiles;
	lq.nQryFiles = this->nQryFiles;
	lq.m_XLow = this->m_XLow;
	lq.m_XHigh = this->m_XHigh;
	lq.m_YLow = this->m_YLow;
	lq.m_YHigh = this->m_YHigh;
	lq.m_ZLow = this->m_ZLow;
	lq.m_ZHigh = this->m_ZHigh;
	lq.m_RefList = this->m_RefList;
	lq.m_QryList = this->m_QryList;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	if (m_bMaxDiameter) pApp->LsqfitMin(&lq, this, NULL);
	else pApp->Lsqfit(&lq, this, NULL);
	EnableCtrl();
	UpdateData(FALSE);
}


void CDlgLsqfit::OnBnClickedLsqfitStop()
{
	bStarted = false;
}

void CDlgLsqfit::OnBnClickedLsqfitQueue()
{
	UpdateData();
	if ((nRefFiles == 0)||(nQryFiles == 0)) return;
	//m_Result.Empty();
	LSQFIT_QUEUE lq;
	lq.nRefFiles = this->nRefFiles;
	lq.nQryFiles = this->nQryFiles;
	lq.m_XLow = this->m_XLow;
	lq.m_XHigh = this->m_XHigh;
	lq.m_YLow = this->m_YLow;
	lq.m_YHigh = this->m_YHigh;
	lq.m_ZLow = this->m_ZLow;
	lq.m_ZHigh = this->m_ZHigh;
	lq.m_RefList = this->m_RefList;
	lq.m_QryList = this->m_QryList;
	lq.m_bMaxDiameter =  this->m_bMaxDiameter;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->dlgQueue.AddLsqfitQueue(&lq);
	//
	CDialog::OnOK();
}

void CDlgLsqfit::OnBnClickedLsqfitMaxdia()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData();
	EnableCtrl();
}

BOOL CDlgLsqfit::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ここに初期化を追加してください
	if (m_bMaxDiameter) m_XHigh = 2000;
	UpdateData(FALSE);
	EnableCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
