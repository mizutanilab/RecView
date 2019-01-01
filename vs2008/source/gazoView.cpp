// gazoView.cpp : CGazoView クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "gazo.h"

#include <math.h>//sin, cos
#include "gazoDoc.h"
#include "gazoView.h"
#include "MainFrm.h"
#include "cxyz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGazoView

IMPLEMENT_DYNCREATE(CGazoView, CView)

BEGIN_MESSAGE_MAP(CGazoView, CView)
	//{{AFX_MSG_MAP(CGazoView)
	ON_COMMAND(ID_TOOLBAR_MAG, OnToolbarMag)
	ON_COMMAND(ID_TOOLBAR_MIN, OnToolbarMin)
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	//manually added
	//ON_COMMAND(IDEX_RECONST_EXEC1, OnReconstExec1)
	//added by vs2008
	ON_COMMAND(IDM_VIEW_BOXAXISLABEL, &CGazoView::OnViewBoxaxislabel)
	//then manually removed
	//ON_UPDATE_COMMAND_UI(IDM_VIEW_BOXAXISLABEL, &CGazoView::OnUpdateViewBoxaxislabel)
	ON_COMMAND(ID_ANALYSIS_POLYGONLASSO, &CGazoView::OnAnalysisPolygonlasso)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_POLYGONLASSO, &CGazoView::OnUpdateAnalysisPolygonlasso)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGazoView クラスの構築/消滅

CGazoView::CGazoView()
{
	// TODO: この場所に構築用のコードを追加してください。
	hBitmap = NULL;
	lpBmPixel = NULL;
	iMagnify = 1;
	lpBmpInfo=(LPBITMAPINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
																		sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (BITMAP_NCOLORS - 1));
	for (int i=0; i<BITMAP_NCOLORS; i++) {
		(lpBmpInfo->bmiColors)[i].rgbBlue = i;
		(lpBmpInfo->bmiColors)[i].rgbGreen = i;
		(lpBmpInfo->bmiColors)[i].rgbRed = i;
	}
//	pPix = NULL;
//	maxPix = 0;
	pbOverlay = NULL;
	maxOverlay = 0;
	for (int i=0; i<8; i++) {pdOverlayCoeff[i] = 0;}
	ixdim = 0; iydim = 0;
	iBoxCentX = 100; iBoxCentY = 200; iBoxSizeX = 200; iBoxSizeY = 100; iBoxAngle = 0;
	bBoxEnabled = false;
	bLButtonDown = false;
	bBoxMove = false;
	iPickedBoxPnt = -1;
	hCursor = ::GetCursor();
	bRButtonDown = false;
	bRedLine = false;
	bDragScrollEnabled = false;
	iScrollH = 0; iScrollV = 0;
	bPolygonEnabled = false;
	bPolygonMove = false;
	iPickedPolygonPnt = -1;
	InitPolygon(200, 200, 100, 100);
	dlgPolygon.SetView(this);
}

CGazoView::~CGazoView()
{
	if (lpBmpInfo) HeapFree(GetProcessHeap(),0,lpBmpInfo);
//	if (pPix) delete [] pPix;
	if (pbOverlay) delete [] pbOverlay;
	if (hBitmap) DeleteObject(hBitmap);
	if (dlgPolygon.m_hWnd) dlgPolygon.DestroyWindow();
	//lpBmPixel is automatically deleted by DeleteObject
}

BOOL CGazoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。
	cs.style |= WS_VSCROLL | WS_HSCROLL;
	//CGazoDoc* pd = GetDocument();
	//ASSERT_VALID(pd);
	//pd->GetDimension(&(cs.x), &(cs.y));
	//cs.x = 100; cs.y = 100;

	return CView::PreCreateWindow(cs);
}

void CGazoView::GetBoxParams(int* xcent, int* ycent, int* xsize, int* ysize, int* angle, bool* enb) {
	*xcent = iBoxCentX;
	*ycent = iBoxCentY;
	*xsize = iBoxSizeX;
	*ysize = iBoxSizeY;
	*angle = iBoxAngle;
	*enb = bBoxEnabled;
}

void CGazoView::SetBoxParams(int xcent, int ycent, int xsize, int ysize, int angle) {
	iBoxCentX = xcent;
	iBoxCentY = ycent;
	iBoxSizeX = xsize;
	iBoxSizeY = ysize;
	iBoxAngle = angle;
}

void CGazoView::EnableBox(bool bEnable) {
	bBoxEnabled = bEnable;
}

bool CGazoView::GetLineParams(int* xstart, int* ystart, int* xend, int* yend) {
	if (xstart) *xstart = pntRedFrom.x;
	if (ystart) *ystart = pntRedFrom.y;
	if (xend) *xend = pntRedTo.x;
	if (yend) *yend = pntRedTo.y;
	return bRedLine;
}

/////////////////////////////////////////////////////////////////////////////
// CGazoView クラスの描画

void CGazoView::SetOverlay(int** ppOverlay, int ixd, int iyd, int izd, int iBrightness, int iContrast) {
	if (izd == 0) return;
	CGazoDoc* pd = GetDocument();
	if (pd == NULL) return;
	for (int i=0; i<izd; i++) {if (ppOverlay[i] == NULL) return;}
	const int ixdarg = ixdim;
	//if (ixdarg & 3) ixd = ((int)(ixdarg / 4) + 1) * 4;
	const int idim = ixdim * iydim;
	if (idim > maxOverlay) {
		BYTE* tpbOverlay = pbOverlay;
		try {pbOverlay = new BYTE[idim];}
		catch(CException* e) {
			e->Delete();
			pbOverlay = tpbOverlay; return;
		}
		if (tpbOverlay) delete [] tpbOverlay;
		maxOverlay = idim;
	}
	//ixdim = ixd;
	//iydim = iyd;
	CXyz cx1 = CXyz(pd->dlgOverlay.m_ru1, pd->dlgOverlay.m_rv1, pd->dlgOverlay.m_rw1);
	CXyz cx2 = CXyz(pd->dlgOverlay.m_ru2, pd->dlgOverlay.m_rv2, pd->dlgOverlay.m_rw2);
	CXyz cx3 = CXyz(pd->dlgOverlay.m_ru3, pd->dlgOverlay.m_rv3, pd->dlgOverlay.m_rw3);
	CXyz cPlaneNorm = (cx2 - cx1) * (cx3 - cx1);
	double dPlaneSec = cPlaneNorm.X(cx1);
	TReal a = pdOverlayCoeff[0];
	TReal b = pdOverlayCoeff[1];
	TReal c = pdOverlayCoeff[2];
	TReal d = pdOverlayCoeff[3];
	TReal e = pdOverlayCoeff[4];
	TReal f = pdOverlayCoeff[5];
	TReal g = pdOverlayCoeff[6];
	TReal h = pdOverlayCoeff[7];
	for (int i=0; i<iydim; i++) {
		for (int j=0; j<ixdim; j++) {
			int ipix = 0;
			const int ix = (int)((a*j + b*i + c) / (g*j + h*i + 1) + 0.5);
			const int iy = (int)((d*j + e*i + f) / (g*j + h*i + 1) + 0.5);
			const int iz = (fabs(cPlaneNorm.z) < 1E-6) ? 0 :
				(int)((dPlaneSec - cPlaneNorm.x * ix - cPlaneNorm.y * iy) / cPlaneNorm.z);
			if ((iy >= 0)&&(iy < iyd)&&(ix >= 0)&&(ix < ixd)&&(iz >= 0)&&(iz < izd))
				ipix = ((ppOverlay[iz])[ix + iy * ixd] - iBrightness) * 255 / iContrast;
			if (ipix > 255) ipix = 255; else if (ipix < 0) ipix = 0;
			pbOverlay[j + (iydim - 1 - i) * ixdim] = ipix;
		}
	}
	if (ixdarg & 3) {//fill end-pixels in the 4n+1, 4n+2, 4n+3 length strip
		for (int i=0; i<iydim; i++) {
			for (int j=ixdarg; j<ixdim; j++) {pbOverlay[j + i * ixdim] = 128;}
		}
	}
	UpdateBitmap(ixdarg, iBrightness, iContrast);
}

void CGazoView::SetPixels(int ixd, int iyd, int iBrightness, int iContrast) {
	const int ixdarg = ixd;
	if (ixdarg & 3) ixd = ((int)(ixdarg / 4) + 1) * 4;
//	const int idim = ixd * iyd;
//	if (idim > maxPix) {
//		BYTE* tpPix = pPix;
//		if ((pPix = new BYTE[idim]) == NULL) {
//			pPix = tpPix; return;
//		}
//		if (tpPix) delete [] tpPix;
//		maxPix = idim;
//	}
	ixdim = ixd;
	iydim = iyd;
	/*
	CGazoDoc* pd = GetDocument();
	if (pd->bColor) {
		for (int i=0; i<iydim; i++) {
			for (int j=0; j<ixdim; j++) {
				for (int k=0; k<3; k++) {
					int ipix = (((pPixel[j + i * ixdarg] >> (k*8)) & 0xff) - iBrightness) * 255 / iContrast;
					if (ipix > 255) ipix = 255; else if (ipix < 0) ipix = 0;
					pPix[j + (iydim - 1 - i) * ixdim] |= (ipix << (k*8));
				}
			}
		}
		if (ixdarg & 3) {//fill end-pixels in the 4n+1, 4n+2, 4n+3 length strip
			for (int i=0; i<iydim; i++) {
				for (int j=ixdarg; j<ixdim; j++) {pPix[j + i * ixdim] = RGB(128, 128, 128);}
			}
		}
	} else {
		for (int i=0; i<iydim; i++) {
			for (int j=0; j<ixdim; j++) {
				//int ipix = (pPixel[j + i * ixdim] - iBrightness) * 255 / iContrast;
				int ipix = (pPixel[j + i * ixdarg] - iBrightness) * 255 / iContrast;
				if (ipix > 255) ipix = 255; else if (ipix < 0) ipix = 0;
				pPix[j + (iydim - 1 - i) * ixdim] = ipix;
			}
		}
		if (ixdarg & 3) {//fill end-pixels in the 4n+1, 4n+2, 4n+3 length strip
			for (int i=0; i<iydim; i++) {
				for (int j=ixdarg; j<ixdim; j++) {pPix[j + i * ixdim] = 128;}
			}
		}
	}//(bColor)
	*/
	UpdateBitmap(ixdarg, iBrightness, iContrast);
}

void CGazoView::UpdateBitmap(int ixdarg, int iBrightness, int iContrast) {
	const bool bCreate = (lpBmPixel) ? false : true;
	CDC* pDC = this->GetDC();
	/*
	if (lpBmPixel) {
		//AfxMessageBox("not allocate");
		for (int i=0; i<iydim; i++) {
			for (int j=0; j<ixdim; j++) {
				int igray = pPix[j + i * ixdim];
				//151012 lpBmPixel[j + i * ixdim] = RGB(igray, igray, igray);
				if (pbOverlay == NULL) {
					lpBmPixel[j + i * ixdim] = RGB(igray, igray, igray);
				} else {
					lpBmPixel[j + i * ixdim] = RGB(0, igray, pbOverlay[j + i * ixdim]);
				}
			}
		}
		return;
	}*/
	if (bCreate) {
		if (hBitmap) DeleteObject(hBitmap);
		BITMAPINFOHEADER* bh = &(lpBmpInfo->bmiHeader);
		bh->biSize = sizeof(BITMAPINFOHEADER); 
		bh->biWidth = ixdim; 
		bh->biHeight = iydim; 
		bh->biPlanes = 1; 
//		bh->biBitCount = 8;
		bh->biBitCount = 32;
		bh->biCompression = BI_RGB; 
		bh->biSizeImage = 0; 
		bh->biXPelsPerMeter = 10000; 
		bh->biYPelsPerMeter = 10000; 
//		bh->biClrUsed = BITMAP_NCOLORS; 
		bh->biClrUsed = 0; 
		bh->biClrImportant = 0; 
		//CDC* pDC = this->GetDC();
//		if (pbOverlay == NULL) {
//			hBitmap = CreateDIBitmap(pDC->m_hDC, &(lpBmpInfo->bmiHeader), CBM_INIT, pPix,
//										lpBmpInfo, DIB_RGB_COLORS);
//			if (hBitmap) {this->ReleaseDC(pDC); return;}
//		}
		//if NULL, use CreateDIBSection
//		bh->biBitCount = 32;
		hBitmap = CreateDIBSection(pDC->m_hDC, lpBmpInfo, DIB_RGB_COLORS, (void**)&lpBmPixel, NULL, 0);
		if (!hBitmap) {AfxMessageBox("Out of memory: bitmap"); this->ReleaseDC(pDC); return;}
	}//(bCreate)
	//copy bitmap
	CGazoDoc* pd = GetDocument();
	if (pd->bColor) {
		//CString line; line.Format("%d %d", iBrightness, iContrast); AfxMessageBox(line);
		for (int i=0; i<iydim; i++) {
			for (int j=0; j<ixdim; j++) {
				lpBmPixel[j + (iydim - 1 - i) * ixdim] = 0;
				for (int k=0; k<3; k++) {
					int ipix = (((pd->pPixel[j + i * ixdarg] >> (k*8)) & 0xff) - iBrightness) * 255 / iContrast;
					if (ipix > 255) ipix = 255; else if (ipix < 0) ipix = 0;
					lpBmPixel[j + (iydim - 1 - i) * ixdim] |= (ipix << (k*8));
				}
			}
		}
		if (ixdarg & 3) {//fill end-pixels in the 4n+1, 4n+2, 4n+3 length strip
			for (int i=0; i<iydim; i++) {
				for (int j=ixdarg; j<ixdim; j++) {lpBmPixel[j + i * ixdim] = RGB(128, 128, 128);}
			}
		}
		//for (int i=0; i<iydim; i++) {
		//	for (int j=0; j<ixdim; j++) {
		//		lpBmPixel[j + i * ixdim] = pPix[j + i * ixdim];
		//	}
		//}
	} else {
		for (int i=0; i<iydim; i++) {
			for (int j=0; j<ixdim; j++) {
				//int ipix = (pPixel[j + i * ixdim] - iBrightness) * 255 / iContrast;
				int ipix = (pd->pPixel[j + i * ixdarg] - iBrightness) * 255 / iContrast;
				if (ipix > 255) ipix = 255; else if (ipix < 0) ipix = 0;
				//pPix[j + (iydim - 1 - i) * ixdim] = ipix;
				if (pbOverlay == NULL) {
					lpBmPixel[j + (iydim - 1 - i) * ixdim] = RGB(ipix, ipix, ipix);
				} else {
					lpBmPixel[j + (iydim - 1 - i) * ixdim] = RGB(0, ipix, pbOverlay[j + i * ixdim]);
				}
			}
		}
		if (ixdarg & 3) {//fill end-pixels in the 4n+1, 4n+2, 4n+3 length strip
			for (int i=0; i<iydim; i++) {
				//160731 for (int j=ixdarg; j<ixdim; j++) {lpBmPixel[j + i * ixdim] = 128;}
				for (int j=ixdarg; j<ixdim; j++) {lpBmPixel[j + i * ixdim] = RGB(128, 128, 128);}
			}
		}
		//for (int i=0; i<iydim; i++) {
		//	for (int j=0; j<ixdim; j++) {
		//		int igray = pPix[j + i * ixdim];
		//		//151012 lpBmPixel[j + i * ixdim] = RGB(igray, igray, igray);
		//		if (pbOverlay == NULL) {
		//			lpBmPixel[j + i * ixdim] = RGB(igray, igray, igray);
		//		} else {
		//			lpBmPixel[j + i * ixdim] = RGB(0, igray, pbOverlay[j + i * ixdim]);
		//		}
		//	}
		//}
	}//(bColor)
	if (bCreate) this->ReleaseDC(pDC);
}

void CGazoView::OnDraw(CDC* pDC)
{
	//CGazoDoc* pd = GetDocument();
	//ASSERT_VALID(pd);
	// TODO: この場所にネイティブ データ用の描画コードを追加します。
	if (!hBitmap) return;
	CBitmap Bitmap;
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = MemDC.SelectObject(Bitmap.FromHandle(hBitmap));
	CRect rect;
	GetClientRect(&rect);
	COLORREF iGray = RGB(128, 128, 128);
	pDC->FillSolidRect(rect, iGray);
	CPoint pCenter = rect.CenterPoint();
	TReal rMagnify = iMagnify;
	if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
	int ixpos = GetScrollPos(SB_HORZ);
	int iypos = GetScrollPos(SB_VERT);
	int ixWidthImg = (int)(ixdim * rMagnify);
	int iyWidthImg = (int)(iydim * rMagnify);
	int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
	int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
	//CString text; text.Format("sc %d %d %d %d", iypos, iy0img, ixWidthImg, iyWidthImg);
	//CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	//pf->m_wndStatusBar.SetPaneText(0, text);
	int isMode = pDC->SetStretchBltMode(COLORONCOLOR);
	//int isMode = pDC->SetStretchBltMode(HALFTONE);
	POINT brushOrg;
	::SetBrushOrgEx(pDC->m_hDC, 0, 0, &brushOrg);
	pDC->StretchBlt(ix0img, iy0img, ixWidthImg, iyWidthImg, &MemDC, 0, 0, ixdim, iydim, SRCCOPY);
	pDC->SetStretchBltMode(isMode);
	::SetBrushOrgEx(pDC->m_hDC, brushOrg.x, brushOrg.y, NULL);
	MemDC.SelectObject(pOldBitmap);
	//131021 if (bBoxEnabled) {
	if (bBoxEnabled && (iBoxSizeX > 0) && (iBoxSizeY > 0)) {
		const double rBoxCentX = iBoxCentX * rMagnify + ix0img;
		const double rBoxCentY = iBoxCentY * rMagnify + iy0img;
		const double rBoxSizeX = iBoxSizeX * rMagnify;
		const double rBoxSizeY = iBoxSizeY * rMagnify;
		const double csa = cos(iBoxAngle * DEG_TO_RAD);
		const double sna = sin(iBoxAngle * DEG_TO_RAD);
		const double v1x = rBoxSizeX * 0.5 * csa;
		const double v1y = rBoxSizeX * 0.5 * sna;
		const double v2x = rBoxSizeY * 0.5 * (-sna);
		const double v2y = rBoxSizeY * 0.5 * csa;
		pntBox[0].x = (int)(rBoxCentX - v1x - v2x);
		pntBox[0].y = (int)(rBoxCentY - v1y - v2y);
		pntBox[1].x = (int)(rBoxCentX + v1x - v2x);
		pntBox[1].y = (int)(rBoxCentY + v1y - v2y);
		pntBox[2].x = (int)(rBoxCentX + v1x + v2x);
		pntBox[2].y = (int)(rBoxCentY + v1y + v2y);
		pntBox[3].x = (int)(rBoxCentX - v1x + v2x);
		pntBox[3].y = (int)(rBoxCentY - v1y + v2y);
		pntBox[4] = pntBox[0];
		CPen mPen1( PS_SOLID, 1, RGB(0,255,0) );
		CPen* pOldPen = pDC->SelectObject(&mPen1);
		pDC->Polyline(pntBox, 5);
		pDC->SelectObject(pOldPen);
		if (((CGazoApp*)AfxGetApp())->bShowBoxAxis) {
			CFont labelfont;
			labelfont.CreatePointFont(90, "Arial", pDC);//pt 9, Arial, use current device context
			CFont* orgFont = pDC->SelectObject(&labelfont);
			pDC->SetBkMode(TRANSPARENT);
			double e1x = pntBox[1].x - pntBox[0].x;
			double e1y = pntBox[1].y - pntBox[0].y;
			double e2x = pntBox[3].x - pntBox[0].x;
			double e2y = pntBox[3].y - pntBox[0].y;
			double e1r = 150;//sqrt(e1x * e1x + e1y * e1y);
			double e2r = 150;//sqrt(e2x * e2x + e2y * e2y);
			pDC->TextOut((int)(pntBox[0].x + e1x * 30 / e1r), (int)(pntBox[0].y + e1y * 30 / e1r), "x");
			pDC->TextOut((int)(pntBox[0].x + e2x * 30 / e2r), (int)(pntBox[0].y + e2y * 30 / e2r), "y");
			pDC->SelectObject(orgFont);
			mPen1.DeleteObject();
			labelfont.DeleteObject();
		}//if bShowBoxAxis
	}
	if (bPolygonEnabled) {//180424
		for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
			pntPolygon[i].x = (int)(iPolygonX[i] * rMagnify + ix0img);
			pntPolygon[i].y = (int)(iPolygonY[i] * rMagnify + iy0img);
		}
		pntPolygon[CGAZOVIEW_NPOLYGON] = pntPolygon[0];
		CPen mPen1( PS_SOLID, 1, RGB(255,255,0) );
		CPen* pOldPen = pDC->SelectObject(&mPen1);
		pDC->Polyline(pntPolygon, CGAZOVIEW_NPOLYGON+1);
		pDC->SelectObject(pOldPen);
	}
	if (bRedLine) {
		CPen mPen1( PS_SOLID, 1, RGB(255,0,0) );
		CPen* pOldPen = pDC->SelectObject(&mPen1);
		CPoint pnt1, pnt2;
		pnt1.x = (int)(pntRedFrom.x * rMagnify + ix0img);
		pnt1.y = (int)(pntRedFrom.y * rMagnify + iy0img);
		pDC->MoveTo(pnt1);
		pnt2.x = (int)(pntRedTo.x * rMagnify + ix0img);
		pnt2.y = (int)(pntRedTo.y * rMagnify + iy0img);
		pDC->LineTo(pnt2);
		double ex = (pnt2.x - pnt1.x);
		double ey = (pnt2.y - pnt1.y);
		double elen = sqrt(ex * ex + ey * ey);
		if (elen) {
			ex /= elen; ey /= elen;
			pnt1.x = pnt2.x + (int)(5 * rMagnify * (- ex + ey));
			pnt1.y = pnt2.y + (int)(5 * rMagnify * (- ey - ex));
			pDC->LineTo(pnt1);
			pnt1.x = pnt2.x + (int)(5 * rMagnify * (- ex - ey));
			pnt1.y = pnt2.y + (int)(5 * rMagnify * (- ey + ex));
			pDC->MoveTo(pnt2);
			pDC->LineTo(pnt1);
		}
		pDC->SelectObject(pOldPen);
		mPen1.DeleteObject();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGazoView クラスの印刷

BOOL CGazoView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// デフォルトの印刷準備
	return DoPreparePrinting(pInfo);
}

void CGazoView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CGazoView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}

/////////////////////////////////////////////////////////////////////////////
// CGazoView クラスの診断

#ifdef _DEBUG
void CGazoView::AssertValid() const
{
	CView::AssertValid();
}

void CGazoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGazoDoc* CGazoView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGazoDoc)));
	return (CGazoDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGazoView クラスのメッセージ ハンドラ

void CGazoView::OnToolbarMag() 
{
	iMagnify++;
	//150101 uppper limit
	TReal rMagnify = iMagnify;
	if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
	int ixWidthImg = (int)(ixdim * rMagnify);
	int iyWidthImg = (int)(iydim * rMagnify);
	if ((ixWidthImg > 32767)||(iyWidthImg > 32767)) iMagnify--;
	//
	//if (iMagnify > 100) iMagnify = 100;
	InvalidateRect(NULL, FALSE);
	CString text;
	if (iMagnify < 1) text.Format("Size x1/%d", 2-iMagnify);
	else text.Format("Size x%d", iMagnify);
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	pf->m_wndStatusBar.SetPaneText(0, text);
}

void CGazoView::OnToolbarMin() 
{
	iMagnify--;
	if (iMagnify < -100) iMagnify = -100;
	InvalidateRect(NULL, FALSE);
	CString text;
	if (iMagnify < 1) text.Format("Size x1/%d", 2-iMagnify);
	else text.Format("Size x%d", iMagnify);
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	pf->m_wndStatusBar.SetPaneText(0, text);
}

void CGazoView::GetCoord(CPoint* pPnt) {
	TReal rMagnify = iMagnify;
	if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
	//
	CRect rect;
	GetClientRect(&rect);
	CPoint pCenter = rect.CenterPoint();
	int ixpos = GetScrollPos(SB_HORZ);
	int iypos = GetScrollPos(SB_VERT);
	int ixWidthImg = (int)(ixdim * rMagnify);
	int iyWidthImg = (int)(iydim * rMagnify);
	int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
	int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
	//
	pPnt->x = (int)((pPnt->x - ix0img) / rMagnify);
	pPnt->y = (int)((pPnt->y - iy0img) / rMagnify);
	CGazoDoc* pd = GetDocument();
	ASSERT_VALID(pd);
	int ixd, iyd;
	pd->GetDimension(&ixd, &iyd);
	if (pPnt->x >= ixd) pPnt->x = ixd - 1;
	if (pPnt->y >= iyd) pPnt->y = iyd - 1;
	if (pPnt->x < 0) pPnt->x = 0;
	if (pPnt->y < 0) pPnt->y = 0;
}

bool CGazoView::PointInBox(CPoint point) {
	if (!bBoxEnabled) return false;
	int idetx1 = (pntBox[1].y - pntBox[0].y) * (point.x - pntBox[0].x)
								- (pntBox[1].x - pntBox[0].x) * (point.y - pntBox[0].y);
	int idetx2 = (pntBox[2].y - pntBox[3].y) * (point.x - pntBox[3].x)
								- (pntBox[2].x - pntBox[3].x) * (point.y - pntBox[3].y);
	int idety1 = (pntBox[3].y - pntBox[0].y) * (point.x - pntBox[0].x)
								- (pntBox[3].x - pntBox[0].x) * (point.y - pntBox[0].y);
	int idety2 = (pntBox[2].y - pntBox[1].y) * (point.x - pntBox[1].x)
								- (pntBox[2].x - pntBox[1].x) * (point.y - pntBox[1].y);
	if ((idetx1 * idetx2 < 0) && (idety1 * idety2 < 0)) return true;
	return false;
}

void CGazoView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CGazoDoc* pd = GetDocument();
	ASSERT_VALID(pd);
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	CString text = "";
	if (!bBoxMove) {
		if (bBoxEnabled && !bLButtonDown) {
			//150101if (PointInBox(point)) ::SetCursor(((CGazoApp*)AfxGetApp())->hCursorRot);
			//else ::SetCursor(hCursor);
		}
	}
	if (bPolygonEnabled) {
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
		if (bPolygonMove) {
			int ipx0 = (int)( (pntLButton.x + point.x - ix0img) / rMagnify) - iPolygonX[0];
			int ipy0 = (int)( (pntLButton.y + point.y - iy0img) / rMagnify) - iPolygonY[0];
			for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
				iPolygonX[i] += ipx0;
				iPolygonY[i] += ipy0;
			}
			dlgPolygon.UpdateCurrentPolygon();
			InvalidateRect(NULL, FALSE);
		} else if (bLButtonDown) {
			if (iPickedPolygonPnt >= 0) {
				iPolygonX[iPickedPolygonPnt] = (int)((point.x - ix0img) / rMagnify);
				iPolygonY[iPickedPolygonPnt] = (int)((point.y - iy0img) / rMagnify);
			}
			dlgPolygon.UpdateCurrentPolygon();
			InvalidateRect(NULL, FALSE);
		}
	} else if (bBoxEnabled) {
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
		if (bBoxMove && (iBoxSizeX >= 0) &&(iBoxSizeY >= 0)) {
			iBoxCentX = (int)( (pntLButton.x + point.x - ix0img) / rMagnify);
			iBoxCentY = (int)( (pntLButton.y + point.y - iy0img) / rMagnify);
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2,text);
			pd->dlgHist.UpdateParam();
			InvalidateRect(NULL, FALSE);
		} else if (bLButtonDown) {
			int iBoxDirX = 1; int iBoxDirY = 1;
			switch (iPickedBoxPnt) {
				case 0: {iBoxDirX = -1; iBoxDirY = -1; break;}
				case 1: {iBoxDirY = -1; break;}
				case 2: {break;}
				case 3: {iBoxDirX = -1; break;}
				default: {break;}
			}
			iBoxCentX = (int)( ((pntLButton.x + point.x) * 0.5 - ix0img) / rMagnify );
			iBoxCentY = (int)( ((pntLButton.y + point.y) * 0.5 - iy0img) / rMagnify );
			//CString msg; msg.Format("%d %d", point.x - pntLButton.x, point.y - pntLButton.y);
			//pf->m_wndStatusBar.SetPaneText(1, msg);
			const CPoint pntDif = point - pntLButton;
			const double boxDiag = sqrt((double)(pntDif.x * pntDif.x + pntDif.y * pntDif.y));
			if (boxDiag > 0) {
				double alpha = 0;
				if (pntDif.y >= 0) alpha = acos(pntDif.x / boxDiag) - iBoxAngle * DEG_TO_RAD;
				else alpha = acos(pntDif.x / boxDiag) + iBoxAngle * DEG_TO_RAD;
				iBoxSizeX = abs( (int)(boxDiag * cos(alpha) / rMagnify) );
				iBoxSizeY = abs( (int)(boxDiag * sin(alpha) / rMagnify) );
			}
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", 
				iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2,text);
			pd->dlgHist.UpdateParam();
			InvalidateRect(NULL, FALSE);
		}
	} else if (bDragScrollEnabled && bLButtonDown) {//150102
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		const int iprevx = GetScrollPos(SB_HORZ);
		const int iprevy = GetScrollPos(SB_VERT);
		const int ixpos = iScrollH - (int)(100 * (point.x - pntLButton.x) / (rMagnify * ixdim));
		const int iypos = iScrollV - (int)(100 * (point.y - pntLButton.y) / (rMagnify * iydim));
		if ((iprevx != ixpos)||(iprevy != iypos)) {
			SetScrollPos(SB_HORZ, ixpos);
			SetScrollPos(SB_VERT, iypos);
			InvalidateRect(NULL, FALSE);
		}
	}
	
	{//180424 } else {
		CPoint pnt = point;
		GetCoord(&pnt);
		int ix = (int)pnt.x;
		int iy = (int)pnt.y;
		int iabs = 0;
		double absCoeff = 0;
		//if ((pd->pPixel) && (ix >=0) && (iy >= 0) && (ix < ixdim) && (iy < iydim)) {
			//absCoeff = (pd->pPixel)[ix + iy * ixdim];
		int idocx, idocy;
		pd->GetDimension(&idocx, &idocy);
		if ((pd->pPixel) && (ix >=0) && (iy >= 0) && (ix < idocx) && (iy < idocy)) {
			iabs = (pd->pPixel)[ix + iy * idocx];
		}
		if (pd->pixDiv > 0) {
			absCoeff = iabs / pd->pixDiv + pd->pixBase;
			//text.Format("(x,y)=(%d %d), LAC=%8f", ix, iy, absCoeff);
			text.Format("(x,y)=(%d %d), LAC=%8.3f Raw=%d", ix, iy, absCoeff, iabs);
		} else {
			if (pd->bColor) {
				text.Format("(x,y)=(%d %d), RGB=(%d %d %d)", ix, iy, 
					(iabs >> 16) & 0xff, (iabs >> 8) & 0xff, iabs & 0xff);
			} else {
				text.Format("(x,y)=(%d %d), Intensity=%d", ix, iy, iabs);
			}
		}
		pf->m_wndStatusBar.SetPaneText(0,text);
	}
	if (bRButtonDown) {
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
		pntRedTo.x = (int)((point.x - ix0img) / rMagnify);
		pntRedTo.y = (int)((point.y - iy0img) / rMagnify);
		InvalidateRect(NULL, FALSE);
	}
	CView::OnMouseMove(nFlags, point);
}

void CGazoView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int ipos = GetScrollPos(SB_VERT);
	switch (nSBCode) {
		case SB_LINEDOWN: {}
		case SB_PAGEDOWN: {ipos++; break;}
		case SB_LINEUP: {}
		case SB_PAGEUP: {ipos--; break;}
		case SB_THUMBTRACK: {}
		case SB_THUMBPOSITION: {ipos = nPos; break;}
		case SB_ENDSCROLL: {break;}
		default: {
			//CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
			//pf->m_wndStatusBar.SetPaneText(0,"else");
			break;}
	}
	//CString text; text.Format("sc %d", ipos);
	//CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	//pf->m_wndStatusBar.SetPaneText(0, text);
	//npos === 0 ... 100
	SetScrollPos(SB_VERT, ipos);
	InvalidateRect(NULL, FALSE);
	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CGazoView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//AfxMessageBox("WM_HSCROLL");
	int ipos = GetScrollPos(SB_HORZ);
	switch (nSBCode) {
		case SB_LINERIGHT: {}
		case SB_PAGERIGHT: {ipos++; break;}
		case SB_LINELEFT: {}
		case SB_PAGELEFT: {ipos--; break;}
		case SB_THUMBTRACK: {}
		case SB_THUMBPOSITION: {ipos = nPos; break;}
		case SB_ENDSCROLL: {break;}
		default: {break;}
	}
	SetScrollPos(SB_HORZ, ipos);
	InvalidateRect(NULL, FALSE);
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGazoView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	CString text;
	bLButtonDown = true;
	//CString msg; msg.Format("%d %d", this->GetFocus(), pf->GetFocus()); AfxMessageBox(msg);
	//if (this->GetActiveWindow()) AfxMessageBox("is selected");
	//CPoint prevPnt = pntLButton;
	if (bPolygonEnabled) {
		iPickedPolygonPnt = -1;
		for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
			int ix = point.x - pntPolygon[i].x;
			int iy = point.y - pntPolygon[i].y;
			if (ix * ix + iy * iy < 25) {
				iPickedPolygonPnt = i; break;
			}
		}
		bPolygonMove = false;
		if (iPickedPolygonPnt < 0) {
			TReal rMagnify = iMagnify;
			if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
			CRect rect;
			GetClientRect(&rect);
			CPoint pCenter = rect.CenterPoint();
			int ixpos = GetScrollPos(SB_HORZ);
			int iypos = GetScrollPos(SB_VERT);
			int ixWidthImg = (int)(ixdim * rMagnify);
			int iyWidthImg = (int)(iydim * rMagnify);
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
			CPoint pnt;
			pnt.x = (int)((point.x - ix0img) / rMagnify);
			pnt.y = (int)((point.y - iy0img) / rMagnify);
			if (PointInPolygon(pnt)) {
				bPolygonMove = true;
				pntLButton = CPoint((int)(iPolygonX[0] * rMagnify + ix0img), 
									(int)(iPolygonY[0] * rMagnify + iy0img)) - point;
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			}
		}
		SetCapture();
	} else if (bBoxEnabled) {
		//compare pntBox
		iPickedBoxPnt = -1;
		for (int i=0; i<4; i++) {
			int ix = point.x - pntBox[i].x;
			int iy = point.y - pntBox[i].y;
			if (ix * ix + iy * iy < 25) {
				iPickedBoxPnt = i; break;
			}
		}
		bBoxMove = false;
		if (iPickedBoxPnt < 0) {
			if (PointInBox(point)) {
				TReal rMagnify = iMagnify;
				if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
				CRect rect;
				GetClientRect(&rect);
				CPoint pCenter = rect.CenterPoint();
				int ixpos = GetScrollPos(SB_HORZ);
				int iypos = GetScrollPos(SB_VERT);
				int ixWidthImg = (int)(ixdim * rMagnify);
				int iyWidthImg = (int)(iydim * rMagnify);
				int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
				int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
				bBoxMove = true;
				pntLButton = CPoint((int)(iBoxCentX * rMagnify + ix0img), 
									(int)(iBoxCentY * rMagnify + iy0img)) - point;
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
				//hCursor = ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			} else {
				//bBoxMove = false;
				bBoxEnabled = false;
				pf->m_wndStatusBar.SetPaneText(2,"");
			}
		} else {
			switch (iPickedBoxPnt) {
				case 0: {pntLButton = pntBox[2]; break;}
				case 1: {pntLButton = pntBox[3]; break;}
				case 2: {pntLButton = pntBox[0]; break;}
				case 3: {pntLButton = pntBox[1]; break;}
			}
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", 
				iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2,text);
		}
		SetCapture();
	} else {
		pntLButton = point;
		if (((CGazoApp*)AfxGetApp())->bDragScroll) {
			bDragScrollEnabled = true;
			iScrollH = GetScrollPos(SB_HORZ);
			iScrollV = GetScrollPos(SB_VERT);
		} else {
			bBoxEnabled = true;
			CRect rect;
			GetClientRect(&rect);
			CPoint pCenter = rect.CenterPoint();
			TReal rMagnify = iMagnify;
			if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
			int ixpos = GetScrollPos(SB_HORZ);
			int iypos = GetScrollPos(SB_VERT);
			int ixWidthImg = (int)(ixdim * rMagnify);
			int iyWidthImg = (int)(iydim * rMagnify);
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
			iBoxSizeX = 1;
			iBoxSizeY = 1;
			iBoxAngle = 0;
			iBoxCentX = (int)((point.x - ix0img) / rMagnify);
			iBoxCentY = (int)((point.y - iy0img) / rMagnify);
			iPickedBoxPnt = 2;
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", 
				iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2,text);
		}
	}
	CGazoDoc* pd = GetDocument();
	ASSERT_VALID(pd);
	pd->dlgHist.UpdateParam();
	InvalidateRect(NULL, FALSE);
	
	CView::OnLButtonDown(nFlags, point);
}

void CGazoView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	bLButtonDown = false;
	bBoxMove = false;
	bPolygonMove = false;
	bDragScrollEnabled = false;
	ReleaseCapture();
	SetCursor(hCursor);
	
	CView::OnLButtonUp(nFlags, point);
}

BOOL CGazoView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	//CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	CGazoDoc* pd = GetDocument();
	if ((pApp->bWheelToGo) || (GetKeyState(VK_SHIFT) < 0)) {
		if (pd) {
			if (zDelta < 0) pd->ProceedImage(1);
			else if (zDelta > 0) pd->ProceedImage(-1);
		}
	} else if (!bLButtonDown) {
		/*150101
		if (::GetCursor() == ((CGazoApp*)AfxGetApp())->hCursorRot) {
			iBoxAngle -= zDelta * 2 / WHEEL_DELTA;
			CString text;
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", 
				iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2,text);
			InvalidateRect(NULL, FALSE);
		} else {
			int ipos = GetScrollPos(SB_VERT);
			ipos -= zDelta * 2 / WHEEL_DELTA;
			SetScrollPos(SB_VERT, ipos);
			InvalidateRect(NULL, FALSE);
			//CView::OnVScroll(nSBCode, nPos, pScrollBar);
		}*/
		if (zDelta < 0) OnToolbarMin();
		else if (zDelta > 0) OnToolbarMag();
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CGazoView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (bRedLine) {
		bRedLine = false;
		InvalidateRect(NULL, FALSE);
		CView::OnRButtonDown(nFlags, point);
		return;
	} else {
		bRedLine = true; bRButtonDown = true;
	}
	CRect rect;
	GetClientRect(&rect);
	CPoint pCenter = rect.CenterPoint();
	TReal rMagnify = iMagnify;
	if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
	int ixpos = GetScrollPos(SB_HORZ);
	int iypos = GetScrollPos(SB_VERT);
	int ixWidthImg = (int)(ixdim * rMagnify);
	int iyWidthImg = (int)(iydim * rMagnify);
	int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / 100);
	int iy0img = pCenter.y - (int)(iyWidthImg * iypos / 100);
	pntRedFrom.x = (int)((point.x - ix0img) / rMagnify);
	pntRedFrom.y = (int)((point.y - iy0img) / rMagnify);
	pntRedTo = pntRedFrom;
	InvalidateRect(NULL, FALSE);
	
	CView::OnRButtonDown(nFlags, point);
}

void CGazoView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	bRButtonDown = false;
	//pntRButtonUp = point;
	
	CView::OnRButtonUp(nFlags, point);
}

void CGazoView::OnViewBoxaxislabel()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	CGazoApp* app = (CGazoApp*)AfxGetApp();
	if (app->bShowBoxAxis) app->bShowBoxAxis = false; else app->bShowBoxAxis = true;
	InvalidateRect(NULL, FALSE);
}

//void CGazoView::OnUpdateViewBoxaxislabel(CCmdUI *pCmdUI)
//{
//	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
//}

void CGazoView::OnAnalysisPolygonlasso()//180425
{
	if (bPolygonEnabled) {
		if (dlgPolygon.IsWindowVisible()) {
			bPolygonEnabled = false;
			dlgPolygon.ShowWindow(SW_HIDE);
			dlgPolygon.MakePolygonList();
		} else {
			if (!dlgPolygon.m_hWnd) dlgPolygon.Create(IDD_POLYGON);
			dlgPolygon.ShowWindow(SW_SHOW); //AfxMessageBox("ShowWindow");
			dlgPolygon.UpdateCurrentPolygon();
		}
	} else {
		bPolygonEnabled = true;
		if (bBoxEnabled) {
			InitPolygon(iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY);
			bBoxEnabled = false;
		}
		if (!dlgPolygon.m_hWnd) dlgPolygon.Create(IDD_POLYGON);
		if (dlgPolygon.IsWindowVisible()) dlgPolygon.SetForegroundWindow();
		else dlgPolygon.ShowWindow(SW_SHOW);
		dlgPolygon.UpdateCurrentPolygon();
	}
	InvalidateRect(NULL, FALSE);
}

void CGazoView::OnUpdateAnalysisPolygonlasso(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(bPolygonEnabled);
	//if (bBoxEnabled) {pCmdUI->Enable(false); return;}
	//CGazoDoc* pd = GetDocument();
	//if (pd->dlgHist.IsWindowVisible()) {pCmdUI->Enable(false); return;}
	//else pCmdUI->Enable(true);
}

void CGazoView::InitPolygon(int xcent, int ycent, int xsize, int ysize) {
	xsize /= 2; ysize /= 2;
	for(int i=0; i<CGAZOVIEW_NPOLYGON; i++){
		int ix = (int)(cos(i * 2 * __PI / CGAZOVIEW_NPOLYGON) * xsize);
		int iy = (int)(sin(i * 2 * __PI / CGAZOVIEW_NPOLYGON) * ysize);
		iPolygonX[i] = xcent + ix;
		iPolygonY[i] = ycent + iy;
	}
	dlgPolygon.UpdateCurrentPolygon();
//	iPolygonX[0] = xcent + xsize; iPolygonY[0] = ycent;
//	iPolygonX[1] = (int)(xcent + xsize * 0.7); iPolygonY[1] = (int)(ycent + ysize * 0.7);
//	iPolygonX[2] = xcent; iPolygonY[2] = ycent + ysize;
//	iPolygonX[3] = (int)(xcent - xsize * 0.7); iPolygonY[3] = (int)(ycent + ysize * 0.7);
//	iPolygonX[4] = xcent - xsize; iPolygonY[4] = ycent;
//	iPolygonX[5] = (int)(xcent - xsize * 0.7); iPolygonY[5] = (int)(ycent - ysize * 0.7);
//	iPolygonX[6] = xcent; iPolygonY[6] = ycent - ysize;
//	iPolygonX[7] = (int)(xcent + xsize * 0.7); iPolygonY[7] = (int)(ycent - ysize * 0.7);
}

bool CGazoView::PointInPolygon(CPoint point) {//image coords
	if (!bPolygonEnabled) return false;
	CGazoDoc* pd = GetDocument();
	if (!pd) return false;
	return pd->PointInPolygon(point.x, point.y, iPolygonX, iPolygonY);
//	int icount = 0;
//	for(int i=0; i<CGAZOVIEW_NPOLYGON; i++){
//		int i1 = (i == CGAZOVIEW_NPOLYGON-1) ? 0 : i+1;
//		if ( ((iPolygonY[i] <= point.y) && (iPolygonY[i1] > point.y))
//				|| ((iPolygonY[i] > point.y) && (iPolygonY[i1] <= point.y)) ){
//			double dt = (point.y -iPolygonY[i]) / (double)(iPolygonY[i1] - iPolygonY[i]);
//			if (point.x < (iPolygonX[i] + (dt * (iPolygonX[i1] - iPolygonX[i])))) icount++;
//		}
//	}
//	return (icount & 0x01);
}
