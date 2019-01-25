// DlgHistogram.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgHistogram.h"
#include "gazoDoc.h"
#include "gazoView.h"
#include "MainFrm.h"
#include "DlgHistoOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgHistogram ダイアログ


CDlgHistogram::CDlgHistogram(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgHistogram::IDD, pParent)
	//, m_StatMsg(_T("Statistics..."))
	, m_bEnablePolygon(FALSE)
	, m_bHistLog(FALSE)
{
	//{{AFX_DATA_INIT(CDlgHistogram)
	m_HstHigh = _T("");
	m_HstLow = _T("");
	m_HstMax = _T("");
	m_HstUnit = _T("(Intensity)");
	m_CursorHigh = _T("1000.0");
	m_CursorLow = _T("0.0");
	m_Preview = TRUE;
	m_TrmCentX = 0;
	m_TrmCentY = 0;
	m_TrmSizeX = 0;
	m_TrmSizeY = 0;
	m_TrmAngle = 0;
	m_EnableTrm = FALSE;
	m_FileMsg = _T("0 file(s)");
	//}}AFX_DATA_INIT

	m_Prefix = _T("ro");
	m_TrmAvrg = 1;
	m_16bit = FALSE;
	m_RemoveDepth = 0;
	m_RemoveLAC = 5;
	//151014 m_RemoveLAC = 10;

	pd = NULL;
	pHistgPix = NULL;
	iHstDispMax = -1;
	bLButtonDownHigh = false;
	bLButtonDownLow = false;
	iXMouse = 0;
	iYMouse = 0;
	//iLowCursor = 100;
	//iHighCursor = 200;
	fLow = 0;
	fHigh = 1000;
	bInvalidate = false;
	nFiles = 0;
	iStatus = CDLGHIST_IDLE;
	//
	filePath.Empty();
	fileList = new TCHAR[MAX_FILE_DIALOG_LIST];
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, "");
	//
	hBitmap = NULL;
	pBitmapPix = NULL;
	lpBmpInfo=(LPBITMAPINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
					sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (BITMAP_NCOLORS - 1));
	//white
	(lpBmpInfo->bmiColors)[0].rgbBlue = 255;
	(lpBmpInfo->bmiColors)[0].rgbGreen = 255;
	(lpBmpInfo->bmiColors)[0].rgbRed = 255;
	//black
	(lpBmpInfo->bmiColors)[1].rgbBlue = 0;
	(lpBmpInfo->bmiColors)[1].rgbGreen = 0;
	(lpBmpInfo->bmiColors)[1].rgbRed = 0;
	//red
	(lpBmpInfo->bmiColors)[2].rgbBlue = 0;
	(lpBmpInfo->bmiColors)[2].rgbGreen = 0;
	(lpBmpInfo->bmiColors)[2].rgbRed = 255;
	//green
	(lpBmpInfo->bmiColors)[3].rgbBlue = 0;
	(lpBmpInfo->bmiColors)[3].rgbGreen = 255;
	(lpBmpInfo->bmiColors)[3].rgbRed = 0;
	for (int i=4; i<BITMAP_NCOLORS; i++) {
		(lpBmpInfo->bmiColors)[i].rgbBlue = i;
		(lpBmpInfo->bmiColors)[i].rgbGreen = i;
		(lpBmpInfo->bmiColors)[i].rgbRed = i;
	}
}

CDlgHistogram::~CDlgHistogram() {
	if (lpBmpInfo) HeapFree(GetProcessHeap(),0,lpBmpInfo);
	if (pHistgPix) delete [] pHistgPix;
	if (pBitmapPix) delete [] pBitmapPix;
	if (hBitmap) DeleteObject(hBitmap);
	if (fileList) delete [] fileList;
}

void CDlgHistogram::ParamCopyFrom(const CDlgHistogram& a) {
	m_CursorHigh = a.m_CursorHigh;
	m_CursorLow = a.m_CursorLow;

	m_TrmCentX = a.m_TrmCentX;
	m_TrmCentY = a.m_TrmCentY;
	m_TrmSizeX = a.m_TrmSizeX;
	m_TrmSizeY = a.m_TrmSizeY;
	m_TrmAngle = a.m_TrmAngle;
	m_EnableTrm = a.m_EnableTrm;
	m_bEnablePolygon = a.m_bEnablePolygon;
	m_bHistLog = a.m_bHistLog;

	m_Prefix = a.m_Prefix;
	m_TrmAvrg = a.m_TrmAvrg;
	m_16bit = a.m_16bit;
	m_RemoveDepth = a.m_RemoveDepth;
	m_RemoveLAC = a.m_RemoveLAC;
}

TErr CDlgHistogram::AllocBitmap() {
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	if (pBitmapPix) return 0;
	if (!pd) return 22000;
	pBitmapPix = new BYTE[npix];
	pHistgPix = new int[npix];
	if ((pBitmapPix == NULL)||(pHistgPix == NULL)) {
		if (pBitmapPix) delete [] pBitmapPix;
		if (pHistgPix) delete [] pHistgPix;
		pBitmapPix = NULL;
		pHistgPix = NULL;
		return 22000;
	}
	return 0;
}

void CDlgHistogram::UpdateHistogram() {
	if (AllocBitmap()) return;
	UpdateData();//080315
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	//
	int ixdim, iydim;
	pd->GetDimension(&ixdim, &iydim);
	const int ixy = ixdim * iydim;
	int imin = INT_MAX; int imax = 0;
	for (int i=0; i<ixy; i++) {
		int ip = pd->pPixel[i];
		if (pd->bColor) {ip = ((ip & 0xff) + ((ip >> 8) & 0xff) + ((ip >> 16) & 0xff)) / 3;}
		if (!ip) continue;
		imin = ip < imin ? ip : imin;
		imax = ip > imax ? ip : imax;
	}
	if ((imin == INT_MAX)&&(imax == 0)) {
		imin = 0; imax = CDLGHISTG_BITMAP_WIDTH;
	} else if (imin == imax) {
		imax = imin + CDLGHISTG_BITMAP_WIDTH;
	}
	fLow = imin;
	fHigh = imax;
	m_HstUnit = "(Intensity)";
	if (pd->pixDiv > 0) {
		fLow = imin / pd->pixDiv + pd->pixBase;
		fHigh = imax / pd->pixDiv + pd->pixBase;
		m_HstUnit = "(LAC)";
	}
	m_HstLow.Format("%.1f", fLow);
	m_HstHigh.Format("%.1f", fHigh);
	for (int i=0; i<iWidth; i++) {pHistgPix[i] = 0;}
	for (int i=0; i<ixy; i++) {
		int ip = pd->pPixel[i];
		if (pd->bColor) {ip = ((ip & 0xff) + ((ip >> 8) & 0xff) + ((ip >> 16) & 0xff)) / 3;}
		if (!ip) continue;
		ip = (ip - imin) * (iWidth - 1) / (imax - imin);
		pHistgPix[ip]++;
	}
	if (iHstDispMax < 0) {
		for (int i=0; i<iWidth; i++) {
			int ip = pHistgPix[i];
			iHstDispMax = ip > iHstDispMax ? ip : iHstDispMax;
		}
	}
	if (iHstDispMax <= 0) iHstDispMax = CDLGHISTG_BITMAP_HEIGHT;
	m_HstMax.Format("%6d", iHstDispMax);
	//CString msg; msg.Format("%d %d %d", imin, imax, iHstDispMax);
	//AfxMessageBox("130203-12 " + msg);
	for (int i=0; i<iWidth; i++) {
		int ip = pHistgPix[i] * (iHeight - 1) / iHstDispMax;
		if (ip > iHeight) ip = iHeight;
		for (int j=0; j<ip; j++) {pHistgPix[i + iWidth * j] = 1;}//black histogram
		for (int j=ip; j<iHeight; j++) {pHistgPix[i + iWidth * j] = 0;}//white background
	}
	//for (i=0; i<npix; i++) {pBitmapPix[i] = pHistgPix[i];}
	CopyHistogram();
	UpdateData(FALSE);
}

void CDlgHistogram::CopyHistogram() {
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	for (int i=0; i<npix; i++) {pBitmapPix[i] = pHistgPix[i];}
	//zero line
	int iZero = GetCursorPnt("0.0");
	if ((iZero >= 0)&&(iZero < iWidth)) {
		int icolor = 0;
		for (int i=0; i<iHeight; i++) {
			if (i % 3 == 0) icolor = icolor ? 0 : 1;
			pBitmapPix[iZero + i * iWidth] = icolor;
		}
	}
}

void CDlgHistogram::SetCursor() {
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	int iLowCursor = GetCursorPnt(m_CursorLow);
	int iHighCursor = GetCursorPnt(m_CursorHigh);
	//m_CursorLow = GetIntensityFromCursor(iLowCursor);
	//m_CursorHigh = GetIntensityFromCursor(iHighCursor);
	for (int i=0; i<iHeight; i++) {
		pBitmapPix[iLowCursor + i * iWidth] = 3;//green
		pBitmapPix[iHighCursor + i * iWidth] = 2;//red
	}
}

void CDlgHistogram::UpdateBitmap() {
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	//
	if (hBitmap) DeleteObject(hBitmap);
	BITMAPINFOHEADER* bh = &(lpBmpInfo->bmiHeader);
	bh->biSize = sizeof(BITMAPINFOHEADER); 
	bh->biWidth = iWidth; 
	bh->biHeight = iHeight; 
	bh->biPlanes = 1; 
	bh->biBitCount = 8;
	bh->biCompression = BI_RGB; 
	bh->biSizeImage = 0; 
	//bh->biXPelsPerMeter = 10000; 
	//bh->biYPelsPerMeter = 10000; 
	bh->biXPelsPerMeter = 0;
	bh->biYPelsPerMeter = 0;
	bh->biClrUsed = BITMAP_NCOLORS;
	bh->biClrImportant = 0; 
	CClientDC dc(this);//190122
	hBitmap = CreateDIBitmap(dc.m_hDC, &(lpBmpInfo->bmiHeader), CBM_INIT, pBitmapPix, lpBmpInfo, DIB_RGB_COLORS);
	//190122 hBitmap = CreateDIBitmap(this->GetDC()->m_hDC, &(lpBmpInfo->bmiHeader), CBM_INIT, pBitmapPix, lpBmpInfo, DIB_RGB_COLORS);
	if (hBitmap) m_Bitmap.SetBitmap(hBitmap);
}

void CDlgHistogram::SetDoc(CGazoDoc* pDoc) {if (pDoc) pd = pDoc;}

void CDlgHistogram::UpdateView() {
	if (!m_Preview) return;
	if (!pd) return;
	POSITION pos = pd->GetFirstViewPosition();
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
		if (pv) {
			pv->SetBoxParams(m_TrmCentX, m_TrmCentY, m_TrmSizeX, m_TrmSizeY, m_TrmAngle);
			if (m_EnableTrm) pv->EnableBox(true); else pv->EnableBox(false); 
			if (m_bEnablePolygon) pv->bPolygonEnabled = true; else pv->bPolygonEnabled = false;
		}
	}
	if (pd->pixDiv > 0) {
		int iHigh = (int)( (atof(m_CursorHigh) - pd->pixBase) * pd->pixDiv );
		int iLow = (int)( (atof(m_CursorLow) - pd->pixBase) * pd->pixDiv );
		pd->SetDispLevel(iLow, iHigh);
		pd->UpdateView();
	} else {
		pd->SetDispLevel((int)atof(m_CursorLow), (int)atof(m_CursorHigh));
		pd->UpdateView();
	}
}

void CDlgHistogram::UpdateParam() {
	if (!pd) return;
	POSITION pos = pd->GetFirstViewPosition();
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
		if (pv) {
			bool bFlg;
			pv->GetBoxParams(&m_TrmCentX, &m_TrmCentY, &m_TrmSizeX, &m_TrmSizeY, &m_TrmAngle, &bFlg);
			if (bFlg) m_EnableTrm = TRUE; else m_EnableTrm = FALSE;
			if (pv->bPolygonEnabled) m_bEnablePolygon = TRUE; else m_bEnablePolygon = FALSE;
			if (this->m_hWnd) EnableCtrl();
			break;
		}
	}
	const CString prevCursLow = m_CursorLow;
	const CString prevCursHigh = m_CursorHigh;
	double dens;
	if (pd->pixDiv > 0) {
		dens = pd->iDispLow / pd->pixDiv + pd->pixBase;
		m_CursorLow.Format("%.3f", dens);
		dens = pd->iDispHigh / pd->pixDiv + pd->pixBase;
		m_CursorHigh.Format("%.3f", dens);
	} else {
		dens = pd->iDispLow;
		m_CursorLow.Format("%.1f", dens);
		dens = pd->iDispHigh;
		m_CursorHigh.Format("%.1f", dens);
	}
	if (this->m_hWnd) {
		UpdateData(FALSE);
		if ((m_CursorLow != prevCursLow)||(m_CursorHigh != prevCursHigh)) {
			CopyHistogram();
			SetCursor();
			UpdateBitmap();
			UpdateData(FALSE);
			GetDlgItem(IDC_HISTG_BITMAP)->Invalidate();
		}
	}
}

void CDlgHistogram::EnableCtrl() {
	GetDlgItem(IDC_HISTG_TRMCENTX)->EnableWindow(FALSE);
	GetDlgItem(IDC_HISTG_TRMCENTY)->EnableWindow(FALSE);
	GetDlgItem(IDC_HISTG_TRMSIZEX)->EnableWindow(FALSE);
	GetDlgItem(IDC_HISTG_TRMSIZEY)->EnableWindow(FALSE);
	GetDlgItem(IDC_HISTG_TRMANGLE)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_HISTG_QUEUE)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	if (m_EnableTrm) {
		GetDlgItem(IDC_HISTG_TRMCENTX)->EnableWindow(TRUE);
		GetDlgItem(IDC_HISTG_TRMCENTY)->EnableWindow(TRUE);
		GetDlgItem(IDC_HISTG_TRMSIZEX)->EnableWindow(TRUE);
		GetDlgItem(IDC_HISTG_TRMSIZEY)->EnableWindow(TRUE);
		GetDlgItem(IDC_HISTG_TRMANGLE)->EnableWindow(TRUE);
	}
	if (iStatus == CDLGHIST_IDLE) {
		if (nFiles) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
		GetDlgItem(IDC_HISTG_QUEUE)->EnableWindow(TRUE);
		GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	}
}

void CDlgHistogram::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgHistogram)
	DDX_Control(pDX, IDC_HISTG_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_HISTG_BITMAP, m_Bitmap);
	DDX_Text(pDX, IDC_HISTG_HSTHIGH, m_HstHigh);
	DDX_Text(pDX, IDC_HISTG_HSTLOW, m_HstLow);
	DDX_Text(pDX, IDC_HISTG_HSTMAX, m_HstMax);
	DDX_Text(pDX, IDC_HISTG_HSTUNIT, m_HstUnit);
	DDX_Text(pDX, IDC_HISTG_CURSHIGH, m_CursorHigh);
	DDX_Text(pDX, IDC_HISTG_CURSLOW, m_CursorLow);
	DDX_Check(pDX, IDC_HISTG_PREVIEW, m_Preview);
	DDX_Text(pDX, IDC_HISTG_TRMCENTX, m_TrmCentX);
	DDX_Text(pDX, IDC_HISTG_TRMCENTY, m_TrmCentY);
	DDX_Text(pDX, IDC_HISTG_TRMSIZEX, m_TrmSizeX);
	DDX_Text(pDX, IDC_HISTG_TRMSIZEY, m_TrmSizeY);
	//121126 DDV_MinMaxInt(pDX, m_TrmSizeX, 0, 100000);
	//121126 DDV_MinMaxInt(pDX, m_TrmSizeY, 0, 100000);
	DDX_Text(pDX, IDC_HISTG_TRMANGLE, m_TrmAngle);
	//090728 DDX_Text(pDX, IDC_HISTG_PREFIX, m_Prefix);
	//DDX_Text(pDX, IDC_HISTG_TRMAVRG, m_TrmAvrg);
	//DDV_MinMaxInt(pDX, m_TrmAvrg, 0, 50);
	DDX_Check(pDX, IDC_HISTG_ENTRIM, m_EnableTrm);
	DDX_Text(pDX, IDC_HISTG_FILEMSG, m_FileMsg);
	//DDX_Check(pDX, IDC_HISTG_16BIT, m_16bit);
	//}}AFX_DATA_MAP
	//DDX_Text(pDX, IDC_HISTG_STAT, m_StatMsg);
	DDX_Check(pDX, IDC_HISTG_ENPOLYGON, m_bEnablePolygon);
	DDX_Check(pDX, IDC_HISTG_OUTLOGHIST, m_bHistLog);
}


BEGIN_MESSAGE_MAP(CDlgHistogram, CDialog)
	//{{AFX_MSG_MAP(CDlgHistogram)
	ON_NOTIFY(UDN_DELTAPOS, IDC_HISTG_MAG, OnDeltaposHistgMag)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_EN_KILLFOCUS(IDC_HISTG_CURSHIGH, OnKillfocusHistgCurshigh)
	ON_EN_KILLFOCUS(IDC_HISTG_CURSLOW, OnKillfocusHistgCurslow)
	ON_BN_CLICKED(IDC_HISTG_PREVIEW, OnHistgPreview)
	ON_BN_CLICKED(IDC_HISTG_GETPATH, OnHistgGetpath)
	ON_BN_CLICKED(IDC_HISTG_ENTRIM, OnHistgEntrim)
	ON_EN_KILLFOCUS(IDC_HISTG_TRMCENTX, OnKillfocusHistgTrmcentx)
	ON_EN_KILLFOCUS(IDC_HISTG_TRMCENTY, OnKillfocusHistgTrmcenty)
	ON_EN_KILLFOCUS(IDC_HISTG_TRMSIZEX, OnKillfocusHistgTrmsizex)
	ON_EN_KILLFOCUS(IDC_HISTG_TRMSIZEY, OnKillfocusHistgTrmsizey)
	ON_EN_KILLFOCUS(IDC_HISTG_TRMANGLE, OnKillfocusHistgTrmangle)
	ON_BN_CLICKED(IDC_HISTG_QUEUE, OnHistgQueue)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HISTG_OPT, &CDlgHistogram::OnBnClickedHistgOpt)
	ON_BN_CLICKED(IDC_HISTG_ENPOLYGON, &CDlgHistogram::OnBnClickedHistgEnpolygon)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgHistogram メッセージ ハンドラ

BOOL CDlgHistogram::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	//const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	//const int npix = iWidth * iHeight;
	UpdateHistogram();
	//CopyHistogram();
	SetCursor();
	UpdateData(FALSE);
	EnableCtrl();
	UpdateBitmap();
	//if (hBitmap) m_Bitmap.SetBitmap(hBitmap);
	UpdateBoxMsg();
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDlgHistogram::OnDeltaposHistgMag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	const int iWidth = CDLGHISTG_BITMAP_WIDTH;
	const int iHeight = CDLGHISTG_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	int idlt = pNMUpDown->iDelta; //left = 1 and right = -1

	if (idlt < 0) iHstDispMax = (int)(iHstDispMax * 0.9);
	else iHstDispMax = (int)(iHstDispMax * 1.1);
	UpdateHistogram();
	//CopyHistogram();
	SetCursor();
	UpdateBitmap();
	GetDlgItem(IDC_HISTG_BITMAP)->Invalidate();
	//190122 UpdateView();

	*pResult = 0;
}

int CDlgHistogram::GetCursorPnt(CString arg) {
	//if (pd) {
	//	if (pd->pixDiv > 0) {
	//		dens = (dens - pd->pixBase) * pd->pixDiv;
	//	}
	//}
	double dens = atof(arg);
	int irtn = (int)( (dens - fLow) * (CDLGHISTG_BITMAP_WIDTH - 1) / (fHigh - fLow) );
	if (irtn < 0) irtn = 0;
	else if (irtn >= CDLGHISTG_BITMAP_WIDTH) irtn = CDLGHISTG_BITMAP_WIDTH - 1;
	return irtn;
}

CString CDlgHistogram::GetIntensityFromCursor(int ipos) {
	double dens = ipos * (fHigh - fLow) / (CDLGHISTG_BITMAP_WIDTH - 1) + fLow;
	//if (pd) {
	//	if (pd->pixDiv > 0) {
	//		rtn = rtn / pd->pixDiv + pd->pixBase;
	//	}
	//}
	//rtn = (int)(rtn * 1000 + 0.5) / 1000.;
	CString rtn;
	rtn.Format("%.1f", dens);
	if (pd) {
		if (pd->pixDiv > 0) rtn.Format("%.3f", dens);
	}
	return rtn;
}

void CDlgHistogram::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int iLowCursor = GetCursorPnt(m_CursorLow);
	int iHighCursor = GetCursorPnt(m_CursorHigh);
	POINT pnt = point;
	MapWindowPoints(GetDlgItem(IDC_HISTG_BITMAP), &pnt, 1);
	if ((pnt.x >= 0)&&(pnt.x < CDLGHISTG_BITMAP_WIDTH)&&
			(pnt.y >= 0)&&(pnt.y < CDLGHISTG_BITMAP_HEIGHT)) {
		if (abs(iLowCursor - pnt.x) < CDLGHISTG_BITMAP_NEAR) {
			bLButtonDownLow = true;
			bLButtonDownHigh = false;
			iXMouse = pnt.x;
			iYMouse = pnt.y;
			SetCapture();
		} else if (abs(iHighCursor - pnt.x) < CDLGHISTG_BITMAP_NEAR) {
			bLButtonDownLow = false;
			bLButtonDownHigh = true;
			iXMouse = pnt.x;
			iYMouse = pnt.y;
			SetCapture();
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgHistogram::OnLButtonUp(UINT nFlags, CPoint point) 
{
	bLButtonDownHigh = false;
	bLButtonDownLow = false;
	ReleaseCapture();
	if (bInvalidate) {
		UpdateView();
		bInvalidate = false;
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgHistogram::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (bLButtonDownLow || bLButtonDownHigh) {
		UpdateData();
		int iLowCursor = GetCursorPnt(m_CursorLow);
		int iHighCursor = GetCursorPnt(m_CursorHigh);
		POINT pnt = point;
		MapWindowPoints(GetDlgItem(IDC_HISTG_BITMAP), &pnt, 1);
		//bool bInvalidate = false;
		if (bLButtonDownLow) {
			iLowCursor = pnt.x;
			if (iLowCursor < 0) iLowCursor = 0;
			//else if (iLowCursor >= CDLGHISTG_BITMAP_WIDTH) iLowCursor = CDLGHISTG_BITMAP_WIDTH - 1;
			else if (iLowCursor >= iHighCursor) iLowCursor = iHighCursor - 1;
			m_CursorLow = GetIntensityFromCursor(iLowCursor);
			bInvalidate = true;
		} else if (bLButtonDownHigh) {
			iHighCursor = pnt.x;
			//if (iHighCursor < 0) iHighCursor = 0;
			if (iHighCursor >= CDLGHISTG_BITMAP_WIDTH) iHighCursor = CDLGHISTG_BITMAP_WIDTH - 1;
			else if (iHighCursor <= iLowCursor) iHighCursor = iLowCursor + 1;
			m_CursorHigh = GetIntensityFromCursor(iHighCursor);
			bInvalidate = true;
		}
		iXMouse = pnt.x;
		iYMouse = pnt.y;
		if (bInvalidate) {
			CopyHistogram();
			SetCursor();
			UpdateBitmap();
			UpdateData(FALSE);
			GetDlgItem(IDC_HISTG_BITMAP)->Invalidate();
			//UpdateView();
		}
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CDlgHistogram::OnKillfocusHistgCurshigh() 
{
	CString scr = m_CursorHigh;
	UpdateData();
	if (m_CursorHigh.SpanExcluding("0123456789.-+").GetLength()) {
		m_CursorHigh = scr;
		UpdateData(FALSE);
		return;
	}
	if (GetCursorPnt(m_CursorHigh) <= GetCursorPnt(m_CursorLow)) {
		m_CursorHigh = scr;
		UpdateData(FALSE);
		return;
	}
	CopyHistogram();
	SetCursor();
	//UpdateData(FALSE);
	UpdateBitmap();
	GetDlgItem(IDC_HISTG_BITMAP)->Invalidate();
	UpdateView();
}

void CDlgHistogram::OnKillfocusHistgCurslow() 
{
	CString scr = m_CursorLow;
	UpdateData();
	if (m_CursorLow.SpanExcluding("0123456789.-+").GetLength()) {
		m_CursorLow = scr;
		UpdateData(FALSE);
		return;
	}
	if (GetCursorPnt(m_CursorHigh) <= GetCursorPnt(m_CursorLow)) {
		m_CursorLow = scr;
		UpdateData(FALSE);
		return;
	}
	CopyHistogram();
	SetCursor();
	//UpdateData(FALSE);
	UpdateBitmap();
	GetDlgItem(IDC_HISTG_BITMAP)->Invalidate();
	UpdateView();
}

void CDlgHistogram::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	//AfxMessageBox("not implemented yet"); return;//////////
	if (!pd) return;
	UpdateData();
	UpdateView();
	//GetDlgItem(IDOK)->EnableWindow(FALSE);
	//GetDlgItem(IDC_HISTG_QUEUE)->EnableWindow(FALSE);
	//GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	double cursLow = fLow; double cursHigh = fHigh;
	if (m_CursorHigh.SpanExcluding("0123456789.-+").GetLength() == 0) cursHigh = atof(m_CursorHigh);
	if (m_CursorLow.SpanExcluding("0123456789.-+").GetLength() == 0) cursLow = atof(m_CursorLow);
	//CString line; line.Format("1 %f %f", cursLow, cursHigh); AfxMessageBox(line);
	iStatus = CDLGHIST_BUSY;//150102
	EnableCtrl();
	if (fileList && nFiles) {
		m_Progress.SetRange(0, nFiles);
		m_Progress.SetPos(0);
		m_Progress.SetStep(1);
		BeginWaitCursor();
		FORMAT_QUEUE fq;
		//080703 fq.dLow = fLow;
		//080703 fq.dHigh = fHigh;
		fq.dLow = cursLow;
		fq.dHigh = cursHigh;
		fq.iBoxCentX = m_TrmCentX;
		fq.iBoxCentY = m_TrmCentY;
		fq.iBoxSizeX = m_TrmSizeX;
		fq.iBoxSizeY = m_TrmSizeY;
		fq.iBoxAngle = m_TrmAngle;
		fq.bBoxEnabled = m_EnableTrm;
		fq.iAverage = m_TrmAvrg;
		fq.nFiles = nFiles;
		fq.lpFileList = fileList;
		fq.outFilePrefix = m_Prefix;
		//180621 fq.b16bit = m_16bit;
		fq.uiFlags = (m_16bit) ? FQFLAGS_16BIT : 0;
		fq.uiFlags |= (m_bHistLog) ? FQFLAGS_OUTPUT_HISTG : 0;
		fq.dOspThreshold = m_RemoveLAC;
		fq.iOspDepth = m_RemoveDepth;
		pd->GetDimension(&(fq.iXdim), &(fq.iYdim));
		fq.sPolygonList.Empty();
		if (m_bEnablePolygon) {
			POSITION pos = pd->GetFirstViewPosition();
			while (pos != NULL) {
				CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
				if (pv) fq.sPolygonList = pv->dlgPolygon.sPolygonList;
			}
		}
		TErr err = pd->OutputImageInBox(&fq, &m_Progress);
		EndWaitCursor();
		if (err) {CString line; line.Format("Error %d", err); AfxMessageBox(line);}
		m_Progress.SetPos(nFiles);
	}
	nFiles = 0;
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, "");
	m_FileMsg = "Finished";
	iStatus = CDLGHIST_IDLE;//150102
	EnableCtrl();
	UpdateData(FALSE);
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->prevDlgHistogram.ParamCopyFrom(*this);
	pApp->prevDlgHistogram.m_FileMsg = "saved";
	return;
	
	//CDialog::OnOK();
}

void CDlgHistogram::OnHistgPreview() 
{
	UpdateData();
	if (m_Preview) UpdateView();
}

void CDlgHistogram::OnHistgGetpath() //GetFileList
{
	if (nFiles) {
		if (AfxMessageBox("Delele current selection?", MB_OKCANCEL) == IDCANCEL) return;
	}
	UpdateData();
	nFiles = 0;
	m_FileMsg = _T("0 file(s)");
	//CString line; line.Format("%d", _MAX_PATH); AfxMessageBox(line); return;
	if (!fileList) return;
	//if (_tcslen(fileList) == 0) _tcscpy(fileList, filePath + "rec.tif");
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, filePath + "rec.tif");
	static char BASED_CODE szFilter[] = "TIFF files (*.tif)|*.tif|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".tif";
	CFileDialog fileDlg(TRUE, defaultExt, NULL, 
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
	//char fnstring[65535];
	//_tcscpy(fileList, fn);
	fileDlg.m_ofn.nMaxFile = MAX_FILE_DIALOG_LIST;
	fileDlg.m_ofn.lpstrFile = (LPTSTR)fileList;
	if (fileDlg.DoModal() == IDCANCEL) {
		UpdateData(FALSE);
		EnableCtrl();
		return;
	}
	POSITION pos = fileDlg.GetStartPosition();
	nFiles = 0;
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	while (pos) {
		_stprintf_s(path_buffer, _MAX_PATH, fileDlg.GetNextPathName(pos));
		nFiles++;
		if (nFiles == 1) _tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	}
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	//CString fdir = dir;
	//if (fdir.GetLength()) {
	//	_stprintf(path_buffer, fdir.Left(fdir.GetLength()-1));
	//	_tsplitpath( path_buffer, NULL, NULL, fnm, NULL );
	//	fdir = fnm;
	//}
	m_FileMsg.Format("%d files (%s)", nFiles, path_buffer);
	EnableCtrl();
	UpdateData(FALSE);
}

void CDlgHistogram::OnHistgEntrim() 
{
	UpdateData();
	EnableCtrl();
	UpdateView();
}

void CDlgHistogram::OnKillfocusHistgTrmcentx() {UpdateData(); UpdateBoxMsg(); UpdateView();}
void CDlgHistogram::OnKillfocusHistgTrmcenty() {UpdateData(); UpdateBoxMsg(); UpdateView();}
void CDlgHistogram::OnKillfocusHistgTrmsizex() {UpdateData(); UpdateBoxMsg(); UpdateView();}
void CDlgHistogram::OnKillfocusHistgTrmsizey() {UpdateData(); UpdateBoxMsg(); UpdateView();}
void CDlgHistogram::OnKillfocusHistgTrmangle() {UpdateData(); UpdateBoxMsg(); UpdateView();}

void CDlgHistogram::UpdateBoxMsg() {
	CString line;
	line.Format("Box (%d,%d) - (%d,%d)",
		m_TrmCentX - m_TrmSizeX / 2, m_TrmCentY - m_TrmSizeY / 2,
		m_TrmCentX + m_TrmSizeX / 2 - 1, m_TrmCentY + m_TrmSizeY / 2 - 1);
	GetDlgItem(IDC_HISTG_BOXMSG)->SetWindowText(line);
}

void CDlgHistogram::OnHistgQueue() 
{
	if (!pd) return;
	if (!fileList || !nFiles) return;
	UpdateData();
	UpdateView();
	double cursLow = fLow; double cursHigh = fHigh;
	if (m_CursorHigh.SpanExcluding("0123456789.-+").GetLength() == 0) cursHigh = atof(m_CursorHigh);
	if (m_CursorLow.SpanExcluding("0123456789.-+").GetLength() == 0) cursLow = atof(m_CursorLow);
	//
	FORMAT_QUEUE fq;
	//080703 fq.dLow = fLow;
	//080703 fq.dHigh = fHigh;
	fq.dLow = cursLow;
	fq.dHigh = cursHigh;
	fq.iBoxCentX = m_TrmCentX;
	fq.iBoxCentY = m_TrmCentY;
	fq.iBoxSizeX = m_TrmSizeX;
	fq.iBoxSizeY = m_TrmSizeY;
	fq.iBoxAngle = m_TrmAngle;
	fq.bBoxEnabled = m_EnableTrm;
	fq.iAverage = m_TrmAvrg;
	fq.nFiles = nFiles;
	fq.lpFileList = fileList;
	fq.outFilePrefix = m_Prefix;
	//fq.b16bit = m_16bit;
	fq.uiFlags = (m_16bit) ? FQFLAGS_16BIT : 0;
	fq.uiFlags |= (m_bHistLog) ? FQFLAGS_OUTPUT_HISTG : 0;
	fq.dOspThreshold = m_RemoveLAC;
	fq.iOspDepth = m_RemoveDepth;
	pd->GetDimension(&(fq.iXdim), &(fq.iYdim));
	fq.sPolygonList.Empty();
	if (m_bEnablePolygon) {
		POSITION pos = pd->GetFirstViewPosition();
		while (pos != NULL) {
			CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
			if (pv) fq.sPolygonList = pv->dlgPolygon.sPolygonList;
		}
	}

	//pd params
	//rq.filePath = pd->GetPathName();
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->dlgQueue.AddFmtQueue(&fq);
	//
	//ShowWindow(SW_HIDE);
	//DestroyWindow();
	pApp->prevDlgHistogram.ParamCopyFrom(*this);
	pApp->prevDlgHistogram.m_FileMsg = "saved";
	//120803
	nFiles = 0;
	_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, "");
	m_FileMsg = _T("0 file(s)");
	UpdateData(FALSE);
	//
	//150102 CDialog::OnOK();
	ShowWindow(SW_HIDE);
	DestroyWindow();
	iStatus = CDLGHIST_IDLE;
}

void CDlgHistogram::OnBnClickedHistgOpt()
{
	CDlgHistoOpt dlg;
	dlg.m_Prefix = this->m_Prefix;
	dlg.m_TrimAvrg = this->m_TrmAvrg;
	dlg.m_16bit = this->m_16bit;
	dlg.m_RemoveDepth = this->m_RemoveDepth;
	dlg.m_RemoveLAC = this->m_RemoveLAC;
	if (dlg.DoModal() == IDCANCEL) return;
	this->m_Prefix = dlg.m_Prefix;
	this->m_TrmAvrg = dlg.m_TrimAvrg;
	this->m_16bit = dlg.m_16bit;
	if (dlg.m_RemoveDepth >= 0) this->m_RemoveDepth = dlg.m_RemoveDepth;
	this->m_RemoveLAC = dlg.m_RemoveLAC;
}

void CDlgHistogram::OnCancel() 
{
	//AfxMessageBox("OnCancel");
	UpdateData();
	ShowWindow(SW_HIDE);
	DestroyWindow();
	iStatus = CDLGHIST_IDLE;
	//150102 CDialog::OnCancel();
}

void CDlgHistogram::OnBnClickedHistgEnpolygon()
{
	UpdateData();
	EnableCtrl();
	UpdateView();
	if (m_bEnablePolygon) {
		POSITION pos = pd->GetFirstViewPosition();
		while (pos != NULL) {
			CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
			if (pv) {
				if (pv->dlgPolygon.sPolygonList.IsEmpty()) AfxMessageBox("No polygon data stored");
			}
		}
	}
}
