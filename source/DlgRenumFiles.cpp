// DlgRenumFiles.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "cxyz.h"
#include "DlgRenumFiles.h"
#include "MainFrm.h"
#include <sys\timeb.h> //_timeb, _ftime
#include <float.h> //FLT_MAX


// CDlgRenumFiles ダイアログ

IMPLEMENT_DYNAMIC(CDlgRenumFiles, CDialog)

CDlgRenumFiles::CDlgRenumFiles(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRenumFiles::IDD, pParent)
	, m_FileList(_T(""))
	, m_FileListMsg(_T(""))
	, m_Prefix(_T("p"))
	, m_StartIndex(1)
	, m_OutPath(_T(""))
	, m_ResliceOrgZ(0)
	, m_ResliceOrgY(0)
	, m_ResliceOrgX(0)
	, m_ResliceEndX(0)
	, m_ResliceEndY(0)
	, m_ResliceEndZ(0)
{
	nFiles = 0;
	m_FileListMsg.Format("Number of image: %d", nFiles);
}

CDlgRenumFiles::~CDlgRenumFiles()
{
}

void CDlgRenumFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RENUM_FILELIST, m_FileList);
	DDX_Text(pDX, IDC_RENUM_MSG, m_FileListMsg);
	DDX_Text(pDX, IDC_RENUM_PREFIX, m_Prefix);
	DDX_Text(pDX, IDC_RENUM_STARTINDEX, m_StartIndex);
	DDV_MinMaxInt(pDX, m_StartIndex, 0, 1000000);
	DDX_Text(pDX, IDC_RENUM_OUTPATH, m_OutPath);
	DDX_Text(pDX, IDC_RENUM_ROTZ, m_ResliceOrgZ);
	//DDV_MinMaxDouble(pDX, m_ResliceRotZ, -360, 360);
	DDX_Text(pDX, IDC_RENUM_ROTY, m_ResliceOrgY);
	//DDV_MinMaxDouble(pDX, m_ResliceRotY, -360, 360);
	DDX_Text(pDX, IDC_RENUM_ROTX, m_ResliceOrgX);
	//DDV_MinMaxDouble(pDX, m_ResliceRotX, -360, 360);
	DDX_Text(pDX, IDC_RENUM_ENDX, m_ResliceEndX);
	DDX_Text(pDX, IDC_RENUM_ENDY, m_ResliceEndY);
	DDX_Text(pDX, IDC_RENUM_ENDZ, m_ResliceEndZ);
}


BEGIN_MESSAGE_MAP(CDlgRenumFiles, CDialog)
	ON_BN_CLICKED(IDC_RENUM_SETFILE, &CDlgRenumFiles::OnBnClickedRenumSetfile)
	ON_BN_CLICKED(IDC_RENUM_SETPATH, &CDlgRenumFiles::OnBnClickedRenumSetpath)
END_MESSAGE_MAP()


// CDlgRenumFiles メッセージ ハンドラ

void CDlgRenumFiles::OnBnClickedRenumSetfile()
{
	UpdateData();
	TCHAR* fileList = new TCHAR[MAX_FILE_DIALOG_LIST];
	if (!fileList) return;
	CString filePath = "";//////////////
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, filePath + "ro.tif");
	static char BASED_CODE szFilter[] = "ro files (ro*.tif)|ro*.tif|TIFF files (*.tif)|*.tif|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".tif";
	CFileDialog fileDlg(TRUE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
	fileDlg.m_ofn.nMaxFile = MAX_FILE_DIALOG_LIST;
	fileDlg.m_ofn.lpstrFile = (LPTSTR)fileList;
	if (fileDlg.DoModal() == IDCANCEL) return;
	POSITION pos = fileDlg.GetStartPosition();
	//nFiles = 0;
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	//TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
		m_FileList += path_buffer;
		m_FileList += "\r\n";
		nFiles++;
		//if (nRefFiles == 1) _tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	}
	if (m_OutPath.IsEmpty()) {
		TCHAR folder[_MAX_PATH];//_MAX_PATH = 260, typically
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
		_stprintf_s(folder, _MAX_PATH, path_buffer);
		_tsplitpath_s(folder, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "", "");
		//AfxMessageBox(path_buffer);//130216
		m_OutPath = path_buffer;
	}

	//_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	//m_FileMsg.Format("%d files (%s)", nFiles, path_buffer);
	//EnableCtrl();
	if (fileList) delete [] fileList;
	m_FileListMsg.Format("Number of image: %d", nFiles);
	UpdateData(FALSE);
}

void CDlgRenumFiles::OnOK()
{
	UpdateData();
	if (m_OutPath.IsEmpty()) {AfxMessageBox("Set output path."); return;}
	if (nFiles == 0) {AfxMessageBox("No source file."); return;}
	//iDatasetSel = m_HisDataset.GetCurSel();
	//fn.Format(fmt, iDatasetSel);
	//UpdateData();
	//m_Suffix = m_Suffix.SpanExcluding("01234567890") + fn;

	//output logs
	struct _timeb tstruct;
	_ftime_s( &tstruct );
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	_stprintf_s(path_buffer, _MAX_PATH, m_FileList.SpanExcluding(_T("\r\n")));
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "recviewlog", ".txt");
	CStdioFile flog;
	if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "0recviewlog", ".txt");
		if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "_recviewlog", ".txt");
		} else {
			flog.Close();
		}
	} else {
		flog.Close();
	}
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	if (flog.Open(path_buffer, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		flog.SeekToEnd();
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString sVer = pApp->sProgVersion; sVer.Replace('\n', ' ');
		CString line;
		line.Format("Renumbering [%s] %s\n", stime.Left(24), sVer);
		flog.WriteString(line);
		line.Format(" Output path: %s\n", m_OutPath);
		flog.WriteString(line);
		line.Format(" Output prefix: %s\n", m_Prefix);
		flog.WriteString(line);
		line.Format(" Start index: %d\n", m_StartIndex);
		flog.WriteString(line);
		line.Format(" Files processed: %s ...\n", m_FileList.SpanExcluding(_T("\r\n")));
		flog.WriteString(line);
		line.Format(" Number of files: %d\n", nFiles);
		flog.WriteString(line);
		if ((m_ResliceEndZ-m_ResliceOrgZ != 0)||(m_ResliceEndY-m_ResliceOrgY != 0)||(m_ResliceEndX-m_ResliceOrgX != 0)) {
			line.Format(" Reslicing vector origin: (%d %d %d)\n", m_ResliceOrgX, m_ResliceOrgY, m_ResliceOrgZ);
			flog.WriteString(line);
			line.Format(" Reslicing vector end: (%d %d %d)\n", m_ResliceEndX, m_ResliceEndY, m_ResliceEndZ);
			flog.WriteString(line);
		}
		flog.WriteString("---------------------------------------------------\n");
		flog.Close();
	}//120624

	::CoInitialize(NULL);//initilaize COM
	IProgressDialog *pDlg = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER,
										IID_IProgressDialog, (void**)&pDlg);

	CMainFrame* pf = NULL;
	pf = (CMainFrame*) AfxGetMainWnd();
	if ((m_ResliceEndZ-m_ResliceOrgZ != 0)||(m_ResliceEndY-m_ResliceOrgY != 0)||(m_ResliceEndX-m_ResliceOrgX != 0)) {
		//constant pixDiv and pixBase are assumed. Entirely revised: 180505
		if (AfxMessageBox("Resliced images will be saved in 8bit format.\r\n Proceed?", MB_OKCANCEL) == IDCANCEL) {
			CDialog::OnOK();
			return;
		}
		CString str = m_FileList;
		int iPos = 0;
		CString fn = str.Tokenize(_T("\r\n"), iPos);
		//XY size
		TCHAR path_buffer[_MAX_PATH];
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		_stprintf_s(path_buffer, _MAX_PATH, fn);
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
		CString sfnm = fnm;
		sfnm.TrimRight("0123456789.");
		if (sfnm == m_Prefix) {
			CString msg; msg.Format("Duplicated file name: %s ==> %s", sfnm, m_Prefix); AfxMessageBox(msg);
			return;
		}
		TErr err = 21001;
		CFile fp;
		int ixref = 0, iyref = 0;
		int nbuf = 0;
		int* ibuf = NULL;
		float pixDiv = 1, pixBase = 0, fCenter = 0, pw = 1;
		int iFilter = 0, nSino = 0;
		if ((_tcscmp(ext, ".tif") == 0)||(_tcscmp(ext, ".TIF") == 0)) {
			if (fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
				err = ReadTif(&fp, &ibuf, &nbuf, &iyref, &ixref, &pixDiv, &pixBase, &fCenter, &iFilter, &pw, &nSino);
				if (pixDiv == 0) pixDiv = 1;
//				err = ReadTif(&fp, NULL, &nbuf, &iyref, &ixref, NULL, NULL, NULL, NULL, NULL);
				fp.Close();
			}
		}
		if (err) {
			CString msg; msg.Format("File read error %d", err); AfxMessageBox(msg);
			CDialog::OnOK();
			return;
		}
		//resliced min-max
		const int ixcent = ixref / 2;
		const int iycent = iyref / 2;
		const int izcent = nFiles / 2;
		int ixmin = INT_MAX, iymin = INT_MAX, izmin = INT_MAX;
		int ixmax = INT_MIN, iymax = INT_MIN, izmax = INT_MIN;
		TReal b[9];
		//(x')   (b[0] b[3] b[6])   (x)        (x)   (b[0] b[1] b[2])   (x')
		//(y') = (b[1] b[4] b[7]) * (y)   ,    (y) = (b[3] b[4] b[5]) * (y')
		//(z')   (b[2] b[5] b[8])   (z)        (z)   (b[6] b[7] b[8])   (z')
		CXyz e1(1,0,0), e2(0,1,0), e3(0,0,1);
		CXyz e3p = CXyz(m_ResliceEndX-m_ResliceOrgX, m_ResliceEndY-m_ResliceOrgY, m_ResliceEndZ-m_ResliceOrgZ);
		e3p.UnitLength();
		CXyz e1p = e2 * e3p;
		CXyz e2p = e3p * e1p;
		if (e3p.Length2() < 1E-6) {
			e2p = e3p * e1;
			e1p = e2p * e3p;
		}
		b[0] = e1p.X(e1); b[3] = e1p.X(e2); b[6] = e1p.X(e3);
		b[1] = e2p.X(e1); b[4] = e2p.X(e2); b[7] = e2p.X(e3);
		b[2] = e3p.X(e1); b[5] = e3p.X(e2); b[8] = e3p.X(e3);

		TReal x1 = b[0] * (-ixcent) + b[3] * (-iycent) + b[6] * (-izcent);
		TReal y1 = b[1] * (-ixcent) + b[4] * (-iycent) + b[7] * (-izcent);
		TReal z1 = b[2] * (-ixcent) + b[5] * (-iycent) + b[8] * (-izcent);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//CString line, msg = "";
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent, -iycent, -izcent, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent + ixref-1) + b[3] * (-iycent) + b[6] * (-izcent);
		y1 = b[1] * (-ixcent + ixref-1) + b[4] * (-iycent) + b[7] * (-izcent);
		z1 = b[2] * (-ixcent + ixref-1) + b[5] * (-iycent) + b[8] * (-izcent);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent + ixref-1, -iycent, -izcent, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent) + b[3] * (-iycent + iyref-1) + b[6] * (-izcent);
		y1 = b[1] * (-ixcent) + b[4] * (-iycent + iyref-1) + b[7] * (-izcent);
		z1 = b[2] * (-ixcent) + b[5] * (-iycent + iyref-1) + b[8] * (-izcent);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent, -iycent + iyref-1, -izcent, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent + ixref-1) + b[3] * (-iycent + iyref-1) + b[6] * (-izcent);
		y1 = b[1] * (-ixcent + ixref-1) + b[4] * (-iycent + iyref-1) + b[7] * (-izcent);
		z1 = b[2] * (-ixcent + ixref-1) + b[5] * (-iycent + iyref-1) + b[8] * (-izcent);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent + ixref-1, -iycent + iyref-1, -izcent, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent) + b[3] * (-iycent) + b[6] * (-izcent + nFiles-1);
		y1 = b[1] * (-ixcent) + b[4] * (-iycent) + b[7] * (-izcent + nFiles-1);
		z1 = b[2] * (-ixcent) + b[5] * (-iycent) + b[8] * (-izcent + nFiles-1);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent, -iycent, -izcent + nFiles-1, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent + ixref-1) + b[3] * (-iycent) + b[6] * (-izcent + nFiles-1);
		y1 = b[1] * (-ixcent + ixref-1) + b[4] * (-iycent) + b[7] * (-izcent + nFiles-1);
		z1 = b[2] * (-ixcent + ixref-1) + b[5] * (-iycent) + b[8] * (-izcent + nFiles-1);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent + ixref-1, -iycent, -izcent + nFiles-1, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent) + b[3] * (-iycent + iyref-1) + b[6] * (-izcent + nFiles-1);
		y1 = b[1] * (-ixcent) + b[4] * (-iycent + iyref-1) + b[7] * (-izcent + nFiles-1);
		z1 = b[2] * (-ixcent) + b[5] * (-iycent + iyref-1) + b[8] * (-izcent + nFiles-1);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//line.Format("%d %d %d ==> %f %f %f\r\n", -ixcent, -iycent + iyref-1, -izcent + nFiles-1, x1, y1, z1); msg += line;
		x1 = b[0] * (-ixcent + ixref-1) + b[3] * (-iycent + iyref-1) + b[6] * (-izcent + nFiles-1);
		y1 = b[1] * (-ixcent + ixref-1) + b[4] * (-iycent + iyref-1) + b[7] * (-izcent + nFiles-1);
		z1 = b[2] * (-ixcent + ixref-1) + b[5] * (-iycent + iyref-1) + b[8] * (-izcent + nFiles-1);
		ixmin = (x1 < ixmin) ? (int)x1 : ixmin; ixmax = (x1 > ixmax) ? (int)x1 : ixmax;
		iymin = (y1 < iymin) ? (int)y1 : iymin; iymax = (y1 > iymax) ? (int)y1 : iymax;
		izmin = (z1 < izmin) ? (int)z1 : izmin; izmax = (z1 > izmax) ? (int)z1 : izmax;
		//CString line;
		//line.Format("%d-%d %d-%d %d-&d", ixmin, ixmax, iymin, iymax, izmin, izmax); 
		//AfxMessageBox(line);
		//output images
		const unsigned int ixsize = ixmax - ixmin + 1;
		const unsigned int iysize = iymax - iymin + 1;
		const unsigned int izsize = izmax - izmin + 1;
		const unsigned int nOut = ixsize * iysize;
		int nCache = 0;
		for (int i=izmin; i<=izmax; i++) {
			int iz1min = INT_MAX, iz1max = INT_MIN;
			int iz1 = (int)(b[6] * ixmin + b[7] * iymin + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmin + b[7] * iymax + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmax + b[7] * iymin + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmax + b[7] * iymax + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1max = (iz1max+1 > nFiles-1) ? nFiles-1 : iz1max+1;
			iz1min = (iz1min < 0) ? 0 : iz1min;
			nCache = (iz1max-iz1min+1 > nCache) ? iz1max-iz1min+1 : nCache;
		}
		//alloc memory
		MEMORYSTATUSEX memory;
		memory.dwLength = sizeof(memory);
		GlobalMemoryStatusEx(&memory);
		unsigned __int64 imem = (memory.ullTotalPhys / 1024);//memory in kbytes
		#ifndef _WIN64
		if (imem > (1<<21)) imem = (1<<21);//2 GB max for x86 platform
		#endif
		unsigned __int64 ullSize = (unsigned __int64)ixref * (unsigned __int64)iyref * (unsigned __int64)nCache * sizeof(unsigned short) / 1024u;
		if (ullSize >= imem) {
			CString msg;
			msg.Format("Out of memory\r\n %lld kbytes data / %lld kbytes memory", ullSize, imem);
			AfxMessageBox(msg);
			CDialog::OnOK();
			return;
		}
		float** ppOrgPixel = NULL;
		int* piCache = NULL;
		CString* psPath = NULL;
		const int nxny = nbuf;
		try {
			ppOrgPixel = new float*[nCache];
			for (int i=0; i<nCache; i++) {
				ppOrgPixel[i] = new float[nxny];
			}
			piCache = new int[nCache];
			psPath = new CString[nFiles];
		}
		catch(CException* e) {
			e->Delete();
			if (ppOrgPixel) {
				for (int i=0; i<nCache; i++) {
					if (ppOrgPixel[i]) delete [] ppOrgPixel[i];
				}
				delete [] ppOrgPixel;
			}
			if (piCache) delete [] piCache;
			if (psPath) delete [] psPath;
			CString msg;
			msg.Format("Out of memory\r\n %d x %d x %d voxels", ixref, iyref, nCache);
			AfxMessageBox(msg);
			CDialog::OnOK();
			return;
		}
		for (int i=0; i<nCache; i++) {
			memset(ppOrgPixel[i], 0, sizeof(float) * nxny);
			piCache[i] = -1;
		}
		str = m_FileList;
		iPos = 0;
		for (int i=0; i<nFiles; i++) {psPath[i] = str.Tokenize(_T("\r\n"), iPos);}
		//read files
		if (pDlg) {
			pDlg->SetTitle(L"Reslicing files");
			pDlg->SetLine(1, L"Reading files...", FALSE, NULL);
			pDlg->StartProgressDialog(NULL, NULL, PROGDLG_NORMAL | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
		}
		bool bError = false;
		unsigned char* pucOut = NULL;
		try {pucOut = new unsigned char[nOut];}
		catch(CException* e) {
			e->Delete();
			if (ppOrgPixel) {
				for (int i=0; i<nCache; i++) {
					if (ppOrgPixel[i]) delete [] ppOrgPixel[i];
				}
				delete [] ppOrgPixel;
			}
			AfxMessageBox("Out of memory");
			CDialog::OnOK();
			return;
		}
		if (pDlg) pDlg->SetLine(1, L"Reslice files...", FALSE, NULL);
		const int idigit = (int)log10((double)izsize) + 1;
		CString fmt;
		fmt.Format("%s%%0%dd.tif", m_Prefix, idigit);
		int fidx = m_StartIndex;
		for (int i=izmin; i<=izmax; i++) {
			::ProcessMessage();
			CString msg; msg.Format("Reslicing file: " + m_OutPath + fmt, fidx);
			if (pf) pf->m_wndStatusBar.SetPaneText(1, msg);
			//limits
			int iz1min = INT_MAX, iz1max = INT_MIN;
			int iz1 = (int)(b[6] * ixmin + b[7] * iymin + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmin + b[7] * iymax + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmax + b[7] * iymin + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1 = (int)(b[6] * ixmax + b[7] * iymax + b[8] * i + izcent);
			iz1min = (iz1 < iz1min)? iz1 : iz1min;
			iz1max = (iz1 > iz1max)? iz1 : iz1max;
			iz1max = (iz1max+1 > nFiles-1) ? nFiles-1 : iz1max+1;
			iz1min = (iz1min < 0) ? 0 : iz1min;
			if (pDlg) pDlg->SetLine(2, (CStringW)("Read:"), FALSE, NULL);
			for (int m=iz1min; m<=iz1max; m++) {
				int iMod = m % nCache;
				if (piCache[iMod] == m) continue;
				//read slice m
				_stprintf_s(path_buffer, _MAX_PATH, psPath[m]);
				_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, ext, _MAX_EXT);
				if ((_tcscmp(ext, ".tif") != 0)&&(_tcscmp(ext, ".TIF") != 0)) continue;
				if (pDlg) pDlg->SetLine(2, (CStringW)("Read: " + psPath[m]), FALSE, NULL);
				if (fp.Open(psPath[m], CFile::modeRead | CFile::shareDenyWrite)) {
					if (ReadTif(&fp, &ibuf, &nbuf, &iyref, &ixref) == 0) {
						for (int j=0; j<nxny; j++) {(ppOrgPixel[iMod])[j] = (float)ibuf[j];}
					}
					fp.Close();
				}
				piCache[iMod] = m;
			}
			int iz = i - izmin;
			for (int j=iymin; j<=iymax; j++) {
				int iy = j - iymin;
				for (int k=ixmin; k<=ixmax; k++) {
					int ix = k - ixmin;
					const unsigned int idx = iy * ixsize + ix;
					x1 = b[0] * k + b[1] * j + b[2] * i + ixcent;
					y1 = b[3] * k + b[4] * j + b[5] * i + iycent;
					z1 = b[6] * k + b[7] * j + b[8] * i + izcent;
					if ( (x1 < 0)||(x1 > ixref-1)||(y1 < 0)||(y1 > iyref-1)|| (z1 < 0)||(z1 > nFiles-1) ) {pucOut[idx] = 0;}
					else {
						//interpolation
						const int ix1 = (int)x1;
						const int iy1 = (int)y1;
						const int iz1 = ((int)z1) % nCache;
						const int iz11 = ((int)z1 + 1) % nCache;
						TReal dx = x1 - ix1;
						TReal dy = y1 - iy1;
						TReal dz = z1 - (int)(z1);
						int idx1 = iy1 * ixref + ix1;
						float p0 = (ppOrgPixel[iz1])[idx1];
						float p1 = (x1 < ixref-1) ? (ppOrgPixel[iz1])[idx1+1] : p0;
						float p2 = (y1 < iyref-1) ? (ppOrgPixel[iz1])[idx1+ixref] : p0;
						float p3 = ((x1 < ixref-1)&&(y1 < iyref-1)) ? (ppOrgPixel[iz1])[idx1+1+ixref] : p0;
						float p4 = p0, p5 = p1, p6 = p2, p7 = p3;
						if (z1 < nFiles-1) {
							p4 = (ppOrgPixel[iz11])[idx1];
							p5 = (x1 < ixref-1) ? (ppOrgPixel[iz11])[idx1+1] : p0;
							p6 = (y1 < iyref-1) ? (ppOrgPixel[iz11])[idx1+ixref] : p0;
							p7 = ((x1 < ixref-1)&&(y1 < iyref-1)) ? (ppOrgPixel[iz11])[idx1+1+ixref] : p0;
						}
						TReal pixIntp = ( ((1-dx)*p0+dx*p1)*(1-dy) + ((1-dx)*p2+dx*p3)*dy )*(1-dz) +
									( ((1-dx)*p4+dx*p5)*(1-dy) + ((1-dx)*p6+dx*p7)*dy )*dz;
						pucOut[idx] = (pixIntp > 255) ? 255 : ( (pixIntp < 0) ? 0 : (unsigned char)(pixIntp) );
					}
				}
			}
			CString imageDesc;
			imageDesc.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%.6f ",
				nSino, pw, pixDiv, pixBase, fCenter, iFilter, 55.111697, 0.461227);
//         1         2         3         4         5         6         7
//1234567890123456789012345678901234567890123456789012345678901234567890
//1800	1.000000	595.040478	-5.550398	-312.000000	1	55.111697	0.461227 
			CString dstfn;
			dstfn.Format(m_OutPath + fmt, fidx++);
			if (pDlg) pDlg->SetLine(3, (CStringW)("Save: " + dstfn), FALSE, NULL);
			CFile ofile;
			if (ofile.Open(dstfn, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) {
				if (err = WriteTifMonochrome(&ofile, pucOut, iysize, ixsize, imageDesc)) break;//8bit output
				ofile.Close();
			}
			//CString msg; msg.Format("z1min=%f z1max=%f", z1min, z1max); AfxMessageBox(msg);
			//break;///////////////131110
		}
		if (pDlg) pDlg->SetLine(2, (CStringW)"Finished.", FALSE, NULL);
		if (ibuf) delete [] ibuf;
		if (ppOrgPixel) {
//			for (int i=0; i<nFiles; i++) {
			for (int i=0; i<nCache; i++) {
				if (ppOrgPixel[i]) delete [] ppOrgPixel[i];
			}
			delete [] ppOrgPixel;
		}
		if (piCache) delete [] piCache;
		if (psPath) delete [] psPath;
		if (pucOut) delete [] pucOut;
		if (pDlg) pDlg->StopProgressDialog();
	} else {//((m_ResliceRotZ != 0)||(m_ResliceRotY != 0)||(m_ResliceRotX != 0))
		//	AfxMessageBox("131110");
		//	CDialog::OnOK();
		//	return;
		const int idigit = (int)log10((double)nFiles) + 1;
		CString fmt;
		fmt.Format("%s%%0%dd.tif", m_Prefix, idigit);
		if (pDlg) {
			pDlg->SetTitle(L"Renumbering files");
			//pDlg->SetAnimation(NULL, IDR_AVI);
			pDlg->SetLine(1, L"Renumbering files", FALSE, NULL);
			pDlg->StartProgressDialog(NULL, NULL, PROGDLG_NORMAL | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
		}

		CString str = m_FileList;
		int iPos = 0;
		int idx = m_StartIndex;
		for (int i=0; i<nFiles; i++) {
			CString fn= str.Tokenize(_T("\r\n"), iPos);
			if (fn.IsEmpty()) continue;
			CString dstfn;
			dstfn.Format(m_OutPath + fmt, idx++);
			pDlg->SetLine(2, (CStringW)(fn + " ===> "), FALSE, NULL);
			pDlg->SetLine(3, (CStringW)(dstfn), FALSE, NULL);
			CString msg; msg.Format("Reformatting files: %s", dstfn);
			if (pf) pf->m_wndStatusBar.SetPaneText(1, msg);

			BOOL bErr = ::MoveFile(fn, dstfn);
			if (bErr == 0) {
				CString msg = "ERROR.\r\n", scr;
				if (AfxMessageBox(msg + "\r\nFiles:\r\n " + fn + "\r\n " + dstfn + "\r\nContinue?", MB_YESNO)
						== IDNO) break;
			}
			//Sleep(1000);
		}

		pDlg->SetLine(2, (CStringW)"Finished.", FALSE, NULL);
		pDlg->SetLine(3, (CStringW)"", FALSE, NULL);
		if (pDlg) pDlg->StopProgressDialog();
	}

	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Finished");
	::CoUninitialize();//unload COM
	//AfxMessageBox("OK");
	CDialog::OnOK();
}

int __stdcall RenumFolderBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	//SHBrowseForFolder callback function to initialize default folder
	if (uMsg == BFFM_INITIALIZED) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	return 0;
}

void CDlgRenumFiles::OnBnClickedRenumSetpath()
{
	/*test code 130216
	CString fn = "";
	static char BASED_CODE defaultExt[] = ".";
	static char BASED_CODE szFilter[] = 
		"Folders (*)|*.|All files (*.*)|*.*||";
	CFileDialog dlg(TRUE, defaultExt, fn, OFN_HIDEREADONLY, szFilter, NULL);
	if (dlg.DoModal() != IDOK) return;
	fn = dlg.GetPathName();
	AfxMessageBox(fn);
	return;///*///

	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;
	TCHAR szDisplayName[MAX_PATH];
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	_stprintf_s(path_buffer, _MAX_PATH, m_OutPath);

	bInfo.hwndOwner = AfxGetMainWnd()->m_hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDisplayName;
	bInfo.lpszTitle = _T("Output folder"); 
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = RenumFolderBrowseCallbackProc;
	bInfo.lParam = (LPARAM)path_buffer;

	pIDList = ::SHBrowseForFolder(&bInfo);
	if (pIDList == NULL) return;
	else {
		if (!::SHGetPathFromIDList(pIDList, szDisplayName)) return;
		m_OutPath = szDisplayName;
		if (m_OutPath.Right(1) != "\\") m_OutPath += "\\";
		//AfxMessageBox(m_Outpath);
		UpdateData(FALSE);
		::CoTaskMemFree( pIDList );
	}
	return;
}
