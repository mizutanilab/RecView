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
	ON_COMMAND(ID_ANALYSIS_CIRCLELASSO, &CGazoView::OnAnalysisCirclelasso)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_CIRCLELASSO, &CGazoView::OnUpdateAnalysisCirclelasso)
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
	bCircleLassoEnabled = false;//251205
	bCircleLassoMove = false;//251205
	InitCircleLasso(200, 200, 100, 100);//251205
	dlgCircleLasso.SetView(this);//251205
	pntMouse.SetPoint(0, 0);
}

CGazoView::~CGazoView()
{
	if (hBitmap) DeleteObject(hBitmap);
	if (lpBmpInfo) HeapFree(GetProcessHeap(),0,lpBmpInfo);
//	if (pPix) delete [] pPix;
	if (pbOverlay) delete [] pbOverlay;
	//190122 if (hBitmap) DeleteObject(hBitmap);
	if (dlgPolygon.m_hWnd) dlgPolygon.DestroyWindow();
	//lpBmPixel is automatically deleted by DeleteObject
	if (dlgCircleLasso.m_hWnd) dlgCircleLasso.DestroyWindow();
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
	//190122 CDC* pDC = this->GetDC();
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
		CDC* pDC = this->GetDC();//190122
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
		this->ReleaseDC(pDC);//190122
		if (!hBitmap) {AfxMessageBox("Out of memory: bitmap"); return;}
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
	//190122 if (bCreate) this->ReleaseDC(pDC);
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
	int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
	int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
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
	Bitmap.DeleteObject();//190122
	MemDC.DeleteDC();//190122
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
			//190122 mPen1.DeleteObject();
			labelfont.DeleteObject();
		}//if bShowBoxAxis
		mPen1.DeleteObject();//190122
	}
	if (bPolygonEnabled) {//180424
		CPen mPen1( PS_SOLID, 1, RGB(255,255,0) );
		CPen* pOldPen = pDC->SelectObject(&mPen1);
		for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
			pntPolygon[i].x = (int)(iPolygonX[i] * rMagnify + ix0img);
			pntPolygon[i].y = (int)(iPolygonY[i] * rMagnify + iy0img);
			pDC->Ellipse(pntPolygon[i].x - 5, pntPolygon[i].y - 5, pntPolygon[i].x + 5, pntPolygon[i].y + 5);
		}
		pntPolygon[CGAZOVIEW_NPOLYGON] = pntPolygon[0];
		pDC->Polyline(pntPolygon, CGAZOVIEW_NPOLYGON+1);
		pDC->SelectObject(pOldPen);
	}
	if (bCircleLassoEnabled) {//251205
		//CPen mPen1(PS_SOLID, 1, RGB(255, 255, 0));
		//CPen mPen2(PS_SOLID, 1, RGB(255, 0, 255));
		//CPen mPen3(PS_SOLID, 1, RGB(0, 255, 255));
		//CBrush brush1(RGB(255, 255, 0));
		//CBrush brush2(RGB(255, 0, 255));
		//CBrush brush3(RGB(0, 255, 255));
		COLORREF lassoColor[5] = { RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255), RGB(0, 255, 0), RGB(0, 0, 255) };
		CPen penLasso[5]; 
		CBrush brushLasso[5]; 
		for (int i = 0; i < 5; i++) {
			penLasso[i].CreatePen(PS_SOLID, 1, lassoColor[i]);
			brushLasso[i].CreateSolidBrush(lassoColor[i]);
		}
		//CPen penLasso[] = { CPen(PS_SOLID, 1, RGB(255, 255, 0)), CPen(PS_SOLID, 1, RGB(255, 0, 255)), CPen(PS_SOLID, 1, RGB(0, 255, 255)), 
		//					CPen(PS_SOLID, 1, RGB(0, 255, 0)), CPen(PS_SOLID, 1, RGB(0, 0, 255)) };
		//CBrush brushLasso[] = { CBrush(RGB(255, 255, 0)), CBrush(RGB(255, 0, 255)), CBrush(RGB(0, 255, 255)), 
		//						CBrush(RGB(0, 255, 0)), CBrush(RGB(0, 0, 255)) };
		CPen* pOldPen = pDC->SelectObject(&(penLasso[0]));
		CBrush* pOldBrush = pDC->SelectObject(&(brushLasso[0]));
		double cx = (iCircleLasso[0] * rMagnify + ix0img);
		double cy = (iCircleLasso[1] * rMagnify + iy0img);
		double rx = (iCircleLasso[2] * rMagnify);
		//251213 double ry = (iCircleLasso[3] * rMagnify);
		for (int i = 0; i <= 360; i++) {
			double expfac = 0.0;
			int jdot = 0;
			for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
				double fa = iCircleLasso[j] * CGAZOVIEW_NPARAMCIRCLELASSO3_CONST;
				double deg = iCircleLasso[j + 1];
				double fb = iCircleLasso[j + 2] * CGAZOVIEW_NPARAMCIRCLELASSO5_CONST;
				const double th = i - deg < -180 ? i - deg + 360 : i - deg > 180 ? i - deg - 360 : i - deg;
				expfac += fa * exp(-fb * (th*th));
				if ((th == 0) && (abs(iCircleLasso[j]) > 0)) jdot = j;
			}
			double rth = rx * (1.0 + expfac);//rx *( 1.0 + fa * exp(-fb * (th*th)) );
			int ix = (int)(cx + rth * cos(i / 180.0 * __PI));
			int iy = (int)(cy + rth * sin(i / 180.0 * __PI));
			if (i) pDC->LineTo(ix, iy); else pDC->MoveTo(ix, iy);
			if (jdot) {
				pDC->SelectObject(&(penLasso[jdot/3-1])); 
				pDC->SelectObject(&(brushLasso[jdot/3-1]));
				//if (jdot == 6) {pDC->SelectObject(&mPen2); pDC->SelectObject(&brush2);}
				//else if (jdot == 9) { pDC->SelectObject(&mPen3); pDC->SelectObject(&brush3);}
				pDC->Ellipse(ix - 5, iy - 5, ix + 5, iy + 5);
				pDC->MoveTo(ix, iy);
				pDC->SelectObject(&(penLasso[0]));
				pDC->SelectObject(&(brushLasso[0]));
			}
		}
		//pDC->MoveTo(ix + ir, iy);
		//pDC->AngleArc(ix, iy, ir, 0, 360);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
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
	int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
	int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
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
	bool bInvalidate = false;
	if (iPickedBoxPnt < 0) {
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
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
			if (bPolygonMove) {
				int ipx0 = (int)((pntLButtonPolygon.x + point.x - ix0img) / rMagnify) - iPolygonX[0];
				int ipy0 = (int)((pntLButtonPolygon.y + point.y - iy0img) / rMagnify) - iPolygonY[0];
				for (int i = 0; i < CGAZOVIEW_NPOLYGON; i++) {
					iPolygonX[i] += ipx0;
					iPolygonY[i] += ipy0;
				}
				dlgPolygon.UpdateCurrentPolygon();
				//InvalidateRect(NULL, FALSE);
				bInvalidate = true;
			}
			else if (bLButtonDown) {
				if (iPickedPolygonPnt >= 0) {
					iPolygonX[iPickedPolygonPnt] = (int)((point.x - ix0img) / rMagnify);
					iPolygonY[iPickedPolygonPnt] = (int)((point.y - iy0img) / rMagnify);
				}
				dlgPolygon.UpdateCurrentPolygon();
				//InvalidateRect(NULL, FALSE);
				bInvalidate = true;
			}
		}
		if (bCircleLassoEnabled && (iPickedPolygonPnt < 0)) {
			TReal rMagnify = iMagnify;
			if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
			CRect rect;
			GetClientRect(&rect);
			CPoint pCenter = rect.CenterPoint();
			int ixpos = GetScrollPos(SB_HORZ);
			int iypos = GetScrollPos(SB_VERT);
			int ixWidthImg = (int)(ixdim * rMagnify);
			int iyWidthImg = (int)(iydim * rMagnify);
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
			if (bCircleLassoEdit) {
				int ix = (int)((point.x - ix0img) / rMagnify) - iCircleLasso[0];
				int iy = (int)((point.y - iy0img) / rMagnify) - iCircleLasso[1];
				const double dth = atan2(iy, ix) / __PI * 180.;
				//iCircleLasso[4] = (int) deg;
				const double rth = sqrt(ix * ix + iy * iy);
				const double rx = iCircleLasso[2] > 0 ? iCircleLasso[2] : 1;
				const double expfac = rth / rx - 1.0;
				//present expfac
				double expfac0 = 0;
				for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
					double fa = iCircleLasso[j] * CGAZOVIEW_NPARAMCIRCLELASSO3_CONST;
					double deg = iCircleLasso[j + 1];
					double fb = iCircleLasso[j + 2] * CGAZOVIEW_NPARAMCIRCLELASSO5_CONST;
					const double th = dth - deg < -180 ? dth - deg + 360 : dth - deg > 180 ? dth - deg - 360 : dth - deg;
					expfac0 += fa * exp(-fb * (th*th));
				}
				//reform exp
				for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
					//const double fb = iCircleLasso[j + 2] * CGAZOVIEW_NPARAMCIRCLELASSO5_CONST;
					double ddeg = dth - iCircleLasso[j + 1];
					ddeg = ddeg < -180 ? ddeg + 360 : ddeg > 180 ? ddeg - 360 : ddeg;
					if (abs(iCircleLasso[j]) > 0) {//lasso is not exact circle
						if (abs(ddeg) < 5) {//hump clicked
							iCircleLasso[j] += (int)((expfac - expfac0) / CGAZOVIEW_NPARAMCIRCLELASSO3_CONST);
							iCircleLasso[j] = iCircleLasso[j] < -500 ? -500 : iCircleLasso[j] > 500 ? 500 : iCircleLasso[j];
							iCircleLasso[j + 1] = (int)dth;
							bInvalidate = true;
							break;
						}
					}
				}
				//add a new exp
				bool bAdd = true;
				for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
					if (abs(iCircleLasso[j]) > 0) {
						double ddeg = dth - iCircleLasso[j + 1];
						ddeg = ddeg < -180 ? ddeg + 360 : ddeg > 180 ? ddeg - 360 : ddeg;
						if (abs(ddeg) < 10) { bAdd = false; break; }//skip addtion itself if a near point already exists.
					}
				}
				if (bAdd) {
					for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
						if (abs(iCircleLasso[j]) > 0) continue;//skip existing entry
						iCircleLasso[j] = (int)((expfac - expfac0) / CGAZOVIEW_NPARAMCIRCLELASSO3_CONST);
						iCircleLasso[j + 1] = (int)dth;
						bInvalidate = true;
						break;
					}
				}
				if (bInvalidate) dlgCircleLasso.UpdateCurrentCircle();
				//InvalidateRect(NULL, FALSE);
			} else if (bCircleLassoMove) {
				int ipx0 = (int)((pntLButtonCircle.x + point.x - ix0img) / rMagnify) - iCircleLasso[0];
				int ipy0 = (int)((pntLButtonCircle.y + point.y - iy0img) / rMagnify) - iCircleLasso[1];
				iCircleLasso[0] += ipx0;
				iCircleLasso[1] += ipy0;
				dlgCircleLasso.UpdateCurrentCircle();
				//InvalidateRect(NULL, FALSE);
				bInvalidate = true;
			}
		}
	}
	if (bBoxEnabled && pf->bEnableEditTrimbox) {
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
		if (bBoxMove && (iBoxSizeX >= 0) && (iBoxSizeY >= 0) && !bPolygonMove && !bCircleLassoMove && (iPickedPolygonPnt < 0)) {
			iBoxCentX = (int)((pntLButton.x + point.x - ix0img) / rMagnify);
			iBoxCentY = (int)((pntLButton.y + point.y - iy0img) / rMagnify);
			text.Format("Center (%d %d) Size (%d %d) Tilt %d", iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2, text);
			pd->dlgHist.UpdateParam();
			//InvalidateRect(NULL, FALSE);
			bInvalidate = true;
		}
		if (bLButtonDown && (iPickedBoxPnt >= 0)) {
			int iBoxDirX = 1; int iBoxDirY = 1;
			switch (iPickedBoxPnt) {
			case 0: {iBoxDirX = -1; iBoxDirY = -1; break; }
			case 1: {iBoxDirY = -1; break; }
			case 2: {break; }
			case 3: {iBoxDirX = -1; break; }
			default: {break; }
			}
			iBoxCentX = (int)(((pntLButton.x + point.x) * 0.5 - ix0img) / rMagnify);
			iBoxCentY = (int)(((pntLButton.y + point.y) * 0.5 - iy0img) / rMagnify);
			//CString msg; msg.Format("%d %d", point.x - pntLButton.x, point.y - pntLButton.y);
			//pf->m_wndStatusBar.SetPaneText(1, msg);
			const CPoint pntDif = point - pntLButton;
			const double boxDiag = sqrt((double)(pntDif.x * pntDif.x + pntDif.y * pntDif.y));
			if (boxDiag > 0) {
				double alpha = 0;
				if (pntDif.y >= 0) alpha = acos(pntDif.x / boxDiag) - iBoxAngle * DEG_TO_RAD;
				else alpha = acos(pntDif.x / boxDiag) + iBoxAngle * DEG_TO_RAD;
				iBoxSizeX = abs((int)(boxDiag * cos(alpha) / rMagnify));
				iBoxSizeY = abs((int)(boxDiag * sin(alpha) / rMagnify));
			}
			text.Format("Center (%d %d) Size (%d %d) Tilt %d",
				iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
			pf->m_wndStatusBar.SetPaneText(2, text);
			pd->dlgHist.UpdateParam();
			//InvalidateRect(NULL, FALSE);
			bInvalidate = true;
		}
	}
	if (bDragScrollEnabled && bLButtonDown) {//150102
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		const int iprevx = GetScrollPos(SB_HORZ);
		const int iprevy = GetScrollPos(SB_VERT);
		const int ixpos = iScrollH - (int)(CGV_HSCROLL_RANGE * (point.x - pntLButton.x) / (rMagnify * ixdim));
		const int iypos = iScrollV - (int)(CGV_VSCROLL_RANGE * (point.y - pntLButton.y) / (rMagnify * iydim));
		if ((iprevx != ixpos)||(iprevy != iypos)) {
			SetScrollPos(SB_HORZ, ixpos);
			SetScrollPos(SB_VERT, iypos);
			//InvalidateRect(NULL, FALSE);
			bInvalidate = true;
		}
	}
	
	{//180424 } else {
		pntMouse = point;
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
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
		pntRedTo.x = (int)((point.x - ix0img) / rMagnify);
		pntRedTo.y = (int)((point.y - iy0img) / rMagnify);
		//InvalidateRect(NULL, FALSE);
		bInvalidate = true;
	}
	if (bInvalidate) InvalidateRect(NULL, FALSE);
	CView::OnMouseMove(nFlags, point);
}

void CGazoView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int ipos = GetScrollPos(SB_VERT);
	switch (nSBCode) {
		case SB_LINEDOWN: {}
		case SB_PAGEDOWN: {ipos+= CGV_VSCROLL_RANGE/100; break;}
		case SB_LINEUP: {}
		case SB_PAGEUP: {ipos-= CGV_VSCROLL_RANGE/100; break;}
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
		case SB_PAGERIGHT: {ipos+= CGV_HSCROLL_RANGE/100; break;}
		case SB_LINELEFT: {}
		case SB_PAGELEFT: {ipos-= CGV_HSCROLL_RANGE/100; break;}
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
	CMainFrame* pf = (CMainFrame*)AfxGetMainWnd();
	CString text;
	bLButtonDown = true;
	//CString msg; msg.Format("%d %d", this->GetFocus(), pf->GetFocus()); AfxMessageBox(msg);
	//if (this->GetActiveWindow()) AfxMessageBox("is selected");
	//CPoint prevPnt = pntLButton;
	bool bAppLassoRbtn = false;
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	if (pApp) bAppLassoRbtn = pApp->bLassoRbtn;
	if (bAppLassoRbtn == false) ButtonDownToLassoEdit(point);
	/*
	if (bPolygonEnabled) {
		iPickedPolygonPnt = -1;
		for (int i = 0; i < CGAZOVIEW_NPOLYGON; i++) {
			int ix = point.x - pntPolygon[i].x;
			int iy = point.y - pntPolygon[i].y;
			if (ix * ix + iy * iy <= iPickArea) {
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
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
			CPoint pnt;
			pnt.x = (int)((point.x - ix0img) / rMagnify);
			pnt.y = (int)((point.y - iy0img) / rMagnify);
			if (PointInPolygon(pnt)) {
				bPolygonMove = true;
				pntLButtonPolygon = CPoint((int)(iPolygonX[0] * rMagnify + ix0img),
					(int)(iPolygonY[0] * rMagnify + iy0img)) - point;
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			}
		}
		SetCapture();
	}
	if (bCircleLassoEnabled) {
		bCircleLassoMove = false;
		bCircleLassoEdit = false;
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
		CPoint pnt;
		pnt.x = (int)((point.x - ix0img) / rMagnify);
		pnt.y = (int)((point.y - iy0img) / rMagnify);
		if (PointOnCircleLassoLine(pnt)) {
			bCircleLassoEdit = true;
			bPolygonMove = false;
		} else if (PointInCircleLasso(pnt)) {
			bCircleLassoMove = true;
			pntLButtonCircle = CPoint((int)(iCircleLasso[0] * rMagnify + ix0img),
				(int)(iCircleLasso[1] * rMagnify + iy0img)) - point;
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
		}
		SetCapture();
	}*/
	const int iPickArea = CGAZOVIEW_LASSOPICK * CGAZOVIEW_LASSOPICK;// 400;//20 x 20//201126
	bBoxMove = false;
	//if (!bPolygonMove && !bCircleLassoMove && (iPickedPolygonPnt < 0)) {
	if ((iPickedPolygonPnt < 0) && (!bCircleLassoEdit)) {
		if (bBoxEnabled && pf->bEnableEditTrimbox) {
			//compare pntBox
			iPickedBoxPnt = -1;
			for (int i = 0; i < 4; i++) {
				int ix = point.x - pntBox[i].x;
				int iy = point.y - pntBox[i].y;
				if (ix * ix + iy * iy <= iPickArea) {
					iPickedBoxPnt = i; break;
				}
			}
			bBoxMove = false;
			if (iPickedBoxPnt < 0) {
				if (!bPolygonMove && !bCircleLassoMove) {
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
						int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
						int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
						bBoxMove = true;
						pntLButton = CPoint((int)(iBoxCentX * rMagnify + ix0img),
							(int)(iBoxCentY * rMagnify + iy0img)) - point;
						::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
						//hCursor = ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
					} else {
						//bBoxMove = false;
						bBoxEnabled = false;
						pf->m_wndStatusBar.SetPaneText(2, "");
					}
				}
			} else {
				switch (iPickedBoxPnt) {
				case 0: {pntLButton = pntBox[2]; break; }
				case 1: {pntLButton = pntBox[3]; break; }
				case 2: {pntLButton = pntBox[0]; break; }
				case 3: {pntLButton = pntBox[1]; break; }
				}
				text.Format("Center (%d %d) Size (%d %d) Tilt %d",
					iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
				pf->m_wndStatusBar.SetPaneText(2, text);
			}
			SetCapture();
		} else if (!bPolygonMove && !bCircleLassoMove) {
			pntLButton = point;
			if (((CGazoApp*)AfxGetApp())->bDragScroll) {
				bDragScrollEnabled = true;
				iScrollH = GetScrollPos(SB_HORZ);
				iScrollV = GetScrollPos(SB_VERT);
			} else if (pf->bEnableEditTrimbox) {
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
				int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
				int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
				iBoxSizeX = 1;
				iBoxSizeY = 1;
				iBoxAngle = 0;
				iBoxCentX = (int)((point.x - ix0img) / rMagnify);
				iBoxCentY = (int)((point.y - iy0img) / rMagnify);
				iPickedBoxPnt = 2;
				text.Format("Center (%d %d) Size (%d %d) Tilt %d",
					iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY, iBoxAngle);
				pf->m_wndStatusBar.SetPaneText(2, text);
			}
		}
	}
	CGazoDoc* pd = GetDocument();
	if (pd) pd->dlgHist.UpdateParam();
	InvalidateRect(NULL, FALSE);
	
	CView::OnLButtonDown(nFlags, point);
}

void CGazoView::ButtonDownToLassoEdit(CPoint point) {
	const int iPickArea = CGAZOVIEW_LASSOPICK * CGAZOVIEW_LASSOPICK;// 400;//20 x 20//201126
	if (bPolygonEnabled) {
		iPickedPolygonPnt = -1;
		for (int i = 0; i < CGAZOVIEW_NPOLYGON; i++) {
			int ix = point.x - pntPolygon[i].x;
			int iy = point.y - pntPolygon[i].y;
			if (ix * ix + iy * iy <= iPickArea) {
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
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
			CPoint pnt;
			pnt.x = (int)((point.x - ix0img) / rMagnify);
			pnt.y = (int)((point.y - iy0img) / rMagnify);
			if (PointInPolygon(pnt)) {
				bPolygonMove = true;
				pntLButtonPolygon = CPoint((int)(iPolygonX[0] * rMagnify + ix0img),
					(int)(iPolygonY[0] * rMagnify + iy0img)) - point;
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			}
		}
		SetCapture();
	}
	if (bCircleLassoEnabled) {
		bCircleLassoMove = false;
		bCircleLassoEdit = false;
		TReal rMagnify = iMagnify;
		if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		CRect rect;
		GetClientRect(&rect);
		CPoint pCenter = rect.CenterPoint();
		int ixpos = GetScrollPos(SB_HORZ);
		int iypos = GetScrollPos(SB_VERT);
		int ixWidthImg = (int)(ixdim * rMagnify);
		int iyWidthImg = (int)(iydim * rMagnify);
		int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
		int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
		CPoint pnt;
		pnt.x = (int)((point.x - ix0img) / rMagnify);
		pnt.y = (int)((point.y - iy0img) / rMagnify);
		if (PointOnCircleLassoLine(pnt)) {
			bCircleLassoEdit = true;
			bPolygonMove = false;
		}
		else if (PointInCircleLasso(pnt)) {
			bCircleLassoMove = true;
			pntLButtonCircle = CPoint((int)(iCircleLasso[0] * rMagnify + ix0img),
				(int)(iCircleLasso[1] * rMagnify + iy0img)) - point;
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
		}
		SetCapture();
	}
}

void CGazoView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	bLButtonDown = false;
	bBoxMove = false;
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	if (pApp) {
		if (pApp->bLassoRbtn == false) {
			bPolygonMove = false;
			bCircleLassoMove = false;//251205
			bCircleLassoEdit = false;//251205
			iPickedBoxPnt = -1;//251205
			iPickedPolygonPnt = -1;//251205
		}
	}
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
	//consts
	TReal rMagnify = iMagnify;
	if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
	const int ixpos = GetScrollPos(SB_HORZ);
	const int iypos = GetScrollPos(SB_VERT);
	const int ixWidthImg = (int)(ixdim * rMagnify);
	const int iyWidthImg = (int)(iydim * rMagnify);
	CRect rect;
	GetClientRect(&rect);
	const CPoint pCenter = rect.CenterPoint();
	const int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
	const int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
	CPoint ptcl = pt;
	ScreenToClient(&ptcl);
	//distance from circle-lasso center
	const double cx = (iCircleLasso[0] * rMagnify + ix0img) - ptcl.x;
	const double cy = (iCircleLasso[1] * rMagnify + iy0img) - ptcl.y;
	//CString line; line.Format("%d %d\r\n%d %d\r\n%f %f", ptcl.x, ptcl.y, ix0img, iy0img, cx, cy); AfxMessageBox(line);
	int idelta = (int)(sqrt(cx * cx + cy * cy) * 0.1);
	idelta = (idelta < 1) ? 1 : idelta;
	idelta = (idelta > ixWidthImg + iyWidthImg) ? ixWidthImg + iyWidthImg : idelta;
	//CPoint in image coord
	CPoint pntimg;
	pntimg.x = (int)((ptcl.x - ix0img) / rMagnify);
	pntimg.y = (int)((ptcl.y - iy0img) / rMagnify);
	if ((pApp->bWheelToGo) || (GetKeyState(VK_SHIFT) < 0) || (GetKeyState(VK_F1) < 0)) {
		if (pd) {
			if (pd->parentDoc) {
				if (pd->parentDoc->dlgReconst.m_hWnd) {
					const int iTrimWidth = pd->parentDoc->dlgReconst.m_Trim;
					int iXdim = 0, iYdim = 0;
					pd->parentDoc->GetDimension(&iXdim, &iYdim);
					if (pd->parentDoc->dlgReconst.m_bOffsetCT) iXdim *= 2;
					iXdim -= iTrimWidth * 2;
					const int iInterpolation = pd->parentDoc->dlgReconst.m_Interpolation;
					const int iZooming = (iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
					const int iBinning = (iInterpolation == 0) ? 4 : ((iInterpolation == 1) ? 2 : 1);
					const int ixlen = iXdim / iBinning;
					const int ipintp = (int)pow((double)2, iZooming);
					const int iTargetdim = ixlen * ipintp;
					if (pd->ixdim == iTargetdim) {
						const int iTargetSlice = (pd->dlgReconst.iContext & CDLGRECONST_CONTEXT_TOMO1) ? 1 : ((pd->dlgReconst.iContext & CDLGRECONST_CONTEXT_TOMO2) ? 2 : 0);
						if ((GetKeyState(VK_SHIFT) < 0) || (pApp->bWheelToGo)) pd->parentDoc->dlgReconst.IncDecCenter(iTargetSlice, zDelta);
						else if (GetKeyState(VK_F1) < 0) pd->parentDoc->dlgReconst.IncDecTilt(zDelta > 0 ? 5 : -5);
						if (pd->parentDoc->dlgReconst.iStatus == CDLGRECONST_IDLE) pd->parentDoc->dlgReconst.CalcTomogram(iTargetSlice, pd);
						else pd->parentDoc->dlgReconst.iStatus |= CDLGRECONST_WHEEL;
					}
				}
			}
			else {
				if (zDelta < 0) pd->ProceedImage(1);
				else if (zDelta > 0) pd->ProceedImage(-1);
			}
		}
	} else if (GetKeyState(VK_F2) < 0) {//251205
		if (bCircleLassoEnabled) {
			if (zDelta < 0) iCircleLasso[5] = (iCircleLasso[5] - idelta) > 10 ? (iCircleLasso[5] - idelta) : 10;
			else if (zDelta > 0) iCircleLasso[5] = (iCircleLasso[5] + idelta) < 200 ? (iCircleLasso[5] + idelta) : 200;
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_F3) < 0) {//251205
		if (bCircleLassoEnabled) {
			if (zDelta < 0) iCircleLasso[8] = (iCircleLasso[8] - idelta) > 10 ? (iCircleLasso[8] - idelta) : 10;
			else if (zDelta > 0) iCircleLasso[8] = (iCircleLasso[8] + idelta) < 200 ? (iCircleLasso[8] + idelta) : 200;
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_F4) < 0) {//251205
		if (bCircleLassoEnabled) {
			if (zDelta < 0) iCircleLasso[11] = (iCircleLasso[11] - idelta) > 10 ? (iCircleLasso[11] - idelta) : 10;
			else if (zDelta > 0) iCircleLasso[11] = (iCircleLasso[11] + idelta) < 200 ? (iCircleLasso[11] + idelta) : 200;
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_F5) < 0) {//260110
		if (bCircleLassoEnabled) {
			if (zDelta < 0) iCircleLasso[14] = (iCircleLasso[14] - idelta) > 10 ? (iCircleLasso[14] - idelta) : 10;
			else if (zDelta > 0) iCircleLasso[14] = (iCircleLasso[14] + idelta) < 200 ? (iCircleLasso[14] + idelta) : 200;
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_F6) < 0) {//260110
		if (bCircleLassoEnabled) {
			if (zDelta < 0) iCircleLasso[17] = (iCircleLasso[17] - idelta) > 10 ? (iCircleLasso[17] - idelta) : 10;
			else if (zDelta > 0) iCircleLasso[17] = (iCircleLasso[17] + idelta) < 200 ? (iCircleLasso[17] + idelta) : 200;
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_F7) < 0) {//251205
		if (bCircleLassoEnabled) {
			if (zDelta < 0) {
				for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
					iCircleLasso[j+1] = iCircleLasso[j+1]-idelta < -180 ? iCircleLasso[j+1]-idelta + 360 : iCircleLasso[j+1]-idelta;
				}
			} else if (zDelta > 0) {
				for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
					iCircleLasso[j+1] = iCircleLasso[j+1]+idelta > 180 ? iCircleLasso[j+1]+idelta - 360 : iCircleLasso[j+1]+idelta;
				}
			}
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
		}
	} else if (GetKeyState(VK_CONTROL) < 0) {//251205
		if (bCircleLassoEnabled) {
			if (zDelta < 0) {
				if (iCircleLasso[2] > idelta) iCircleLasso[2] -= idelta;
			} else if (zDelta > 0) {
				iCircleLasso[2] += idelta; //iCircleLasso[3] += idelta;
			}
			dlgCircleLasso.UpdateCurrentCircle();
			InvalidateRect(NULL, FALSE);
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
		//190628==>
		//TReal rMagnify = iMagnify;
		//if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
		//CRect rect;
		//GetClientRect(&rect);
		//CPoint pCenter = rect.CenterPoint();
		//int ixpos = GetScrollPos(SB_HORZ);
		//int iypos = GetScrollPos(SB_VERT);
		//double dx0img = pCenter.x - ixdim * rMagnify * ixpos / CGV_HSCROLL_RANGE;
		//double dy0img = pCenter.y - iydim * rMagnify * iypos / CGV_VSCROLL_RANGE;
		const double dConstX = (pntMouse.x - ix0img) / (ixdim * rMagnify);
		const double dConstY = (pntMouse.y - iy0img) / (iydim * rMagnify);
		int iMagnifyNext = iMagnify;
		bool bScroll = true;
		if (zDelta < 0) {
			iMagnifyNext--;
			if (iMagnifyNext < -100) {iMagnifyNext = -100; bScroll = false;}
		} else if (zDelta > 0) {
			iMagnifyNext++;
			TReal rMagnifyNext = iMagnifyNext;
			if (rMagnifyNext < 1) rMagnifyNext = 1.0 / (2 - rMagnifyNext);
			if (((int)(ixdim * rMagnifyNext) > 32767) || ((int)(iydim * rMagnifyNext) > 32767)) { iMagnifyNext--; bScroll = false; }
		}
		if (bScroll) {
			rMagnify = iMagnifyNext;
			if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
			int ixpos1 = (int)(((double)(pCenter.x - pntMouse.x) / (ixdim * rMagnify) + dConstX) * CGV_HSCROLL_RANGE + 0.5);
			int iypos1 = (int)(((double)(pCenter.y - pntMouse.y) / (iydim * rMagnify) + dConstY) * CGV_VSCROLL_RANGE + 0.5);
			SetScrollPos(SB_HORZ, ixpos1);
			SetScrollPos(SB_VERT, iypos1);
		}
		//==>190628
		if (zDelta < 0) OnToolbarMin();
		else if (zDelta > 0) OnToolbarMag();
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CGazoView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	bRButtonDown = true;
	bool bAppLassoRbtn = false;
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	if (pApp) bAppLassoRbtn = pApp->bLassoRbtn;
	if (bAppLassoRbtn) ButtonDownToLassoEdit(point);
	if (!bPolygonMove && (iPickedBoxPnt < 0) && !bCircleLassoMove && !bCircleLassoEdit) {
		if (bRedLine) {
			bRedLine = false;
		} else {
			bRedLine = true;
			CRect rect;
			GetClientRect(&rect);
			CPoint pCenter = rect.CenterPoint();
			TReal rMagnify = iMagnify;
			if (rMagnify < 1) rMagnify = 1.0 / (2 - rMagnify);
			int ixpos = GetScrollPos(SB_HORZ);
			int iypos = GetScrollPos(SB_VERT);
			int ixWidthImg = (int)(ixdim * rMagnify);
			int iyWidthImg = (int)(iydim * rMagnify);
			int ix0img = pCenter.x - (int)(ixWidthImg * ixpos / CGV_HSCROLL_RANGE);
			int iy0img = pCenter.y - (int)(iyWidthImg * iypos / CGV_VSCROLL_RANGE);
			pntRedFrom.x = (int)((point.x - ix0img) / rMagnify);
			pntRedFrom.y = (int)((point.y - iy0img) / rMagnify);
			pntRedTo = pntRedFrom;
		}
	}
	if (bAppLassoRbtn) {
		CGazoDoc* pd = GetDocument();
		if (pd) pd->dlgHist.UpdateParam();
	}
	InvalidateRect(NULL, FALSE);
	CView::OnRButtonDown(nFlags, point);
}

void CGazoView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	bRButtonDown = false;
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	if (pApp) {
		if (pApp->bLassoRbtn) {
			bPolygonMove = false;
			bCircleLassoMove = false;//251205
			bCircleLassoEdit = false;//251205
			iPickedBoxPnt = -1;//251205
			iPickedPolygonPnt = -1;//251205
		}
	}
	ReleaseCapture();
	SetCursor(hCursor);

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
			//190628 bBoxEnabled = false;
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

void CGazoView::OnAnalysisCirclelasso()//251205
{
	if (bCircleLassoEnabled) {
		if (dlgCircleLasso.IsWindowVisible()) {
			bCircleLassoEnabled = false;
			dlgCircleLasso.ShowWindow(SW_HIDE);
			dlgCircleLasso.MakeCircleList();
		}
		else {
			if (!dlgCircleLasso.m_hWnd) dlgCircleLasso.Create(IDD_CIRCLELASSO);
			dlgCircleLasso.ShowWindow(SW_SHOW); //AfxMessageBox("ShowWindow");
			dlgCircleLasso.UpdateCurrentCircle();
		}
	}
	else {
		bCircleLassoEnabled = true;
		if (bBoxEnabled) {
			InitCircleLasso(iBoxCentX, iBoxCentY, iBoxSizeX, iBoxSizeY);
		}
		if (!dlgCircleLasso.m_hWnd) dlgCircleLasso.Create(IDD_CIRCLELASSO);
		if (dlgCircleLasso.IsWindowVisible()) dlgCircleLasso.SetForegroundWindow();
		else dlgCircleLasso.ShowWindow(SW_SHOW);
		dlgCircleLasso.UpdateCurrentCircle();
	}
	InvalidateRect(NULL, FALSE);
}

void CGazoView::OnUpdateAnalysisCirclelasso(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(bCircleLassoEnabled);
}

void CGazoView::InitCircleLasso(int xcent, int ycent, int xsize, int ysize) {
	iCircleLasso[0] = xcent;
	iCircleLasso[1] = ycent;
	iCircleLasso[2] = (xsize + ysize) / 2;//radius
	for (int j = 3; j < CGAZOVIEW_NPARAMCIRCLELASSO; j += 3) {
		iCircleLasso[j] = 0;//hump
		iCircleLasso[j+1] = 0;//angle
		iCircleLasso[j+2] = 50;//kurtosis
	}
	dlgCircleLasso.UpdateCurrentCircle();
}

bool CGazoView::PointInCircleLasso(CPoint point) {//image coords
	if (!bCircleLassoEnabled) return false;
	CGazoDoc* pd = GetDocument();
	if (!pd) return false;
	return pd->PointInCircleLasso(point.x, point.y, iCircleLasso);
	//int ix = iCircleLasso[0] - point.x;
	//int iy = iCircleLasso[1] - point.y;
	//int ir = iCircleLasso[2];
	//if (ix * ix + iy * iy < ir * ir) return true; else return false;
}

bool CGazoView::PointOnCircleLassoLine(CPoint point) {//image coords
	if (!bCircleLassoEnabled) return false;
	CGazoDoc* pd = GetDocument();
	if (!pd) return false;
	return pd->PointOnCircleLassoLine(point.x, point.y, iCircleLasso);
}

void CGazoView::OnInitialUpdate()
{//190628
	CView::OnInitialUpdate();

	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。
	SetScrollRange(SB_HORZ, 0, CGV_HSCROLL_RANGE);
	SetScrollRange(SB_VERT, 0, CGV_VSCROLL_RANGE);
}
