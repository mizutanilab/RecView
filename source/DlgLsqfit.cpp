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
	GetDlgItem(IDC_LSQFIT_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	if (bStarted) {
		GetDlgItem(IDC_LSQFIT_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_LSQFIT_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
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
	DDV_MinMaxInt(pDX, m_XHigh, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_YLOW, m_YLow);
	DDV_MinMaxInt(pDX, m_YLow, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_YHIGH, m_YHigh);
	DDV_MinMaxInt(pDX, m_YHigh, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_ZLOW, m_ZLow);
	DDV_MinMaxInt(pDX, m_ZLow, -100, 100);
	DDX_Text(pDX, IDC_LSQFIT_ZHIGH, m_ZHigh);
	DDV_MinMaxInt(pDX, m_ZHigh, -100, 100);
}


BEGIN_MESSAGE_MAP(CDlgLsqfit, CDialog)
	ON_BN_CLICKED(IDC_LSQFIT_REFSET, &CDlgLsqfit::OnBnClickedLsqfitRefset)
	ON_BN_CLICKED(IDC_LSQFIT_START, &CDlgLsqfit::OnBnClickedLsqfitStart)
	ON_BN_CLICKED(IDC_LSQFIT_QRYSET, &CDlgLsqfit::OnBnClickedLsqfitQryset)
	ON_BN_CLICKED(IDC_LSQFIT_STOP, &CDlgLsqfit::OnBnClickedLsqfitStop)
	ON_BN_CLICKED(IDC_LSQFIT_QUEUE, &CDlgLsqfit::OnBnClickedLsqfitQueue)
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
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	//TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
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
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
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
	pApp->Lsqfit(&lq, this, NULL);
	/*120828
	short** ppRefPixel = new short*[nRefFiles];
	int* pMaxRefPixel = new int[nRefFiles];
	for (int i=0; i<nRefFiles; i++) {
		ppRefPixel[i] = NULL;
		pMaxRefPixel[i] = 0;
	}
	short** ppQryPixel = new short*[nQryFiles];
	int* pMaxQryPixel = new int[nQryFiles];
	for (int i=0; i<nQryFiles; i++) {
		ppQryPixel[i] = NULL;
		pMaxQryPixel[i] = 0;
	}
	const int nLsqList = (m_XHigh - m_XLow + 1) * (m_YHigh - m_YLow + 1) * (m_ZHigh - m_ZLow + 1);
	CString* sLsqList = new CString[nLsqList];
	for (int i=0; i<nLsqList; i++) {sLsqList[i].Empty();}

	CFile fp;
	int* ibuf = NULL;
	//Reading reference image set
	CString str = m_RefList;
	int iPos = 0;
	int ixref = -1, iyref = -1;
	for (int i=0; i<nRefFiles; i++) {
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {m_Result += "Not found: " + fn; break;}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			m_Result += "Unknown format: " + fn;
			fp.Close();
			break;
		}
		if (ixref < 0) {ixref = ixdim; iyref = iydim;}
		else if ((ixdim != ixref)||(iydim != iyref)) {
			m_Result += "Image size not matched: " + fn;
			fp.Close();
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		m_Result += " Ref: " + fn + "\r\n";
		fp.Close();
		ppRefPixel[i] = new short[nbuf];
		pMaxRefPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppRefPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppRefPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppRefPixel[i])[j] = SHRT_MAX;
			else (ppRefPixel[i])[j] = (short)(absCoeff);
			//(ppRefPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		UpdateData(FALSE);
	}
	//Reading query image set
	str = m_QryList;
	iPos = 0;
	int ixqry = -1, iyqry = -1;
	for (int i=0; i<nQryFiles; i++) {
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {m_Result += "Not found: " + fn; break;}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			m_Result += "Unknown format: " + fn;
			fp.Close();
			break;
		}
		if (ixqry < 0) {ixqry = ixdim; iyqry = iydim;}
		else if ((ixdim != ixqry)||(iydim != iyqry)) {
			m_Result += "Image size not matched: " + fn;
			fp.Close();
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		m_Result += " Qry: " + fn + "\r\n";
		fp.Close();
		ppQryPixel[i] = new short[nbuf];
		pMaxQryPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppQryPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppQryPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppQryPixel[i])[j] = SHRT_MAX;
			else (ppQryPixel[i])[j] = (short)(absCoeff);
			//(ppQryPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		UpdateData(FALSE);
	}
	bStarted = true;
	EnableCtrl();
	int iLsqList = 0;
	RECONST_INFO ri[MAX_CPU];
	struct _timeb tstruct; double tm0;
	_ftime_s( &tstruct );
	tm0 = tstruct.time + tstruct.millitm * 0.001;
	//output logs
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	_stprintf_s(path_buffer, _MAX_PATH, m_QryList.SpanExcluding(_T("\r\n")));
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "0recviewlog", ".txt");
	//AfxMessageBox(path_buffer); return;//////////////
	CStdioFile flog;
	if (flog.Open(path_buffer, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::typeText)) {
		flog.SeekToEnd();
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line;
		line.Format("LSQ fit [%s]\r\n", stime.Left(24));
		flog.WriteString(line);
		flog.WriteString(m_Result);
	}
	//lsq fitting
	int nCPU = 1;
	if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		nCPU = (int)(pApp->dlgProperty.iCUDA);
		short** d_ppRefPixel = new short*[nRefFiles];
		short** d_ppQryPixel = new short*[nQryFiles];
		unsigned __int64* h_result = new unsigned __int64[ixref * 2];
		unsigned __int64* d_result = NULL;
		for (int i=0; i<nRefFiles; i++) {d_ppRefPixel[i] = NULL;}
		for (int i=0; i<nQryFiles; i++) {d_ppQryPixel[i] = NULL;}
		CudaLsqfitMemAlloc(d_ppRefPixel, d_ppQryPixel, pMaxRefPixel, pMaxQryPixel, 
						ppRefPixel, ppQryPixel, nRefFiles, nQryFiles, ixref, &d_result);
		for (int ix=m_XLow; ix<=m_XHigh; ix++) {
			for (int iy=m_YLow; iy<=m_YHigh; iy++) {
				for (int iz=m_ZLow; iz<=m_ZHigh; iz++) {
					int nlsq = 0;
					__int64 ilsq = 0;
					for (int jrz=0; jrz<nRefFiles; jrz++) {
						const int jqz = jrz + iz;
						if ((jqz < 0)||(jqz >= nQryFiles)) continue;
						short* d_ref = d_ppRefPixel[jrz];
						short* d_qry = d_ppQryPixel[jqz];
						CudaLsqfitHost(d_ref, d_qry, ixref, iyref, ixqry, iyqry,
										ix, iy, &ilsq, &nlsq, d_result, h_result);
					}
					if (nlsq) {
						TReal rlsq = sqrt(ilsq / (TReal)nlsq);
						TReal dilsq = (double)ilsq;
						//120624 CString msg; msg.Format("%d %d %d %f %.0f/%d\r\n", ix, iy, iz, rlsq, dilsq, nlsq);
						//120624 m_Result += msg;
						CString msg; msg.Format("%f (%d %d %d) %.0f/%d\r\n", rlsq, ix, iy, iz, dilsq, nlsq);
						m_Result = "RMSD (dx dy dz) SumDiff/NSum\r\n" + msg;
						sLsqList[iLsqList++] = msg;
					}
					ProcessMessage();
					if (bStarted == false) break;
				}
				UpdateData(FALSE);
				if (bStarted == false) break;
			}
			if (bStarted == false) break;
		}
		CudaLsqfitMemFree(d_ppRefPixel, d_ppQryPixel, nRefFiles, nQryFiles, d_result);
		if (d_ppRefPixel) delete [] d_ppRefPixel;
		if (d_ppQryPixel) delete [] d_ppQryPixel;
		if (h_result) delete [] h_result;
	} else {//pApp->dlgProperty.m_ProcessorType
		nCPU = (int)(pApp->dlgProperty.iCPU);
		for (int ix=m_XLow; ix<=m_XHigh; ix++) {
			for (int iy=m_YLow; iy<=m_YHigh; iy++) {
				for (int iz=m_ZLow; iz<=m_ZHigh; iz+=nCPU) {
					for (int i=nCPU-1; i>=0; i--) {
						ri[i].hThread = NULL;
						ri[i].iStartSino = i;
						if (i) ri[i].bMaster = false; else ri[i].bMaster = true;
						ri[i].iStatus = RECONST_INFO_IDLE;
						ri[i].i64result = 0;//double
						ri[i].drStart = 0;//int
						if (iz + i > m_ZHigh) continue;
						ri[i].iStatus = RECONST_INFO_BUSY;
						ri[i].max_d_ifp = nRefFiles;
						ri[i].max_d_igp = nQryFiles;
						ri[i].ixdim = ix;
						ri[i].iInterpolation = iy;
						ri[i].iLenSinogr = ixref;
						ri[i].iMultiplex = iyref;
						ri[i].iOffset = ixqry;
						ri[i].maxSinogrLen = iyqry;
						ri[i].drEnd = iz + i;
						ri[i].ppRef = ppRefPixel;
						ri[i].ppQry = ppQryPixel;
						void* pArg = (void*)(&(ri[i]));
						if (i) {
							ri[i].hThread = (unsigned int)_beginthreadex( NULL, 0, LsqfitThread, pArg, 0, &(ri[i].threadID) );
						} else {
							LsqfitThread(&(ri[i]));
						}
					}
					int ist = RECONST_INFO_IDLE;
					do {
						ist = RECONST_INFO_IDLE;
						for (int i=nCPU-1; i>=0; i--) ist |= ri[i].iStatus;
					} while (ist != RECONST_INFO_IDLE);
					for (int i=nCPU-1; i>=0; i--) {if (ri[i].hThread) CloseHandle((HANDLE)(ri[i].hThread));}//120723
					for (int i=0; i<nCPU; i++) {
						if (iz + i > m_ZHigh) continue;
						__int64 ilsq = ri[i].i64result;//__int64
						int nlsq = ri[i].drStart;//int
						if (nlsq) {
							TReal rlsq = sqrt(ilsq / (TReal)nlsq);
							TReal dilsq = (double)ilsq;
							//120624 CString msg; msg.Format("%d %d %d %f %.0f/%d\r\n", ix, iy, iz+i, rlsq, dilsq, nlsq);
							//120624 m_Result += msg;
							CString msg; msg.Format("%f (%d %d %d) %.0f/%d\r\n", rlsq, ix, iy, iz+i, dilsq, nlsq);
							m_Result = "RMSD (dx dy dz) SumDiff/NSum\r\n" + msg;
							sLsqList[iLsqList++] = msg;
						}
					}
				/*for (int iz=m_ZLow; iz<=m_ZHigh; iz++) {
					__int64 ilsq = 0;
					int nlsq = 0;
					for (int jrz=0; jrz<nRefFiles; jrz++) {
						const int jqz = jrz + iz;
						if ((jqz < 0)||(jqz >= nQryFiles)) continue;
						int* pRef = ppRefPixel[jrz];
						int* pQry = ppQryPixel[jqz];
						for (int jry=0; jry<iyref; jry++) {
							const int jqy = jry + iy;
							if ((jqy < 0)||(jqy >= iyqry)) continue;
							int idx0r = jry * ixref;
							int idx0q = jqy * ixqry;
							for (int jrx=0; jrx<ixref; jrx++) {
								const int jqx = jrx + ix;
								if ((jqx < 0)||(jqx >= ixqry)) continue;
								__int64 idiff = pRef[idx0r + jrx] - pQry[idx0q + jqx];
								ilsq += idiff * idiff;
								nlsq++;
							}
						}
					}
					if (nlsq) {
						TReal rlsq = sqrt(ilsq / (TReal)nlsq);
						TReal dilsq = (double)ilsq;
						CString msg; msg.Format("(%d %d %d) %f %f/%d\r\n", ix, iy, iz, rlsq, dilsq, nlsq);
						m_Result += msg;
						if ((minlsq < 0)||(rlsq < minlsq)) {
							minlsq = rlsq;
							mx = ix; my = iy; mz = iz;
						}
					}///*
					ProcessMessage();
					if (bStarted == false) break;
				}
				UpdateData(FALSE);
				if (bStarted == false) break;
			}
			if (bStarted == false) break;
		}
	}
	//sort
	CString** pLsqList = new CString*[iLsqList];
	for (int i=0; i<iLsqList; i++) {pLsqList[i] = &(sLsqList[i]);}
	qsort( (void *)pLsqList, (size_t)iLsqList, sizeof(CString*), LsqFitCompare );
	m_Result.Empty();
	for (int i=0; i<iLsqList; i++) {m_Result += " " + *(pLsqList[i]);}
	_ftime_s( &tstruct );
	TReal tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	if (bStarted) {
		CString msg;
		TReal minlsq = 0; int mx = 0, my = 0, mz = 0;
		sscanf_s(*(pLsqList[0]), "%lf (%d %d %d)", &minlsq, &mx, &my, &mz);
		msg.Format(" Min: ref(0 0 0)=qry(%d %d %d) rmsd=%f\r\n RMSD (dx dy dz) SumDiff/NSum\r\n", mx, my, mz, minlsq);
		m_Result = msg + m_Result;
		if (flog.m_hFile != CFile::hFileNull) {
			flog.WriteString(msg);
			for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]));}
		}
		msg.Format("CPU=%fsec\r\n", tcpu);
		m_Result += msg;
	}
	if (flog.m_hFile != CFile::hFileNull) {
		flog.WriteString("---------------------------------------------------\r\n");
		flog.Close();
	}
	bStarted = false;
	//delete images
	for (int i=0; i<nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
	if (ppRefPixel) delete [] ppRefPixel;
	if (pMaxRefPixel) delete [] pMaxRefPixel;
	for (int i=0; i<nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
	if (ppQryPixel) delete [] ppQryPixel;
	if (pMaxQryPixel) delete [] pMaxQryPixel;
	if (sLsqList) delete [] sLsqList;
	if (pLsqList) delete [] pLsqList;
	120828*/
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
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->dlgQueue.AddLsqfitQueue(&lq);
	//
	CDialog::OnOK();
}
