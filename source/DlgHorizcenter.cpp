// DlgHorizcenter.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgHorizcenter.h"
#include "gazoDoc.h"


// CDlgHorizcenter ダイアログ

IMPLEMENT_DYNAMIC(CDlgHorizcenter, CDialog)

CDlgHorizcenter::CDlgHorizcenter(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgHorizcenter::IDD, pParent)
	, mBack(300)
	, mOutpath(_T(""))
	, mList(_T("(none)"))
	, mAvg(5)
	, mRelative(0)
{
	pd = NULL;
}

void CDlgHorizcenter::SetDoc(CGazoDoc* pDoc) {if (pDoc) pd = pDoc;}

CDlgHorizcenter::~CDlgHorizcenter()
{
}

void CDlgHorizcenter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HORIZCEN_BACK, mBack);
	DDV_MinMaxInt(pDX, mBack, 0, 65535);
	//DDX_Text(pDX, IDC_HORIZCEN_OUTPATH, mOutpath);
	DDX_Text(pDX, IDC_MSG_TEXT, mList);
	DDX_Text(pDX, IDC_HORIZCEN_AVG, mAvg);
	DDV_MinMaxInt(pDX, mAvg, 1, 100);
	DDX_Radio(pDX, IDC_RADIO_RELATIVE, mRelative);
}


BEGIN_MESSAGE_MAP(CDlgHorizcenter, CDialog)
	ON_BN_CLICKED(IDC_HORIZCEN_GETLIST, &CDlgHorizcenter::OnBnClickedHorizcenGetlist)
	//ON_BN_CLICKED(IDC_HORIZCEN_SETPATH, &CDlgHorizcenter::OnBnClickedHorizcenSetpath)
	ON_BN_CLICKED(IDC_HORIZCEN_SAVE, &CDlgHorizcenter::OnBnClickedHorizcenSave)
END_MESSAGE_MAP()


// CDlgHorizcenter メッセージ ハンドラ

void CDlgHorizcenter::OnBnClickedHorizcenGetlist()
{
	UpdateData();
	int ixd, iyd;
	pd->GetDimension(&ixd, &iyd);
	if ((ixd < 1)||(iyd < 1)) return;
	int* pCenter = new int[iyd];
	mList.Empty();
	int ntotal = 0, itotal = 0;
	for (int j=0; j<iyd; j++) {
		int nsum = 0, icent = 0;
		for (int i=0; i<ixd; i++) {
			int ipix = pd->pPixel[j * ixd + i];
			if (pd->bColor) {ipix = ((ipix & 0xff) + ((ipix >> 8) & 0xff) + ((ipix >> 16) & 0xff)) / 3;}
			if (ipix < mBack) continue;
			icent += i * ipix;
			nsum += ipix;
		}
		if (nsum > 0) {
			pCenter[j] = icent / nsum;
			itotal += pCenter[j];
			ntotal++;
		} else {
			pCenter[j] =  -1;
		}
	}
	itotal = (ntotal > 0) ? (itotal / ntotal) : 0;
	for (int j=0; j<iyd; j++) {
		int isum = 0, nsum = 0;
		for (int i=0; i<mAvg; i++) {
			const int idx = j - mAvg/2 + i;
			if ((idx < 0)||(idx >= iyd)) continue;
			if (pCenter[idx] < 0) continue;
			isum += pCenter[idx];
			nsum++;
		}
		CString line;
		if (mRelative == 0) {//relative
			line.Format("%d %d\r\n", j, (nsum > 0) ? (isum / nsum - itotal) : 0);
		} else {//absolute
			line.Format("%d %d\r\n", j, (nsum > 0) ? (isum / nsum) : -1);
		}
		mList += line;
	}
	if (pCenter) delete [] pCenter;
	UpdateData(FALSE);
}

void CDlgHorizcenter::OnBnClickedHorizcenSave()
{
	if (!pd) return;
	static char BASED_CODE szFilter[] = "text files (*.txt)|*.txt|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".txt";
	CFileDialog fileDlg(FALSE, defaultExt, mOutpath, OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) return;
	mOutpath = fileDlg.GetPathName();
	CFile flist;
	if (flist.Open(mOutpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) {
		flist.Write(mList, mList.GetLength());
		flist.Close();
	}
}
