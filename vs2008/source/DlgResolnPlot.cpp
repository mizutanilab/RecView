// DlgResolnPlot.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgResolnPlot.h"


// CDlgResolnPlot ダイアログ

IMPLEMENT_DYNAMIC(CDlgResolnPlot, CDialog)

CDlgResolnPlot::CDlgResolnPlot(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgResolnPlot::IDD, pParent)
	, m_sReport(_T(""))
	, m_sYLow(_T(""))
	, m_sYHigh(_T(""))
	, m_bPlotR(TRUE)
	, m_bPlotG(TRUE)
	, m_bPlotB(TRUE)
	, m_dPixelWidth(0.5)
{
	m_dLeft = 0;
	m_dRight = 0.05;
	m_dLow = 0;
	m_dHigh = 30;
	m_CursorRight = _T("0.03");
	m_CursorLeft = _T("0.001");
	m_psResolnList = NULL;
	m_iMaxResolnList = 0;
	bLButtonDownHigh = false;
	bLButtonDownLow = false;
	iXMouse = 0;
	iYMouse = 0;
	bInvalidate = false;
	bool m_bColor = false;
	m_sFileName.Empty();
	m_piDim[0] = 0; m_piDim[1] = 0; m_piDim[2] = 0; m_piDim[3] = 0;

	hBitmap = NULL;
	pBitmapPix = NULL;
	pPlotPix = NULL;
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
	//blue
	(lpBmpInfo->bmiColors)[2].rgbBlue = 255;
	(lpBmpInfo->bmiColors)[2].rgbGreen = 0;
	(lpBmpInfo->bmiColors)[2].rgbRed = 0;
	//green
	(lpBmpInfo->bmiColors)[3].rgbBlue = 0;
	(lpBmpInfo->bmiColors)[3].rgbGreen = 128;
	(lpBmpInfo->bmiColors)[3].rgbRed = 0;
	//red
	(lpBmpInfo->bmiColors)[4].rgbBlue = 0;
	(lpBmpInfo->bmiColors)[4].rgbGreen = 0;
	(lpBmpInfo->bmiColors)[4].rgbRed = 255;
	//cyan
	(lpBmpInfo->bmiColors)[5].rgbBlue = 255;
	(lpBmpInfo->bmiColors)[5].rgbGreen = 255;
	(lpBmpInfo->bmiColors)[5].rgbRed = 0;
	//gray
	(lpBmpInfo->bmiColors)[6].rgbBlue = 192;
	(lpBmpInfo->bmiColors)[6].rgbGreen = 192;
	(lpBmpInfo->bmiColors)[6].rgbRed = 192;
	for (int i=7; i<BITMAP_NCOLORS; i++) {
		(lpBmpInfo->bmiColors)[i].rgbBlue = i;
		(lpBmpInfo->bmiColors)[i].rgbGreen = i;
		(lpBmpInfo->bmiColors)[i].rgbRed = i;
	}
}

CDlgResolnPlot::~CDlgResolnPlot()
{
	if (lpBmpInfo) HeapFree(GetProcessHeap(),0,lpBmpInfo);
	if (pPlotPix) delete [] pPlotPix;
	if (pBitmapPix) delete [] pBitmapPix;
	if (hBitmap) DeleteObject(hBitmap);
}

void CDlgResolnPlot::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESOLNPLOT_BITMAP, m_Bitmap);
	DDX_Text(pDX, IDC_RESOLNPLOT_MSG, m_sReport);
	DDX_Text(pDX, IDC_RESOLNPLOT_YLOW, m_sYLow);
	DDX_Text(pDX, IDC_RESOLNPLOT_YHIGH, m_sYHigh);
	DDX_Check(pDX, IDC_RESOLNPLOT_PLOTR, m_bPlotR);
	DDX_Check(pDX, IDC_RESOLNPLOT_PLOTG, m_bPlotG);
	DDX_Check(pDX, IDC_RESOLNPLOT_PLOTB, m_bPlotB);
	DDX_Text(pDX, IDC_RESOLNPLOT_PIXELWIDTH, m_dPixelWidth);
}


BEGIN_MESSAGE_MAP(CDlgResolnPlot, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(UDN_DELTAPOS, IDC_RESOLNPLOT_MAG, &CDlgResolnPlot::OnDeltaposResolnplotMag)
	ON_BN_CLICKED(IDC_RESOLNPLOT_PLOTR, &CDlgResolnPlot::OnBnClickedResolnplotPlotR)
	ON_BN_CLICKED(IDC_RESOLNPLOT_PLOTG, &CDlgResolnPlot::OnBnClickedResolnplotPlotG)
	ON_BN_CLICKED(IDC_RESOLNPLOT_PLOTB, &CDlgResolnPlot::OnBnClickedResolnplotPlotB)
	ON_BN_CLICKED(IDC_RESOLNPLOT_UPDATE, &CDlgResolnPlot::OnBnClickedResolnplotUpdate)
	ON_BN_CLICKED(IDC_RESOLNPLOT_CSV, &CDlgResolnPlot::OnBnClickedResolnplotCsv)
END_MESSAGE_MAP()


// CDlgResolnPlot メッセージ ハンドラ

BOOL CDlgResolnPlot::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_psResolnList) {
		m_dLeft = 0;
		m_dRight = 0.05;
		m_dLow = m_psResolnList[0].dLogMod[0];
		m_dHigh = m_dLow;
		for (int i=0; i<m_iMaxResolnList; i++) {
			if (fabs(m_psResolnList[i].dDistance2) < 1E-8) continue;
			if (m_bColor) {
				for (int n=0; n<3; n++) {
					double dy = m_psResolnList[i].dLogMod[n];
					m_dLow = dy < m_dLow ? dy : m_dLow;
					m_dHigh = dy > m_dHigh ? dy : m_dHigh;
				}
			} else {
				m_dLow = m_psResolnList[i].dLogMod[0] < m_dLow ? m_psResolnList[i].dLogMod[0] : m_dLow;
				m_dHigh = m_psResolnList[i].dLogMod[0] > m_dHigh ? m_psResolnList[i].dLogMod[0] : m_dHigh;
			}
		}
	}
	UpdateHistogram();
	UpdateCursorAndRegression();
	UpdateData(FALSE);
	EnableCtrl();
	UpdateBitmap();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgResolnPlot::UpdateHistogram() {
	if (AllocBitmap()) return;
	UpdateData();//080315
	UpdateReport();
	const int iWidth = CDLGRESOLNPLOT_BITMAP_WIDTH;
	const int iHeight = CDLGRESOLNPLOT_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	//
	for (int i=0; i<npix; i++) {pPlotPix[i] = 0;}
	for (int i=0; i<m_iMaxResolnList; i++) {
		double dx = m_psResolnList[i].dDistance2;
		if (fabs(dx) < 1E-8) continue;
		int ix = (int)((dx - m_dLeft) / (m_dRight - m_dLeft) * CDLGRESOLNPLOT_BITMAP_WIDTH);
		if ((ix < 0)||(ix >= CDLGRESOLNPLOT_BITMAP_WIDTH)) continue;
		if (m_bColor) {
			for (int n=0; n<3; n++) {
				double dy = m_psResolnList[i].dLogMod[n];
				int iy = (int)((dy - m_dLow) / (m_dHigh - m_dLow) * CDLGRESOLNPLOT_BITMAP_HEIGHT);
				if ((iy < 0)||(iy >= CDLGRESOLNPLOT_BITMAP_HEIGHT)) continue;
				pPlotPix[ix + iWidth * iy] = n+2;
			}
		} else {
			double dy = m_psResolnList[i].dLogMod[0];
			int iy = (int)((dy - m_dLow) / (m_dHigh - m_dLow) * CDLGRESOLNPLOT_BITMAP_HEIGHT);
			if ((iy < 0)||(iy >= CDLGRESOLNPLOT_BITMAP_HEIGHT)) continue;
			pPlotPix[ix + iWidth * iy] = 1;
		}
	}
	CopyHistogram();
	UpdateData(FALSE);
}

TErr CDlgResolnPlot::AllocBitmap() {
	const int iWidth = CDLGRESOLNPLOT_BITMAP_WIDTH;
	const int iHeight = CDLGRESOLNPLOT_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	if (pBitmapPix) return 0;
	try {
		pBitmapPix = new BYTE[npix];
		pPlotPix = new int[npix];
	}
	catch (CException* e) {
		if (pBitmapPix) delete [] pBitmapPix;
		if (pPlotPix) delete [] pPlotPix;
		pBitmapPix = NULL;
		pPlotPix = NULL;
		e->Delete(); return 22000;
	}
	return 0;
}

void CDlgResolnPlot::CopyHistogram() {
	const int iWidth = CDLGRESOLNPLOT_BITMAP_WIDTH;
	const int iHeight = CDLGRESOLNPLOT_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	for (int i=0; i<npix; i++) {pBitmapPix[i] = pPlotPix[i];}
	//zero line
	//int iZero = GetCursorPnt("0.0");
	//if ((iZero >= 0)&&(iZero < iWidth)) {
	//	int icolor = 0;
	//	for (int i=0; i<iHeight; i++) {
	//		if (i % 3 == 0) icolor = icolor ? 0 : 1;
	//		pBitmapPix[iZero + i * iWidth] = icolor;
	//	}
	//}
}

int CDlgResolnPlot::GetCoordX(double dx) {
	int irtn = (int)( (dx - m_dLeft) * (CDLGRESOLNPLOT_BITMAP_WIDTH - 1) / (m_dRight - m_dLeft) );
	if (irtn < 0) irtn = 0;
	else if (irtn >= CDLGRESOLNPLOT_BITMAP_WIDTH) irtn = CDLGRESOLNPLOT_BITMAP_WIDTH - 1;
	return irtn;
}

int CDlgResolnPlot::GetCoordY(double dy) {
	int irtn = (int)( (dy - m_dLow) * (CDLGRESOLNPLOT_BITMAP_HEIGHT - 1) / (m_dHigh - m_dLow) );
	if (irtn < 0) irtn = 0;
	else if (irtn >= CDLGRESOLNPLOT_BITMAP_HEIGHT) irtn = CDLGRESOLNPLOT_BITMAP_HEIGHT - 1;
	return irtn;
}

void CDlgResolnPlot::UpdateCursorAndRegression() {
	int iCursorLeft = GetCoordX(atof(m_CursorLeft));
	int iCursorRight = GetCoordX(atof(m_CursorRight));

	for (int i=0; i<CDLGRESOLNPLOT_BITMAP_HEIGHT; i++) {
		pBitmapPix[iCursorLeft + i * CDLGRESOLNPLOT_BITMAP_WIDTH] = 5;//cursor color
		pBitmapPix[iCursorRight + i * CDLGRESOLNPLOT_BITMAP_WIDTH] = 5;
	}
	for (int i=0; i<CDLGRESOLNPLOT_BITMAP_WIDTH; i++) {
		double dx = m_dLeft + (m_dRight - m_dLeft) * i / (CDLGRESOLNPLOT_BITMAP_WIDTH - 1);
		int iy = GetCoordY(m_dYSection + dx * m_dSlope);
		if ((iy > 0)&&(iy < CDLGRESOLNPLOT_BITMAP_HEIGHT-1)) 
			pBitmapPix[i + iy * CDLGRESOLNPLOT_BITMAP_WIDTH] = 6;//regression color
	}
}

void CDlgResolnPlot::EnableCtrl() {
	if (m_bColor) {
		GetDlgItem(IDC_RESOLNPLOT_PLOTR)->EnableWindow(TRUE);
		GetDlgItem(IDC_RESOLNPLOT_PLOTG)->EnableWindow(TRUE);
		GetDlgItem(IDC_RESOLNPLOT_PLOTB)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESOLNPLOT_PLOTR)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESOLNPLOT_PLOTG)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESOLNPLOT_PLOTB)->EnableWindow(FALSE);
	}
	//GetDlgItem(IDC_HISTG_TRMCENTX)->EnableWindow(FALSE);
}

void CDlgResolnPlot::UpdateBitmap() {
	const int iWidth = CDLGRESOLNPLOT_BITMAP_WIDTH;
	const int iHeight = CDLGRESOLNPLOT_BITMAP_HEIGHT;
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
	bh->biXPelsPerMeter = 10000; 
	bh->biYPelsPerMeter = 10000; 
	bh->biClrUsed = BITMAP_NCOLORS; 
	bh->biClrImportant = 0; 
	hBitmap = CreateDIBitmap(this->GetDC()->m_hDC, &(lpBmpInfo->bmiHeader), CBM_INIT, pBitmapPix,
																lpBmpInfo, DIB_RGB_COLORS);
	if (hBitmap) m_Bitmap.SetBitmap(hBitmap);
}

void CDlgResolnPlot::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int iLowCursor = GetCoordX(atof(m_CursorLeft));
	int iHighCursor = GetCoordX(atof(m_CursorRight));
	POINT pnt = point;
	MapWindowPoints(GetDlgItem(IDC_RESOLNPLOT_BITMAP), &pnt, 1);
	if ((pnt.x >= 0)&&(pnt.x < CDLGRESOLNPLOT_BITMAP_WIDTH)&&
			(pnt.y >= 0)&&(pnt.y < CDLGRESOLNPLOT_BITMAP_HEIGHT)) {
		if (abs(iLowCursor - pnt.x) < CDLGRESOLNPLOT_BITMAP_NEAR) {
			bLButtonDownLow = true;
			bLButtonDownHigh = false;
			iXMouse = pnt.x;
			iYMouse = pnt.y;
			SetCapture();
		} else if (abs(iHighCursor - pnt.x) < CDLGRESOLNPLOT_BITMAP_NEAR) {
			bLButtonDownLow = false;
			bLButtonDownHigh = true;
			iXMouse = pnt.x;
			iYMouse = pnt.y;
			SetCapture();
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgResolnPlot::OnLButtonUp(UINT nFlags, CPoint point) 
{
	bLButtonDownHigh = false;
	bLButtonDownLow = false;
	ReleaseCapture();
	if (bInvalidate) {
		bInvalidate = false;
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgResolnPlot::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (bLButtonDownLow || bLButtonDownHigh) {
		UpdateData();
		int iLowCursor = GetCoordX(atof(m_CursorLeft));
		int iHighCursor = GetCoordX(atof(m_CursorRight));
		POINT pnt = point;
		MapWindowPoints(GetDlgItem(IDC_RESOLNPLOT_BITMAP), &pnt, 1);
		//bool bInvalidate = false;
		if (bLButtonDownLow) {
			iLowCursor = pnt.x;
			if (iLowCursor < 0) iLowCursor = 0;
			//else if (iLowCursor >= CDLGHISTG_BITMAP_WIDTH) iLowCursor = CDLGHISTG_BITMAP_WIDTH - 1;
			else if (iLowCursor >= iHighCursor) iLowCursor = iHighCursor - 1;
			m_CursorLeft = GetIntensityFromCursor(iLowCursor);
			bInvalidate = true;
			//CString line; line.Format("%d %s", iLowCursor, m_CursorLeft); AfxMessageBox(line);
		} else if (bLButtonDownHigh) {
			iHighCursor = pnt.x;
			//if (iHighCursor < 0) iHighCursor = 0;
			if (iHighCursor >= CDLGRESOLNPLOT_BITMAP_WIDTH) iHighCursor = CDLGRESOLNPLOT_BITMAP_WIDTH - 1;
			else if (iHighCursor <= iLowCursor) iHighCursor = iLowCursor + 1;
			m_CursorRight = GetIntensityFromCursor(iHighCursor);
			bInvalidate = true;
		}
		iXMouse = pnt.x;
		iYMouse = pnt.y;
		if (bInvalidate) {
			UpdateReport();
			CopyHistogram();
			UpdateCursorAndRegression();
			UpdateBitmap();
			UpdateData(FALSE);
			GetDlgItem(IDC_RESOLNPLOT_BITMAP)->Invalidate();
		}
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CDlgResolnPlot::UpdateReport() {
	m_sYLow.Format("%.2f", m_dLow);
	m_sYHigh.Format("%.2f", m_dHigh);
	CString line;
	double dSqrtRight = sqrt(m_dRight);
	line.Format("Graph width\r\n %.5f pixel(-2)\r\n %.5f pixel(-1) \r\n %.2f pixel\r\n",
		m_dRight, dSqrtRight, 1./dSqrtRight);
	m_sReport = line;
	double dSqrtCurLeft = sqrt(atof(m_CursorLeft));
	double dSqrtCurRight = sqrt(atof(m_CursorRight));
	line.Format("Cursor left\r\n %.5f pixel(-2)\r\n %.5f pixel(-1) \r\n %.2f pixel\r\n", 
		atof(m_CursorLeft), dSqrtCurLeft, 1./dSqrtCurLeft);
	m_sReport += line;
	line.Format("Cursor right\r\n %.5f pixel(-2)\r\n %.5f pixel(-1) \r\n %.2f pixel\r\n", 
		atof(m_CursorRight), dSqrtCurRight, 1./dSqrtCurRight);
	m_sReport += line;
	GetSlope();
	line.Format("Slope\r\n %.1f pixel(-2)\r\n", m_dSlope);
	m_sReport += line;
	line.Format("Pixel width\r\n %.6f um\r\n", m_dPixelWidth);
	m_sReport += line;
	double dResoln = sqrt(-1.38629436 * m_dSlope) / __PI;
	line.Format("Resolution\r\n %.1f pixel\r\n %.5f um", dResoln, dResoln * m_dPixelWidth);
	m_sReport += line;
	if (dResoln < 2.0) m_sReport += "\r\n !Below Nyquist limit!";
}

CString CDlgResolnPlot::GetIntensityFromCursor(int ipos) {
	double dens = ipos * (m_dRight - m_dLeft) / (CDLGRESOLNPLOT_BITMAP_WIDTH - 1) + m_dLeft;
	CString rtn;
	rtn.Format("%f", dens);
	return rtn;
}

void CDlgResolnPlot::OnDeltaposResolnplotMag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	const int iWidth = CDLGRESOLNPLOT_BITMAP_WIDTH;
	const int iHeight = CDLGRESOLNPLOT_BITMAP_HEIGHT;
	const int npix = iWidth * iHeight;
	int idlt = pNMUpDown->iDelta; //left = 1 and right = -1

	if (idlt < 0) m_dRight = m_dRight * 0.8;
	else m_dRight = m_dRight * 1.25;
	UpdateHistogram();
	UpdateCursorAndRegression();
	UpdateBitmap();
	GetDlgItem(IDC_RESOLNPLOT_BITMAP)->Invalidate();

	*pResult = 0;
}

void CDlgResolnPlot::GetSlope() {
	const double dCurLeft = atof(m_CursorLeft);
	const double dCurRight = atof(m_CursorRight);
	double dsumx = 0, dsumx2 = 0, dsumy = 0, dsumxy = 0;
	int icount = 0;
	for (int i=0; i<m_iMaxResolnList; i++) {
		double dx = m_psResolnList[i].dDistance2;
		if (fabs(dx) < 1E-8) continue;
		if ((dx < dCurLeft)||(dx > dCurRight)) continue;
		if (m_bColor) {
			for (int n=0; n<3; n++) {
				switch (n) {
					case 0: {if (!m_bPlotB) continue; break;}
					case 1: {if (!m_bPlotG) continue; break;}
					case 2: {if (!m_bPlotR) continue; break;}
				}
				double dy = m_psResolnList[i].dLogMod[n];
				dsumx += dx;
				dsumx2 += (dx * dx);
				dsumy += dy;
				dsumxy += (dx * dy);
				icount++;
			}
		} else {
			double dy = m_psResolnList[i].dLogMod[0];
			dsumx += dx;
			dsumx2 += (dx * dx);
			dsumy += dy;
			dsumxy += (dx * dy);
			icount++;
		}
	}
	m_dSlope = (dsumx * dsumx) - icount * dsumx2;
	m_dSlope = (m_dSlope == 0) ? 0 : ((dsumx * dsumy) - icount * dsumxy) / m_dSlope;
	m_dYSection = (icount == 0) ? 0 : ((dsumy - dsumx * m_dSlope) / icount);
}

void CDlgResolnPlot::OnBnClickedResolnplotPlotR()
{
	UpdateHistogram();
	UpdateCursorAndRegression();
	UpdateBitmap();
	GetDlgItem(IDC_RESOLNPLOT_BITMAP)->Invalidate();
}

void CDlgResolnPlot::OnBnClickedResolnplotPlotG()
{
	OnBnClickedResolnplotPlotR();
}

void CDlgResolnPlot::OnBnClickedResolnplotPlotB()
{
	OnBnClickedResolnplotPlotR();
}

void CDlgResolnPlot::OnBnClickedResolnplotUpdate()
{
	UpdateData();
	UpdateReport();
	UpdateData(FALSE);
}

void CDlgResolnPlot::OnBnClickedResolnplotCsv()
{
	static char BASED_CODE szFilter[] = "csv files (*.csv)|*.csv|text files (*.txt)|*.txt|All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = ".csv";
	CFileDialog fileDlg(FALSE, defaultExt, m_sFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) return;
	m_sFileName = fileDlg.GetPathName();
	CStdioFile flog;
	if (!flog.Open(m_sFileName, CFile::modeReadWrite | CFile::modeCreate | CFile::shareDenyWrite | CFile::typeText)) {
		return;
	}

	flog.WriteString("Resolution estimation in the Fourier domain\n");
	flog.WriteString(" Open this file with a spread sheet software.\n");
	flog.WriteString(" Plot y=(column B) vs x=(column A) as a scatter chart and find linear regression.\n");
	flog.WriteString(" Then estimate resoln with: (pixel width) * sqrt(-slope * 2 * ln2) / pi.\n");
	flog.WriteString(" Ref: R. Mizutani et al. J. Microsc. 261 57-66 (2016).\n");
	CString line;
	line.Format("Original image, %d x %d\nFFT dimensions, %d x %d\n", m_piDim[0], m_piDim[1], m_piDim[2], m_piDim[3]);
	flog.WriteString(line);
	flog.WriteString("Binning, 5 x 5 pixels\nOmit pixels around axis, 10 pixels\n----\n");
	flog.WriteString("Slope, Enter value here\n");
	flog.WriteString("Pixel width, Enter value here, um\n");
	flog.WriteString("Gaussian sigma, =SQRT(-B11)/(2*3.14159)\n");
	flog.WriteString("Resolution, =B13*2*SQRT(2*LN(2))*B12, um\n");
	flog.WriteString("-----\nFollowing are outputs of the implemented regression function\n");
	CString sMsg = m_sReport;
	sMsg.Remove('\r');
	flog.WriteString(sMsg + "\n-----\n");
	if (m_bColor) flog.WriteString("Distance^2, log(Modulus^2)blue, green, red\n");
	else flog.WriteString("Distance^2, log(Modulus^2)\n");
	for (int i=0; i<m_iMaxResolnList; i++) {
		double dDist = m_psResolnList[i].dDistance2;
		if (fabs(dDist) < 1E-8) continue;
		CString line;
		if (m_bColor) 
			line.Format("%.10f, %.4f, %.4f, %.4f\n", 
				dDist, m_psResolnList[i].dLogMod[0], m_psResolnList[i].dLogMod[1], m_psResolnList[i].dLogMod[2]);
		else line.Format("%.10f, %.4f\n", dDist, m_psResolnList[i].dLogMod[0]);
		flog.WriteString(line);
	}
	flog.Close();
}
