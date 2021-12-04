// gazoDoc.cpp : CGazoDoc クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "gazo.h"

#include <math.h>
#include <sys\timeb.h> //_timeb, _ftime
#include <process.h> //_beginthread
#include <float.h> // FLT_MAX
#include "gazoDoc.h"
#include "gazoView.h"
#include "MainFrm.h"
#include "DlgReconst.h"
#include "cerror.h"
#include "cfft.h"
#include "ChildFrm.h"
#include "DlgHistogram.h"
#include "DlgQueue.h"
#include "DlgMessage.h"
#include "DlgProperty.h"
#include "DlgReconOpt.h"
#include "DlgHorizcenter.h"
//#include "DlgLsqfit.h"
#include "DlgGeneral.h"
#include "DlgOverlay.h"
#include "DlgResolnPlot.h"
#include "DlgFrameList.h"

//CUDA declaration
#include "cudaReconst.h"
//#ifdef _MSC_VER
//	#if _MSC_VER >=1910 //VS2017 or later
//		#ifdef _WIN64
//			#include <cuda_runtime.h>
//		#else
//			extern "C" cudaError_t cudaFreeHost(void* ptr);//190110
//			extern "C" cudaError_t cudaHostAlloc(void** ptr, size_t size, unsigned int flags);//190110
//			extern "C" cudaError_t cudaMallocHost(int** ptr, size_t size);
//			extern "C" cudaError_t cudaStreamDestroy(cudaStream_t stream);//190707
//			extern "C" cudaError_t cudaSetDevice(int device);//190710
//			extern "C" cudaError_t cudaDeviceSynchronize(void);
//			extern "C" cudaError_t cudaDeviceReset(void);
//		#endif
//	#else
//		#include <cuda_runtime.h>
//	#endif
//#else
//	#include <cuda_runtime.h>
//#endif

//OpenCL
#include "clReconst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGazoDoc
//extern int iThreadStatus;
//extern CGazoDoc* pDocThread;

IMPLEMENT_DYNCREATE(CGazoDoc, CDocument)

BEGIN_MESSAGE_MAP(CGazoDoc, CDocument)
	//{{AFX_MSG_MAP(CGazoDoc)
	ON_COMMAND(ID_TOOLBAR_BRIGHT, OnToolbarBright)
	ON_COMMAND(ID_TOOLBAR_DARK, OnToolbarDark)
	ON_COMMAND(ID_TOOLBAR_CNTDOWN, OnToolbarCntdown)
	ON_COMMAND(ID_TOOLBAR_CNTUP, OnToolbarCntup)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_BRIGHT, OnUpdateToolbarBright)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_DARK, OnUpdateToolbarDark)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_CNTDOWN, OnUpdateToolbarCntdown)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_CNTUP, OnUpdateToolbarCntup)
	ON_COMMAND(IDM_TOMO_RECONST, OnTomoReconst)
	ON_COMMAND(IDM_HLP_DEBUG, OnHlpDebug)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_HISTG, OnUpdateTomoHistg)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_RECONST, OnUpdateTomoReconst)
	ON_COMMAND(IDM_TOMO_HISTG, OnTomoHistg)
	ON_COMMAND(ID_TOOLBAR_FNFWDONE, OnToolbarFnfwdOne)
	ON_COMMAND(ID_TOOLBAR_FNREVONE, OnToolbarFnrevOne)
	ON_COMMAND(ID_TOOLBAR_FNFWD, OnToolbarFnfwd)
	ON_COMMAND(ID_TOOLBAR_FNFF, OnToolbarFnff)
	ON_COMMAND(ID_TOOLBAR_FNREV, OnToolbarFnrev)
	ON_COMMAND(ID_TOOLBAR_FNFR, OnToolbarFnfr)
	ON_UPDATE_COMMAND_UI(IDM_HLP_DEBUG, OnUpdateHlpDebug)
	ON_COMMAND(IDM_TOMO_AXIS, OnTomoAxis)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_AXIS, OnUpdateTomoAxis)
	ON_COMMAND(ID_TOOLBAR_FNSF, OnToolbarFnsf)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_FNSF, OnUpdateToolbarFnsf)
	ON_COMMAND(ID_TOOLBAR_FNSR, OnToolbarFnsr)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_FNSR, OnUpdateToolbarFnsr)
	ON_COMMAND(ID_TOOLBAR_PLPAUSE, OnToolbarFnPause)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_PLPAUSE, OnUpdateToolbarFnPause)
	ON_COMMAND(IDM_TOMO_STAT, OnTomoStat)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_STAT, OnUpdateTomoStat)
	ON_COMMAND(IDM_TOMO_LINE, OnTomoLine)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_LINE, OnUpdateTomoLine)
	//}}AFX_MSG_MAP
//	ON_COMMAND(IDM_TOMO_PROPERTY, &CGazoDoc::OnTomoProperty)
//	ON_UPDATE_COMMAND_UI(IDM_TOMO_PROPERTY, &CGazoDoc::OnUpdateTomoProperty)
ON_COMMAND(IDM_TOMO_REFRAC, &CGazoDoc::OnTomoRefrac)
ON_UPDATE_COMMAND_UI(IDM_TOMO_REFRAC, &CGazoDoc::OnUpdateTomoRefrac)
//ON_COMMAND(IDM_TOMO_LSQFIT, &CGazoDoc::OnTomoLsqfit)
ON_COMMAND(IDM_TOMO_COMMENT, &CGazoDoc::OnTomoComment)
ON_COMMAND(IDM_TOMO_HORIZCENT, &CGazoDoc::OnTomoHorizcent)

	ON_COMMAND(ID_TOOLBAR_FNJUMPR, OnToolbarFnJumpR)
	ON_COMMAND(ID_TOOLBAR_FNJUMPF, OnToolbarFnJumpF)

	ON_COMMAND(ID_TOMOGRAPHY_RESOLUTIONREPORT, &CGazoDoc::OnTomographyResolutionReport)
	ON_COMMAND(IDM_TOMO_FOURIER, &CGazoDoc::OnTomoFourier)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_FOURIER, &CGazoDoc::OnUpdateTomoFourier)
	ON_COMMAND(ID_TOMOGRAPHY_GAUSSIANCONVOLUTION, &CGazoDoc::OnTomographyGaussianconvolution)
	ON_COMMAND(ID_ANALYSIS_ADDNOISE, &CGazoDoc::OnAnalysisAddnoise)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_ADDNOISE, &CGazoDoc::OnUpdateAnalysisAddnoise)
	ON_COMMAND(IDM_OVERLAY, &CGazoDoc::OnMenuOverlay)
	ON_UPDATE_COMMAND_UI(IDM_OVERLAY, &CGazoDoc::OnUpdateMenuOverlay)
	ON_COMMAND(ID_ANALYSIS_ENLARGE, &CGazoDoc::OnAnalysisEnlarge)
	ON_COMMAND(ID_ANALYSIS_RADIALPROFILE, &CGazoDoc::OnAnalysisRadialprofile)
	ON_COMMAND(ID_TOOLBAR_HISTG, &CGazoDoc::OnTomoHistg)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_HISTG, &CGazoDoc::OnUpdateTomoHistg)
	ON_COMMAND(ID_ANALYSIS_SUBTRACT, &CGazoDoc::OnAnalysisSubtract)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_SUBTRACT, &CGazoDoc::OnUpdateAnalysisSubtract)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGazoDoc クラスの構築/消滅

extern CError error;

CGazoDoc::CGazoDoc()
{
	// TODO: この位置に１度だけ呼ばれる構築用のコードを追加してください。
	InitAll();
}

void CGazoDoc::ClearAll() {
	ixdim = 0;
	iydim = 0;
	ioxdim = 0;
	ioydim = 0;
	izOverlay = 0;
	fileComment = "(not defined)";
	iPixelMax = 0;
	iPixelMin = INT_MAX;
	iDispLow = 0;
	iDispHigh = USHRT_MAX;
	iLenSinogr = 0;
	iCurrSinogr = -1;
	iCurrTrim = -1;
	numOfSampleSinogr = 0;
	parentDoc = NULL;
	pixDiv = 0;
	pixBase = 0;
	logPath = "";
	dataPath = "";
	dataPrefix = "q";
	dataSuffix = ".img";
	fCenter = 0;
	nReconst = 0;
	bCmdLine = false;
	bFromQueue = false;
	//bUnderCalc = false;
	bDebug = false;
	dAxisCenter = 0;
	dAxisGrad = 0;
	maxHisFrame = 0;
	iFramePerDataset = -1;
	nDarkFrame = 0;
	dlgReconst.m_nDataset = 1;
	//210618 iLossFrameSet = -1;
	ullLossFrameSet = 0;//210618
	for (int i=0; i<MAX_CPU; i++) {
		ri[i].hThread = NULL;
		ri[i].iStatus = RECONST_INFO_IDLE;
		ri[i].iStartSino = i;
		//ri[i].bMaster = false;
		ri[i].pDoc = (unsigned int*)this;
		ri[i].piDrift = NULL;
		ri[i].stream1 = NULL;//190529
		ri[i].stream2 = NULL;
	}
	//ri[0].bMaster = true;
	uiDocStatus = CGAZODOC_STATUS_RESET;
	m_iFlag = 0;
	bColor = false;
	m_sFramesToExclude.Empty();
	m_lHDF5DataSize0 = 0;
	m_sHisAvgFiles.Empty();//201125
}

void CGazoDoc::InitAll() {
	ClearAll();
	pPixel = NULL;
	maxPixel = 0;
	for (int i=0; i<CGAZODOC_MAXOVERLAY; i++) {
		ppOverlay[i] = NULL;
		pMaxOverlay[i] = 0;
	}
	iSinogr = NULL;
	maxSinogrLen = 0;
	maxSinogrWidth = 0;
	ofSinogr = NULL;
	maxOfSinogrLen = 0;
	maxOfSinogrWidth = 0;
	fname = NULL;
	fexp = NULL;
	fdeg = NULL;
	bInc = NULL;
	maxImageEntry = 0;
	iReconst = NULL;
	maxReconst = 0;
	dlgReconst.SetDoc(this);
	dlgRefraction.SetDoc(this);
	dlgHist.SetDoc(this);//121127
	dlgOverlay.SetDoc(this);//151012
	fFilter = NULL;
	maxFilter = 0;
	convList = NULL;
	maxConvList = 0;
	for (int i=0; i<MAX_CPU; i++) {
		////CUDA
		ri[i].d_ifp = NULL;
		ri[i].max_d_ifp = 0;
		ri[i].d_p = NULL;
		ri[i].max_d_p = 0;
		ri[i].d_px = NULL;
		ri[i].max_d_px = 0;
		ri[i].d_filt = NULL;
		ri[i].max_d_filt = 0;
		ri[i].d_strip = NULL;
		ri[i].max_d_strip = 0;
		ri[i].d_igp = NULL;
		ri[i].max_d_igp = 0;
		ri[i].fftplan = NULL;
		ri[i].ifftdim = 0;
//		ri[i].d_fcos = NULL;
//		ri[i].d_fsin = NULL;
//		ri[i].max_d_fcos = 0;
	}
	//190121 ri[0].bMaster = true;
}

CGazoDoc::~CGazoDoc() {
	DeleteAll();
}

void CGazoDoc::DeleteAll() 
{
	//AfxMessageBox("DeleteAll");//120723
	if (pPixel) delete [] pPixel;
	for (int i=0; i<CGAZODOC_MAXOVERLAY; i++) {
		if (ppOverlay[i]) delete [] ppOverlay[i];
	}
	if (iReconst) delete [] iReconst;
	if (iSinogr) {
		for (int i=0; i<maxSinogrLen; i++) {if (iSinogr[i]) delete [] iSinogr[i];}
		delete [] iSinogr;
	}
	if (ofSinogr) {
		for (int i=0; i<maxOfSinogrLen; i++) {if (ofSinogr[i]) delete [] ofSinogr[i];}
		delete [] ofSinogr;
	}
	if (fexp) delete [] fexp;
	if (fdeg) delete [] fdeg;
	if (bInc) delete [] bInc;
	if (convList) delete [] convList;
	if (fFilter) delete [] fFilter;
	if (fname) delete [] fname;
	if (dlgReconst.m_hWnd) dlgReconst.DestroyWindow();
	if (dlgRefraction.m_hWnd) dlgRefraction.DestroyWindow();
	if (dlgHist.m_hWnd) dlgHist.DestroyWindow();
	if (dlgOverlay.m_hWnd) dlgOverlay.DestroyWindow();

	GPUMemFree();//191001

	InitAll();

	//100515
	//for (int i=0; i<MAX_CPU; i++) {
	//	CudaReconstMemFree(&(ri[i]));
	//}
	//190107
	//191001 GPUMemFree();
	//CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	//if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
	//	int nCPU = (int)(pApp->dlgProperty.iCUDA);
	//	for (int i = 0; i < nCPU; i++) { CudaReconstMemFree(&(ri[i])); }
	//}
	//else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
	//	int nCPU = (int)(pApp->dlgProperty.iATIstream);
	//	for (int i = 0; i < nCPU; i++) { CLReconstMemFree(&(ri[i])); }
	//}
}

void CGazoDoc::GPUMemFree(int iProcessorType, bool bCudaDeviceReset) {
	CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	iProcessorType = (iProcessorType == CDLGPROPERTY_PROCTYPE_ND) ? pApp->dlgProperty.m_ProcessorType : iProcessorType;
	if (iProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		int nCPU = (int)(pApp->dlgProperty.iCUDA);
		for (int i = 0; i < nCPU; i++) { 
			//CString line; line.Format("191001 CudaResourceFree %d %d %s", ri, i, bCudaDeviceReset ? "reset" : "no reset");
			//AfxMessageBox(line);
			CudaReconstResourceFree(&(ri[i]), bCudaDeviceReset);//190710
			//if (ri[i].stream1) { cudaStreamDestroy(ri[i].stream1); ri[i].stream1 = NULL; }//190529
			//if (ri[i].stream2) { cudaStreamDestroy(ri[i].stream2); ri[i].stream2 = NULL; }//190529
			//CudaReconstMemFree(&(ri[i]));
			//if (bCudaDeviceReset) {//190708
			//	cudaSetDevice(ri[i].iStartSino);
			//	cudaDeviceSynchronize();
			//	cudaDeviceReset();
			//}
		}
	}
	else if (iProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
		int nCPU = (int)(pApp->dlgProperty.iATIstream);
		for (int i = 0; i < nCPU; i++) { CLReconstMemFree(&(ri[i])); }
	}
}

BOOL CGazoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)

	return TRUE;
}

void CGazoDoc::GetDimension(int* ix, int* iy) {
	if ((ixdim > 0)&&(iydim > 0)) {*ix = ixdim; *iy = iydim;}
	else {*ix = 0; *iy = 0;}
}
void CGazoDoc::SetDimension(int ix, int iy) {ixdim = ix; iydim = iy;}

CString CGazoDoc::GetLogPath() {return logPath;}
CString CGazoDoc::GetDataPath() {return dataPath;}
CString CGazoDoc::GetDataPrefix() {return dataPrefix;}
CString CGazoDoc::GetDataSuffix() {return dataSuffix;}
void CGazoDoc::SetLogPath(CString arg) {logPath = arg;}
void CGazoDoc::SetDataPrefix(CString arg) {dataPrefix = arg;}
void CGazoDoc::SetDataSuffix(CString arg) {dataSuffix = arg;}
void CGazoDoc::SetDataPath(CString arg) {dataPath = arg;}
void CGazoDoc::SetQueueMode(bool arg) {bFromQueue = arg;}

void CGazoDoc::InitContrast() {
	if (!pPixel) return;
	const int idim = ixdim * iydim;
	if (!idim) return;
	iPixelMax = 0;
	iPixelMin = INT_MAX;//2147483647;
	double dAvg = 0, dSig = 0;
	for (int i=0; i<idim; i++) {
		int ip = pPixel[i];
		if (bColor) {ip = ((ip & 0xff) + ((ip >> 8) & 0xff) + ((ip >> 16) & 0xff)) / 3;}
		iPixelMax = iPixelMax > ip ? iPixelMax : ip;
		iPixelMin = iPixelMin < ip ? iPixelMin : ip;
		dAvg += ip;
		dSig += (__int64)ip * (__int64)ip;
//		iPixelMax = iPixelMax > pPixel[i] ? iPixelMax : pPixel[i];
//		iPixelMin = iPixelMin < pPixel[i] ? iPixelMin : pPixel[i];
//		dAvg += pPixel[i];
//		dSig += (__int64)pPixel[i] * (__int64)pPixel[i];
	}
	dAvg /= idim;
	//CString msg; msg.Format("Avg %f  Sig %f\r\nMin %d  Max %d", dAvg, dSig/idim, iPixelMin, iPixelMax); AfxMessageBox(msg);
	dSig = sqrt(dSig / idim - dAvg * dAvg);
	//iBrightness = iPixelMin;
	//iContrast = iPixelMax - iPixelMin;
	iDispLow = (iPixelMin > dAvg - 5. * dSig) ? iPixelMin : (int)(dAvg - 5. * dSig);
	iDispHigh = (iPixelMax < dAvg + 5. * dSig) ? iPixelMax : (int)(dAvg + 5. * dSig);
	if (iDispHigh <= iDispLow) iDispHigh = iDispLow + 1;
	//CString line; line.Format("%d %d", iPixelMax, iPixelMin); AfxMessageBox(line);
	//if (iContrast < 1) iContrast = 1;
}

void CGazoDoc::SetDispLevel(int iLow, int iHigh) {
	if (iHigh > iLow) {
		iDispHigh = iHigh; iDispLow = iLow;
	}
}

void CGazoDoc::UpdateView(bool bInit) {
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		if (bInit) {
			//130207 dlgReconst.Init(iydim, ixdim, ixdim/2, ixdim/2);
			dlgReconst.Init(iydim, ixdim, 0, 0);
			InitContrast();
			pv->OnInitialUpdate();//190628
			pv->SetScrollPos(SB_VERT, CGV_VSCROLL_RANGE / 2);
			pv->SetScrollPos(SB_HORZ, CGV_HSCROLL_RANGE / 2);
			pv->SetBoxParams(ixdim/2, iydim/2, ixdim/2, iydim/2, 0);
			pv->InitPolygon(ixdim/2, iydim/2, ixdim/2, iydim/2);//180424
			CGazoApp* pApp = (CGazoApp*) AfxGetApp();
			if (pApp->prevDlgHistogram.m_FileMsg == "saved") {
				dlgHist.ParamCopyFrom(pApp->prevDlgHistogram);
				pv->SetBoxParams(dlgHist.m_TrmCentX, dlgHist.m_TrmCentY, dlgHist.m_TrmSizeX, dlgHist.m_TrmSizeY, dlgHist.m_TrmAngle);
				if ((dlgHist.m_TrmSizeX < 0)||(dlgHist.m_TrmSizeY < 0)) dlgHist.m_EnableTrm = FALSE;
				//131021 if (dlgHist.m_EnableTrm) pv->EnableBox(true); else pv->EnableBox(false);
				int iHigh = (int)( (atof(dlgHist.m_CursorHigh) - pixBase) * pixDiv );
				int iLow = (int)( (atof(dlgHist.m_CursorLow) - pixBase) * pixDiv );
				SetDispLevel(iLow, iHigh);
			} else {//121127
				CString sarg;
				if (pixDiv > 0) {
					double dens = iDispLow / pixDiv + pixBase;
					sarg.Format("%.3f", dens);
					dlgHist.m_CursorLow = sarg;
					dens = iDispHigh / pixDiv + pixBase;
					sarg.Format("%.3f", dens);
					dlgHist.m_CursorHigh = sarg;
				} else {
					double dens = iDispLow;
					sarg.Format("%.1f", dens);
					dlgHist.m_CursorLow = sarg;
					dens = iDispHigh;
					sarg.Format("%.1f", dens);
					dlgHist.m_CursorHigh = sarg;
				}
			}
		}
		pv->SetPixels(ixdim, iydim, iDispLow, iDispHigh - iDispLow);
		pv->SetOverlay(ppOverlay, ioxdim, ioydim, izOverlay, iDispLow, iDispHigh - iDispLow);
		pv->InvalidateRect(NULL, FALSE);
	}
}

void CGazoDoc::EnableSystemMenu(bool bEnable) {
	POSITION pos = GetFirstViewPosition();
	if (pos) {
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		if (pv) {
			CChildFrame* pf = (CChildFrame*) pv->GetParentFrame();
			if (pf) {
				CMenu* pMenu = pf->GetSystemMenu(FALSE);
				if (pMenu) {
					if (bEnable) pMenu->EnableMenuItem(SC_CLOSE, MF_ENABLED);
					else pMenu->EnableMenuItem(SC_CLOSE, MF_GRAYED);
				}
			}
		}
	}
}

bool CGazoDoc::PointInPolygon(int ix, int iy, int* piPolygonX, int* piPolygonY) {//image coords
	int icount = 0;
	for(int i=0; i<CGAZOVIEW_NPOLYGON; i++){
		int i1 = (i == CGAZOVIEW_NPOLYGON-1) ? 0 : i+1;
		if ( ((piPolygonY[i] <= iy) && (piPolygonY[i1] > iy))
				|| ((piPolygonY[i] > iy) && (piPolygonY[i1] <= iy)) ){
			double dt = (iy - piPolygonY[i]) / (double)(piPolygonY[i1] - piPolygonY[i]);
			if (ix < (piPolygonX[i] + (dt * (piPolygonX[i1] - piPolygonX[i])))) icount++;
		}
	}
	return (icount & 0x01);
}

TErr CGazoDoc::GetPolygon(CString sSliceNumber, CString sPolygonList, int* piPolygonX, int* piPolygonY) {
//AfxMessageBox(sSliceNumber + "\r\n" + sPolygonList);
	if (sPolygonList.IsEmpty()) {
		POSITION pos = GetFirstViewPosition();
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		if (pv == NULL) return 18062001;
		for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
			piPolygonX[i] = pv->iPolygonX[i];
			piPolygonY[i] = pv->iPolygonY[i];
		}
		return 0;
	}
	int ipg1x[CGAZOVIEW_NPOLYGON], ipg1y[CGAZOVIEW_NPOLYGON];
	int ipg2x[CGAZOVIEW_NPOLYGON], ipg2y[CGAZOVIEW_NPOLYGON];
	CString spg1 = "", spg2 = "";
	int ipos = 0;
	for (;;) {
		const CString sEntry = sPolygonList.Tokenize(_T("\r\n"), ipos);
		if (sEntry.IsEmpty()) break;
		const CString sFrmNum = sEntry.SpanExcluding(" ");
		if (sFrmNum.IsEmpty()) break;
		const CString sCoords = sEntry.Mid(sFrmNum.GetLength()).TrimLeft();
		if (sFrmNum == sSliceNumber) {
			int jpos = 0;
//CString msg = "180620-01\r\n", line;
			for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
				CString sPoint = sCoords.Tokenize(_T(")"), jpos);
				if (sPoint.IsEmpty()) return 18062002;
				piPolygonX[j] = atoi(sPoint.Mid(1).SpanExcluding(" "));
				piPolygonY[j] = atoi(sPoint.Mid(sPoint.SpanExcluding(" ").GetLength()));
//line.Format("(%d %d)", piPolygonX[j], piPolygonY[j]); msg += line;
			}
//AfxMessageBox(msg);
			return 0;
		} else if (atoi(sFrmNum) > atoi(sSliceNumber)) {
			int jpos = 0;
			bool bComplete = true;
			for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
				CString sPoint = sCoords.Tokenize(_T(")"), jpos);
				if (sPoint.IsEmpty()) {bComplete = false; break;}
				ipg2x[j] = atoi(sPoint.Mid(1).SpanExcluding(" "));
				ipg2y[j] = atoi(sPoint.Mid(sPoint.SpanExcluding(" ").GetLength()));
			}
			if (bComplete) {
				spg2 = sEntry.SpanExcluding(" ");
				break;
			}
		} else {
			int jpos = 0;
			bool bComplete = true;
			for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
				CString sPoint = sCoords.Tokenize(_T(")"), jpos);
				if (sPoint.IsEmpty()) {bComplete = false; break;}
				ipg1x[j] = atoi(sPoint.Mid(1).SpanExcluding(" "));
				ipg1y[j] = atoi(sPoint.Mid(sPoint.SpanExcluding(" ").GetLength()));
			}
			if (bComplete) spg1 = sEntry.SpanExcluding(" ");
		}
	}
	const int ipg1 = atoi(spg1), ipg2 = atoi(spg2);
	if (spg1.IsEmpty()) {
		if (spg2.IsEmpty()) return 18062003;
		for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
			piPolygonX[j] = ipg2x[j]; piPolygonY[j] = ipg2y[j];
		}
	} else {
		if ((spg2.IsEmpty())||(ipg1 == ipg2)) {
			for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
				piPolygonX[j] = ipg1x[j]; piPolygonY[j] = ipg1y[j];
			}
		} else {
			const double dr = (atof(sSliceNumber) - ipg1) / (ipg2 - ipg1);
			for (int j=0; j<CGAZOVIEW_NPOLYGON; j++) {
				piPolygonX[j] = (int)(ipg1x[j] + dr * (ipg2x[j] - ipg1x[j]));
				piPolygonY[j] = (int)(ipg1y[j] + dr * (ipg2y[j] - ipg1y[j]));
			}
		}
	}
	return 0;
}

TErr CGazoDoc::OutputImageInBox(FORMAT_QUEUE* fq, CProgressCtrl* progress, CString* psMsg) {
	TErr err = 0;
	if (!fq) return 21031;
	const CString prefix = fq->outFilePrefix;
	LPTSTR lpFile = fq->lpFileList;
	int nFiles = fq->nFiles;
	const int navg = fq->iAverage;
	if (!lpFile) return 21031;
	if (!navg) return 21033;
	//int kxdim = ixdim;
	//int kydim = iydim;
	const double dHigh = fq->dHigh; 
	const double dLow = fq->dLow; 
	//CString line; line.Format("2 %f %f", dLow, dHigh); AfxMessageBox(line);
	int kxdim = fq->iXdim;
	int kydim = fq->iYdim;
	//pv->GetBoxParams(&ixcent, &iycent, &ixsize, &iysize, &iangle, &bEnb);
	int ixcent = fq->iBoxCentX;
	int iycent = fq->iBoxCentY;
	int ixsize = fq->iBoxSizeX;
	int iysize = fq->iBoxSizeY;
	int iangle = fq->iBoxAngle;
	const bool bEnb = (fq->bBoxEnabled) ? true : false;
	if (!bEnb) {ixsize = kxdim; iysize = kydim;}
	const int ixy = ixsize * iysize;
	const CString sPolygonList = fq->sPolygonList;
	const bool bHistLog = (fq->uiFlags & FQFLAGS_OUTPUT_HISTG) ? true : false;
	__int64 pllHistLog[256];
	unsigned __int64 ullHistLogCount = 0; 
	for (int i=0; i<256; i++) {pllHistLog[i] = 0;}
	unsigned char* pixOut = NULL;
	int* pixOut16 = NULL;
	int* pixbuf = NULL;
	int maxIntensity = 255;
	try {
		if (fq->uiFlags & FQFLAGS_16BIT) {
			maxIntensity = 65535;
			pixOut16 = new int[ixy];
		} else {
			pixOut = new unsigned char[ixy];
		}
		pixbuf = new int[ixy];
	}
	catch (CException* e) {
		if (pixOut) delete [] pixOut;
		if (pixOut16) delete [] pixOut16;
		if (pixbuf) delete [] pixbuf;
		e->Delete(); return 21032;
	}
	//sort files
	bool bNull = false;
	if (nFiles < 0) {
		for (int i=0; i<MAX_FILE_DIALOG_LIST; i++) {
			if (lpFile[i] == NULL) {
				if (bNull) break;
				nFiles++; bNull = true;
			} else {
				bNull = false;
			}
		}
		if (nFiles > 1) nFiles--;//nfile: 1==>1 file; 3==>2 files; 4==>3files etc.
	}
	char** flist = NULL;
	try {flist = new char*[nFiles];}
	catch (CException* e) {
		if (flist) delete [] flist; 
		if (pixbuf) delete [] pixbuf;
		if (pixOut) delete [] pixOut;
		if (pixOut16) delete [] pixOut16;
		e->Delete(); return 21032;
	}
	if (nFiles > 1) {//multiple selection
		bNull = false;
		int idx = 0;
		for (int i=0; i<MAX_FILE_DIALOG_LIST; i++) {
			if (lpFile[i] == NULL) {
				if (bNull) break;
				bNull = true; flist[idx++] = &(lpFile[i + 1]);
				if (idx >= nFiles) break;
			} else {
				bNull = false;
			}
		}
		//sort
		qsort( (void *)flist, (size_t)nFiles, sizeof(char*), StringCompare );
	}
	//120501 log output
	CString sfpath = lpFile;
	sfpath.TrimLeft('"'); sfpath.TrimRight('"');
	const CString sfinput = (nFiles > 1) ? (sfpath + "\\" + flist[0]) : sfpath;
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, sfinput);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	const CString sdataPath = path_buffer;
	CStdioFile flog;
	CString sLogFileName = "recviewlog.txt";
	if (!flog.Open(sdataPath + sLogFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
		sLogFileName = "0recviewlog.txt";
		if (!flog.Open(sdataPath + sLogFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
			sLogFileName = "_recviewlog.txt";
		}
		else {
			flog.Close();
		}
	}
	else {
		flog.Close();
	}
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	bool bflogOpen = false;
	//AfxMessageBox(sdataPath + sLogFileName);
	if (flog.Open(sdataPath + sLogFileName, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		bflogOpen = true;
		flog.SeekToEnd();
		struct _timeb tstruct;
		_ftime_s( &tstruct );
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line;
		CString sVer = pApp->sProgVersion; sVer.Replace('\n', ' ');
		line.Format("Image conversion [%s] %s\n", stime.Left(24), sVer);
		flog.WriteString(line);
		if (fq->nFiles > 1) {
			flog.WriteString(" Source files: " + sfinput + " etc...\n");
		} else {
			flog.WriteString(" Source file: " + sfinput + "\n");
		}
		line.Format(" Number of files: %d\n", fq->nFiles);
		flog.WriteString(line);
		line.Format(" Source image sizes: x=%d, y=%d\n", fq->iXdim, fq->iYdim);
		flog.WriteString(line);
		line.Format(" LAC limits: low=%.2f, high=%.2f\n", fq->dLow, fq->dHigh);
		flog.WriteString(line);
		if (fq->bBoxEnabled) {
			flog.WriteString(" Trimming enabled\n");
			line.Format("  Box center: x=%d, y=%d\n", fq->iBoxCentX, fq->iBoxCentY);
			flog.WriteString(line);
			line.Format("  Box sizes: x=%d, y=%d\n", fq->iBoxSizeX, fq->iBoxSizeY);
			flog.WriteString(line);
			line.Format("  Box tilt: %d deg\n", fq->iBoxAngle);
			flog.WriteString(line);
		}
		if (fq->iAverage > 1) {
			line.Format(" Averaging: %d pixel\n", fq->iAverage);
			flog.WriteString(line);
		}
		if (fq->uiFlags & FQFLAGS_16BIT) flog.WriteString(" Output 16 bit TIFF files\n"); else flog.WriteString(" Output 8 bit TIFF files\n");
		flog.WriteString(" Output file prefix: " + fq->outFilePrefix + "\n");
		if (fq->iOspDepth) {
			line.Format(" Carving depth: %d pixels\n", fq->iOspDepth);
			flog.WriteString(line);
			line.Format("  LAC threshold: %.2f\n", fq->dOspThreshold);
			flog.WriteString(line);
		}
		if (!sPolygonList.IsEmpty()) {
			line.Format(" Polygon list:\n%s", sPolygonList);
			flog.WriteString(line);
		}
		//120803 flog.WriteString("---------------------------------------------------\r\n");
		//flog.Close();
	}
	//
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	if (!bflogOpen) {//141205
		if (psMsg) {*psMsg = "**ErrorOnOpenLogFile**";}//130210
		else {AfxMessageBox("Error on opening a log file.");}
		if (pf) pf->m_wndStatusBar.SetPaneText(1, "");
		if (pixbuf) delete [] pixbuf;
		if (pixOut) delete [] pixOut;
		if (pixOut16) delete [] pixOut16; 
		if (flist) delete [] flist;
		return err;
	}
	//
	CFileStatus fstatus;
	int izavg = 0;
	int* pixIn = NULL; int nPixIn = 0;
	float tpixDiv, tpixBase, tfCenter, tfPixelWidth;
	int tiFilter, tiSino;
	CFile file;
	for (int n=0; n<nFiles; n++) {
		//120427
		::ProcessMessage();
		if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
		//
		if (!izavg) { for (int i=0; i<ixy; i++) {pixbuf[i] = 0;} }
		//CString finput = lpFile;
		//if (nFiles > 1) {finput += "\\"; finput += flist[n];}
		const CString finput = (nFiles > 1) ? (sfpath + "\\" + flist[n]) : sfpath;
		if (pf) pf->m_wndStatusBar.SetPaneText(1, "Reformatting " + finput);
		//read file
		if (!file.Open(finput, CFile::modeRead | CFile::shareDenyWrite)) {err = 21032; continue;}
		err = ReadTif(&file, &pixIn, &nPixIn, &kydim, &kxdim,
									&tpixDiv, &tpixBase, &tfCenter, &tiFilter, &tfPixelWidth, &tiSino);
		file.Close();
		if (err) continue;
		//090724 two-dimensional OSP
		if (fq->iOspDepth > 0) {
			int iRemoved = 0;
			RemoveSurface(pixIn, fq, kxdim, kydim, tpixDiv, tpixBase, &iRemoved);
			//CString msg; msg.Format("Removed %d %f", iRemoved, kxdim * kydim * 0.8); AfxMessageBox(msg);
			if (iRemoved > kxdim * kydim * 0.8) {//if removed area is greater than 80% of image
				if (flog.GetStatus(fstatus)) {flog.WriteString("   Truncated: " + finput + "\n");}//120803
				if (psMsg) {*psMsg = "**Truncated**";}//130210
			}
		}
		//polygon lasso
		int ipgx[CGAZOVIEW_NPOLYGON], ipgy[CGAZOVIEW_NPOLYGON];
		bool bEnPolygon = false;
		CString sFrm = finput;
		sFrm.MakeReverse();
		const int iExt = sFrm.Find('.');
		if ((!sPolygonList.IsEmpty())&&(iExt >= 0)) {
			sFrm = sFrm.Mid(iExt+1).SpanIncluding("01234567890").MakeReverse();
			if (!sFrm.IsEmpty()) {
				if (!GetPolygon(sFrm, sPolygonList, ipgx, ipgy)) bEnPolygon = true;
			}
		}
		//trimming
		int kDispHigh = (int)((dHigh - tpixBase) * tpixDiv);
		int kDispLow = (int)((dLow - tpixBase) * tpixDiv);
		if (!bEnb) {
			for (int i=0; i<ixsize; i++) {
				for (int j=0; j<iysize; j++) {
					if (bEnPolygon) {
						if (!PointInPolygon(i, j, ipgx, ipgy)) continue;
					}
					int ipix = (int)( ((double)pixIn[i + j * kxdim] - kDispLow) * maxIntensity / (kDispHigh - kDispLow) );
					if (ipix > maxIntensity) ipix = maxIntensity; else if (ipix < 0) ipix = 0;
					//pixbuf[i + j * ixsize] += (unsigned char)ipix;
					pixbuf[i + j * ixsize] += ipix;
				}
			}
		} else {
			double csa = cos(iangle * DEG_TO_RAD);
			double sna = sin(iangle * DEG_TO_RAD);
			for (int i=0; i<ixsize; i++) {
				//120427
				::ProcessMessage();
				if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
				//
				for (int j=0; j<iysize; j++) {
					int ix = i - ixsize / 2;
					int iy = j - iysize / 2;
					double gx = ix * csa - iy * sna + ixcent;
					double gy = ix * sna + iy * csa + iycent;
					int ipix = 0;
					if ((gx >= 0)&&(gy >= 0)&&(gx <= kxdim-1)&&(gy <= kydim-1)) {
						int igx = (int)gx;
						int igy = (int)gy;
						if (bEnPolygon) {
							if (!PointInPolygon(igx, igy, ipgx, ipgy)) continue;
						}
						double dx = gx - igx;
						double dy = gy - igy;
						if ((fabs(dx) < 0.00001)&&(fabs(dy) < 0.00001)) {//090727
							ipix = (int)(((double)pixIn[igx + igy * kxdim] - kDispLow) * maxIntensity / (kDispHigh - kDispLow));
						} else {
							//interpolated intensity
							if (igx == kxdim-1) {igx--; dx = 1;}
							if (igy == kydim-1) {igy--; dy = 1;}
							//plane determined by least square fitting
							int is0 = pixIn[igx + igy * kxdim];
							int is1 = pixIn[(igx + 1) + igy * kxdim];
							int is2 = pixIn[igx + (igy + 1) * kxdim];
							int is3 = pixIn[(igx + 1) + (igy + 1) * kxdim];
							double ap = 0.5 * (is1 + is3 - is0 - is2);
							double bp = 0.5 * (is2 + is3 - is0 - is1);
							double cp = 0.25 * ((is0 + is1 + is2 + is3) - 2 * (ap + bp));
							ipix = (int)( ((ap * dx + bp * dy + cp) - kDispLow) * maxIntensity / (kDispHigh - kDispLow) );
						}
					}
					if (ipix > maxIntensity) ipix = maxIntensity; else if (ipix < 0) ipix = 0;
					//pixbuf[i + j * ixsize] += (unsigned char)ipix;
					pixbuf[i + j * ixsize] += ipix;
				}
			}
		}
		if (progress) progress->StepIt(); else ::ProcessMessage();
		//120427===>
		if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
		//===>120427
		izavg++;
		if ((n < nFiles - 1) && (izavg < navg)) continue;
		//averaging
		int isx = ixsize / navg;
		if (ixsize % navg) isx++;
		int isy = iysize / navg;
		if (iysize % navg) isy++;
		for (int i=0; i<ixsize; i+=navg) {
			for (int j=0; j<iysize; j+=navg) {
				int ipavg = 0, ipix = 0;
				for (int k=0; k<navg; k++) {
					if (i + k >= ixsize) continue;
					for (int m=0; m<navg; m++) {
						if (j + m >= iysize) continue;
						ipix += pixbuf[i + k + (j + m) * ixsize];
						ipavg++;
					}
				}
				ipix /= ipavg * izavg;
				if (ipix > maxIntensity) ipix = maxIntensity; else if (ipix < 0) ipix = 0;
				if (fq->uiFlags & FQFLAGS_16BIT) {
					pixOut16[i / navg + j / navg * isx] = ipix;
					if (bHistLog) {pllHistLog[ipix >> 8]++; ullHistLogCount++;}
				} else {
					pixOut[i / navg + j / navg * isx] = (unsigned char)ipix;
					if (bHistLog) {pllHistLog[ipix]++; ullHistLogCount++;}
				}
			}
		}
		//CString line; line.Format("%s %d", finput, izavg); AfxMessageBox(line);
		//output
		izavg = 0;
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, finput);
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		CString filename = fnm;
		filename = prefix + filename.Mid(filename.SpanExcluding("0123456789.").GetLength());
		_tcscpy_s(fnm, filename);
		_tcscpy_s(ext, ".tif");
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
		CFile file;
		if (!file.Open(path_buffer, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) continue;
		CString imageDesc;
		imageDesc.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%.6f ",
			tiSino, tfPixelWidth, maxIntensity/(dHigh - dLow), dLow, tfCenter, tiFilter, 
			55.111697, 0.461227);
		if (fq->uiFlags & FQFLAGS_16BIT) err = WriteTifMonochrome16(&file, pixOut16, isy, isx, imageDesc);
		else err = WriteTifMonochrome(&file, pixOut, isy, isx, imageDesc);
		file.Close();
	}
	if (flog.GetStatus(fstatus)) {//120803
		if (bHistLog) {
			flog.WriteString(" Histogram\n pixel\tLAC\tn\tFreq\n");
			for (int i=0; i<256; i++) {
				CString line;
				line.Format(" %d\t%.2f\t%lld\t%.6f\n", 
					(fq->uiFlags & FQFLAGS_16BIT) ? i*256 : i, i * (dHigh - dLow) / maxIntensity + dLow,
					pllHistLog[i], (double)pllHistLog[i] / ullHistLogCount);
				flog.WriteString(line);
			}
		}
		flog.WriteString("---------------------------------------------------\n");
		flog.Close();
	}
	//AfxMessageBox(line);
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "");
	if (pixbuf) delete [] pixbuf;
	if (pixOut) delete [] pixOut;
	if (pixOut16) delete [] pixOut16; 
	if (pixIn) delete [] pixIn;
	if (flist) delete [] flist;
	return err;
}

#define GZD_RMSRF_PROCESSED -1000
#define GZD_RMSRF_MARKED -1001
void CGazoDoc::RemoveSurface(int* pixIn, FORMAT_QUEUE* fq, int kxdim, int kydim, 
							 float tpixDiv, float tpixBase, int* piRemoved) {
	//
	//struct _timeb tstruct;
	//_ftime_s( &tstruct );
	//double tm0 = tstruct.time + tstruct.millitm * 0.001;
	//
	if (piRemoved) *piRemoved = 0;
	const int iOspThreshold = (int)((fq->dOspThreshold - tpixBase) * tpixDiv);
	//get outer region
	pixIn[0] = GZD_RMSRF_MARKED;
	bool bCont = true;
	while (bCont) {
		bCont = false;
		int icent = -1;
		for (int iy=0; iy<kydim; iy++) {
			//int icent = iy * kxdim-1;
			for (int ix=0; ix<kxdim; ix++) {
				icent++;
				//const int icent = ix + iy * kxdim;
				if (pixIn[icent] != GZD_RMSRF_MARKED) continue;
				bCont = true;
				pixIn[icent] = GZD_RMSRF_PROCESSED;
				if (iy > 0) {
					const int jcent0 = icent - kxdim;
					if (ix > 0) {
						int jcent = jcent0 - 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
					if ((pixIn[jcent0] != GZD_RMSRF_PROCESSED) && (pixIn[jcent0] < iOspThreshold)) pixIn[jcent0] = GZD_RMSRF_MARKED;
					if (ix < kxdim-1) {
						int jcent = jcent0 + 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
				}
				{
					if (ix > 0) {
						int jcent = icent - 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
					if ((pixIn[icent] != GZD_RMSRF_PROCESSED) && (pixIn[icent] < iOspThreshold)) pixIn[icent] = GZD_RMSRF_MARKED;
					if (ix < kxdim-1) {
						int jcent = icent + 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
				}
				if (iy < kydim-1) {
					const int jcent0 = icent + kxdim;
					if (ix > 0) {
						int jcent = jcent0 - 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
					if ((pixIn[jcent0] != GZD_RMSRF_PROCESSED) && (pixIn[jcent0] < iOspThreshold)) pixIn[jcent0] = GZD_RMSRF_MARKED;
					if (ix < kxdim-1) {
						int jcent = jcent0 + 1;
						if ((pixIn[jcent] != GZD_RMSRF_PROCESSED) && (pixIn[jcent] < iOspThreshold)) pixIn[jcent] = GZD_RMSRF_MARKED;
					}
				}
				/*
				for (int jx=ix-1; jx<=ix+1; jx++) {
					if ((jx < 0)||(jx >= kxdim)) continue;
					for (int jy=iy-1; jy<=iy+1; jy++) {
						if ((jy < 0)||(jy >= kydim)) continue;
						const int jcent = jx + jy * kxdim;
						if (pixIn[jcent] == GZD_RMSRF_PROCESSED) continue;
						else if (pixIn[jcent] < iOspThreshold) pixIn[jcent] = GZD_RMSRF_MARKED;
					}//jy
				}//jx
				///*///
			}//ix
		}//iy
	}//while(bCont)
	//_ftime_s( &tstruct );
	//double tcpu1 = tstruct.time + tstruct.millitm * 0.001 - tm0;
	//trace border
	for (int ix=0; ix<kxdim; ix++) {
		for (int iy=0; iy<kydim; iy++) {
			const int icent = ix + iy * kxdim;
			bool bBorder = false;
			if (pixIn[icent] != GZD_RMSRF_PROCESSED) continue;
			if (ix-1 >= 0) {
				if ((pixIn[icent-1] != GZD_RMSRF_PROCESSED)&&(pixIn[icent-1] != GZD_RMSRF_MARKED)) bBorder = true;
			}
			if (ix+1 < kxdim) {
				if ((pixIn[icent+1] != GZD_RMSRF_PROCESSED)&&(pixIn[icent+1] != GZD_RMSRF_MARKED)) bBorder = true;
			}
			if (iy-1 >= 0) {
				if ((pixIn[icent-kxdim] != GZD_RMSRF_PROCESSED)&&(pixIn[icent-kxdim] != GZD_RMSRF_MARKED)) bBorder = true;
			}
			if (iy+1 < kydim) {
				if ((pixIn[icent+kxdim] != GZD_RMSRF_PROCESSED)&&(pixIn[icent+kxdim] != GZD_RMSRF_MARKED)) bBorder = true;
			}
			if (bBorder) pixIn[icent] = GZD_RMSRF_MARKED;
		}
	}
	//_ftime_s( &tstruct );
	//double tcpu2 = tstruct.time + tstruct.millitm * 0.001 - tm0;
	//remove surface
	const int idep = fq->iOspDepth;
	const int idep2 = idep * idep;
	int icent = -1;
	for (int iy=0; iy<kydim; iy++) {
		for (int ix=0; ix<kxdim; ix++) {
			icent++;
			//const int icent = ix + iy * kxdim;
			if (pixIn[icent] == GZD_RMSRF_PROCESSED) pixIn[icent] = 0;
			else if (pixIn[icent] == GZD_RMSRF_MARKED) {
				//if (pixIn[icent] != GZD_RMSRF_MARKED) pixIn[icent] = 0;
				for (int jx=1; jx<=idep; jx++) {
					const int jx2 = jx * jx;
					for (int jy=1; jy<=idep; jy++) {
						if (jx2 + jy*jy > idep2) continue;
						const int ijxm = ix - jx;
						const int ijxp = ix + jx;
						const int ijym = iy - jy;
						const int ijyp = iy + jy;
						if (ijxm >= 0) {
							if (ijym >= 0) {
								const int jcent = ijxm + ijym * kxdim;
								if (pixIn[jcent] != GZD_RMSRF_MARKED) pixIn[jcent] = 0;
							}
							if (ijyp < kydim) {
								const int jcent = ijxm + ijyp * kxdim;
								if (pixIn[jcent] != GZD_RMSRF_MARKED) pixIn[jcent] = 0;
							}
						}
						if (ijxp < kxdim) {
							if (ijym >= 0) {
								const int jcent = ijxp + ijym * kxdim;
								if (pixIn[jcent] != GZD_RMSRF_MARKED) pixIn[jcent] = 0;
							}
							if (ijyp < kydim) {
								const int jcent = ijxp + ijyp * kxdim;
								if (pixIn[jcent] != GZD_RMSRF_MARKED) pixIn[jcent] = 0;
							}
						}
					}//jy
				}//jx
				/*
				for (int jx=ix-idep; jx<=ix+idep; jx++) {
					if ((jx < 0)||(jx >= kxdim)) continue;
					const int jx2 = (jx - ix) * (jx - ix);
					for (int jy=iy-idep; jy<=iy+idep; jy++) {
						if ((jy < 0)||(jy >= kydim)) continue;
						if (jx2 + (jy-iy)*(jy-iy) > idep2) continue;
						const int jcent = jx + jy * kxdim;
						if (pixIn[jcent] == GZD_RMSRF_MARKED) continue;
						pixIn[jcent] = 0;
					}//jy
				}//jx
				*/
			}//if MARKED
		}//iy
	}//ix
	//erase MARKED pixels
	for (int ix=0; ix<kxdim; ix++) {
		for (int iy=0; iy<kydim; iy++) {
			const int icent = ix + iy * kxdim;
			if (pixIn[icent] == GZD_RMSRF_MARKED) pixIn[icent] = 0;
			if ((pixIn[icent] == 0)&&(piRemoved)) (*piRemoved)++;//120803
		}
	}
	//_ftime_s( &tstruct );
	//double tcpu3 = tstruct.time + tstruct.millitm * 0.001 - tm0;
	//CString msg; msg.Format("%f %f %f", tcpu1, tcpu2, tcpu3); AfxMessageBox(msg);
}

/////////////////////////////////////////////////////////////////////////////
// CGazoDoc シリアライゼーション

void CGazoDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: この位置に保存用のコードを追加してください。
		//const CString fns = dlgReconst.m_Suffix;
		ar.Flush();
		CFile* fp = ar.GetFile();
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, fp->GetFilePath());
		TCHAR ext[_MAX_EXT];
		_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
		TErr err = 21011;
		if (((_tcscmp(ext, ".tif") == 0)||(_tcscmp(ext, ".TIF") == 0))&&(!bColor)) {
			CString imageDesc;
			imageDesc.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%.6f ",
				numOfSampleSinogr, dlgReconst.m_PixelWidth, pixDiv, pixBase, fCenter, 
				dlgReconst.m_Filter + 1, 55.111697, 0.461227);
//         1         2         3         4         5         6         7
//1234567890123456789012345678901234567890123456789012345678901234567890
//1800	1.000000	595.040478	-5.550398	-312.000000	1	55.111697	0.461227 
			err = WriteTifMonochrome16(fp, pPixel, iydim, ixdim, imageDesc);
		} else {
			AfxMessageBox("Not supported");
		}
	}
	else
	{
		// TODO: この位置に読み込み用のコードを追加してください。
		ar.Flush();
		CFile* fp = ar.GetFile();
		TErr err = ReadFile(fp);
		//160803
		if (err) {
			CString line; line.Format("Not supported: %d", err); AfxMessageBox(line);
		}
		UpdateView(/*bInit=*/true);
	}
}

TErr CGazoDoc::ReadFile(CFile* fp) {
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fp->GetFilePath());
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, ext, _MAX_EXT);
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	const CString sPath = path_buffer;
	TErr err = 21001;
	m_cimage.Destroy();
	//120803 iLossFrameSet = -1;
	if ((_tcscmp(ext, ".tif") == 0)||(_tcscmp(ext, ".TIF") == 0)) {
		float fPrevPixDiv = pixDiv, fPrevPixBase = pixBase;
		pixDiv = 0; pixBase = 0;
		float pw = (float)dlgReconst.m_PixelWidth;
		err = ReadTif(fp, &pPixel, &maxPixel, &iydim, &ixdim, &pixDiv, &pixBase,
									&fCenter, &(dlgReconst.m_Filter), &pw);
		if (err == WARN_READIMAGE_SIZECHANGE) {//160803
			pixDiv = fPrevPixDiv; pixBase = fPrevPixBase; return err;
		} else if (err) {
			HRESULT hResult = m_cimage.Load(fp->GetFilePath());
			if (SUCCEEDED(hResult)) err = 0;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		if (err) error.Log(err);
	} else if ((_tcscmp(ext, ".img") == 0)||(_tcscmp(ext, ".IMG") == 0)) {
		err = ReadITEX(fp, &pPixel, &maxPixel, &iydim, &ixdim, &fileComment);
		error.Log(err);
	}
	else if ((_tcscmp(ext, ".his") == 0) || (_tcscmp(ext, ".HIS") == 0)) {
		HIS_Header his, ehis;
		if (fp->GetPosition() == 0) maxHisFrame = -1;
		err = ReadHIS(fp, &pPixel, &maxPixel, &iydim, &ixdim, &his, &fileComment);
		error.Log(err);
		TReal rAvgS = 0, rSigS = 0;
		const int ixydim = ixdim * iydim;
		for (int i=0; i<ixydim; i++) {
			rAvgS += pPixel[i]; rSigS += pPixel[i] * pPixel[i];
		}
		rAvgS /= ixydim;
		rSigS = sqrt(rSigS / ixydim - rAvgS * rAvgS);
		if (maxHisFrame < 0) {
			maxHisFrame = his.n_image1 + (his.n_image2 << 16);
			//210618==>
			int nset = 1;
			if (CountFrameFromConvBat(sPath) > 0) nset = dlgReconst.m_nDataset;
			else { err = 21061801; error.Log(err); }
			//==>210618
			//210618 if ((maxHisFrame > 2) &&(iLossFrameSet < 0)) {//120715
			if ((maxHisFrame > 2) && (ullLossFrameSet == 0) && (!err)) {//210618
				//analyse frame loss
				int* ePixel = NULL;
				int emaxPixel = 0, eiydim, eixdim;
				CString efileComment;
				TReal rAvgE0 = 0, rAvgE1 = 0, sig1 = 0;
				//210618 see nset-1 dataset. This works if the lostframe was filled with Io images and not dark images.
				if (nset >= 2) {
					SkipHISframeFast(fp, iFramePerDataset * (nset - 1) - 2);
					ReadHIS(fp, &ePixel, &emaxPixel, &eiydim, &eixdim, &ehis, &efileComment);
					const int eixydim = eiydim * eixdim;
					for (int i = 0; i < eixydim; i++) { rAvgE1 += ePixel[i]; }
					rAvgE1 /= eixydim;
					sig1 = fabs((rAvgE1 - rAvgS) / rSigS);
					//210618 SkipHISframeFast(fp, 1);
				}
				SkipHISframeFast(fp, iFramePerDataset-1);//210618
				//210618 SkipHISframeFast(fp, maxHisFrame - 3);
				//final -1
				//ReadHIS(fp, &ePixel, &emaxPixel, &eiydim, &eixdim, &ehis, &efileComment);
				//const int eixydim = eiydim * eixdim;
				//for (int i=0; i<eixydim; i++) {rAvgE1 += ePixel[i];}
				//rAvgE1 /= eixydim;
				//TReal sig1 = fabs((rAvgE1 - rAvgS) / rSigS);
				//final dataset
				ReadHIS(fp, &ePixel, &emaxPixel, &eiydim, &eixdim, &ehis, &efileComment);
				const int eixydim = eiydim * eixdim;
				for (int i=0; i<eixydim; i++) {rAvgE0 += ePixel[i];}
				rAvgE0 /= eixydim;
				TReal sig0 = fabs((rAvgE0 - rAvgS) / rSigS);
				//210618 if (sig1 < 10) AfxMessageBox("Multiple frames have been lost.\r\nThis should be fixed manually.");
				//else if (sig0 < 10) {//difference between the last image and the dark current is less than 10 sigma
				if ((sig0 < 10)||(sig1 < 10)) {//difference between the last image and the dark current is less than 10 sigma
					//210618 if (CountFrameFromConvBat(sPath) > 0){
					if (!err) {
						//141229 const int nset = maxHisFrame / iFramePerDataset;
						//210618 const int nset = dlgReconst.m_nDataset;
						fp->SeekToBegin();
						//210618 ReadHIS(fp, &ePixel, &emaxPixel, &eiydim, &eixdim, &ehis, &efileComment);//to skip comment of the first frame
						SkipHISframe(fp, 1);//skip first frame
						//210618 iLossFrameSet = nset-1;//assume loss-set as the final set if the lost frames are not found in the 0 to nset-2 sets.
						for (int i=0; i<nset; i++) {
							const int nlast = 2;//max lostframes per set
							SkipHISframeFast(fp, iFramePerDataset - nlast - 1);//move to last frame
							bool bLost = false;
							for (int j = 0; j < nlast; j++) {
								ReadHIS(fp, &ePixel, &emaxPixel, &eiydim, &eixdim, &ehis, &efileComment);
								TReal rAvgE2 = 0;
								for (int j = 0; j < eixydim; j++) { rAvgE2 += ePixel[j]; }
								rAvgE2 /= eixydim;
								//210618 if (fabs((rAvgE0 - rAvgS) / rSigS) < 10) { iLossFrameSet = i; break; }
								if (fabs((rAvgE2 - rAvgS) / rSigS) < 10) { ullLossFrameSet |= ((nlast - j) << (i * 4)); bLost = true; break; }
							}
							if ((bLost == false)&&(i < nset-1)) SkipHISframeFast(fp, 1);//skip first frame
							if (bLost && (i >= 16)) {
								CString msg; msg.Format("Lost frame found in dataset %d. This should be fixed manually.", i); AfxMessageBox(msg);
							}
						}
					}
				}
				if (ePixel) delete [] ePixel;//120718
				if (ullLossFrameSet) {
					CString msg, msg2 = "", line;
					for (int i = 0; i < nset; i++) { line.Format("set%d #lostframe=%d\r\n", i, (ullLossFrameSet >> (i * 4)) & 0x0f); msg2 += line; }
					msg.Format("!!! LOST FRAME FOUND !!!\r\n Dark:%f\r\n Last frames of\r\n  Nset-1:%f sigma:%f\r\n  Lastset:%f sigma:%f\r\n%s", 
												rAvgS, rAvgE1, sig1, rAvgE0, sig0, msg2); 
					error.Log(1, msg);
					if (AfxMessageBox(msg + "Fix this?", MB_YESNO) == IDNO) {
						ullLossFrameSet = 0;
						msg = "The lost frame problem should be fixed manually.";
						AfxMessageBox(msg);
						error.Log(1, msg);
					} else {
						error.Log(1, "Lostframe fixed.");
					}
				}
			}
		}
		//CString msg; msg.Format("%d", maxHisFrame); AfxMessageBox(msg);
		error.Log(err);
		if (err) {CString msg; msg.Format("%d", err); AfxMessageBox(msg);}
	} else if ((_tcscmp(ext, ".h5") == 0)||(_tcscmp(ext, ".H5") == 0)) {
		/*/160616
		CString msg;
		hdf5.SetFile(fp);
		hdf5.Dump(&msg);
		CDlgMessage dlg;
		dlg.m_Msg = msg;
		dlg.DoModal();
		return;
		///*///
		fileComment.Empty();
		err = ReadHDF5Frame(fp, &pPixel, &maxPixel, &iydim, &ixdim, &hdf5, 0, -1, &fileComment);
		error.Log(err);
	} else {//(_tcscmp(ext, ".his") == 0)
		HRESULT hResult = m_cimage.Load(fp->GetFilePath());
		if (SUCCEEDED(hResult)) err = 0;
	}
	if (!m_cimage.IsNull()) {//if c_image.Load
		//pPixel, maxPixel, iydim, ixdim
		const int ix = m_cimage.GetWidth();
		const int iy = m_cimage.GetHeight();
		if (err = AllocPixelBuf(ix, iy)) return err;
		const int iBPP = m_cimage.GetBPP();
		bColor = false;
		switch (iBPP) {
			case 24: {}
			case 32: {
				const int iXinc = iBPP / 8;
				BYTE bRGB[3];
				const int iPitch = m_cimage.GetPitch();
				BYTE* pbBits = (BYTE*)(m_cimage.GetBits());
				for (int j=0; j<iy; j++) {
					for (int i=0; i<ix; i++) {
						bRGB[0] = pbBits[iPitch * j + i * iXinc + 0];
						bRGB[1] = pbBits[iPitch * j + i * iXinc + 1];
						bRGB[2] = pbBits[iPitch * j + i * iXinc + 2];
						pPixel[i + j * ix] = bRGB[0] | (bRGB[1] << 8) | (bRGB[2] << 16);
						if ( !((bRGB[0] == bRGB[1])&&(bRGB[1] == bRGB[2])) ) {
							bColor = true;
						}
					}
				}
				break;}
			default: {
				BYTE bRGB[3];
				for (int j=0; j<iy; j++) {
					for (int i=0; i<ix; i++) {
						COLORREF cDot = m_cimage.GetPixel(i, j);
						bRGB[0] = cDot & 0xff;
						bRGB[1] = (cDot >> 8) & 0xff;
						bRGB[2] = (cDot >> 16) & 0xff;
						pPixel[i + j * ix] = cDot;
						if ( !((bRGB[0] == bRGB[1])&&(bRGB[1] == bRGB[2])) ) {
							bColor = true;
						}
					}
				}
				break;}
		}
		if (!bColor) {
			for (int j=0; j<iy; j++) {
				for (int i=0; i<ix; i++) {pPixel[i + j * ix] &= 0xff;}
			}
		}
	}
	//160803
	return err;
	//if (err) {
	//	CString line; line.Format("Not supported: %d", err); AfxMessageBox(line); return;
	//}
	//if (err) {AfxMessageBox("Not supported"); return;}
}

/////////////////////////////////////////////////////////////////////////////
// CGazoDoc クラスの診断

#ifdef _DEBUG
void CGazoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGazoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGazoDoc コマンド

void CGazoDoc::CheckDispLimit() {
	if (iDispLow >= iPixelMax - 1) iDispLow = iPixelMax - 1;
	else if (iDispLow <= iPixelMin) iDispLow = iPixelMin;
	if (iDispHigh >= iPixelMax) iDispHigh = iPixelMax;
	else if (iDispHigh <= iPixelMin + 1) iDispHigh = iPixelMin + 1;
	if (iDispHigh <= iDispLow) iDispHigh = iDispLow + 1;
}

void CGazoDoc::AdjBrightness(int iAdj) {
	iDispLow += iAdj;
	iDispHigh += iAdj;
	CheckDispLimit();
	//if (iBrightness - iContrast / 10 < 0) return;
	//iBrightness -= iContrast / 10;
	dlgHist.UpdateParam();
	UpdateView();
}

void CGazoDoc::OnToolbarBright() 
{
	AdjBrightness(-(iDispHigh - iDispLow) / 5);
//	iDispLow -= iContrast / 5;
//	iDispHigh -= iContrast / 5;
//	CheckDispLimit();
//	dlgHist.UpdateParam();
//	UpdateView();
}

void CGazoDoc::OnToolbarDark() 
{
	AdjBrightness((iDispHigh - iDispLow) / 5);
//	int iContrast = iDispHigh - iDispLow;
//	iDispLow += iContrast / 5;
//	iDispHigh += iContrast / 5;
//	CheckDispLimit();
//	dlgHist.UpdateParam();
//	UpdateView();
}

void CGazoDoc::AdjContrast(int iAdj) {
	iDispLow += iAdj;
	iDispHigh -= iAdj;
	CheckDispLimit();
	//if (iContrast + iContrast / 10 > iPixelMax - iPixelMin) return;
	//iContrast += iContrast / 10;
	dlgHist.UpdateParam();
	UpdateView();
}

void CGazoDoc::OnToolbarCntdown() 
{
	AdjContrast(-(iDispHigh - iDispLow)/5);
//	int iContrast = iDispHigh - iDispLow;
//	iDispLow -= iContrast / 5;
//	iDispHigh += iContrast / 5;
//	CheckDispLimit();
//	dlgHist.UpdateParam();
//	UpdateView();
}

void CGazoDoc::OnToolbarCntup()
{
	AdjContrast((iDispHigh - iDispLow)/5);
//	int iContrast = iDispHigh - iDispLow;
//	iDispLow += iContrast / 5;
//	iDispHigh -= iContrast / 5;
//	CheckDispLimit();
//	dlgHist.UpdateParam();
//	UpdateView();
}

void CGazoDoc::OnUpdateToolbarBright(CCmdUI* pCmdUI) 
{
	if (iDispLow >= iPixelMax - 1) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateToolbarDark(CCmdUI* pCmdUI) 
{
	if (iDispHigh <= iPixelMin + 1) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateToolbarCntdown(CCmdUI* pCmdUI) 
{
	//if (iContrast + iContrast / 10 > iPixelMax - iPixelMin) {pCmdUI->Enable(false); return;}
	if (iDispHigh - iDispLow >= iPixelMax - iPixelMin) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateToolbarCntup(CCmdUI* pCmdUI) 
{
	//if (iContrast - iContrast / 10 < 1) {pCmdUI->Enable(false); return;}
	if (iDispHigh - iDispLow <= 1) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnTomoReconst() 
{
	if (parentDoc) {parentDoc->OnTomoReconst(); return;}
	TErr err = 0;
	//
	if (!dlgReconst.m_hWnd) {
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
		dataPath = path_buffer;
		dataPrefix = fnm;
		dataSuffix = ext;
		//121013 dataPrefix = dataPrefix.SpanExcluding("0123456789");
		//
//AfxMessageBox(dataPrefix + dataSuffix + " 1");
		CString title = dataPath;
		if (dataSuffix.MakeUpper() != ".H5") {
			if (dataPath.GetLength()) {
				_stprintf_s(path_buffer, _MAX_PATH, dataPath.Left(dataPath.GetLength()-1));
				_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnm, _MAX_FNAME, NULL, 0);
				title = fnm;
			}
		} else {
			title = fnm;
		}
//AfxMessageBox(title);
		//
		if ( err = LoadLogFile() ) {error.Log(err); return;}
		SetFramesToExclude();

		//121013 iLenSinogr is determined in LoadLogFile
		const int ideg = (int)(log10((double)iLenSinogr)) + 1;
		const int ipos = dataPrefix.GetLength() - ideg;
		//190620 if ((ipos > 0)&&(dataSuffix.MakeUpper() != ".H5")) dataPrefix = dataPrefix.Left(ipos);
		if ((ipos > 0) && (dataSuffix.MakeUpper() != ".H5") && (dataSuffix.MakeUpper() != ".HIS")) dataPrefix = dataPrefix.Left(ipos);
		//160617 else dataPrefix = dataPrefix.SpanExcluding("0123456789");
		//CString msg; msg.Format("%d %d %s", ideg, ipos, dataPrefix); AfxMessageBox(msg);
		//
		CGazoApp* pApp = (CGazoApp*) AfxGetApp();
		//100315 if (pApp->prevPixelWidth > 0) dlgReconst.m_PixelWidth = pApp->prevPixelWidth;
		if (pApp->prevDlgReconst.iStatus == CDLGRECONST_BUSY) {
			dlgReconst.ParamCopyFrom(pApp->prevDlgReconst);
			dlgReconst.AdjustSliceWithBinning();
		}
		//141209 repetitive offset CT recon.
		if (dlgReconst.m_bOffsetCT) {//when given from ParamCopyFrom
			if ( err = LoadLogFile(dlgReconst.m_bOffsetCT) ) {error.Log(err); return;}
		}
		//090806
		if (dlgReconst.m_Outpath.IsEmpty()) dlgReconst.m_Outpath = dataPath;
		//111108
		if (dlgReconst.m_sDriftListPath.IsEmpty()) dlgReconst.m_sDriftListPath = dataPath;
		//121013===>
		if (dataSuffix.MakeUpper() == ".HIS") {
			CountFrameFromConvBat();
			//CString msg; msg.Format("121218 %d %d", maxHisFrame, iFramePerDataset); AfxMessageBox(msg);
			//dlgReconst.m_iDatasetSize = iFramePerDataset;
			//141229 dlgReconst.m_nDataset = maxHisFrame / iFramePerDataset;
		} else {//tif or img
			dlgReconst.m_nDataset = 1;
		}//===>121013
		if (dlgReconst.m_nDataset > 1) {
			CString fmt;
			fmt.Format("rec%%0%dd", (int)log10((double)(dlgReconst.m_nDataset)) + 1);
			dlgReconst.m_Suffix.Format(fmt, 0);
		}
		//
		//100315 if (!dlgReconst.m_hWnd) dlgReconst.Create(IDD_RECONST);
		dlgReconst.Create(IDD_RECONST);
		//090216
		POSITION pos = GetFirstViewPosition();
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		POINT coord;
		coord.x = 0; coord.y = 0;
		pv->ClientToScreen(&coord);
		dlgReconst.SetWindowPos(&CWnd::wndTop, coord.x, coord.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//
		dlgReconst.SetWindowText("Recon " + title);
	}
	if (dlgReconst.IsWindowVisible()) dlgReconst.SetForegroundWindow();
	else dlgReconst.ShowWindow(SW_SHOW);
//AfxMessageBox(dataPath + dataPrefix + dataSuffix + " 2");
}

TErr CGazoDoc::AllocPixelBuf(int ix, int iy) {
	//ixdim = ix; iydim = iy;
	//return 0;///////
	//&pPixel, &maxPixel
	TErr err = 0;
	const int nData = ix * iy;
	if (maxPixel < nData) {
		if (pPixel) delete [] pPixel;
		pPixel = NULL;
	}
	if (!pPixel) {
		try {pPixel = new int[nData];}
		catch(CException* e) {e->Delete(); err = 20111; return err;}
//		if ((pPixel = new int[nData]) == NULL) {err = 20111; return err;}
		maxPixel = nData;
		for (int i=0; i<maxPixel; i++) {pPixel[i] = 0;}
	}
	ixdim = ix; iydim = iy;
	return err;
}

TErr CGazoDoc::BatchReconst(RECONST_QUEUE* rq) {
	//Batch execution
	TErr err = 0;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	//090118 const int iavail = pApp->iAvailableCPU;
	//100515 const int iavail = pApp->dlgProperty.iCPU;
	//pApp->iAvailableCPU = 0;
	//
	TCHAR path_buffer[_MAX_PATH]; TCHAR fnm[_MAX_FNAME];
	_stprintf_s(path_buffer, _MAX_PATH, rq->dataPath.Left(rq->dataPath.GetLength()-1));
	_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnm, _MAX_FNAME, NULL, 0);
	const CString dataName = fnm;
	//
	const int iy1 = rq->iLayer1;
	const int iy2 = rq->iLayer2;
	const double fc1 = rq->dCenter1;
	const double fc2 = rq->dCenter2;
	const int iInterpolation = rq->iInterpolation;
	const int iBinning = (rq->iInterpolation == 0) ? 4 : ((rq->iInterpolation == 1) ? 2 : 1);
	ullLossFrameSet = rq->ullLossFrameSet;//210621 CalcAvgFromHis does not use rq, so the value is moved from rq to the native variable.
	//
	//int iy1, iy2; double fc1, fc2;
	//dlgReconst.GetLayer(&iy1, &iy2);
	if (iy1 < 0) return 21050;
	if (iy2 < 0) return 21050;
	if (iy1 >= rq->iYdim) return 21050;
	if (iy2 >= rq->iYdim) return 21050;
	if (iy1 > iy2) return 21050;
	//dlgReconst.GetCenter(&fc1, &fc2);
	//const int iInterpolation = dlgReconst.m_Interpolation;
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	bool btmp = bCmdLine;
	bCmdLine = true;
	double fc;
	//const int ideg = (int)(log10(iydim)) + 1;
	//161113 const int ideg = 4;
	const int ideg = (int)(log10((double)rq->iYdim)) + 1;
	//120501 log output
	CStdioFile flog;
	CString sLogFileName = "recviewlog.txt";
	if (!flog.Open(rq->dataPath + sLogFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
		sLogFileName = "0recviewlog.txt";
		if (!flog.Open(rq->dataPath + sLogFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
			sLogFileName = "_recviewlog.txt";
		} else {
			flog.Close();
		}
	} else {
		flog.Close();
	}
	if (flog.Open(rq->dataPath + sLogFileName, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		flog.SeekToEnd();
		struct _timeb tstruct;
		_ftime_s( &tstruct );
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line, sVer;
		sVer = pApp->sProgVersion; sVer.Replace('\n', ' ');
		line.Format("Reconst [%s] %s\n", stime.Left(24), sVer);
		flog.WriteString(line);
		if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) flog.WriteString(" Reconstruction kernel: x86/x64\n");
		else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
			if (pApp->dlgProperty.bUseCUDAFFT) flog.WriteString(" Reconstruction kernel: CUDA-FFT\n");
			else flog.WriteString(" Reconstruction kernel: CUDA\n");
		} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) flog.WriteString(" Reconstruction kernel: AMD\n");
		else flog.WriteString(" Reconstruction kernel: Unknown\n");
		if (rq->itexFileSuffix.MakeUpper() == ".HIS") {
			flog.WriteString(" Data file: " + rq->dataPath + rq->itexFilePrefix + rq->itexFileSuffix + "\n");
		} else {
			flog.WriteString(" Data files: " + rq->dataPath + rq->itexFilePrefix + "*" + rq->itexFileSuffix + "\n");
		}
		line.Format(" From layer %d (center=%.2f) to layer %d (center=%.2f)\n", rq->iLayer1, rq->dCenter1, rq->iLayer2, rq->dCenter2);
		flog.WriteString(line);
		if (rq->iLayer1 == rq->iLayer2) {
			line.Format("  Axis increment %.3f\n", rq->dAxisInc); flog.WriteString(line);
		}
		line.Format(" Dataset: %d\n", rq->iDatasetSel);
		if (rq->ullLossFrameSet != 0) {
			flog.WriteString(line);
			line.Format(" Frame(s) lost in each dataset (end->set0): %016I64x\n", rq->ullLossFrameSet);
		}
		flog.WriteString(line);
		line.Format(" Pixel width: %f um\n", rq->dPixelWidth);
		flog.WriteString(line);
		switch (rq->iFilter) {
			case CDLGRECONST_FILT_HAN: {line.Format(" Filter func: Hann, cutoff: %f\n", rq->dCutoff); break;}
			case CDLGRECONST_FILT_HAM: {line.Format(" Filter func: Hamming, cutoff: %f\n", rq->dCutoff); break;}
			case CDLGRECONST_FILT_RAMP: {line.Format(" Filter func: Rectangular, cutoff: %f\n", rq->dCutoff); break;}
			case CDLGRECONST_FILT_PARZN: {line.Format(" Filter func: Parzen, cutoff: %f\n", rq->dCutoff); break;}
			case CDLGRECONST_FILT_BUTER: {line.Format(" Filter func: Butterworth, cutoff: %f, order: %f\n", rq->dCutoff, rq->dOrder); break;}
		}
		flog.WriteString(line);
		line.Format(" Tilt angle: %.1f deg\n Trimming: %d pixel\n", rq->fTiltAngle, rq->iTrimWidth);
		flog.WriteString(line);
		switch (rq->iInterpolation) {
			case 0: {line = " Binning/zooming: x4 binning\n"; break;}
			case 1: {line = " Binning/zooming: x2 binning\n"; break;}
			case 2: {line = " Binning/zooming: none\n"; break;}
			case 3: {line = " Binning/zooming: x2 zooming\n"; break;}
			case 4: {line = " Binning/zooming: x4 zooming\n"; break;}
			case 5: {line = " Binning/zooming: x8 zooming\n"; break;}
		}
		flog.WriteString(line);
		flog.WriteString(" Experiment log file: " + rq->logFileName + "\n");
		flog.WriteString(" Output files: " + rq->outFilePath + rq->outFilePrefix + "*.tif\n");
		line.Format(" Raw sinogram dimensions: x=%d, y=%d\n", rq->iRawSinoXdim, rq->iSinoYdim);
		flog.WriteString(line);
		line.Format(" Source image dimensions: x=%d, y=%d\n", rq->iXdim, rq->iYdim);
		flog.WriteString(line);
		if (rq->bOffsetCT) flog.WriteString(" Offset CT enabled\n");
		if ((rq->dReconFlags) & RQFLAGS_ANGINTP) flog.WriteString(" Angular interpolation enabled\n");
		if ((rq->dReconFlags) & RQFLAGS_ZERNIKE) flog.WriteString(" Zernike contrast\n");
		if ((rq->dReconFlags) & RQFLAGS_DRIFTPARAMS) {
			flog.WriteString(" Drift correction\n");
			line.Format("  Drift start frame=%d, end frame=%d\n", rq->drStart, rq->drEnd);
			flog.WriteString(line);
			if (rq->drOmit) {
				flog.WriteString("  Drifting frames were omitted\n");
			} else {
				line.Format("  Drift vector x=%.2f, y=%.2f\n", rq->drX, rq->drY);
				flog.WriteString(line);
			}
		}
		if ( !(rq->sFramesToExclude.IsEmpty()) ) {
			flog.WriteString(" Frames not used in reconstruction: ");
			flog.WriteString(rq->sFramesToExclude + "\n");
			flog.WriteString("  (frame# h0000=HISdark/white; b0000=HDF5dark; w0000=HDF5white; s0000=skip; 0000=replacedWithAdjacents)\n");
		}
		line.Format(" Sample frame range: %d - %d\n", rq->iSampleFrameStart, rq->iSampleFrameEnd);
		flog.WriteString(line);
		if ((rq->dReconFlags) & RQFLAGS_DRIFTLIST) {
			flog.WriteString(" Drift correction\r\n  Drift list: ");
			flog.WriteString(rq->sDriftListPath + "\n");
		}
		flog.WriteString("---------------------------------------------------\n");
		flog.Close();
	}
	//
	/*090724
	MEMORYSTATUS memory;
	memory.dwLength = sizeof(memory);
	GlobalMemoryStatus(&memory);
	unsigned int imem = (unsigned int)(memory.dwTotalPhys);
	*/
	MEMORYSTATUSEX memory;
	memory.dwLength = sizeof(memory);
	GlobalMemoryStatusEx(&memory);
	DWORDLONG dwlmem = memory.ullTotalPhys / 1024;//memory in kbytes
	#ifndef _WIN64
	if (dwlmem > (1<<21)) dwlmem = (1<<21);//2 GB max for x86 platform
	#endif
	int iMultiplex = (int)(dwlmem / (iLenSinogr * rq->iXdim / iBinning * sizeof(short) / 1024) * pApp->dlgProperty.iMemory / 100);
	//CString line; line.Format("090724 %d %d", imem, iMultiplex); AfxMessageBox(line);//090724
	if (rq->bOffsetCT) iMultiplex /= 2;
	if ((iy2 - iy1 + 1)/iBinning < iMultiplex) iMultiplex = (iy2 - iy1 + 1)/iBinning;
	iMultiplex = (iMultiplex < 1) ? 1 : iMultiplex;
	//progress bar
	const int iCycle = (int)ceil((iy2 - iy1 + 1.0) / iBinning / iMultiplex);
	const int iProgressEnd = PROGRESS_BAR_UNIT * iCycle + iMultiplex * iCycle * PROGRESS_BAR_UNIT;
	if (dlgReconst.m_hWnd)	{
		dlgReconst.m_Progress.SetRange32(0, iProgressEnd);
		dlgReconst.m_Progress.SetPos(0);
		dlgReconst.m_Progress.SetStep(1);
	}
	const int iZooming = (iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int ipintp = (int) pow((double)2, iZooming);
	const int ixdimp = rq->iXdim * ipintp / iBinning;
	//Body code
	if (iy1 != iy2) {//181202
		for (int i=iy1; i<=iy2; i+=iMultiplex*iBinning) {
			if (i + iBinning-1 > iydim-1) break;
			CString fn;
			//int iyend = ((i+iMultiplex*iBinning-1) < iy2) ? (i+iMultiplex*iBinning-1) : iy2;
			int iyend = i+iMultiplex*iBinning-1;
			if (iyend > iy2) {
				iyend = iy2;
				iMultiplex = (iy2 - i + 1) / iBinning;
				if ((iy2 - i + 1) % iBinning) iMultiplex++;
			}
			fn.Format(" layer %d-%d", i, iyend);
			pf->m_wndStatusBar.SetPaneText(1, "Generating sinogram " + dataName + fn);
			double deltaCent = 0;
			if (iy1 == iy2) {
				fc = fc1;
			} else {
				deltaCent = (fc2 - fc1) / (iy2 - iy1);
				fc = fc1 + deltaCent * (i - iy1);
			}
			if ( err = GenerateSinogram(rq, i, fc, deltaCent, iMultiplex) ) {error.Log(err); return err;}
			//if (dlgReconst.m_hWnd) dlgReconst.m_Progress.SetPos(iProgressEnd/2);
			if (dlgReconst.iStatus == CDLGRECONST_STOP) {
				if (pf) {pf->m_wndStatusBar.SetPaneText(1, "Aborted");}
				return 0;
			}
			//layer
			for (int j=0; j<iMultiplex*iBinning; j+=iBinning) {
				::ProcessMessage();
				if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
				int iLayer = i + j;
				if (iLayer > iy2) break;
				if (iLayer + iBinning-1 > iydim-1) break;
				if (iy1 == iy2) fc = fc1;
				else fc = fc1 + (fc2 - fc1) * (iLayer - iy1) / (iy2 - iy1);
				fn.Format(" layer %d center %.3f", iLayer, fc);
				pf->m_wndStatusBar.SetPaneText(1, "Backprojection " + dataName + fn);
				double tcpu = 0; float pixelBase = 0, pixelDiv = 1;
				if ( err = DeconvBackProj(rq, fc, iMultiplex, j/iBinning, 
											&numOfSampleSinogr, &tcpu, &pixelBase, &pixelDiv) ) {
					error.Log(err);
				}
				if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
				fn.Format("%09d", iLayer);
				fn = rq->outFilePath + rq->outFilePrefix + fn.Right(ideg) + ".tif";
				CFile file;
				if (!file.Open(fn, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) continue;
				CString imageDesc;
				imageDesc.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%.6f ",
					numOfSampleSinogr, rq->dPixelWidth, pixelDiv, pixelBase, fc, 
					rq->iFilter + 1, 55.111697, 0.461227);
				if (err = WriteTifMonochrome16(&file, iReconst, ixdimp, ixdimp, imageDesc)) error.Log(err);
				file.Close();
			}
			if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
		}
	} else {//(iy1 == iy2)
		for (fc=floor(fc1 + 0.5); fc<=fc2; fc+=rq->dAxisInc) {
			::ProcessMessage();
			CString fn;
			fn.Format(" layer %d center %.3f", iy1, fc);
			pf->m_wndStatusBar.SetPaneText(1, "Generating sinogram " + dataName + fn);
			if ( err = GenerateSinogram(rq, iy1, fc, 0, 1) ) {error.Log(err); return err;}
			if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
			//layer
			int iLayer = iy1;
			if (iLayer + iBinning-1 > iydim-1) break;
			fn.Format(" layer %d center %.3f", iLayer, fc);
			pf->m_wndStatusBar.SetPaneText(1, "Backprojection " + dataName + fn);
			double tcpu = 0; float pixelBase = 0, pixelDiv = 1;
			if ( err = DeconvBackProj(rq, fc, 1, 0, &numOfSampleSinogr, &tcpu, &pixelBase, &pixelDiv) ) {
				error.Log(err);
			}
			if (dlgReconst.iStatus == CDLGRECONST_STOP) break;
			CString fmt; fmt.Format("%%0%dd_%%.1f", ideg);
			fn.Format(fmt, iLayer, fc);
			fn = rq->outFilePath + rq->outFilePrefix + fn + ".tif";
			CFile file;
			if (!file.Open(fn, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) continue;
			CString imageDesc;
			imageDesc.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%.6f ",
				numOfSampleSinogr, rq->dPixelWidth, pixelDiv, pixelBase, fc, 
				rq->iFilter + 1, 55.111697, 0.461227);
			if (err = WriteTifMonochrome16(&file, iReconst, ixdimp, ixdimp, imageDesc)) error.Log(err);
			file.Close();
			if (rq->bOffsetCT) ResetSinogram();
		}
	}//if (iy1 != iy2)
	bCmdLine = btmp;
	if (dlgReconst.m_hWnd) dlgReconst.m_Progress.SetPos(iProgressEnd);
	if (dlgReconst.iStatus == CDLGRECONST_STOP) {
		pf->m_wndStatusBar.SetPaneText(1, "Aborted");
	} else {
		pf->m_wndStatusBar.SetPaneText(1, "Finished");
	}
	pf->m_wndStatusBar.SetPaneText(0, "Ready");
	//100515==> delete GPU Memory if any
	GPUMemFree(CDLGPROPERTY_PROCTYPE_ND, true);
	//if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
	//	int nCPU = (int)(pApp->dlgProperty.iCUDA);
	//	for (int i=0; i<nCPU; i++) {CudaReconstMemFree(&(ri[i]));}
	//} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
	//	int nCPU = (int)(pApp->dlgProperty.iATIstream);
	//	for (int i=0; i<nCPU; i++) {CLReconstMemFree(&(ri[i]));}
	//}
	//==>100515
	return err;
}

void CGazoDoc::ShowTomogram(RECONST_QUEUE* rq, int iy, double fc, CGazoDoc* pdTarget) {//, int ifilter, int iInterpolation, CString fsuffix) {
	TErr err = 0;
	const int iMultiplex = 1; const int iOffset = 0;
	const int ifilter = rq->iFilter;
	//const int iInterpolation = rq->iInterpolation;
	const CString fsuffix = rq->outFilePrefix;
	//
	if ( err = GenerateSinogram(rq, iy, fc, 0, iMultiplex) ) {error.Log(err); CString line; line.Format("%d", err); AfxMessageBox(line); return;}
	double tcpu = 0; float pixelBase = 0, pixelDiv = 1;
	err = DeconvBackProj(rq, fc, iMultiplex, iOffset, &numOfSampleSinogr, &tcpu, &pixelBase, &pixelDiv);
//int ipx = 34207;
//while (ipx == 34207) {
//	err = DeconvBackProj(rq, fc, iMultiplex, iOffset, &numOfSampleSinogr, &tcpu, &pixelBase, &pixelDiv);
//	ipx = iReconst[nReconst * 1001 + 999];
//}
//181223
//CString msg; msg.Format("%d", iReconst[nReconst * 1001 + 999]); AfxMessageBox(msg);
//
	//delete GPU memory if any
	//190107 moved to CDlgReconst::OnCancel()
//	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
//	if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
//		int nCPU = (int)(pApp->dlgProperty.iCUDA);
//		for (int i=0; i<nCPU; i++) {CudaReconstMemFree(&(ri[i]));}
//	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
//		int nCPU = (int)(pApp->dlgProperty.iATIstream);
//		for (int i=0; i<nCPU; i++) {CLReconstMemFree(&(ri[i]));}
//	}
	if (err) {error.Log(err); return;}
	if ((dlgReconst.iStatus == CDLGRECONST_STOP)||(dlgReconst.iStatus & CDLGRECONST_WHEEL)) return;
	//Generate View
	bool bReuseView = false;
	if (pdTarget) {
		if (pdTarget->ixdim == nReconst) bReuseView = true;
	}
	if (bReuseView) {
		CGazoDoc* pcd = pdTarget;
		const int ixdimp = nReconst;
		const int ixdim2 = ixdimp * ixdimp;
		//if ((pcd->pPixel = new int[ixdim2]) == NULL) return;
		pcd->SetModifiedFlag(TRUE);
		pcd->pixBase = pixelBase;
		pcd->pixDiv = pixelDiv;
		pcd->fCenter = (float)(-fc);
		for (int i = 0; i < ixdim2; i++) { pcd->pPixel[i] = iReconst[i]; }
		pcd->UpdateView();
		CString title, fn;
		fn.Format("%09d", iy);
		fn = fsuffix + fn.Right((int)(log10((double)rq->iYdim)) + 1);
		title.Format("%s reconstructed (y=%d center=%.1f) from %s / %.2f sec",
			fn, iy, fc, rq->dataPath, tcpu);
		pcd->SetTitle(title);
	} else {
		CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
		CGazoDoc* pcd = pcv->GetDocument();
		const int ixdimp = nReconst;
		const int ixdim2 = ixdimp * ixdimp;
		if ((pcd->pPixel = new int[ixdim2]) == NULL) return;
		pcd->maxPixel = ixdim2;
		pcd->ixdim = ixdimp;
		pcd->iydim = ixdimp;
		pcd->parentDoc = this;
		pcd->SetModifiedFlag(TRUE);
		pcd->pixBase = pixelBase;
		pcd->pixDiv = pixelDiv;
		pcd->fCenter = (float)(-fc);
		for (int i=0; i<ixdim2; i++) {pcd->pPixel[i] = iReconst[i];}
		pcd->UpdateView(/*bInit=*/true);
		CString title, fn;
		fn.Format("%09d", iy);
		//161113 fn = fsuffix + fn.Right((int)(log10((double)iLenSinogr)) + 1);
		fn = fsuffix + fn.Right((int)(log10((double)rq->iYdim)) + 1);
		title.Format("%s reconstructed (y=%d center=%.1f) from %s / %.2f sec",
											fn, iy, fc, rq->dataPath, tcpu);
		pcd->SetTitle(title);
		pcd->dataPath = rq->dataPath;
		pcd->dataPrefix = rq->itexFilePrefix;
		pcd->dataSuffix = rq->itexFileSuffix;
		pcd->dlgReconst.iContext = this->dlgReconst.iContext;
		//CString line; line.Format("%d", pcd->dlgReconst.iContext); AfxMessageBox(line);
	}
	return;
}

void CGazoDoc::ShowSinogram(RECONST_QUEUE* rq, int iy, double fc) {
	TErr err = 0;
//CString msg; msg.Format("iLenSinogr=%d", iLenSinogr); AfxMessageBox(msg);
	const int iMultiplex = 1; const int iOffset = 0;
	const int ifilter = rq->iFilter;
	//const int iInterpolation = rq->iInterpolation;
	const CString fsuffix = rq->outFilePrefix;
	//
	if ( err = GenerateSinogram(rq, iy, fc, 0, iMultiplex) ) {error.Log(err); CString line; line.Format("%d", err); AfxMessageBox(line); return;}
	if (dlgReconst.iStatus == CDLGRECONST_STOP) return;
	//Generate View
	CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
	CGazoDoc* pcd = pcv->GetDocument();
	pcd->parentDoc = this;
	pcd->SetModifiedFlag(TRUE);
	pcd->pixBase = 0;
	pcd->pixDiv = 0;
	pcd->fCenter = (float)(-fc);
	const int iBinning = (rq->iInterpolation == 0) ? 4 : ((rq->iInterpolation == 1) ? 2 : 1);
	const int ixdims = rq->iRawSinoXdim / iBinning;
	//160803 const int ixdims = maxOfSinogrWidth;
	if (rq->bOffsetCT) {
		const int iydims = maxOfSinogrLen - 1;
		const int ixydim = ixdims * iydims;
		if ((pcd->pPixel = new int[ixydim]) == NULL) return;
		pcd->maxPixel = ixydim;
		pcd->ixdim = ixdims;
		pcd->iydim = iydims;
		for (int j=0; j<iydims; j++) {
			for (int i=0; i<ixdims; i++) {
				pcd->pPixel[i + j * ixdims] = (ofSinogr[j+1])[i];//skip y=0 because intensity=0
			}
		}
	} else {
		//const int iydims = maxSinogrLen - 1;
		int iydims = 0;
		for (int j=0; j<maxSinogrLen - 1; j++) {
			if (!(bInc[j] & CGAZODOC_BINC_SAMPLE)) continue;//skip incident
			if (fdeg[j] > 180.) continue;//skip the last image
			iydims++;
		}
		const int ixydim = ixdims * iydims;
		if ((pcd->pPixel = new int[ixydim]) == NULL) return;
		pcd->maxPixel = ixydim;
		pcd->ixdim = ixdims;
		pcd->iydim = iydims;
		int k = 0;
		for (int j=0; j<maxSinogrLen - 1; j++) {
			if (!(bInc[j] & CGAZODOC_BINC_SAMPLE)) continue;
			if (fdeg[j] > 180.) continue;
			for (int i=0; i<ixdims; i++) {
				pcd->pPixel[i + k * ixdims] = (iSinogr[j])[i];
			}
			k++;
		}
	}
	pcd->UpdateView(/*bInit=*/true);
	CString title;
	title.Format("%s sinogram (y=%d)", rq->dataPath, iy);
	pcd->SetTitle(title);
	pcd->dataPath = rq->dataPath;
	pcd->dataPrefix = rq->itexFilePrefix;
	pcd->dataSuffix = rq->itexFileSuffix;
	return;
}

TErr CGazoDoc::LoadLogFileAlloc(DWORD ilen) {
	if (ilen > (unsigned int)maxImageEntry) {
		if (fname) delete [] fname;
		if (fexp) delete [] fexp;
		if (fdeg) delete [] fdeg;
		if (bInc) delete [] bInc;
		try {
			fname = new CString[ilen];
			fexp = new float[ilen];
			fdeg = new float[ilen];
			bInc = new char[ilen];
		}
		catch(CException* e) {
			e->Delete();
			AfxMessageBox("Run out of memory");
			if (fname) delete [] fname;
			if (fexp) delete [] fexp;
			if (fdeg) delete [] fdeg;
			if (bInc) delete [] bInc;
			fname = NULL;
			fexp = NULL;
			fdeg = NULL;
			bInc = NULL;
			maxImageEntry = 0;
//			fclose(flog);
			return 16062801;
		}
		maxImageEntry = ilen;
	}
	return 0;
}

TErr CGazoDoc::LoadLogFile(BOOL bOffsetCT) {
	DWORD ipos = 0;
//	__int64 lDataSize0 = hdf5.m_plDataSize[0];
	if (dataSuffix.MakeUpper() == ".H5") {
		//APS HDF5
		CString fn = dataPath + dataPrefix + dataSuffix;
		CFile fhdf5;
		if (!fhdf5.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			fhdf5.Close(); if (!bFromQueue) AfxMessageBox("file open error"); return 1652601;
		}
		//get m_lHDF5DataSize0
		hdf5.SetFile(&fhdf5);
		TErr err = 0;
		if (err = hdf5.ReadSuperBlock(NULL)) return err;
		if (err = hdf5.FindChildSymbol("exchange", -1, NULL)) return err;
		hdf5.MoveToChildTree();
		if (err = hdf5.FindChildSymbol("data", -1, NULL)) return err;
		if (hdf5.m_sChildTitle.Left(4) != "data") return 16052221;
		if (err = hdf5.GetDataObjHeader(NULL)) return err;
		m_lHDF5DataSize0 = hdf5.m_plDataSize[0];
		//
		err = ReadHDF5Theta(&fhdf5, &hdf5, NULL, &ipos, &fileComment);
		if (err) {//fly scan
			m_iFlag &= ~CGAZODOC_FLAG_HDF5STEPSCAN;
			fhdf5.Close();
//161107	iHDF5DummyFrame = 1;//skip the first frame because it's a white image
			//161106 ipos = (DWORD)(lDataSize0 - iHDF5DummyFrame - 1);//-1 is to skip the last image. It's probably 180 deg image.
			ipos = (DWORD)(m_lHDF5DataSize0);//-1 is to skip the last image. It's probably 180 deg image.
			ipos += 2;//+2 to include pre and post white images
//CString msg; msg.Format("160617 %d %lld", ipos, hdf5.m_plDataSize[0]); AfxMessageBox(msg);
			if (err = LoadLogFileAlloc((DWORD)ipos)) return err;
			for (int i=1; i<=(int)(ipos-2); i++) {
				fdeg[i] = 180.f * i / (ipos-2 - 1);//-1 to exclude the last sample image which is same with a flat image
				fexp[i] = fdeg[i];
				fname[i].Format("%05d", i);
				bInc[i] = CGAZODOC_BINC_SAMPLE;
			}
			//pre white image
			fdeg[0] = fdeg[1] - 1;
			fexp[0] = fexp[1] - 1;
			fname[0].Format("%05d", 0);
			bInc[0] = CGAZODOC_BINC_WHITE;
			//
			fdeg[ipos-1] = fdeg[ipos-2] + 1;
			fexp[ipos-1] = fexp[ipos-2] + 1;
			fname[ipos-1].Format("%05d", ipos-1);
			bInc[ipos-1] = CGAZODOC_BINC_WHITE;
			iLenSinogr = ipos + 1;//including dark.img
//CString msg; msg.Format("160629-2 %d", iLenSinogr); AfxMessageBox(msg);
		} else {//step scan
//161112			if (ipos > 450) iHDF5DummyFrame = 3;//ignore the first 3 frames when it's real data but not the test set
			m_iFlag |= CGAZODOC_FLAG_HDF5STEPSCAN;
			ipos += 2;//+2 to include pre and post white images
			const DWORD ilen = ipos;
			if (err = LoadLogFileAlloc(ilen)) {fhdf5.Close(); return err;}
			err = hdf5.ReadTheta(&(fdeg[1]), NULL);//fdeg[0] is the entry for pre white image;
			fhdf5.Close();
			if (err) return err;
			//fill with dummy data
			for (int i=1; i<(int)(ipos-1); i++) {
				fexp[i] = fdeg[i];
				fname[i].Format("%05d", i);
				bInc[i] = CGAZODOC_BINC_SAMPLE;
			}
			//pre white image
			fdeg[0] = fdeg[1] - 1;
			fexp[0] = fexp[1] - 1;
			fname[0].Format("%05d", 0);
			bInc[0] = CGAZODOC_BINC_WHITE;
			//
//161112			if (fabs(fabs(fdeg[1] - fdeg[ipos-2]) - 180.) < 1E-6) {
//				//if the last sample frame is 180 deg from the start, 
//				// do not use that frame and replace with white image
//				bInc[ipos-2] = CGAZODOC_BINC_WHITE;
//				iLenSinogr = ipos;//place dark.img at the end of sinogram
//			} else {
				//generate an entry for the post white image
				fdeg[ipos-1] = fdeg[ipos-2] + 1;
				fexp[ipos-1] = fexp[ipos-2] + 1;
				fname[ipos-1].Format("%05d", ipos-1);
				bInc[ipos-1] = CGAZODOC_BINC_WHITE;
				iLenSinogr = ipos + 1;//including dark.img
//			}
		}
	} else {
		//SPring-8 output.log
		CString fn = logPath;
		if (fn.IsEmpty()) {
			fn = dataPath + "output.log";
		}
		FILE* flog = NULL;
		errno_t errn = fopen_s(&flog, fn, "rt");
		if (errn) {
			if (bFromQueue) return 16062802;
			logPath.Empty();
			static char BASED_CODE szFilter[] = "Log files (*.log)|*.log|All Files (*.*)|*.*||";
			static char BASED_CODE defaultExt[] = ".log";
			CFileDialog fileDlg(TRUE, defaultExt, fn,
									OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
			if (fileDlg.DoModal() == IDCANCEL) {
				AfxMessageBox("No output.log file speciifed");
				return 21013;
			}
			fn = fileDlg.GetPathName();
			errn = fopen_s(&flog, fn, "rt");
			if (errn) {
				AfxMessageBox("File not found");
				return 21010;
			}
		}
		CStdioFile stdioLog(flog);
//		DWORD ipos = 0;
		CString line;
		while (stdioLog.ReadString(line)) {
			if (line.IsEmpty()) continue;
			ipos++;
		}
		if (ipos == 0) {//==>211204
			if (!bFromQueue) AfxMessageBox("Empty output.log file"); 
			fclose(flog); return 21120401;
		}//==>211204
		const DWORD ilen = ipos;
		stdioLog.SeekToBegin();
		TErr err = 0;
		if (err = LoadLogFileAlloc(ilen)) {fclose(flog); return err;}
		ipos = 0;
		char cfname[20];
		while (stdioLog.ReadString(line)) {
			if (line.IsEmpty()) continue;
			if (ipos >= ilen) { if (!bFromQueue) AfxMessageBox("Too much output.log lines"); err = 21120402; break; }
			if (sscanf_s(line, "%s %f %f %c", cfname, 20, &(fexp[ipos]), &(fdeg[ipos]), &(bInc[ipos]), 1) != 4) {
				if (!bFromQueue) AfxMessageBox("An irregular line found in output.log:\r\n" + line); err = 21120403; break;
			}
			fname[ipos] = cfname;
			if (bInc[ipos] == '1') bInc[ipos] = CGAZODOC_BINC_SAMPLE; else bInc[ipos] = CGAZODOC_BINC_WHITE;
			ipos++;
		}
		fclose(flog);
		if (err) return err;//211204
		logPath = fn;
		//141204==>
		//degree column seems to be given in #pulse in some beamtimes.
//161103		int ideg = (int)(fdeg[ipos-1]);
//161103		if ((ideg != 180)&&(ideg != 360)) {
		//210618 if ((abs(fdeg[ipos-1] - 180) > 3)&&(abs(fdeg[ipos-1] - 360) > 3)) {
		if (abs(fdeg[ipos - 1]) > 10000) {
			for (unsigned int i=0; i<ipos; i++) {fdeg[i] /= 500.;}
//161112
//			const int nframe = (int)(ipos / 10) * 10;
//			double dstep = 180.0 / nframe;
//			if (bOffsetCT) dstep = 360.0 / nframe;
//			double dangle = 0.0;
//			for (unsigned int i=0; i<ipos-3; i++) {
//				fdeg[i] = (float)dangle;
//				dangle += dstep;
//			}
//			fdeg[ipos-3] = fdeg[ipos-4];
//			fdeg[ipos-2] = fdeg[ipos-4] + (float)dstep;
//			fdeg[ipos-1] = fdeg[ipos-2];
			//CString msg = "";
			//msg.Format("output.log angle\r\n%d %f\r\n%d %f\r\n%d %f\r\n%d %f\r\n%d %f",
			//	0, fdeg[0], ipos-4, fdeg[ipos-4], ipos-3, fdeg[ipos-3], 
			//	ipos-2, fdeg[ipos-2], ipos-1, fdeg[ipos-1]);
			//AfxMessageBox(msg);
		}
		//==>141204
		iLenSinogr = ipos + 1;//including dark.img
	}
if (bDebug) {CString msg; msg.Format("160629-1 iLenSinogr=%d ipos=%d", iLenSinogr, ipos); AfxMessageBox(msg);}
	//
//	for (int i=0; i<ipos; i++) {
//		CString line; line.Format("%s %f %f %d\r\n", fname[i], fexp[i], fdeg[i], bInc[i]);
//		msg += line;
//	}
//	CDlgMessage dlg;
//	dlg.m_Msg = msg;
//	dlg.DoModal();
//	msg.Format("%d", ipos); AfxMessageBox(msg);

	return 0;
}

TErr CGazoDoc::SetFramesToExclude() {
	//161105
	if (!m_sFramesToExclude.IsEmpty()) return 0;
	const int ipos = iLenSinogr - 1;
	m_sFramesToExclude = " ";
	const int idigit = (int)log10((double)ipos) + 1;
	CString fmt;
	fmt.Format(" s%%0%dd", idigit);
//	float fdegmin = FLT_MAX, fdegmax = -FLT_MAX;
//	for (int i=0; i<(int)ipos; i++) {
//		fdegmin = ((fdeg[i] < fdegmin)&&(bInc[i] & CGAZODOC_BINC_SAMPLE)) ? fdeg[i] : fdegmin;
//		fdegmax = ((fdeg[i] > fdegmax)&&(bInc[i] & CGAZODOC_BINC_SAMPLE)) ? fdeg[i] : fdegmax;
//	}
	float fdegStart = 0, fdegEnd = 180;
	for (int i=0; i<(int)ipos; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		fdegStart = fdeg[i]; break;
	}
	for (int i=ipos-1; i>=0; i--) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		fdegEnd = fdeg[i]; break;
	}
	const double dDegEnd = (abs(fdegEnd - fdegStart - 360) < 10) ? 359.99 : 179.99;
	//m_iDlgFL_SampleFrameStart represents frame# for HDF5 and the fdeg[] array position for other formats
	if (dataSuffix.MakeUpper() == ".H5") {
		if ((m_iFlag & CGAZODOC_FLAG_HDF5STEPSCAN)&&(m_lHDF5DataSize0 > 0)) {
			const int inSampleFrames = ipos - 2;
			dlgReconst.m_iDlgFL_SampleFrameStart = (int)(m_lHDF5DataSize0 - inSampleFrames);
			for (int i=0; i<dlgReconst.m_iDlgFL_SampleFrameStart; i++) {
				CString line; line.Format(fmt, i); m_sFramesToExclude += line;
			}
			for (int i=1; i<(int)ipos; i++) {
				if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
				if (fdeg[i] - fdegStart >= dDegEnd) {
					CString line; line.Format(fmt, i-1 + dlgReconst.m_iDlgFL_SampleFrameStart); m_sFramesToExclude += line;
				} else {
					dlgReconst.m_iDlgFL_SampleFrameEnd = i-1 + dlgReconst.m_iDlgFL_SampleFrameStart;
				}
			}
		} else {
			//dlgReconst.m_iDlgFL_SampleFrameStart = i-1 because an averaged white frame is not at the beginning of the "data" block
			for (int i=0; i<(int)ipos; i++) {
				if ((bInc[i] & CGAZODOC_BINC_SAMPLE)&&(fdeg[i] == fdegStart)) {dlgReconst.m_iDlgFL_SampleFrameStart = i-1; break;}
			}
			for (int i=1; i<(int)ipos; i++) {
				if (bInc[i] & CGAZODOC_BINC_SAMPLE) {
					if (fdeg[i] - fdegStart >= dDegEnd) {
						CString line; line.Format(fmt, i-1); m_sFramesToExclude += line;
					} else {
						dlgReconst.m_iDlgFL_SampleFrameEnd = i-1;
					}
				}
			}
			CString fmt2; fmt2.Format(" b%%0%dd", idigit);
			CString line; line.Format(fmt2, 0);
			m_sFramesToExclude += line;
		}

		if (bDebug) {
			CString line; 
			line.Format("ipos: %d\r\nstart: %d\r\nend: %d\r\nexclude: %s", 
				ipos, dlgReconst.m_iDlgFL_SampleFrameStart, dlgReconst.m_iDlgFL_SampleFrameEnd,
				m_sFramesToExclude);
			AfxMessageBox(line);
		}
	} else {
		//dlgReconst.m_iDlgFL_SampleFrameStart = 1 because an averaged white frame is at the beginning
		//201125 dummy frames in dark and flat series are not listed here but detected in DlgFrameList::OnInitDialog
		//because the detection is not necessary for the recon calc (CalcAvgFromHis reads convList) but needed for consistent displaying in DlgFrameList
		//210619 update for 190deg rotation
		double dnotch = 0.1;
		for (int i = 1; i < ipos; i++) {
			if (fdeg[i] != fdeg[i - 1]) {dnotch = fabs(fdeg[i] - fdeg[i - 1]); break;}
		}
		for (int i = ipos-1; i >= 1; i--) {
			if (bInc[i] & CGAZODOC_BINC_SAMPLE) {
				if (fdeg[i] <= fdegEnd - dnotch) {
					dlgReconst.m_iDlgFL_SampleFrameEnd = i;
					break;
				}
				else {
					CString line; line.Format(fmt, i); m_sFramesToExclude += line;
				}
			}
		}
		const double dFLend = dlgReconst.m_iDlgFL_SampleFrameEnd ? fdeg[dlgReconst.m_iDlgFL_SampleFrameEnd] : fdegEnd;
		for (int i = 0; i < ipos; i++) {
			if (bInc[i] & CGAZODOC_BINC_SAMPLE) {
				if (dFLend - fdeg[i] >= dDegEnd) {
					CString line; line.Format(fmt, i); m_sFramesToExclude += line;
				}
				else {
					dlgReconst.m_iDlgFL_SampleFrameStart = i;
					break;
				}
			}
		}
		//210619
		//for (int i=0; i<(int)ipos; i++) {
		//	if ((bInc[i] & CGAZODOC_BINC_SAMPLE)&&(fdeg[i] == fdegStart)) {dlgReconst.m_iDlgFL_SampleFrameStart = i; break;}
		//}
		//for (int i=0; i<(int)ipos; i++) {
		//	if (bInc[i] & CGAZODOC_BINC_SAMPLE) {
		//		if (fdeg[i] - fdegStart >= dDegEnd) {
		//			CString line; line.Format(fmt, i); m_sFramesToExclude += line;
		//		} else {
		//			dlgReconst.m_iDlgFL_SampleFrameEnd = i;
		//		}
		//	}
		//}

		if (bDebug) {
			CString line;
			line.Format("fdegEnd %f\r\nstart: %d %f\r\nend: %d %f\r\nexclude: %s", 
				fdegEnd,
				dlgReconst.m_iDlgFL_SampleFrameStart, fdeg[dlgReconst.m_iDlgFL_SampleFrameStart], 
				dlgReconst.m_iDlgFL_SampleFrameEnd, fdeg[dlgReconst.m_iDlgFL_SampleFrameEnd],
				m_sFramesToExclude);
			AfxMessageBox(line);
		}
	}

	return 0;
}

TErr CGazoDoc::SetConvList(CString sDataPath, CString sFilePrefix, CString sFileSuffix,
							int iDatasetSel, CMainFrame* pf) {
	//set iFramePerDataset;
	CountFrameFromConvBat();
	CString frmPrefix = "";
	//141229 const int nset = maxHisFrame / iFramePerDataset;
	const int nset = dlgReconst.m_nDataset;
	if (nset > 1) {
		const int idigit = (int)log10((double)nset) + 1;
		CString fmt;
		fmt.Format("_%%0%dd", idigit);
		frmPrefix.Format(fmt, iDatasetSel);
	}
	//alloc convList
	const DWORD ilen = iLenSinogr;//rq->iSinoYdim + 1;
	if (ilen > (unsigned int)maxConvList) {
		if (convList) delete [] convList;
		convList = new CString[ilen];
		maxConvList = ilen;
	}
	if (convList == NULL) {
		AfxMessageBox("Run out of memory");
		maxConvList = 0;
		return 21014;
	}
	for (unsigned int i=0; i<maxConvList; i++) {convList[i].Empty();}
	//open conv.bat file
	CString fn = sDataPath + "conv.bat";
	FILE* fconv = NULL;
	errno_t errn = fopen_s(&fconv, fn, "rt");
	if (errn) {
		if (bFromQueue) return 21014;
		logPath.Empty();
		static char BASED_CODE szFilter[] = "Conv.bat files (*.bat)|*.bat|All Files (*.*)|*.*||";
		static char BASED_CODE defaultExt[] = ".bat";
		CFileDialog fileDlg(TRUE, defaultExt, fn,
													OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
		if (fileDlg.DoModal() == IDCANCEL) {
			AfxMessageBox("No conv.bat file speciifed");
			return 21013;
		}
		fn = fileDlg.GetPathName();
		errn = fopen_s(&fconv, fn, "rt");
		if (errn) {
			AfxMessageBox("File not found");
			return 21010;
		}
	}
	CStdioFile stdioConv(fconv);
	CString line;
	//DWORD ipos = 0;
	//char cfname[20];
	TErr err;
	CString token[CGAZOAPP_CMD_MAXTOKEN];
	m_sHisAvgFiles.Empty();//201125
	while (stdioConv.ReadString(line)) {
		if (line.IsEmpty()) continue;
		const CString cmd = line.SpanExcluding("\t ");
		line = line.Mid(cmd.GetLength());
		line.TrimLeft();
		int icmd = CGAZOAPP_CMD_NONE;
		if (cmd == "ren") icmd = CGAZOAPP_CMD_REN;
		else if (cmd == "copy") icmd = CGAZOAPP_CMD_COPY;
		else if (cmd == "img_ave") icmd = CGAZOAPP_CMD_AVG;
		else continue;
		int narg = 0;
		for (int i=0; i<CGAZOAPP_CMD_MAXTOKEN; i++) {
			token[i] = line.SpanExcluding("\t ");
			if (token[i].IsEmpty()) {narg = i; break;}
			line = line.Mid(token[i].GetLength());
			line.TrimLeft();
			if (line.IsEmpty()) {narg = i+1; break;}
			if (i == CGAZOAPP_CMD_MAXTOKEN - 1) {fclose(fconv); return 21015;}
		}
		if (pf) {
			CString msg; msg.Format("Processing image: %s", token[narg-1]);
			pf->m_wndStatusBar.SetPaneText(0, msg);
		}
		const CString destfn = token[narg-1].SpanExcluding(".");
		if (destfn.IsEmpty()) continue;
		unsigned int idx;
		if (destfn == "dark") idx = ilen -1;
		else if (destfn.GetAt(0) == 'q') {
			idx = (unsigned int)atoi(destfn.Mid(1)) - 1;//because it starts from a0001 but not a0000
			if (idx >= ilen -1) continue;
		} else {
			continue;
		}
		if (!convList[idx].IsEmpty()) {fclose(fconv); return 21016;}
		switch (icmd) {
			case CGAZOAPP_CMD_AVG: {
				if (nset > 1) token[narg-1] = destfn + frmPrefix + token[narg-1].Mid(destfn.GetLength());//111108
				convList[idx] = "a " + token[narg-1];
				err = CalcAvgFromHis(sDataPath, sFilePrefix + sFileSuffix, token, narg, iDatasetSel);
				if (err) return err;
				for (int i = 0; i < narg-1; i++) m_sHisAvgFiles += token[i] + "/" + destfn + " ";//201125
				break;}
			case CGAZOAPP_CMD_REN: {
				convList[idx] = "r " + token[0].SpanExcluding(".").Mid(1);
				break;}
			case CGAZOAPP_CMD_COPY: {
				if (token[0].GetAt(0) == 'q') convList[idx] = "q " + token[0].SpanExcluding(".").Mid(1);
				else convList[idx] = "r " + token[0].SpanExcluding(".").Mid(1);
				break;}
			default: {fclose(fconv); return 21017;}
		}
		/*
		if (narg > 2) {
			if (atoi(token[0].SpanExcluding(".").Mid(1)) < atoi(token[1].SpanExcluding(".").Mid(1))) {
				for (int i=0; i<narg-1; i++) {
					if ((token[i].GetAt(0) != 'a')&&(token[i].GetAt(0) != 'q')) {fclose(fconv); return 21018;}
					convList[idx] += " " + token[i].SpanExcluding(".").Mid(1);
				}
			} else {
				for (int i=narg-2; i>=0; i--) {
					if ((token[i].GetAt(0) != 'a')&&(token[i].GetAt(0) != 'q')) {fclose(fconv); return 21018;}
					convList[idx] += " " + token[i].SpanExcluding(".").Mid(1);
				}
			}
		} else {
			if ((token[0].GetAt(0) != 'a')&&(token[0].GetAt(0) != 'q')) {fclose(fconv); return 21018;}
			convList[idx] += " " + token[0].SpanExcluding(".").Mid(1);
		}
		*/
	}
	fclose(fconv);
	//110914
	//CString msg = "";
	//for (int i=0; i<6; i++) {
	//	line.Format("%d %s\r\n", i, convList[i]); msg += line;
	//}
	//for (int i=ilen-6; i<ilen; i++) {
	//	line.Format("%d %s\r\n", i, convList[i]); msg += line;
	//}
	//AfxMessageBox(msg);
	for (unsigned int i=0; i<ilen; i++) {
		if (convList[i].IsEmpty()) return 21019;
	}
//	CString line2; line2.Format("%s\r\n%s\r\n%s\r\n%d", sDataPath, sFilePrefix, sFileSuffix, iDatasetSel); AfxMessageBox(line2);
//	CString line2; line2.Format("201125 %s", m_sHisAvgFiles); AfxMessageBox(line2);
	return 0;
}

int CGazoDoc::CountFrameFromConvBat(CString sDataPath) {
	//int nDarkFrame = 0;
	if (iFramePerDataset <= 0) {
		//CString sDataPath;
		if (sDataPath.IsEmpty()) {//120715
			if (!dataPath.IsEmpty()) {
				sDataPath = dataPath;
			} else {
				TCHAR path_buffer[_MAX_PATH];
				_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
				TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];
				_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
				_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
				sDataPath = path_buffer;
			}
		}
		//count number of frames
		//open conv.bat file
		CString fn = sDataPath + "conv.bat";
		FILE* fconv = NULL;
		errno_t errn = fopen_s(&fconv, fn, "rt");
		if (errn) {
			if (bFromQueue) return -1;
			logPath.Empty();
			static char BASED_CODE szFilter[] = "Conv.bat files (*.bat)|*.bat|All Files (*.*)|*.*||";
			static char BASED_CODE defaultExt[] = ".bat";
			CFileDialog fileDlg(TRUE, defaultExt, fn,
														OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
			if (fileDlg.DoModal() == IDCANCEL) {
				AfxMessageBox("No conv.bat file specified");
				return -1;
			}
			fn = fileDlg.GetPathName();
			errn = fopen_s(&fconv, fn, "rt");
			if (errn) {
				AfxMessageBox("File not found");
				return -1;
			}
		}
		CStdioFile stdioConv(fconv);
		CString line;
		CString token;
		int nframe = 0;
		int maxframeno = 0;//201125
		while (stdioConv.ReadString(line)) {
			if (line.IsEmpty()) continue;
			const CString cmd = line.SpanExcluding("\t ");
			line = line.Mid(cmd.GetLength());
			line.TrimLeft();
			if ((cmd != "ren")&&(cmd != "copy")&&(cmd != "img_ave")) continue;
			bool bDarkAvg = false;
			const int iprevFrame = nframe;
			while (!line.IsEmpty()) {
				token = line.SpanExcluding("\t ");
				if (token.IsEmpty()) break;
				if (token.GetAt(0) == 'a') {
					nframe++;
					int num = atoi(token.SpanExcluding(".").Mid(1));//201125
					maxframeno = num > maxframeno ? num : maxframeno;//201125
				} else if (token.SpanExcluding(".") == "dark") bDarkAvg = true;
				line = line.Mid(token.GetLength());
				line.TrimLeft();
			}
			if (bDarkAvg && (cmd == "img_ave")) nDarkFrame = nframe - iprevFrame;
		}
		//120720 fclose(fconv);
		stdioConv.Close();
		//201125 iFramePerDataset = (nframe > 0) ? nframe : -1;
		iFramePerDataset = (maxframeno > 0) ? maxframeno : -1;
	}
	//141229 Get number of dataset
	if ((maxHisFrame >= 0)&&(iFramePerDataset != 0)&&(iFramePerDataset - nDarkFrame != 0)&&(dlgReconst.m_nDataset <= 1)) {
		//201125if (maxHisFrame % iFramePerDataset == 0) {
		if ((maxHisFrame % iFramePerDataset) < ((maxHisFrame - nDarkFrame) % (iFramePerDataset - nDarkFrame))) {
			dlgReconst.m_nDataset = maxHisFrame / iFramePerDataset;
			iFramePerDataset = maxHisFrame / dlgReconst.m_nDataset;//201125 
			nDarkFrame = 0;
		//201125 } else if ((maxHisFrame - nDarkFrame) % (iFramePerDataset - nDarkFrame) == 0) {
		} else {
			dlgReconst.m_nDataset = (maxHisFrame - nDarkFrame) / (iFramePerDataset - nDarkFrame);
			iFramePerDataset = (maxHisFrame - nDarkFrame) / dlgReconst.m_nDataset + nDarkFrame;//201125
		}
	}
	//CString msg; msg.Format("201125 nDataset:%d\r\nmaxHisFrame:%d\r\niFramePerDataset:%d\r\nnDarkFrame:%d", 
	//		dlgReconst.m_nDataset, maxHisFrame, iFramePerDataset, nDarkFrame); AfxMessageBox(msg);
	return iFramePerDataset;
}

int CalcAvgFromHisCompare( const void *arg1, const void *arg2 ) {
	//if ( ** ( CString** ) arg1 > ** ( CString** ) arg2 ) return 1;
	//else if ( ** ( CString** ) arg1 < ** ( CString** ) arg2 ) return -1;
	//else return 0;
	return (** ( CString** ) arg1).Compare(** ( CString** ) arg2);
	//120624 return ( ** ( CString** ) arg1 > ** ( CString** ) arg2 );
}

TErr CGazoDoc::CalcAvgFromHis(CString path, CString fnhis, CString* files, int nfiles, int iDatasetSel) {
	int ixdim = 0;
	int iydim = 0;
	int* pData = NULL;
	int maxData = 0;
	int* pSum = NULL;
	TErr err = 0;
	CString comment = "";
	//111108
	CountFrameFromConvBat();
	//141229 const int nset = maxHisFrame / iFramePerDataset;
	const int nset = dlgReconst.m_nDataset;
	//
	const int idigit = (int)log10((double)iFramePerDataset) + 1;
	CString fmt; fmt.Format(" h%%0%dd", idigit);
	CFile fimg;
	//return if the same file exists
	if (fimg.Open(path + files[nfiles-1], CFile::modeRead | CFile::shareDenyWrite)) {
		int iDummy = 0;
		CString sPrevComment = "";
		TErr err = ReadITEX(&fimg, NULL, NULL, &iDummy, &iDummy, &sPrevComment);
		fimg.Close();
//AfxMessageBox(files[nfiles-1] + "\r\n" + sPrevComment);
CString msg = "CalcAvgFromHis\r\n[File]\r\n" + files[nfiles-1] + "\r\n[Previous comment]\r\n" + sPrevComment + "\r\n[Frames to be averaged (negative indicates frame loss)]\r\n", line;
		//if (sPrevComment.Find("skip ") < 0) return 0;
		bool bReturn = true;
		for (int i=0; i<nfiles-1; i++) {
			int iframetag = atoi(files[i].SpanExcluding(".").Mid(1)) - 1;
			if (nset > 1) {
				//calc iframe corresponding to set 0
				if (nDarkFrame > 0) {//141229 if dark frames were taken only in the first set.
					if (files[nfiles -1].Left(4) == "dark") {
						//141229 iframe is OK
					} else {
						iframetag = ((iframetag-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + nDarkFrame;
					}
				} else {//if dark frames were taken at every dataset.
					iframetag = (iframetag % iFramePerDataset);//111108
				}
			}
			//iframe % iFramePerDataset is taken because the avg_img command points incident images at the end of the a.his.
			if (ullLossFrameSet != 0) {
				int nlostbefore = 0;
				for (int i = 0; i < iDatasetSel; i++) {nlostbefore += (ullLossFrameSet >> (i * 4)) & 0x0f;}
				iframetag -= nlostbefore;
				const int nlost = (ullLossFrameSet >> (iDatasetSel * 4)) & 0x0f;
				if ((iframetag - nDarkFrame) % (iFramePerDataset - nDarkFrame) > (iFramePerDataset - nDarkFrame) / 2) { iframetag -= nlost; }
				//210618 if (iLossFrameSet == iDatasetSel) {
				//	if ((iframetag-nDarkFrame) % (iFramePerDataset-nDarkFrame) > (iFramePerDataset-nDarkFrame) / 2) {iframetag--;}
				//} else if (iLossFrameSet < iDatasetSel) {iframetag--;}
			}
if (bDebug) {line.Format("%d ", iframetag); msg += line;}
			CString sTag; sTag.Format(fmt, iframetag);
			if ((m_sFramesToExclude.Find(sTag) >= 0)&&(sPrevComment.Find(sTag) >= 0)) continue;
			else if ((m_sFramesToExclude.Find(sTag) < 0)&&(sPrevComment.Find(sTag) < 0)) continue;
			else {bReturn = false; break;}
		}
if (bDebug) AfxMessageBox(msg);
		if (bReturn) return 0;
	}
	//
	CFile fhis;
	if (!fhis.Open(path + fnhis, CFile::modeRead | CFile::shareDenyWrite)) return 23014;
	int icurrFrame = 0;
	//CString msg = "", line;
	//120427
	CString** pfiles = new CString*[nfiles-1];
	for (int j=0; j<nfiles-1; j++) {pfiles[j] = &(files[j]);}
	qsort( (void *)pfiles, (size_t)nfiles-1, sizeof(CString*), CalcAvgFromHisCompare );
	//120427
	CString sComment = "skip ";
	int iavg = 0;
	for (int i=0; i<nfiles-1; i++) {
		int ixprev = ixdim;
		int iyprev = iydim;
		//120427 int iframe = atoi((files[i]).SpanExcluding(".").Mid(1)) - 1;
		int iframe = atoi((pfiles[i])->SpanExcluding(".").Mid(1)) - 1;
		int iframetag = iframe;
//		CString sTag; sTag.Format(fmt, iframe);
//		if (m_sFramesToExclude.Find(sTag) >= 0) {sComment += sTag; continue;}
		if (nset > 1) {
			if (nDarkFrame > 0) {//141229 if dark frames were taken only in the first set.
				if (files[nfiles -1].Left(4) == "dark") {
					//141229 iframe is OK
				} else {
					iframe = ((iframe-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + (iFramePerDataset-nDarkFrame) * iDatasetSel + nDarkFrame;
					iframetag = ((iframetag-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + nDarkFrame;
				}
			} else {//if dark frames were taken at every dataset.
				iframe = (iframe % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
				iframetag = (iframetag % iFramePerDataset);
			}
		}
		//iframe % iFramePerDataset is taken because the avg_img command points incident images at the end of the a.his.
		//210618
		if (ullLossFrameSet != 0) {
			int nlostbefore = 0;
			for (int i = 0; i < iDatasetSel; i++) { nlostbefore += (ullLossFrameSet >> (i * 4)) & 0x0f; }
			iframe -= nlostbefore; iframetag -= nlostbefore;
			const int nlost = (ullLossFrameSet >> (iDatasetSel * 4)) & 0x0f;
			if ((iframe - nDarkFrame) % (iFramePerDataset - nDarkFrame) > (iFramePerDataset - nDarkFrame) / 2) { iframe -= nlost; iframetag -= nlost; }
		}
		//if (iLossFrameSet >= 0) {
		//	if (iLossFrameSet == iDatasetSel) {
		//		//141229 if (iframe % iFramePerDataset > iFramePerDataset / 2) {iframe--;}
		//		if ((iframe-nDarkFrame) % (iFramePerDataset-nDarkFrame) > (iFramePerDataset-nDarkFrame) / 2) {iframe--; iframetag--;}
		//	} else if (iLossFrameSet < iDatasetSel) {iframe--; iframetag--;}
		//}
		//120715
		if (iframe < 0) continue;
		//161113
		CString sTag; sTag.Format(fmt, iframetag);
		if (m_sFramesToExclude.Find(sTag) >= 0) {sComment += sTag; continue;}

		if (iframe > icurrFrame) {
			if (err = SkipHISframe(&fhis, iframe - icurrFrame)) {
				//141229 CString msg; msg.Format("%d", err); AfxMessageBox(msg);
				//141229 continue;
				break;
			}
		} else if (iframe < icurrFrame) {
			fhis.SeekToBegin();
			if (iframe > 0) {
				if (err = SkipHISframe(&fhis, iframe)) {
					//141229 CString msg; msg.Format("%d", err); AfxMessageBox(msg);
					//141229 continue;
					break;
				}
			}
		}
		HIS_Header his;
		err = ReadHIS(&fhis, &pData, &maxData, &iydim, &ixdim, &his, &comment);
		if (err) {
			break;
			//141229 CString msg;
			//141229 msg.Format("ERROR %d in averaging line %d frame %d file %s", err, i, iframe, files[nfiles-1]);
			//141229 AfxMessageBox(msg);
		}
		icurrFrame = iframe + 1;
		//const CString fn = path + files[i];
		if (ixprev && iyprev) {
			if ((ixprev != ixdim)||(iyprev != iydim)) {err = 23010; break;}
		}
		//line.Format("%s %d\r\n", *(pfiles[i]), iframe); msg += line;//141229
		if (maxData <= 0) continue;
		if (!pSum) {
			pSum = new int[maxData];
			if (!pSum) {err =23012; break;}
			for (int j=0; j<maxData; j++) {pSum[j] = 0;}
		}
		for (int j=0; j<maxData; j++) {pSum[j] += pData[j];}
		iavg++;
	}
	//AfxMessageBox(msg);//141229
	fhis.Close();
	if (pfiles) delete [] pfiles;
	if (err) {
		if (pData) delete [] pData;
		if (pSum) delete [] pSum;
		return err;
	}
	if (pSum == NULL) {
		CString msg;
		msg.Format("frame %d file %s #%d\r\n%s\r\n%s", iDatasetSel, files[nfiles-1], nfiles, path, fnhis);
		AfxMessageBox(msg);
		if (pData) delete [] pData; return 23013;
	}
	if (iavg) {
		//save
		for (int j=0; j<maxData; j++) {pSum[j] /= iavg;}
		if (bDebug) {CString msg; msg.Format("iavg %d\r\n%s", iavg, sComment); AfxMessageBox(msg);}
		const CString fn = path + files[nfiles-1];
		if (fimg.Open(fn, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite)) {
			err = WriteITEX(&fimg, pSum, iydim, ixdim, sComment, 0, 0, 2);
			//AfxMessageBox("CalcAvg WriteITEX 161105\r\n" + sComment);
			if (err) AfxMessageBox("ERROR in averaging. File output:\r\n " + fn);
			fimg.Close();
		}
	} else {
		err = 16111201;
	}
	if (pData) delete [] pData;
	if (pSum) delete [] pSum;
	return err;
}

int CGazoDoc::GetSinogramYdim(BOOL bOffsetCT) {
	if (bOffsetCT) {
		int iRevIdx = -1;
		if (!fdeg) return iRevIdx;
		for (int i=0; i<iLenSinogr-1; i++) {
			if (fdeg[i] > 180.) {iRevIdx = i; break;}
		}
		return iRevIdx;
	} else {
		return iLenSinogr - 1;
	}
}

TErr CGazoDoc::GenerateSinogram(RECONST_QUEUE* rq, int iLayer, double center, double deltaCent, int iMultiplex) {
	TErr err;
	const CGazoApp* pApp = (CGazoApp*)AfxGetApp();
	//CString msg; msg.Format("dReconFlags=%d", rq->dReconFlags); AfxMessageBox(msg);
	const int isino = iLenSinogr;
	const int iTrim = rq->iTrimWidth;
	const int iProgStep = isino / PROGRESS_BAR_UNIT + 1;
	const int iBinning = (rq->iInterpolation == 0) ? 4 : ((rq->iInterpolation == 1) ? 2 : 1);
	rq->dReconFlags &= ~RQFLAGS_SINOGRAMKEPT;//190101
	if ((iLayer == iCurrSinogr)&&(iTrim == iCurrTrim)&&(iMultiplex == 1)&&(rq->bReconOptionUpdated == false)) {
		for (int i=0; i<isino; i++) {
			if ((i % iProgStep == 0)&&(dlgReconst.m_hWnd)) dlgReconst.m_Progress.StepIt();
		}
		//210106 avoid unexpected recon behavior in zoomed recon using CUDA
		//rq->dReconFlags |= RQFLAGS_SINOGRAMKEPT;//190101
		if ((pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) || (rq->iInterpolation <= CDLGRECONST_OPT_ZOOMING_NONE)) {
			rq->dReconFlags |= RQFLAGS_SINOGRAMKEPT;//190101
		}
		return 0;
	}
	const int ixFrm = rq->iRawSinoXdim;
	const int iRecDim = rq->iXdim / iBinning;
	int ixlen = rq->iRawSinoXdim;
	if ((rq->bOffsetCT == false)&&(iTrim > 0)&&(iTrim < ixlen / 2)) ixlen -= iTrim * 2;
	ixlen /= iBinning;
	iCurrTrim = -1;
	iCurrSinogr = -1;
	CString fn = "", line;
	m_sFramesToExclude = rq->sFramesToExclude;//161106
//CString msg;
//msg.Format("%d %d\r\n%d %d", dlgReconst.m_iDlgFL_SampleFrameStart, rq->iSampleFrameStart, dlgReconst.m_iDlgFL_SampleFrameEnd, rq->iSampleFrameEnd);
//AfxMessageBox(msg);
	dlgReconst.m_iDlgFL_SampleFrameStart = rq->iSampleFrameStart;
	dlgReconst.m_iDlgFL_SampleFrameEnd = rq->iSampleFrameEnd;
	//
	//if (maxLenSinogr < isino * ixlen * iMultiplex) {
	if ((maxSinogrLen < isino * iMultiplex)||(maxSinogrWidth < ixlen)) {
		if (iSinogr) {
			for (int i=0; i<maxSinogrLen; i++) {if (iSinogr[i]) delete [] iSinogr[i];}
			delete [] iSinogr;
			iSinogr = NULL;//120720
		}
		try {
			iSinogr = new short*[isino * iMultiplex];
			for (int i=0; i<isino * iMultiplex; i++) {iSinogr[i] = new short[ixlen];}
		}
		catch(CException* e) {
			e->Delete();
			for (int i=0; i<isino * iMultiplex; i++) {if (iSinogr[i]) delete [] iSinogr[i];}
			delete [] iSinogr;//120720
			iSinogr = NULL;//120720
			AfxMessageBox("Run out of memory");
			maxSinogrLen = 0;
			return 21011;
		}
		maxSinogrLen = isino * iMultiplex;
//CString line; line.Format("160629 %d %d %d", isino, iMultiplex, maxSinogrLen); AfxMessageBox(line);
		maxSinogrWidth = ixlen;
	}
	for (int i=0; i<maxSinogrLen; i++) {memset(iSinogr[i], 0, sizeof(short) * maxSinogrWidth);}
	short* sbuf = NULL;
	try {sbuf = new short[ixFrm * iMultiplex * iBinning];}
	//const int ibuf_size = sizeof(short) * ixFrm * iMultiplex;
	catch(CException* e) {
		e->Delete();
		AfxMessageBox("Run out of memory");
		return 21015;
	}
	//
	CMainFrame* pf = NULL;
	if (!bCmdLine) pf = (CMainFrame*) AfxGetMainWnd();
	//his preparation;
	CFile fimg;
	const int iDatasetSel = rq->iDatasetSel;
	int ihisFrame = 0, ihisPrev = -1;//-1 means BOF
	HISHeader hisHeader;
	unsigned char* uctmp = NULL;
	int* pibuf = NULL;
	int maxTmp = 0;
//	CString msg;
	if ((rq->itexFileSuffix == ".his")||(rq->itexFileSuffix == ".HIS")) {
		//110914
		fn = rq->dataPath + rq->itexFilePrefix + rq->itexFileSuffix;//"a.his";
		if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			AfxMessageBox("Image file not found: " + fn + "\r\n" + rq->itexFilePrefix + "\r\n" + rq->dataPath);
			return 21012;
		}
		HIS_Header his;
		if (Read_hishead(&fimg, &his)) return 21012;
		if (maxHisFrame <= 0) maxHisFrame = his.n_image1 + (his.n_image2 << 16);
		fimg.SeekToBegin();
		//
		CountFrameFromConvBat();
		if (err = SetConvList(rq->dataPath, rq->itexFilePrefix, rq->itexFileSuffix, iDatasetSel, pf)) {
			fimg.Close();
			return err;
		}
	} else if (rq->itexFileSuffix.MakeUpper() == ".H5") {
		fn = rq->dataPath + rq->itexFilePrefix + rq->itexFileSuffix;//"a.his";
		if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			AfxMessageBox("Image file not found: " + fn);
			return 21012;
		}
		hdf5.SetFile(&fimg);
		if (err = hdf5.ReadSuperBlock()) {fimg.Close(); return err;}
		if (err = hdf5.FindChildSymbol("exchange", -1)) {fimg.Close(); return err; }
		hdf5.MoveToChildTree();
		if (err = hdf5.FindChildSymbol("data", -1)) {fimg.Close(); return err; }
		if (hdf5.m_sChildTitle.Left(4) != "data") {fimg.Close(); return 16052701;}
		if (err = hdf5.GetDataObjHeader()) {fimg.Close(); return err; }
		//
		try {pibuf = new int[ixFrm * iMultiplex * iBinning];}
		catch(CException* e) {
			e->Delete();
			AfxMessageBox("Run out of memory");
			fimg.Close();
			return 16052702;
		}
//		const __int64 lImageHeight = pHDF5->m_plDataSize[1];
//		const __int64 lImageWidth = pHDF5->m_plDataSize[2];
//		CDlgMessage dlg;
//		dlg.m_Msg = msg;
//		dlg.DoModal();
	}
	//141229 const int nset = maxHisFrame / iFramePerDataset;
	const int nset = dlgReconst.m_nDataset;
	//CString msg; msg.Format("%d %d %d", maxHisFrame, iFramePerDataset, nset); AfxMessageBox(msg);
	//
	int iBitFlag = READTIF16bit;
	const int ideg = (int)(log10((double)isino)) + 1;
	if (nset > 1) line.Format("Reading layer %d of dataset %d ", iLayer, iDatasetSel);
	else line.Format("Reading layer %d of ", iLayer);
//CString msg = "";
	for (int i=0; i<isino-1; i++) {bInc[i] &= ~(CGAZODOC_BINC_NOUSE | CGAZODOC_BINC_SKIP);}
	for (int i=0; i<isino; i++) {
		::ProcessMessage();
		if ((i % iProgStep == 0)&&(dlgReconst.m_hWnd)) dlgReconst.m_Progress.StepIt();
		if ((dlgReconst.iStatus == CDLGRECONST_STOP)||(dlgReconst.iStatus & CDLGRECONST_WHEEL)) {
			if (pf) pf->m_wndStatusBar.SetPaneText(0, "Aborted");
			return 0;
		}
		if (i == isino - 1) fn = "dark" + rq->itexFileSuffix;//".img";
		else fn = rq->itexFilePrefix + fname[i].Right(ideg) + rq->itexFileSuffix;//".img";
		fn = rq->dataPath + fn;
		memset(sbuf, 0, sizeof(short) * ixFrm * iMultiplex * iBinning);
		if ((rq->itexFileSuffix == ".img")||(rq->itexFileSuffix == ".IMG")) {
			if (pf) pf->m_wndStatusBar.SetPaneText(0, line + fn);
			FILE* pfimg = NULL;
			errno_t errn = fopen_s(&pfimg, fn, "rb");
			if (errn) {
				AfxMessageBox("Image file not found: " + fn);
				return 21012;
			}
			//if (err = ReadITEXstrip(pfimg, &(iSinogr[ixlen * i * iMultiplex]), iLayer, ixlen, iMultiplex)) {
			if (err = ReadITEXstrip(pfimg, sbuf, iLayer, ixFrm, iMultiplex * iBinning)) {
				AfxMessageBox("Error in ITEX image format");
				fclose(pfimg);
				return err;
			}
			fclose(pfimg);
		} else if ((rq->itexFileSuffix == ".tif")||(rq->itexFileSuffix == ".TIF")) {
			//AfxMessageBox(line + fn + "\r\n121022-3");
			if (pf) pf->m_wndStatusBar.SetPaneText(0, line + fn);
			if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
				AfxMessageBox("Image file not found: " + fn);
				return 21012;
			}
			if (err = ReadTifStrip(&fimg, sbuf, iLayer, ixFrm, iMultiplex * iBinning, &iBitFlag)) {
				AfxMessageBox("Error in TIFF image format");
				fimg.Close();
				return err;
			}
			fimg.Close();
			//tif routine still gives errors for some data, reported by a germany group
		} else if ((rq->itexFileSuffix == ".his")||(rq->itexFileSuffix == ".HIS")) {
			int cidx = -1;//121126
			if (i == isino - 1) cidx = iLenSinogr - 1;
			else {
				cidx = atoi(fname[i]) - 1;//121126
				if ((cidx < 0)||(cidx > (int)maxConvList)) {
					//CString msg = "", line;
					//for (int j=0; j<6; j++) {line.Format("%d %s\r\n", j, fname[j]); msg += line;}
					//for (int j=isino-6; j<isino; j++) {line.Format("%d %s\r\n", j, fname[j]); msg += line;}
					//AfxMessageBox(msg);//110921
					fimg.Close(); if (uctmp) delete [] uctmp; return 16062901;
				}
			}
			if (convList[cidx].GetAt(0) == 'q') {//point to another image
				cidx = atoi(convList[cidx].Mid(1)) - 1;
				if ((cidx < 0)||(cidx > (int)maxConvList)) {
					fimg.Close(); if (uctmp) delete [] uctmp; return 16062902;
				}
			}
			if (convList[cidx].GetAt(0) == 'r') {
				//110917 read his strip
				ihisFrame = atoi(convList[cidx].Mid(1)) - 1;
				//if (nset > 1) ihisFrame += iFramePerDataset * iDatasetSel;//111108
				//141229 if (nset > 1) ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
				if (nset > 1) {
					if (nDarkFrame > 0) {//141229 if dark frames were taken only in the first set.
						ihisFrame = ((ihisFrame-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + (iFramePerDataset-nDarkFrame) * iDatasetSel + nDarkFrame;
					} else {//if dark frames were taken at every dataset.
						ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
					}
				}
				//210618
				if (ullLossFrameSet != 0) {
					int nlostbefore = 0;
					for (int i = 0; i < iDatasetSel; i++) { nlostbefore += (ullLossFrameSet >> (i * 4)) & 0x0f; }
					ihisFrame -= nlostbefore;
					const int nlost = (ullLossFrameSet >> (iDatasetSel * 4)) & 0x0f;
					if ((ihisFrame - nDarkFrame) % (iFramePerDataset - nDarkFrame) > (iFramePerDataset - nDarkFrame) / 2) { ihisFrame -= nlost; }
				}
				//if (iLossFrameSet >= 0) {
				//	if (iLossFrameSet == iDatasetSel) {
				//		//141229 if (ihisFrame % iFramePerDataset > iFramePerDataset / 2) {
				//		if ((ihisFrame-nDarkFrame) % (iFramePerDataset-nDarkFrame) > (iFramePerDataset-nDarkFrame) / 2) {
				//			ihisFrame--;
				//		}
				//	} else if (iLossFrameSet < iDatasetSel) {ihisFrame--;}
				//}
				//120715
				CString msg; msg.Format("frame %s", fname[i]);
				if (pf) pf->m_wndStatusBar.SetPaneText(0, line + msg);
				if (ihisFrame <= ihisPrev) {
					//CString msg; msg.Format("SEEK %d %d", ihisFrame, ihisPrev); AfxMessageBox(msg);
					fimg.SeekToBegin();
					ihisPrev = -1;//-1 means BOF
				}
				if (err = SkipHISframe(&fimg, ihisFrame - ihisPrev -1)) {
					CString msg; msg.Format("%d %d %s %d", ihisFrame, ihisPrev, convList[cidx], cidx); AfxMessageBox(msg);
					fimg.Close(); if (uctmp) delete [] uctmp; return err;
				}
				if (err = ReadHISstrip(&fimg, &uctmp, &maxTmp, sbuf, iLayer, ixFrm, iMultiplex * iBinning, &hisHeader)) {
 					AfxMessageBox("Error in HIS image format");
					fimg.Close(); if (uctmp) delete [] uctmp; return err;
				}
				//msg.Format("%d %d %s %d (%d %d)", ihisFrame, ihisPrev, convList[cidx], cidx, sbuf[0], sbuf[1]); AfxMessageBox(msg);//210618
				ihisPrev = ihisFrame;
			} else if (convList[cidx].GetAt(0) == 'a') {
				//110914 read strip from qxxxx file
				fn = convList[cidx].Mid(1); fn.TrimLeft();
				fn = rq->dataPath + fn;
				if (pf) pf->m_wndStatusBar.SetPaneText(0, line + fn);
				FILE* pfimg = NULL;
				errno_t errn = fopen_s(&pfimg, fn, "rb");
				if (errn) {
					AfxMessageBox("Image file not found: " + fn);
					fimg.Close(); if (uctmp) delete [] uctmp; return 21012;
				}
				if (err = ReadITEXstrip(pfimg, sbuf, iLayer, ixFrm, iMultiplex * iBinning)) {
					AfxMessageBox("Error in ITEX image format");
					fclose(pfimg);
					fimg.Close(); if (uctmp) delete [] uctmp; return err;
				}
				fclose(pfimg);
			}
		} else if (rq->itexFileSuffix.MakeUpper() == ".H5") {
			CString msg2; msg2.Format("frame %d/%d", i, isino);
			if (pf) pf->m_wndStatusBar.SetPaneText(0, line + msg2);
			if ((i >= isino-2)||(i == 0)) {//dark or white
				const CString sSymbol = (i == isino-1) ? "data_dark" : "data_white";
//AfxMessageBox(sSymbol);
				if (err = hdf5.FindChildSymbol(sSymbol, -1)) {if (pibuf) delete [] pibuf; fimg.Close(); return err;}
				if (err = hdf5.GetDataObjHeader()) {if (pibuf) delete [] pibuf; fimg.Close(); return err;}
				memset(pibuf, 0, sizeof(int) * ixFrm * iMultiplex * iBinning);
				int kstart, kend;
				const int idigit = (int)log10((double)isino) + 1;
				CString fmts;
				if (i == 0) {//pre white
					fmts.Format(" w%%0%dd", idigit);
//					if (rq->dReconFlags & RQFLAGS_SKIPINITIALFLATSINHDF5) {
//						kstart = (int)(hdf5.m_plDataSize[0] / 2);
//						kend = (int)(hdf5.m_plDataSize[0]);
//					} else {
						kstart = 0;
						kend = (int)(hdf5.m_plDataSize[0] / 2);
//					}
				} else if (i == isino-2) {//post white
					fmts.Format(" w%%0%dd", idigit);
					kstart = (int)(hdf5.m_plDataSize[0] / 2);
					kend = (int)(hdf5.m_plDataSize[0]);
				} else {//dark
					fmts.Format(" b%%0%dd", idigit);
					//161106 kstart = 1;//not to use the first dark image because it's a sample image when flyscan
					kstart = 0;//now this can be done with sFramesToExclude
					kend = (int)(hdf5.m_plDataSize[0]);
				}
				//const int kstart = i ? (int)(hdf5.m_plDataSize[0] / 2) : 0;//post:pre white images
				//const int kend = i ? (int)(hdf5.m_plDataSize[0]) : (int)(hdf5.m_plDataSize[0] / 2);//post:pre white images
CString msg = "161106GenSino\r\n";
if (bDebug) msg += sSymbol + "\r\n";
				int kcount = 0;
				for (int k=kstart; k<kend; k++) {
					CString sTag; sTag.Format(fmts, k);
					if (m_sFramesToExclude.Find(sTag) >= 0) {msg += sTag + "\r\n" ; continue;}
					if (err = hdf5.ReadStrip(sbuf, k, iLayer, iMultiplex * iBinning)) {
						AfxMessageBox("Error in HDF5 image format");
						fimg.Close();
						if (pibuf) delete [] pibuf;
						return err;
					}
if (bDebug) {CString line; line.Format("k=%d sbuf[5]=%d\r\n", k, (int)(sbuf[5])); msg += line;}
					for (int j=0; j<ixFrm * iMultiplex * iBinning; j++) {pibuf[j] += sbuf[j];}
					kcount++;
				}
				if (kcount) {
					for (int j=0; j<ixFrm * iMultiplex * iBinning; j++) {sbuf[j] = (short)(pibuf[j] / kcount);}
				} else {
					if ((i == 0)||(i == isino-2)) bInc[i] |= CGAZODOC_BINC_NOUSE;//white
					for (int j=0; j<ixFrm * iMultiplex * iBinning; j++) {sbuf[j] = 0;}
				}
				if (i == 0) {
					if (err = hdf5.FindChildSymbol("data", -1)) {if (pibuf) delete [] pibuf; fimg.Close(); return err;}
					if (err = hdf5.GetDataObjHeader()) {if (pibuf) delete [] pibuf; fimg.Close(); return err;}
				}
if (bDebug) AfxMessageBox(msg + fmts);
			} else {//data
//161112				int idata = i;
//161106		if (idata == isino-3) idata--;//not to use last two images, but copy from its precedent one. 
//The above can be done with the frame selection dialog if needed
				//The last image is ignored by LoadLogfile (line1599).
				//Its precedent (isino-3) is replaced with these codes.
				//isino-1 = dark; isino-2 = white; isino-3 = sample image; 
				//////////////////////////////////////////////////
//161106		if (err = hdf5.ReadStrip(sbuf, idata + iHDF5DummyFrame -1, iLayer, iMultiplex * iBinning)) {
				if (err = hdf5.ReadStrip(sbuf, i - 1, iLayer, iMultiplex * iBinning)) {
					//-1 is to include the pre white image
//CString msg; msg.Format("Sample: %d %d %d %lld", i, idata + iHDF5DummyFrame-1, iLayer, hdf5.m_plDataSize[0]); AfxMessageBox(msg);
					AfxMessageBox("Error in HDF5 image format");
					fimg.Close();
					if (pibuf) delete [] pibuf;
					return err;
				}
			}
		} else {
			AfxMessageBox("Image format not supported");
		}
		const int isize = sizeof(short) * ixlen;
		const int iTrimShift = (rq->bOffsetCT) ? 0 : iTrim;
		//if (rq->bOffsetCT) {
		//	for (int j=0; j<iMultiplex; j++) {
		//		//memcpy_s(iSinogr[i * iMultiplex + j], isize, &(sbuf[j * ixFrm]), isize);
		//	}
		//}
		if (iBinning == 1) {
			for (int j=0; j<iMultiplex; j++) {
				memcpy_s(iSinogr[i * iMultiplex + j], isize, &(sbuf[j * ixFrm + iTrimShift]), isize);
			}
		} else {
			const int iBin2 = iBinning * iBinning;
			for (int j=0; j<iMultiplex; j++) {
				for (int k=0; k<ixlen; k++) {
					int isum = 0;
					for (int ix=0; ix<iBinning; ix++) {
						for (int iy=0; iy<iBinning; iy++) {
							isum += (unsigned short)sbuf[(j * iBinning + iy) * ixFrm + (k * iBinning + ix) + iTrimShift];
						}
					}
					(iSinogr[i * iMultiplex + j])[k] = (short)(isum / iBin2);
				}
			}
		}
	}
//CDlgMessage dlg;
//dlg.m_Msg = msg;
//dlg.DoModal();
	delete [] sbuf;
	if (pibuf) delete [] pibuf;
	if ((rq->itexFileSuffix.MakeUpper() == ".HIS")||(rq->itexFileSuffix.MakeUpper() == ".H5")) {
		fimg.Close();
		if (uctmp) delete [] uctmp;
	}
	iCurrSinogr = iLayer;
	//===>120501 drift list correction
	int* pDriftList = NULL;
	if (rq->dReconFlags & RQFLAGS_DRIFTLIST) {
		CStdioFile fDriftList;
		if (fDriftList.Open(rq->sDriftListPath, CFile::modeRead | CFile::typeText)) {
			pDriftList = new int[isino];
			for (int i=0; i<isino; i++) {pDriftList[i] = 0;}
			CString line;
			while (fDriftList.ReadString(line)) {
				int ipos = -1, idrift = 0;
				sscanf_s(line, "%d %d", &ipos, &idrift);
				if ((ipos >= 0)&&(ipos < isino)) pDriftList[ipos] = idrift;
			}
			fDriftList.Close();
		}
	}
	//===>120501
	//161105===>
	//for (int i=0; i<isino-1; i++) {bInc[i] &= ~(CGAZODOC_BINC_NOUSE | CGAZODOC_BINC_SKIP);}
	if (! (rq->sFramesToExclude.IsEmpty()) ) {
		CString sList = rq->sFramesToExclude;
		sList.TrimLeft();
		int iPos = 0;
		CString msg = "sFramesToExclude 161105\r\n";
		do {
			CString sframe = sList.Tokenize(_T(" "), iPos);
			if (sframe.IsEmpty()) break;
			if (sframe.GetAt(0) == 'h') {
				if (bDebug) msg += sframe + "\r\n";
				//TODO: if all white frames before or after the sample were skipped, NOUSE flag must be set.
			} else if (sframe.GetAt(0) == 'b') {
				if (bDebug) msg += sframe + "\r\n";
				//HDF5
			} else if (sframe.GetAt(0) == 'w') {
				if (bDebug) msg += sframe + "\r\n";
				//HDF5
			} else if (sframe.GetAt(0) == 's') {
				sframe = sframe.Mid(1);
				int idx = atoi(sframe);
				if (rq->itexFileSuffix.MakeUpper() == ".H5") {
					idx++;//list is numbered from zero and bInc[0] is a white frame
					idx -= dlgReconst.m_iDlgFL_SampleFrameStart;
				}
				if ((idx < 0)||(idx >= isino-1)) continue;
				if (!(bInc[idx] & CGAZODOC_BINC_SAMPLE)) continue;//tokens starting with 's' are used only for sample frames 
				bInc[idx] |= (CGAZODOC_BINC_NOUSE | CGAZODOC_BINC_SKIP);
				if (bDebug) {CString line; line.Format("(%d %f)\r\n", idx, fdeg[idx]); msg += line;}
			} else {
				int idx = atoi(sframe);
				if (rq->itexFileSuffix.MakeUpper() == ".H5") {
					idx++;//list is numbered from zero and bInc[0] is a white frame
					idx -= dlgReconst.m_iDlgFL_SampleFrameStart;
				}
				if ((idx < 0)||(idx >= isino-1)) continue;
				int ismp0 = -1, ismp1 = -1;
				if (!(bInc[idx] & CGAZODOC_BINC_SAMPLE)) bInc[idx] |= CGAZODOC_BINC_NOUSE;
				else {
					for (int i=idx-1; i>=0; i--) {
						if (i < 0) break;
						if ((bInc[i] & CGAZODOC_BINC_SAMPLE)&&(!(bInc[i] & CGAZODOC_BINC_NOUSE))) {
							int iPos2 = 0;
							bool bHit = true;
							do {//search that frame in the exclusion list
								CString sframe2 = sList.Tokenize(_T(" "), iPos2);
								if (sframe2.IsEmpty()) break;
								const char clabel = sframe2.GetAt(0);
								if ((clabel == 'h')||(clabel == 'b')||(clabel == 'w')) continue;
								if (clabel == 's') sframe2 = sframe2.Mid(1);
								int idx2 = atoi(sframe2);
								if (rq->itexFileSuffix.MakeUpper() == ".H5") {
									idx2++;//list is numbered from zero and bInc[0] is a white frame
									idx2 -= dlgReconst.m_iDlgFL_SampleFrameStart;
								}
								if (idx2 == i) {bHit = false; break;}
							} while (true);
							if (bHit) {ismp0 = i; break;}
						}
					}
					for (int i=idx+1; i<isino-1; i++) {
						if (i >= isino-1) break;
						if ((bInc[i] & CGAZODOC_BINC_SAMPLE)&&(!(bInc[i] & CGAZODOC_BINC_NOUSE))) {
							int iPos2 = 0;
							bool bHit = true;
							do {//search that frame in the exclusion list
								CString sframe2 = sList.Tokenize(_T(" "), iPos2);
								if (sframe2.IsEmpty()) break;
								const char clabel = sframe2.GetAt(0);
								if ((clabel == 'h')||(clabel == 'b')||(clabel == 'w')) continue;
								if (clabel == 's') sframe2 = sframe2.Mid(1);
								int idx2 = atoi(sframe2);
								if (rq->itexFileSuffix.MakeUpper() == ".H5") {
									idx2++;//list is numbered from zero and bInc[0] is a white frame
									idx2 -= dlgReconst.m_iDlgFL_SampleFrameStart;
								}
//								CString line; line.Format(" ismp1 sframe2=%s idx2=%d i=%d\r\n", sframe2, idx2, i); msg += line;
								if (idx2 == i) {bHit = false; break;}
							} while (true);
							if (bHit) {ismp1 = i; break;}
						}
					}
					if ((ismp0 >= 0)||(ismp1 >= 0)) {
						for (int j=0; j<iMultiplex; j++) {
							for (int k=0; k<ixlen; k++) {
								int isum = 0, nsum = 0;
								if (ismp0 >= 0) {isum += (iSinogr[ismp0 * iMultiplex + j])[k]; nsum++;}
								if (ismp1 >= 0) {isum += (iSinogr[ismp1 * iMultiplex + j])[k]; nsum++;}
								(iSinogr[idx * iMultiplex + j])[k] = (short)(isum / nsum);
							}
						}
					} else {
						bInc[idx] |= (CGAZODOC_BINC_NOUSE | CGAZODOC_BINC_SKIP);
					}
				}
				if (bDebug) {CString line; line.Format("%s\r\n(%d %f) %d %d\r\n", sList, idx, fdeg[idx], ismp0, ismp1); msg += line;}
			}
		} while (true);
		if (bDebug) AfxMessageBox(msg);
	}//===>161105
	//090806===>
	//struct _timeb tstruct; double tm0;
	//_ftime_s( &tstruct );
	//tm0 = tstruct.time + tstruct.millitm * 0.001;
	//
	//const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	//Each processor gives correct sinograms, 
	//but CUDA routine affects the resulant tomograms by memory allocation of d_Strip.
	//Therefore, intel processors should be used for sinogram generation.
	const int nCPU = (int)(pApp->dlgProperty.iCPU);
	for (int i=nCPU-1; i>=0; i--) {
		ri[i].hThread = NULL;
		ri[i].iStatus = RECONST_INFO_BUSY;
		ri[i].iFlag = iBitFlag;
		ri[i].iStepSino = nCPU;
		ri[i].ixdim = ixlen;
		ri[i].iSinogr = iSinogr;
		ri[i].bInc = bInc;
		ri[i].fdeg = fdeg;////
		ri[i].fexp = fexp;////
		ri[i].iMultiplex = iMultiplex;
		ri[i].maxSinogrLen = isino;
		ri[i].drStart = rq->drStart;
		ri[i].drEnd = rq->drEnd;
		ri[i].drX = rq->drX;
		ri[i].drY = rq->drY;
		ri[i].drOmit = rq->drOmit;
		ri[i].dReconFlags = rq->dReconFlags;
		ri[i].piDrift = pDriftList;
		ri[i].iInterpolation = rq->iInterpolation;//160731
		void* pArg = (void*)(&(ri[i]));
		if (i) {
			ri[i].hThread = (unsigned int)_beginthreadex( NULL, 0, GenerateSinogramThread, pArg, 0, &(ri[i].threadID) );
		} else {
			GenerateSinogramThread(&(ri[i]));
		}
	}
	int ist = RECONST_INFO_IDLE;
	do {
		::ProcessMessage();
		ist = RECONST_INFO_IDLE;
		for (int i=nCPU-1; i>=0; i--) {
			if (ri[i].iStatus == RECONST_INFO_ERROR) {
				//AfxMessageBox("ERROR");
				ri[i].iStatus = RECONST_INFO_IDLE;
				dlgReconst.iStatus = CDLGRECONST_STOP;
			}
			ist |= ri[i].iStatus;
		}
	} while (ist != RECONST_INFO_IDLE);
	for (int i=nCPU-1; i>=0; i--) {if (ri[i].hThread) CloseHandle((HANDLE)(ri[i].hThread));}//120723
	//
	if (pDriftList) delete [] pDriftList;//120501
	if (pf) {
		if ((dlgReconst.iStatus == CDLGRECONST_STOP) || (dlgReconst.iStatus & CDLGRECONST_WHEEL)) {
			pf->m_wndStatusBar.SetPaneText(0, "Aborted");
			return 0;
		}
	}
	//_ftime_s( &tstruct );
	//TReal tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	//CString line2; line2.Format("090806 CPU: %f", tcpu); AfxMessageBox(line2); //return 99999;
	//===>090806
	if (rq->bOffsetCT == false) {iCurrTrim = iTrim; return 0;}
	//offset-CT
	int iRevIdx = GetSinogramYdim(rq->bOffsetCT);
	//for (int i=0; i<isino-1; i++) {
	//	if (fdeg[i] > 180.) {iRevIdx = i; break;}
	//}
	if (iRevIdx < 0) return 21051;
	const int iOfSinogrDim = iRevIdx;//090214 isino - 1;
	//TODO: decrease memory usage
	if ((maxOfSinogrLen < iOfSinogrDim * iMultiplex)||(maxOfSinogrWidth < iRecDim)) {
		if (ofSinogr) {
			for (int i=0; i<maxOfSinogrLen; i++) {if (ofSinogr[i]) delete [] ofSinogr[i];}
			delete [] ofSinogr;
		}
		try {
			ofSinogr = new short*[iOfSinogrDim * iMultiplex];
			for (int i=0; i<iOfSinogrDim * iMultiplex; i++) {ofSinogr[i] = new short[iRecDim];}
		}
		catch(CException* e) {
			e->Delete();
			for (int i=0; i<iOfSinogrDim * iMultiplex; i++) {if (ofSinogr[i]) delete [] ofSinogr[i];}
			AfxMessageBox("Run out of memory");
			return 21053;
		}
		maxOfSinogrLen = iOfSinogrDim * iMultiplex;
		maxOfSinogrWidth = iRecDim;
	}
	for (int i=0; i<maxOfSinogrLen; i++) {memset(ofSinogr[i], 0, sizeof(short) * maxOfSinogrWidth);}
	center += HALFPIXEL_OFFSET;
	const int iNativeXdim = rq->iRawSinoXdim * 2;
	//CString line2 = "", scr;
	for (int i=0; i<iOfSinogrDim; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		const double th = fdeg[i];
		while ((fdeg[iRevIdx] - th < 180)|((bInc[iRevIdx] & CGAZODOC_BINC_SAMPLE) == 0)) {
			iRevIdx++;
			if (iRevIdx >= isino - 1) return 21052;
		}
		for (int j=0; j<iMultiplex; j++) {
			short* iStrip1 = iSinogr[i * iMultiplex + j];
			short* iStrip2 = iSinogr[iRevIdx * iMultiplex + j];
			short* ofStrip = ofSinogr[i * iMultiplex + j];
			if (center > ixFrm / 2.) {
				const double fc = (center + deltaCent * j * iBinning + iTrim) / iBinning;
				const int iCent = (int)fc;
				const double dCent = fc - iCent;
				//100312 for (int k=iTrim; k<=iCent; k++) {
				const int iGradSwitch = ixFrm/iBinning-1-iCent;
				for (int k=iTrim/iBinning; k<=iCent-iGradSwitch; k++) {
					ofStrip[k - iTrim/iBinning] = iStrip1[k];
				}
				short lastDens = 0;
				if (dCent < 0.5) {
					//100312 for (int k=iCent+1; k<iNativeXdim-iTrim; k++) {
					for (int k=iCent-iGradSwitch+1; k<(iNativeXdim-iTrim)/iBinning; k++) {
						int idx = iCent - (k - iCent);
						if ((idx >= 0)&&(idx < ixlen-1)) {
							short ir0 = iStrip2[idx + 1];
							short ir1 = iStrip2[idx];
							lastDens = (short)(ir0 + (ir1 - ir0) * (1 - 2 * dCent));
							if (k <= iCent + iGradSwitch) {
								const double ratio = (double)(k - (iCent-iGradSwitch)) / (iGradSwitch * 2 + 1);
								lastDens = (short)(ratio * lastDens + (1. - ratio) * iStrip1[k]);
							}
						}
						ofStrip[k - iTrim/iBinning] = lastDens;
					}
				} else {
					//100312 for (int k=iCent+1; k<iNativeXdim-iTrim; k++) {
					for (int k=iCent-iGradSwitch+1; k<(iNativeXdim-iTrim)/iBinning; k++) {
						int idx = iCent - (k - iCent);
						if ((idx >= 1)&&(idx < ixlen)) {
							short ir0 = iStrip2[idx];
							short ir1 = iStrip2[idx - 1];
							lastDens = (short)(ir1 + (ir0 - ir1) * (2 - 2 * dCent));
							if (k <= iCent + iGradSwitch) {
								const double ratio = (double)(k - (iCent-iGradSwitch)) / (iGradSwitch * 2 + 1);
								lastDens = (short)(ratio * lastDens + (1. - ratio) * iStrip1[k]);
							}
						}
						ofStrip[k - iTrim/iBinning] = lastDens;
					}
				}
			} else {//100212 if (center > ixFrm / 2.)
				const double fc = (center + deltaCent * j * iBinning) / iBinning;
				const int iCent = (int)(fc + 0.5);
				const double dCent = fc - (int)fc;
				//100312 for (int k=ixFrm-1-iTrim; k>=iCent; k--) {
				const int iGradSwitch = iCent;
				for (int k=(ixFrm-1-iTrim)/iBinning; k>=iCent+iGradSwitch; k--) {
					ofStrip[k + (ixFrm - iTrim)/iBinning] = iStrip1[k];
				}
				short lastDens = 0;
				if (dCent < 0.5) {
					//100312 for (int k=iCent-1; k>=iCent*2+1-ixFrm; k--) {
					for (int k=iCent+iGradSwitch-1; k>=iCent*2+1-ixFrm/iBinning; k--) {
						int idx = iCent + ((iCent-1) - k) + 1;
						if ((idx >= 0)&&(idx < ixlen-1)) {
							short ir1 = iStrip2[idx];
							short ir2 = iStrip2[idx + 1];
							lastDens = (short)(ir1 + (ir2 - ir1) * (2 * dCent));
							if (k >= iCent - iGradSwitch) {
								const double ratio = (double)((iCent+iGradSwitch) - k) / (iGradSwitch * 2 + 1);
								lastDens = (short)(ratio * lastDens + (1. - ratio) * iStrip1[k]);
							}
						}
						ofStrip[k + (ixFrm - iTrim)/iBinning] = lastDens;
					}
				} else {
					//100312 for (int k=iCent-1; k>=iCent*2-ixFrm; k--) {
					for (int k=iCent+iGradSwitch-1; k>=iCent*2-ixFrm/iBinning; k--) {
						int idx = iCent + ((iCent-1) - k);
						if ((idx >= 0)&&(idx < ixlen-1)) {
							short ir0 = iStrip2[idx];
							short ir1 = iStrip2[idx + 1];
							lastDens = (short)(ir0 + (ir1 - ir0) * (2 * dCent - 1));
							if (k >= iCent - iGradSwitch) {
								const double ratio = (double)((iCent+iGradSwitch) - k) / (iGradSwitch * 2 + 1);
								lastDens = (short)(ratio * lastDens + (1. - ratio) * iStrip1[k]);
							}
						}
						ofStrip[k + (ixFrm - iTrim)/iBinning] = lastDens;
					}
				}
			}//if (center > ixFrm / 2.)
		}//for(j)
	}
	//AfxMessageBox(line2);
	iCurrTrim = iTrim;
	//rq->iSinoYdim = iOfSinogrDim;
	return 0;
}

void CGazoDoc::OnUpdateHlpDebug(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(bDebug);
}

#ifdef _WIN64
extern "C" __int64 projx64(__int64);
#else
extern "C" int projx32(int);
#endif

void CGazoDoc::OutputCorrelationPlotData() {
	CString outFileName = "ctcorr.txt";
	CDocTemplate* pDocTemplate = GetDocTemplate();
	if (!pDocTemplate) return;
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	if (pos == NULL) return;
	CGazoDoc* pd1 = (CGazoDoc*) pDocTemplate->GetNextDoc(pos);
	if ((pos == NULL)||(pd1 == NULL)) return;
	CGazoDoc* pd2 = (CGazoDoc*) pDocTemplate->GetNextDoc(pos);
	if (pd2 == NULL) return;
	const int ixdim1 = pd1->ixdim;
	const int iydim1 = pd1->iydim;
	if ((ixdim1 != pd2->ixdim)||(iydim1 != pd2->iydim)) return;
	static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = "txt";
	CFileDialog fileDlg(TRUE, defaultExt, outFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) {
		AfxMessageBox("No file speciifed");
		return;
	}
	outFileName = fileDlg.GetPathName();
	FILE* fdata = NULL;
	errno_t errn = fopen_s(&fdata, outFileName, "wt");
	if (fdata == NULL) return;
	CStdioFile stdioData(fdata);
	CString line;
	srand(1);
	double fraction = RAND_MAX * 0.001;
	for (int ix=0; ix<ixdim1; ix++) {
		for (int iy=0; iy<iydim1; iy++) {
			if (rand() > fraction) continue;
			int idx = ix + iy * ixdim1;
			if (pd1->pPixel[idx] == 0) continue;
			if (pd2->pPixel[idx] == 0) continue;
			line.Format("%d %d %d %d\r\n", ix, iy, pd1->pPixel[idx], pd2->pPixel[idx]);
			stdioData.WriteString(line);
		}
	}
	fclose(fdata);
}


void CGazoDoc::OnHlpDebug() 
{
	TErr err = 0;
	CString line = "";
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	POSITION pos = GetFirstViewPosition();
	if (!pos) return;
	CGazoView* pv = (CGazoView*) GetNextView( pos );
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	if (bDebug) bDebug = false; else bDebug = true; return;

	float f1 = 10.3f, f2 = 10.6f;
	int i1 = (int)f1, i2 = (int)f2;
	line.Format("%d %d", i1, i2); AfxMessageBox(line); return;
/*
		const int ixdim = 10;
		const int iZooming = 0;
		const int iIntpDim = (int) pow((double)2, iZooming);
		const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
		const int ndim = (int) pow((double)2, ndimp);
		const double center = 5.1;
		const int ixdimp = ixdim * iIntpDim;
		const int ixdimh = ixdimp / 2;
		const int imargin = ixdimp;
		const int igpdim = (ixdimp + imargin * 2) * DBPT_GINTP;
		int* igp = new int[igpdim];
	srand(1);
	for (int i=0; i<igpdim; i++) {igp[i] = i*10;}//rand();}
			const float th = (10.0f) * (float)DEG_TO_RAD;
			const float fcos = (float)(cos(th) * DBPT_GINTP);
			const float fsin = (float)(-sin(th) * DBPT_GINTP);
			const float fcenter = (ixdimh + (float)center - (int)(center)) * DBPT_GINTP;
			const float foffset = fcenter - ixdimh * (fcos + fsin);
			int iparam6 = ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP;
			int* ipgp = (int*)(iparam6);
			int* ifp = new int[ixdimp * ixdimp];
			int* ifp2 = new int[ixdimp * ixdimp];
			for (int i=0; i<ixdimp * ixdimp; i++) {ifp[i] = 0; ifp2[i] = 0;}
			const int ixdimpg = ixdimp * DBPT_GINTP;
			int param[8];
			param[0] = (DWORD_PTR)(&fcos);
			param[1] = (DWORD_PTR)(&fsin);
			param[2] = (DWORD_PTR)(&foffset);
			param[3] = ixdimpg;
			param[4] = ixdimp;
			param[5] = (DWORD_PTR) ifp2;//iifp;
			param[6] = ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP;
			param[7] = 0;
			projx32((int)param);
			for (int iy=0; iy<ixdimp; iy++) {
				const int ifpidx = iy * ixdimp;
				const float fyoff = iy * fsin + foffset;
				for (int ix=0; ix<ixdimp; ix++) {
					int ix0 = (int)(ix * fcos + fyoff + 0.5);
					if (ix0 < 0) continue;
					if (ix0 >= ixdimpg) continue;
					ifp[ifpidx + ix] += ipgp[ix0];
				}
			}
//	projx32((int)param);
	CString msg = "";
	for (int i=0; i<100; i++) {
		if (ifp[i] != ifp2[i]) {
			line.Format("%d %d %d\r\n", i, ifp[i], ifp2[i]); msg += line;
		}
	}
	AfxMessageBox(msg);
	delete [] igp; delete [] ifp; delete [] ifp2;
	*/
	//CDlgFrameList dlg;
	//dlg.pd = this;
	//dlg.DoModal();

	/*
	CString fpath = this->GetPathName();
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fpath);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	_tmakepath_s( path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	fpath = path_buffer;
	fpath += "*.tif";
	CString sFileList = "";
	GetFileList(fpath, &sFileList);
	CDlgMessage dlg;
	CString str = sFileList;
	int iPos = 0;
	do {
		CString fn = str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) break; else dlg.m_Msg += fn + "\r\n";
	} while (true);
	dlg.DoModal();*/

#ifndef _DEBUG
	return;
#endif

	OutputCorrelationPlotData();
}

TErr CGazoDoc::SetFilter(RECONST_QUEUE* rq, int ndim) {
	if (maxFilter < ndim) {
		float* fflt = fFilter;
		//120720
		try {fFilter = new float[ndim];}
		catch(CException* e) {
			e->Delete();
			fFilter = fflt;
			return 21021;
		}
		if (fflt) delete [] fflt;
		maxFilter = ndim;
	}
	//
	const int ndimh = (int)(ndim * rq->dCutoff);
	//090210 const int ndimh = (int)(ixdim * rq->dCutoff);
	const float order = (float)(rq->dOrder);
	const float revndim = 1.f / ndim;
	//const float frac = (float)i / ndimh;
	switch (rq->iFilter) {
		case CDLGRECONST_FILT_HAN: {
			for (int i=0; i<ndimh; i++) {
				const float frac = (float)i / ndimh;
				fFilter[i] = 0.5f * revndim * i * (float)(1 + cos(__PI * frac));
			}
			for (int i=ndimh; i<ndim; i++) {fFilter[i] = 0;}
			//181223 for (int i = ndimh; i < ndim; i++) {//this gives the same results
			//	const float frac = (float)(ndim-i) / ndimh;
			//	fFilter[i] = 0.5f * revndim * (ndim-i) * (float)(1 + cos(__PI * frac));
			//}
			break;}
		case CDLGRECONST_FILT_HAM: {
			for (int i=0; i<ndimh; i++) {
				const float frac = (float)i / ndimh;
				fFilter[i] = revndim * i * (float)(0.54 + 0.46 * cos(__PI * frac));
			}
			for (int i=ndimh; i<ndim; i++) {fFilter[i] = 0;}
			break;}
		case CDLGRECONST_FILT_RAMP: {
			for (int i=0; i<ndimh; i++) {fFilter[i] = revndim * i;}
			for (int i=ndimh; i<ndim; i++) {fFilter[i] = 0;}
			break;}
		case CDLGRECONST_FILT_PARZN: {
			for (int i=0; i<ndimh/2; i++) {
				const float frac = (float)i / ndimh;
				fFilter[i] = revndim * i * (1 - 6 * frac * frac * (1 - frac));
			}
			for (int i=ndimh/2; i<ndimh; i++) {
				const float frac = (float)i / ndimh;
				fFilter[i] = revndim * i * 2 * (1 - frac) * (1 - frac) * (1 - frac);
			}
			for (int i=ndimh; i<ndim; i++) {fFilter[i] = 0;}
			break;}
		case CDLGRECONST_FILT_BUTER: {
			for (int i=0; i<ndim; i++) {
				const float frac = (float)i / ndimh;
				fFilter[i] = revndim * i / (float)sqrt(1 + pow(frac, order));
			}
			break;}
		default: {
			for (int i=0; i<ndim; i++) {fFilter[i] = 1;}
			break;}
	}

	return 0;
}

#ifndef _WIN64
#define CGAZODOC_DBP_MAX_IXDIMP 32767
#else
#define CGAZODOC_DBP_MAX_IXDIMP 1048575
#endif
TErr CGazoDoc::DeconvBackProj(RECONST_QUEUE* rq, double center, int iMultiplex, int iOffset,
								int* nSinogr, double* tcpu, float* pixelBase, float* pixelDiv) {
	const int iInterpolation = rq->iInterpolation;
	const int iZooming = (iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iBinning = (iInterpolation == 0) ? 4 : ((iInterpolation == 1) ? 2 : 1);
	//090213 const int ixlen = rq->iSinoXdim;
	const int ixlen = rq->iXdim / iBinning;
	const int ipintp = (int) pow((double)2, iZooming);
	const int ndimp = (int)(log((double)ixlen) / LOG2) + 1 + iZooming;
	const int ndim = (int) pow((double)2, ndimp);
	//090213 const int ixdimp = ixdim * ipintp;
	const int ixdimp = ixlen * ipintp;
	const int ixdim2 = ixdimp * ixdimp;
	if (rq->bOffsetCT) {
		if (center <= rq->iRawSinoXdim / 2.) {
			//center = rq->iSinoXdim - center - 1.5;//100212 Trimming not considered
			center = rq->iRawSinoXdim + center;//100213
		}
	}
	if (ixdimp > CGAZODOC_DBP_MAX_IXDIMP) {
		if (!bCmdLine) AfxMessageBox("Too fine interpolation or lengthy image width");
		return 21022;
	}
	if (maxReconst < ixdim2) {
		int* irct = iReconst;
		try {iReconst = new int[ixdim2];}
		catch(CException* e) {
			e->Delete();
			iReconst = irct; 
			return 21021;
		}
		if (irct) delete [] irct;
		maxReconst = ixdim2;
	}
	nReconst = ixdimp;
	memset(iReconst, 0, sizeof(int) * maxReconst);
	//100318 for (int i=0; i<ixdim2; i++) {iReconst[i] = 0;}
	//
	TErr err = SetFilter(rq, ndim);
	if (err) return err;
	////Projection
	CMainFrame* pf = NULL;
	if (!bCmdLine) pf = (CMainFrame*) AfxGetMainWnd();
	*nSinogr = 0;
	CString dataName = rq->dataPath;
	if (dataPath.GetLength()) {
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, dataPath.Left(dataPath.GetLength() - 1));
		TCHAR fnm[_MAX_FNAME]; 
		_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnm, _MAX_FNAME, NULL, 0);
		dataName = fnm;
	}
	struct _timeb tstruct; double tm0;
	_ftime_s( &tstruct );
	tm0 = tstruct.time + tstruct.millitm * 0.001;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	int nCPU = 1;
	if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) {
		nCPU = (int)(pApp->dlgProperty.iCPU);
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		nCPU = (int)(pApp->dlgProperty.iCUDA);
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
		nCPU = (int)(pApp->dlgProperty.iATIstream);
	} else {
		return 21022;
	}
	int** ppiReconst = NULL;
	if (nCPU > 1) {
		if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
			try {
				ppiReconst = new int*[nCPU - 1];
				for (int i = 0; i < nCPU - 1; i++) {
					if (CUDA_MALLOC_HOST_INT(&(ppiReconst[i]), maxReconst * sizeof(int)) != cudaSuccess) AfxThrowMemoryException();
					memset(ppiReconst[i], 0, sizeof(int) * maxReconst);
				}
			}
			catch (CException* e) {
				e->Delete();
				if (ppiReconst) {
					for (int i = 0; i < nCPU - 1; i++) { if (ppiReconst[i]) CUDA_FREE_HOST(ppiReconst[i]); }
					delete[] ppiReconst;
				}
				return 21023;
			}
		} else {//(pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA)
			//190707==>uncommented, though each thread calculates separate iy0-iy1 section.
			try {
				ppiReconst = new int*[nCPU - 1];
				for (int i = 0; i < nCPU - 1; i++) {
					ppiReconst[i] = new int[maxReconst];
					memset(ppiReconst[i], 0, sizeof(int) * maxReconst);
				}
			}
			catch (CException* e) {
				e->Delete();
				if (ppiReconst) {
					for (int i = 0; i < nCPU - 1; i++) { if (ppiReconst[i]) delete[] ppiReconst[i]; }
					delete[] ppiReconst;
				}
				return 21023;
			}
			//==>190707
		}
	}
	for (int i=nCPU-1; i>=0; i--) {
		ri[i].hThread = NULL;
		ri[i].iStatus = RECONST_INFO_BUSY;
		ri[i].iStepSino = nCPU;
		ri[i].ixdim = ixlen;
		ri[i].iInterpolation = iInterpolation;
		ri[i].center = (center + HALFPIXEL_OFFSET) / iBinning;
		ri[i].iLenSinogr = rq->iSinoYdim + 1;//090214 iLenSinogr;
		ri[i].iReconst = (i && ppiReconst) ? ppiReconst[i-1] : iReconst;
		ri[i].nSinogr = nSinogr;
		//ri[i].dataName = dataName;
		strcpy_s(ri[i].dataName, dataName.Left(60));
		ri[i].bInc = bInc;
		ri[i].fdeg = fdeg;
		ri[i].fTiltAngle = rq->fTiltAngle;
		ri[i].iMultiplex = iMultiplex;
		ri[i].iOffset = iOffset;
		ri[i].maxSinogrLen = maxSinogrLen;
		if (rq->bOffsetCT) ri[i].iSinogr = ofSinogr;
		else ri[i].iSinogr = iSinogr;
		ri[i].fFilter = fFilter;
		//110920 ri[i].bAngularIntp = rq->bAngularIntp;
		ri[i].dReconFlags = rq->dReconFlags;
		void* pArg = (void*)(&(ri[i]));
		if (i) {
			//_beginthreadex( NULL, 0, DeconvBackProjThread, pArg, 0, &(ri[i].threadID) );
			ri[i].hThread = (unsigned int)_beginthreadex( NULL, 0, DeconvBackProjThread, pArg, 0, &(ri[i].threadID) );
		} else {
			DeconvBackProjThread(&(ri[i]));
		}
	}
	int ist = RECONST_INFO_IDLE;
	do {
		::ProcessMessage();
		Sleep(10);//181223
		ist = RECONST_INFO_IDLE;
		for (int i=nCPU-1; i>=0; i--) {
			if (ri[i].iStatus == RECONST_INFO_ERROR) {
				//AfxMessageBox("ERROR");
				ri[i].iStatus = RECONST_INFO_IDLE;
				dlgReconst.iStatus = CDLGRECONST_STOP;
			}
			ist |= ri[i].iStatus;
		}
	} while (ist != RECONST_INFO_IDLE);
	for (int i=0; i<nCPU; i++) {
		if (ri[i].hThread) CloseHandle((HANDLE)(ri[i].hThread));
		if ((i >= 1) && ppiReconst) {
			if (ppiReconst[i-1]) {
				for (int j = 0; j < maxReconst; j++) { iReconst[j] += (ppiReconst[i-1])[j]; }
				if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) CUDA_FREE_HOST(ppiReconst[i-1]);
				else delete[] ppiReconst[i - 1];
			}
		}
	}
	if (ppiReconst) delete[] ppiReconst;
	//
	if ((dlgReconst.iStatus == CDLGRECONST_STOP) || (dlgReconst.iStatus & CDLGRECONST_WHEEL)) {
		if (pf) pf->m_wndStatusBar.SetPaneText(0, "Aborted");
		return 0;
	}
	_ftime_s( &tstruct );
	*tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	int imax = 0, imin = INT_MAX;
	for (int i=0; i<ixdim2; i++) {
		imax = iReconst[i] > imax ? iReconst[i] : imax;
		imin = iReconst[i] < imin ? iReconst[i] : imin;
	}
	//CString line; line.Format("%d %d", imin, imax); AfxMessageBox(line);
	if (imin == imax) imax++;//////
	const double scaleFactor = 6.2831853 * 10000. / (rq->dPixelWidth * iBinning);// 1/10000 cm = 1 um
	double sc = scaleFactor / (LOG_SCALE * BACKPROJ_SCALE * (*nSinogr));
	if (rq->dReconFlags & RQFLAGS_ZERNIKE) {//121127
		sc = scaleFactor / (ZERNIKE_SCALE * BACKPROJ_SCALE * (*nSinogr));
	}
	*pixelBase = (float)(sc * ipintp * imin);
	*pixelDiv = (float)( (USHRT_MAX - 1) / sc / (imax - imin) / ipintp); 
	int irad2 = ((ipintp * center / iBinning) < (ixdimp - ipintp * center / iBinning)) ?
									(int)(ipintp * center / iBinning) : (int)(ixdimp - ipintp * center / iBinning);
	irad2 *= irad2;
	const int ixdimh = ixdimp / 2;
	for (int i=0; i<ixdimp; i++) {
		int i2 = i - ixdimh; i2 *= i2;
		for (int j=0; j<ixdimp; j++) {
			int j2 = j - ixdimh;
			int idx = i * ixdimp + j;
			if (i2 + j2 * j2 > irad2) iReconst[idx] = 0;
			else {
				iReconst[idx] = (int)( (float)(iReconst[idx] - imin) * (USHRT_MAX - 1) / (imax - imin) );
			}
		}
	}
	//CString msg; 
	//msg.Format("%f %f\r\n%f %f %d\r\n %f %d %d %d", 
	//	*pixelDiv, *pixelBase, scaleFactor, rq->dPixelWidth, iBinning, sc, *nSinogr, ipintp, imax-imin);
	//AfxMessageBox(msg);
	return 0;
}

void CGazoDoc::ResetSinogram() {
	dAxisCenter = 0;
	dAxisGrad = 0;
	iCurrSinogr = -1;
	iCurrTrim = -1;
}

TErr CGazoDoc::GetAxis(int iTargetSlice, double* pCenter, double* pGrad, 
					   int iAxisOffset, int iDatasetSel, bool bReport) {
	//110112 this routine only supports ITEX files with prefix 'q'.
	if (!bReport && (dAxisCenter > 0)) {
		*pCenter = dAxisCenter;
		*pGrad = dAxisGrad;
		return 0;
	} else if (!bReport && dAxisCenter < 0) {
		return 21050;
	}
	dAxisCenter = -1;
	//log file should be loaded
	//get indexes in the opposite direction
	int idxIncident0 = -1;
	int idxSample0 = -1;
	for (int i=0; i<iLenSinogr-1; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) {
			idxIncident0 = i;
			if (idxSample0 < 0) continue;
		} else {
			if (idxSample0 >= 0) continue;
			idxSample0 = i;
			if (idxIncident0 < 0) continue;
		}
		break;
	}
	const float aSample0 = fdeg[idxSample0];
	//130203 get delta angle
	double deltaAngle = 0.1;
	if (idxSample0+1 < iLenSinogr-1) deltaAngle = fabs(fdeg[idxSample0+1] - fdeg[idxSample0]);
	if (deltaAngle < 1E-6) deltaAngle = 0.1;
	//
	int idxSample1 = -1;
	float diff, minDiff = 190;
	for (int i=0; i<iLenSinogr-1; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		//130203 diff = (float)fabs(fabs(fdeg[i] - aSample0) - 180);
		diff = (float)fabs(fabs(fdeg[i] - aSample0) - (180. - deltaAngle));
		//q1804 at 180 deg might be pointed to images with a higher index, such as a3722.img in the conv.bat file.
		//Therefore q1801 at 179.9 deg would be better.
		if (diff < minDiff) {idxSample1 = i; minDiff = diff;}
	}
	//const float aSample1 = fdeg[idxSample1];
//	int idxIncident1 = -1;
//	minDiff = (float)iLenSinogr;
//	for (int i=iLenSinogr-2; i>=0; i--) {
//		if (bInc[i] & CGAZODOC_BINC_SAMPLE) continue;
//		diff = (float)abs(i - idxSample1);
//		if (diff < minDiff) {idxIncident1 = i; minDiff = diff;}
//	}
//	if ((idxSample0 < 0)||(idxSample1 < 0)||(idxIncident0 < 0)||(idxIncident1 < 0))	return 21041;
	if ((idxSample0 < 0)||(idxSample1 < 0)||(idxIncident0 < 0))	return 21041;
	//read image files
	const int ixydim = ixdim * iydim;
	int* iDark = NULL; int maxDark = 0; int ixDark = 0, iyDark = 0;
	int* iSample0 = NULL; int maxSample0 = 0; int ixSample0 = 0, iySample0 = 0;
	int* iSample1 = NULL; int maxSample1 = 0; int ixSample1 = 0, iySample1 = 0;
	int* iIncident0 = NULL; int maxIncident0 = 0; int ixIncident0 = 0, iyIncident0 = 0;
	int* iIncident1 = NULL; int maxIncident1 = 0; int ixIncident1 = 0, iyIncident1 = 0;
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	const CString imgPath = path_buffer;
	const int ideg = (int)(log10((double)iLenSinogr)) + 1;
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	pf->m_wndStatusBar.SetPaneText(0, "Sample axis estimation: reading files...");
	CFile fimg;
	TErr err = 0;
	if ((_tcscmp(ext, ".img") == 0)||(_tcscmp(ext, ".IMG") == 0)) {
		CString fn;
		const CString fdark = imgPath + "dark.img";
		for (int i=0; i<5; i++) {
			::ProcessMessage();
			switch (i) {
				case 0: {
					/*130206
					if (!fimg.Open(fdark, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					err = ReadITEX(&fimg, &iDark, &maxDark, &iyDark, &ixDark);
					fimg.Close();
					*/
					break;}
				case 1: {
					fn = imgPath + dataPrefix + fname[idxSample0].Right(ideg) + ".img";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iSample0, &maxSample0, &iySample0, &ixSample0);
					fimg.Close();
					break;}
				case 2: {
					fn = imgPath + dataPrefix + fname[idxSample1].Right(ideg) + ".img";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iSample1, &maxSample1, &iySample1, &ixSample1);
					fimg.Close();
					break;}
				case 3: {
					fn = imgPath + dataPrefix + fname[idxIncident0].Right(ideg) + ".img";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0);
					fimg.Close();
					break;}
				case 4: {
					/*130206
					fn = imgPath + dataPrefix + fname[idxIncident1].Right(ideg) + ".img";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iIncident1, &maxIncident1, &iyIncident1, &ixIncident1);
					fimg.Close();
					*/
					break;}
				default: {break;}
			}
			if (err) break;
		}
	} else if ((_tcscmp(ext, ".his") == 0)||(_tcscmp(ext, ".HIS") == 0)) {
		CountFrameFromConvBat();
		//
		CString frmPrefix = "";
		//141229 const int nset = maxHisFrame / iFramePerDataset;
		const int nset = dlgReconst.m_nDataset;
		if (nset > 1) {
			const int idigit = (int)log10((double)nset) + 1;
			CString fmt;
			fmt.Format("_%%0%dd", idigit);
			frmPrefix.Format(fmt, iDatasetSel);
		}
		const CString fdark = imgPath + "dark" + frmPrefix + ".img";
		if (err = SetConvList(imgPath, fnm, ext, iDatasetSel)) return err;
		CString fn = GetPathName();//"a.his";
		CFile fhis;
		if (!fhis.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			AfxMessageBox("Image file not found: " + fn);
			return 21012;
		}
		HIS_Header hisHeader;
		for (int i=0; i<5; i++) {
			CString sDigit; sDigit.Format("%d/4", i);
			pf->m_wndStatusBar.SetPaneText(0, "Sample axis estimation: reading images " + sDigit);
			::ProcessMessage();
			switch (i) {
				case 0: {
					/*130206
					if (!fimg.Open(fdark, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					err = ReadITEX(&fimg, &iDark, &maxDark, &iyDark, &ixDark);
					fimg.Close();
					*/
					break;}
				case 1: {
					int cidx = atoi(fname[idxSample0]) - 1;
					if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					//CString msg2 = convList[cidx];//130203
					while (convList[cidx].GetAt(0) == 'q') {//point to another image
						cidx = atoi(convList[cidx].Mid(1)) - 1;
						if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					}
					if (convList[cidx].GetAt(0) != 'r') {err = 21047; break;}
					fhis.SeekToBegin();
					int ihisFrame = atoi(convList[cidx].Mid(1)) - 1;
					//if (nset > 1) ihisFrame += iFramePerDataset * iDatasetSel;//111108
					//141229 if (nset > 1) ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
					if (nset > 1) {
						if (nDarkFrame > 0) {//141229 if dark frames were taken only in the first set.
							ihisFrame = ((ihisFrame-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + (iFramePerDataset-nDarkFrame) * iDatasetSel + nDarkFrame;
						} else {//if dark frames were taken at every dataset.
							ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
						}
					}
					if (err = SkipHISframe(&fhis, ihisFrame)) break;
					err = ReadHIS(&fhis, &iSample0, &maxSample0, &iySample0, &ixSample0, &hisHeader);
					//CString msg; msg.Format("130203 case1 %d %s %s", idxSample0, msg2, convList[cidx]); AfxMessageBox(msg);
					break;}
				case 2: {
					int cidx = atoi(fname[idxSample1]) - 1;
					if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					//CString msg2; msg2.Format("cidx=%d list=%s", cidx, convList[cidx]);//130203
					while (convList[cidx].GetAt(0) == 'q') {//point to another image
						cidx = atoi(convList[cidx].Mid(1)) - 1;
						if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					}
					if (convList[cidx].GetAt(0) != 'r') {err = 21047; break;}
					fhis.SeekToBegin();
					int ihisFrame = atoi(convList[cidx].Mid(1)) - 1;
					//if (nset > 1) ihisFrame += iFramePerDataset * iDatasetSel;//111108
					//141229 if (nset > 1) ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
					if (nset > 1) {
						if (nDarkFrame > 0) {//141229 if dark frames were taken only in the first set.
							ihisFrame = ((ihisFrame-nDarkFrame) % (iFramePerDataset-nDarkFrame)) + (iFramePerDataset-nDarkFrame) * iDatasetSel + nDarkFrame;
						} else {//if dark frames were taken at every dataset.
							ihisFrame = (ihisFrame % iFramePerDataset) + iFramePerDataset * iDatasetSel;//111108
						}
					}
					//140110 The following line takes much CPU time.
					if (err = SkipHISframe(&fhis, ihisFrame)) break;
					//
					err = ReadHIS(&fhis, &iSample1, &maxSample1, &iySample1, &ixSample1, &hisHeader);
					//CString msg; msg.Format("130203 case2 %d %s %s", idxSample1, msg2, convList[cidx]); AfxMessageBox(msg);
					break;}
				case 3: {
					int cidx = atoi(fname[idxIncident0]) - 1;
					if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					while (convList[cidx].GetAt(0) == 'q') {//point to another image
						cidx = atoi(convList[cidx].Mid(1)) - 1;
						if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					}
					if (convList[cidx].GetAt(0) != 'a') {err = 21047; break;}
					fn = convList[cidx].Mid(1); fn.TrimLeft();
					fn = imgPath + fn;
					//frame# is included by SetConvList()
					//fn = imgPath + "q" + fname[idxIncident0].Right(ideg) + ".img";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0);
					fimg.Close();
					//AfxMessageBox("130203 case3 " + convList[cidx]);
					break;}
				case 4: {
					/*130206
					int cidx = atoi(fname[idxIncident1]) - 1;
					if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					while (convList[cidx].GetAt(0) == 'q') {//point to another image
						cidx = atoi(convList[cidx].Mid(1)) - 1;
						if ((cidx < 0)||(cidx > (int)maxConvList)) {err = 21047; break;}
					}
					if (convList[cidx].GetAt(0) != 'a') {err = 21047; break;}
					fn = convList[cidx].Mid(1); fn.TrimLeft();
					fn = imgPath + fn;
					//frame# is included by SetConvList()
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadITEX(&fimg, &iIncident1, &maxIncident1, &iyIncident1, &ixIncident1);
					fimg.Close();
					//AfxMessageBox("130203 case4 " + convList[cidx]);
					*/
					break;}
				default: {break;}
			}
			if (err) {CString msg; msg.Format("err %d", err); AfxMessageBox(msg);}
			if (err) break;
		}
		fhis.Close();
	} else if ((_tcscmp(ext, ".tif") == 0)||(_tcscmp(ext, ".TIF") == 0)) {
		CString fn;
		const CString fdark = imgPath + "dark.tif";
		for (int i=0; i<5; i++) {
			::ProcessMessage();
			switch (i) {
				case 0: {
					/*130206
					if (!fimg.Open(fdark, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					err = ReadTif(&fimg, &iDark, &maxDark, &iyDark, &ixDark);
					fimg.Close();
					*/
					break;}
				case 1: {
					fn = imgPath + dataPrefix + fname[idxSample0].Right(ideg) + ".tif";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadTif(&fimg, &iSample0, &maxSample0, &iySample0, &ixSample0);
					fimg.Close();
					break;}
				case 2: {
					fn = imgPath + dataPrefix + fname[idxSample1].Right(ideg) + ".tif";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadTif(&fimg, &iSample1, &maxSample1, &iySample1, &ixSample1);
					fimg.Close();
					break;}
				case 3: {
					fn = imgPath + dataPrefix + fname[idxIncident0].Right(ideg) + ".tif";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadTif(&fimg, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0);
					fimg.Close();
					break;}
				case 4: {
					/*130206
					fn = imgPath + dataPrefix + fname[idxIncident1].Right(ideg) + ".tif";
					if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {err = 21047; break;}
					if (!fimg) {err = 21047; break;}
					err = ReadTif(&fimg, &iIncident1, &maxIncident1, &iyIncident1, &ixIncident1);
					fimg.Close();
					*/
					break;}
				default: {break;}
			}
			if (err) break;
		}
	} else if ((_tcscmp(ext, ".h5") == 0)||(_tcscmp(ext, ".H5") == 0)) {
		const CString fn = GetPathName();//"*.h5";
		CFile file;
		if (!file.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) return 28001;
		hdf5.SetFile(&file);
		for (int i=0; i<5; i++) {
			::ProcessMessage();
			switch (i) {
				case 0: {
					//dark not used
					break;}
				case 1: {
					err = ReadHDF5Frame(&file, &iSample0, &maxSample0, &iySample0, &ixSample0, &hdf5, 
										idxSample0, -1);
					break;}
				case 2: {
					err = ReadHDF5Frame(&file, &iSample1, &maxSample1, &iySample1, &ixSample1, &hdf5, 
										idxSample1, -1);
					break;}
				case 3: {//incident0
					if (err = hdf5.ReadSuperBlock()) break;
					if (err = hdf5.FindChildSymbol("exchange", -1)) break;
					hdf5.MoveToChildTree();
					if (err = hdf5.FindChildSymbol("data_white", -1)) break;
					const int ientry = hdf5.m_iChildEntry;
					err = ReadHDF5Frame(&file, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0, &hdf5, 
										0, ientry);
					break;}
				case 4: {
					//incident1 not used
					break;}
				default: {break;}
			}//switch(i)
			if (err) break;
		}//for(i<5)
		file.Close();
	} else {
		return 21051;
	}
	if (err) {
		//CString msg; msg.Format("%d %d %d %d\r\n%s\r\nerr=%d i=%d", idxSample0, idxSample1, idxIncident0, idxIncident1, fn, err, i);
		//AfxMessageBox(msg);//121018
		if (iDark) delete [] iDark;
		if (iSample0) delete [] iSample0;
		if (iSample1) delete [] iSample1;
		if (iIncident0) delete [] iIncident0;
		if (iIncident1) delete [] iIncident1;
		return err;
	}
	//AfxMessageBox("121019-10");
	//CString msg; msg.Format("121019 %d %d %d %d", idxSample0, idxSample1, idxIncident0, idxIncident1); AfxMessageBox(msg);
	//double avgSample = 0;
	//double sigSample = 0;
	for (int j=0; j<ixydim; j++) {
		iSample0[j] = (short)(log( (double)(iIncident0[j]) / (double)(iSample0[j]) ) * LOG_SCALE);
		iSample1[j] = (short)(log( (double)(iIncident0[j]) / (double)(iSample1[j]) ) * LOG_SCALE);
		/*130210
		int iSample = iSample0[j] - iDark[j];
		if (iSample <= 0) iSample0[j] = 0;
		else {
			iSample0[j] = (short)(log( (double)(iIncident0[j] - iDark[j]) / (double)iSample ) * LOG_SCALE);
		}
		iSample = iSample1[j] - iDark[j];
		if (iSample <= 0) iSample1[j] = 0;
		else {
			iSample1[j] = (short)(log( (double)(iIncident1[j] - iDark[j]) / (double)iSample ) * LOG_SCALE);
		}///*///
		//avgSample += iSample0[j];
		//sigSample += (double)iSample0[j] * iSample0[j];
	}
	//avgSample /= ixydim;
	//sigSample = sqrt(sigSample / ixydim - avgSample * avgSample);
	//AfxMessageBox("121019-11");
	//sample detection
	/*130206
	int iLayer1 = iydim; int iLayer2 = 0;
	//CString line, scr; line.Format("%f\r\n", (float)sigSample);
	for (int k=0; k<iydim; k++) {
		int iscore = 0;
		for (int j=0; j<ixdim; j++) {if (iSample0[k * ixdim + j] > sigSample) iscore++;}
		//if ((k < 10)||(k % 50 == 0)) {
		//	scr.Format("%d %d\r\n", k, iscore); line += scr;
		//}
		if ((float)iscore / ixdim > 0.05) {
			if (iLayer1 > k) iLayer1 = k;
			if (iLayer2 < k) iLayer2 = k;
		}
	}
	if (iLayer1 > iLayer2) {iLayer1 = 0; iLayer2 = iydim;}
	*/
	delete [] iDark;
	delete [] iIncident0;
	delete [] iIncident1;
	//match
	const int ixdimh = ixdim / 5 > 100 ? ixdim / 5 : 100;
	//const int istep = 30;
	const int istep = iydim < 40 ? 1 : iydim / 40;
	const int icdim = (int)ceil(iydim / (double)istep);
	//CString line; line.Format("iydim %d icdim %d istep %d", iydim, icdim, istep); AfxMessageBox("GAstep1-2" + line);
	double* sdiff = NULL;
	double* center = NULL;
	double* sdifftotal = NULL;
	int* ndifftotal = NULL;
	try {
		sdiff = new double[ixdimh * 2];
		center = new double[icdim];
		sdifftotal = new double[ixdimh * 2];
		ndifftotal = new int[ixdimh * 2];
	}
	catch(CException* e) {
		e->Delete();
		if (sdiff) delete [] sdiff;
		if (center) delete [] center;
		if (sdifftotal) delete [] sdifftotal;
		if (ndifftotal) delete [] ndifftotal;
		delete [] iSample0; delete [] iSample1;
		return 21045;
	}
	//
	//AfxMessageBox("121019-12");
	pf->m_wndStatusBar.SetPaneText(0, "Sample axis estimation: scanning...");
	int ixCenter2 = 0;
	for (int k=0; k<ixdimh * 2; k++) {ndifftotal[k] = 0; sdifftotal[k] = 0;}
	for (int k=0; k<iydim; k+=istep) {
		::ProcessMessage();
		int icidx = k / istep;
		minDiff = SHRT_MAX; int minIdx = 0;
		const int kixdim = k * ixdim;
		if (iAxisOffset) {
			//offset CT
			const int iScanWidth = (iAxisOffset < ixdim/2) ? iAxisOffset / 2 : (ixdim - iAxisOffset) / 2;
			for (int js=-ixdimh; js<ixdimh; js++) {//shift of iSample1
				const int idifidx = js + ixdimh;
				const int j = js + iAxisOffset;
				sdiff[idifidx] = 0;
				if (abs(js) > iScanWidth) continue;
				int ndiff = 0;
				for (int i=-iScanWidth; i<=iScanWidth; i++) {//inverse index of iSample1
					//if ((idx < 0)||(idx >= ixdim)) continue;
					int idiff = iSample1[j - i + kixdim] - iSample0[j + i + kixdim];
					sdiff[idifidx] += (double)idiff * idiff;
					ndiff++;
					sdifftotal[idifidx] += (double)idiff * idiff;
					ndifftotal[idifidx]++;
				}
				if (ndiff == 0) continue;
				sdiff[idifidx] /= ndiff;
				if (sdiff[idifidx] < minDiff) {minDiff = (float)sdiff[idifidx]; minIdx = js;}
			}
			ixCenter2 = (minIdx + iAxisOffset) * 2;
		} else {
			//non-offset CT
			//CString msg = "", line;//130203
			for (int j=-ixdimh; j<ixdimh; j++) {//shift of iSample1
				const int idifidx = j + ixdimh;
				sdiff[idifidx] = 0;
				int ndiff = 0;
				const int ifrom = j >= 0 ? 0 : -j;
				const int ito = j >= 0 ? ixdim-j : ixdim;
				for (int i=ifrom; i<ito; i++) {//inverse index of iSample1
					int idx = i + j;
					//if ((idx < 0)||(idx >= ixdim)) continue;
					idx = iSample1[ixdim - 1 - i + kixdim] - iSample0[idx + kixdim];
					sdiff[idifidx] += (double)idx * idx;
					ndiff++;
					sdifftotal[idifidx] += (double)idx * idx;
					ndifftotal[idifidx]++;
				}
				if (ndiff == 0) continue;
				sdiff[idifidx] /= ndiff;
				if (sdiff[idifidx] < minDiff) {minDiff = (float)sdiff[idifidx]; minIdx = j;}
				//if ((abs(j) < 120)&&(j % 4 == 0)) {line.Format("%f %f\r\n", (j+ixdim-1)/2., sdiff[idifidx]); msg += line;}
			}
			ixCenter2 = minIdx + ixdim - 1;
			//if (k == 0) AfxMessageBox(msg);
		}
		//get center
		center[icidx] = 0;
		//const int ix1 = minIdx + ixdim - 1;
		if ((minIdx == -ixdimh)||(minIdx == ixdimh - 1)) {//if strip end
			center[icidx] = ixCenter2 / 2.;
			//090212 center[icidx] = (minIdx + ixdim - 1) / 2.;
		} else {
			//f(i) = a(i-b)^2 + c
			double dy1 = sdiff[minIdx + ixdimh];
			double dy0 = sdiff[minIdx - 1 + ixdimh];
			double dy2 = sdiff[minIdx + 1 + ixdimh];
			if (dy2 == dy0) center[icidx] = (double)(ixCenter2 / 2.);
			else {
				dy0 = (dy1 - dy0) / (dy2 - dy0);
				center[icidx] = (4 * dy0 * ixCenter2 - (ixCenter2 * 2 - 1)) / (4 * dy0 - 2) / 2;
			}
		}
		center[icidx] -= HALFPIXEL_OFFSET;//090211
	}
	//overall center 130206
	double minDifftotal = SHRT_MAX; int idifftotal = 0;
	for (int j=-ixdimh; j<ixdimh; j++) {//shift
		const int idifidx = j + ixdimh;
		if (ndifftotal[idifidx] == 0) continue;
		sdifftotal[idifidx] /= ndifftotal[idifidx];
		if (sdifftotal[idifidx] < minDifftotal) {
			minDifftotal = (double)sdifftotal[idifidx]; 
			idifftotal = j;
		}
	}
	int ixCenter2total = 0;
	if (iAxisOffset) {ixCenter2total = (idifftotal + iAxisOffset) * 2;}
	else {ixCenter2total = idifftotal + ixdim - 1;}
	double dcentertotal = ixCenter2total / 2.;
	if ((idifftotal > -ixdimh)&&(idifftotal < ixdimh - 1)) {//if not strip end
		//f(i) = a(i-b)^2 + c
		double dy1 = sdifftotal[idifftotal + ixdimh];
		double dy0 = sdifftotal[idifftotal - 1 + ixdimh];
		double dy2 = sdifftotal[idifftotal + 1 + ixdimh];
		if (dy2 != dy0) {
			dy0 = (dy1 - dy0) / (dy2 - dy0);
			dcentertotal = (4 * dy0 * ixCenter2total - (ixCenter2total * 2 - 1)) / (4 * dy0 - 2) / 2;
		}
	}
	dcentertotal -= HALFPIXEL_OFFSET;
	//
	//calc axis in sample region
	/*130206
	for (int k=iLayer1; k<=iLayer2; k+=istep) {
		int icidx = k / istep;
		sigc += center[icidx];
		sigcx += center[icidx] * center[icidx];
		nsig++;
	}*/
	//130206 double cavg = sigc / nsig;
	//130206 double csig = sqrt(sigcx / nsig - cavg * cavg);
	double cavg = dcentertotal;
	double csig = 1;//eliminate centers deviating 3 pixels or greater
	int isig1 = 0; int isig2 = 0;
	double sigc = 0; double sigcx = 0; int nsig = 0;
	//sigc = 0; sigcx = 0; nsig = 0;
	//130206 for (int k=iLayer1; k<=iLayer2; k+=istep) {
	for (int k=0; k<iydim; k+=istep) {
		int icidx = k / istep;
		if (fabs(cavg - center[icidx]) > csig * 3) continue;//090213
		isig1 += k;
		isig2 += k * k;
		sigc += center[icidx];
		sigcx += k * center[icidx];
		nsig++;
	}
	//center(iy) = coeffb + coeffa * iy
	double coeffb = (isig2 * sigc - isig1 * sigcx) / (nsig * isig2 - isig1 * isig1);
	double coeffa = (sigc - nsig * coeffb) / isig1;
	//
	/*130206
	nsig = 0; isig1 = 0;
	for (int k=iLayer1; k<=iLayer2; k+=istep) {
		nsig++;
		int icidx = k / istep;
		if (fabs(coeffb + coeffa * k - center[icidx]) > 2) isig1++;
	}
	//AfxMessageBox("121019-134");
	if (nsig) isig1 = isig1 * 4 / nsig;//isig1 > 0 ===> axis not determined
	*/
	if (nsig < iydim/istep/2) {
		isig1 = 1; //isig1 != 0 ===> axis not determined
		coeffb = dcentertotal;
		coeffa = 0;
	} else {
		isig1 = 0;
	}
	//
	//AfxMessageBox("121019-14");
	if (bReport) {
		CString line, scr = "", scr2 = "";
		if (isig1) scr2 = "*"; else scr = "*";
		line.Format("Layer   Ideal%s    Each%s\r\n--------------------\r\n", scr, scr2);
		for (int k=0; k<iydim; k+=istep * 2) {
			double fnc = coeffb + coeffa * k;
			scr.Format("%5d   %.2f   %.2f\r\n", k, fnc, center[k/istep]);
			line += scr;
		}
		//130206 scr.Format("--------------------\r\nSample region: y= %d-%d\r\n", iLayer1, iLayer2); line += scr;
		scr.Format("--------------------\r\nOverall center %.2f\r\n", dcentertotal); line += scr;
		line += "*Applied values";
		CDlgMessage dlg;
		dlg.m_Msg = line;
		dlg.DoModal();
	}
	if ((coeffb < 0)||(isig1)) {
		//130206 *pCenter = center[iTargetSlice];
		*pCenter = dcentertotal;
		*pGrad = 0;
	} else {
		dAxisCenter = coeffb;
		dAxisGrad = coeffa;
		*pCenter = coeffb;
		*pGrad = coeffa;
	}
	delete [] center;
	delete [] sdiff;
	delete [] sdifftotal;
	delete [] ndifftotal;
	delete [] iSample0;
	delete [] iSample1;
	pf->m_wndStatusBar.SetPaneText(0, "Ready");
	return 0;
}

void CGazoDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);//140110
	return;

	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
	TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
	if ((_tcscmp(ext, ".img") == 0)||(_tcscmp(ext, ".IMG") == 0)) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	//OnUpdateFileSave(pCmdUI);//080301
	//pCmdUI->Enable(true);
	if (dlgReconst.iStatus & CDLGRECONST_BUSY) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}


void CGazoDoc::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	if (dlgReconst.iStatus & CDLGRECONST_BUSY) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateTomoHistg(CCmdUI* pCmdUI) 
{
	if (dlgReconst.iStatus & CDLGRECONST_BUSY) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);
}

void CGazoDoc::OnUpdateTomoReconst(CCmdUI* pCmdUI) 
{
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	if (parentDoc) {
		bool bDisable = true;
		POSITION pos1 = pApp->GetFirstDocTemplatePosition();
		while (pos1 != NULL) {
			CDocTemplate* pDocTemplate = pApp->GetNextDocTemplate(pos1);
			POSITION pos2 = pDocTemplate->GetFirstDocPosition();
			while (pos2 != NULL) {
				CGazoDoc* pd1 = (CGazoDoc*) pDocTemplate->GetNextDoc(pos2);
				if (pd1 == parentDoc) {bDisable = false; break;}
			}
			if (bDisable == false) break;
		}
		if (bDisable) {pCmdUI->Enable(false); return;}
	}//parentDoc
	//080320 if (pApp->IsBusy()) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);	
}

void CGazoDoc::OnTomoHistg() 
{
	if (!dlgHist.m_hWnd) {//150102
		POSITION pos = GetFirstViewPosition();
		CGazoView* pv = NULL;
		while (pos != NULL) {
			pv = (CGazoView*) GetNextView( pos );
			if (pv) {
				bool bFlg;
				pv->GetBoxParams(&(dlgHist.m_TrmCentX), &(dlgHist.m_TrmCentY),
													&(dlgHist.m_TrmSizeX), &(dlgHist.m_TrmSizeY),
													&(dlgHist.m_TrmAngle), &bFlg);
				if (bFlg) dlgHist.m_EnableTrm = TRUE; else dlgHist.m_EnableTrm = FALSE;
				if (pv->bPolygonEnabled) dlgHist.m_bEnablePolygon = TRUE; else dlgHist.m_bEnablePolygon = FALSE;
			}
		}
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];// TCHAR ext[_MAX_EXT];
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
		dlgHist.filePath = path_buffer;
		UpdateView();
		//150102
		dlgHist.Create(IDD_HISTOGRAM);
		//POSITION pos = GetFirstViewPosition();
		//CGazoView* pv = (CGazoView*) GetNextView( pos );
		POINT coord;
		coord.x = 0; coord.y = 0;
		pv->ClientToScreen(&coord);
		dlgHist.SetWindowPos(&CWnd::wndTop, coord.x, coord.y, 100, 0, SWP_NOSIZE | SWP_NOZORDER);
		//
		dlgHist.SetWindowText("Hist/conv " + dlgHist.filePath);
	}
	if (dlgHist.IsWindowVisible()) dlgHist.SetForegroundWindow();
	else dlgHist.ShowWindow(SW_SHOW);
	//150102 dlgHist.DoModal();
}

/*
void CGazoDoc::OnUpdateTomoQueue(CCmdUI* pCmdUI) 
{
	if (dlgReconst.iStatus & CDLGRECONST_BUSY) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(false);
	return;//////////
	pCmdUI->Enable(true);
}*/

void CGazoDoc::OnToolbarFnPause() {
	uiDocStatus &= ~CGAZODOC_STATUS_FILEFWD;
	uiDocStatus &= ~CGAZODOC_STATUS_FILEREV;
}
void CGazoDoc::OnUpdateToolbarFnPause(CCmdUI* pCmdUI) 
{
	if (uiDocStatus & CGAZODOC_STATUS_FILEFWD) pCmdUI->Enable(true);
	else if (uiDocStatus & CGAZODOC_STATUS_FILEREV) pCmdUI->Enable(true);
	else pCmdUI->Enable(false);
}

void CGazoDoc::OnToolbarFnsf() {
	uiDocStatus |= CGAZODOC_STATUS_FILEFWD;
	uiDocStatus &= ~CGAZODOC_STATUS_FILEREV;
	while (uiDocStatus & CGAZODOC_STATUS_FILEFWD) {
		if (ProceedImage(1)) {
			uiDocStatus &= ~CGAZODOC_STATUS_FILEFWD;
			break;
		}
		for (int i=0; i<30; i++) {
			Sleep(10);
			::ProcessMessage();
		}
	}
}
void CGazoDoc::OnUpdateToolbarFnsf(CCmdUI* pCmdUI) 
{
	//TODO: disable if other doc file undergoes ProceedImage.
	if (uiDocStatus & CGAZODOC_STATUS_FILEFWD) pCmdUI->Enable(false);
	else pCmdUI->Enable(true);
}

void CGazoDoc::OnToolbarFnsr() {
	uiDocStatus |= CGAZODOC_STATUS_FILEREV;
	uiDocStatus &= ~CGAZODOC_STATUS_FILEFWD;
	while (uiDocStatus & CGAZODOC_STATUS_FILEREV) {
		if (ProceedImage(-1)) {
			uiDocStatus &= ~CGAZODOC_STATUS_FILEREV;
			break;
		}
		for (int i=0; i<30; i++) {
			Sleep(10);
			::ProcessMessage();
		}
	}
}
void CGazoDoc::OnUpdateToolbarFnsr(CCmdUI* pCmdUI) 
{
	//TODO: disable if other doc file undergoes ProceedImage.
	if (uiDocStatus & CGAZODOC_STATUS_FILEREV) pCmdUI->Enable(false);
	else pCmdUI->Enable(true);
}

void CGazoDoc::OnToolbarFnfwdOne() {ProceedImage(1);}
void CGazoDoc::OnToolbarFnrevOne() {ProceedImage(-1);}

void CGazoDoc::OnToolbarFnfwd() {ProceedImage(CGAZODOC_FILE_FWD);}
void CGazoDoc::OnToolbarFnff() {ProceedImage(CGAZODOC_FILE_FASTFWD);}
void CGazoDoc::OnToolbarFnrev() {ProceedImage(CGAZODOC_FILE_REV);}
void CGazoDoc::OnToolbarFnfr() {ProceedImage(CGAZODOC_FILE_FASTREV);}
void CGazoDoc::OnToolbarFnJumpR() {ProceedImage(CGAZODOC_FILE_JUMPREV);}
void CGazoDoc::OnToolbarFnJumpF() {ProceedImage(CGAZODOC_FILE_JUMPFWD);}

TErr CGazoDoc::ProceedImage(int nproc) {
	POSITION pos = GetFirstViewPosition();
	CGazoView* pv = (CGazoView*) GetNextView( pos );
	const CString fpath = this->GetPathName();
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fpath);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	CString sExt = ext;
	sExt.MakeUpper();
	if ((sExt == ".IMG")||(sExt == ".TIF")) {
		CString filename = fnm;
		//181202 const CString fprefix = filename.SpanExcluding("0123456789.");
		CString fprefix = filename; 
		fprefix.TrimRight("0123456789.");
		CString fidx = filename.Mid(fprefix.GetLength());
		int ifn = atoi(fidx) + nproc;
		if (ifn < 0) return 28001;
		CString fmt; fmt.Format("%%0%dd", fidx.GetLength());
		fidx.Format(fmt, ifn);
		_stprintf_s(fnm, _MAX_FNAME, fprefix + fidx);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
//AfxMessageBox(fprefix);
//AfxMessageBox(path_buffer);
		CFile file;
		CString sfnext = ""; double difnext = -1;
		const double didx0 = atof(filename.Mid(fprefix.GetLength()));
		if (!file.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite)) {
			//search for the nearest file
			_stprintf_s(fnm, _MAX_FNAME, fprefix + "*");
			_tmakepath_s( path_buffer, _MAX_PATH, drive, dir, fnm, ext);
			CString sFileList = "";
			GetFileList(path_buffer, &sFileList);
			//CDlgMessage dlg;
			int iPos = 0; bool bFound = false;
//AfxMessageBox(sFileList);
			do {
				const CString fn = sFileList.Tokenize(_T("\r\n"), iPos);
				if (fn.IsEmpty()) break;
				if (fn.GetLength() <= fprefix.GetLength()) continue;
				fidx = fn.Mid(fprefix.GetLength());
				if (fidx.SpanIncluding("0123456789.").GetLength() == 0) continue;
				double didx1 = atof(fn.Mid(fprefix.GetLength()));
				if (nproc > 0) {
					if (didx1 > didx0) {
						if ((difnext < 0)||(didx1 < difnext)) {difnext = didx1; sfnext = fn;}
					}
				} else {
					if (didx1 < didx0) {
						if ((difnext < 0)||(didx1 > difnext)) {difnext = didx1; sfnext = fn;}
					}
				}
//CString msg; msg.Format("%s %f %f %s", fn, didx0, didx1, sfnext); AfxMessageBox(msg);
				//dlg.m_Msg += fn + " : " + fn2 + "\r\n";
			} while (true);
			if (difnext < 0) return 28001;
			_stprintf_s(path_buffer, _MAX_PATH, sfnext);
			TCHAR fnextnm[_MAX_FNAME];
			_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnextnm, _MAX_FNAME, NULL, 0);
			_tmakepath_s( path_buffer, _MAX_PATH, drive, dir, fnextnm, ext);
			if (!file.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite)) return 28001;
		}
		TErr err = ReadFile(&file);
		file.Close();
		if (sfnext.IsEmpty()) sfnext = fnm;
		if (err ==  WARN_READIMAGE_SIZECHANGE) {
			if (AfxMessageBox("Image size of " + sfnext + "\r\nis different from the current one.\r\nOpen in a new window?", MB_YESNO) == IDNO)
				return err;//160805
			if (!file.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite)) return 16080501;
			CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
			CGazoDoc* pcd = pcv->GetDocument();
			err = pcd->ReadFile(&file);
			file.Close();
			pcd->UpdateView(/*bInit=*/true);
			pcd->SetPathName(path_buffer, FALSE);
			pcd->SetTitle(path_buffer);
			return err;
		}
		this->SetPathName(path_buffer, FALSE);
		this->SetTitle(path_buffer);
		if (pv) {
			CString sFrm = sfnext.SpanExcluding(".").MakeReverse().SpanIncluding("0123456789").MakeReverse();
			int ipgx[CGAZOVIEW_NPOLYGON], ipgy[CGAZOVIEW_NPOLYGON];
			if (!GetPolygon(sFrm, pv->dlgPolygon.sPolygonList, ipgx, ipgy)) {
				for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
					pv->iPolygonX[i] = ipgx[i]; pv->iPolygonY[i] = ipgy[i];
				}
			}
			pv->dlgPolygon.UpdateCurrentPolygon();
		}
		UpdateView();
/*	} else if ((strncmp(ext, ".tif", 4) == 0)||(strncmp(ext, ".TIF", 4) == 0)) {//111109
		CString filename = fnm;
		const CString fprefix = filename.SpanExcluding("0123456789.");
		CString fidx = filename.Mid(fprefix.GetLength());
		int ifn = atoi(fidx) + nproc;
		if (ifn < 0) return 28001;
		CString fmt; fmt.Format("%%0%dd", fidx.GetLength());
		fidx.Format(fmt, ifn);
		_stprintf_s(fnm, _MAX_FNAME, fprefix + fidx);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
		CFile file;
		if (!file.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite)) return 28001;
		ReadFile(&file);
		//dlgReconst.Init(iydim, ixdim);
		UpdateView();
		//
		this->SetPathName(path_buffer, FALSE);
		this->SetTitle(path_buffer);
		file.Close();*/
	} else if (sExt == ".HIS") {
		CString docTitle = this->GetTitle();
		int iframe = 0;
		if (docTitle.GetAt(0) == '[') {
			iframe = atoi(docTitle.Mid(1).SpanIncluding("0123456789"));
		}
		CFile file;
		if (!file.Open(fpath, CFile::modeRead | CFile::shareDenyWrite)) return 28001;
		HIS_Header his;
		if (Read_hishead(&file, &his)) return 28001;
		file.SeekToBegin();
		//const int maxframe = his.n_image1 + (his.n_image2 << 16);
		if (maxHisFrame <= 0) maxHisFrame = his.n_image1 + (his.n_image2 << 16);
		if ((iframe + nproc >= 0)&&(iframe + nproc < maxHisFrame)) iframe += nproc;
		else return 28001;
		if (iframe >= 1) SkipHISframe(&file, iframe);
		//CString msg; msg.Format("%d", iframe-1); AfxMessageBox(msg);
		TErr err = ReadFile(&file);
		if (err) {
			CString line; line.Format("Not supported: %d", err); AfxMessageBox(line); return err;
		}
		InitContrast();
		UpdateView();
		//
		CountFrameFromConvBat();
		//141229 const int nset = maxHisFrame / iFramePerDataset;
		const int nset = dlgReconst.m_nDataset;
		CString title;
		const int jset = (iframe < iFramePerDataset) ?
			0 : ((iframe-nDarkFrame) / (iFramePerDataset-nDarkFrame));
		const int jframe = (iframe < iFramePerDataset) ? 
			iframe : ((iframe-nDarkFrame) % (iFramePerDataset-nDarkFrame));
		//141229 if (nset > 1) title.Format("[%d(dataset %d frame %d)] %s", iframe, iframe / iFramePerDataset, iframe % iFramePerDataset, fpath);
		if (nset > 1) title.Format("[%d(dataset %d frame %d)] %s", iframe, jset, jframe, fpath);
		else title.Format("[%d] %s", iframe, fpath);
		this->SetTitle(title);
		file.Close();
		if (pv) pv->dlgPolygon.UpdateCurrentPolygon();
	} else if (sExt == ".H5") {
		CString docTitle = this->GetTitle();
		int iframe = 0;//ientry = hdf5.m_iChildEntry;
		CString sSymbol = "data";
		if (docTitle.GetAt(0) == '[') {
			const CString sFrame = docTitle.Mid(1).SpanIncluding("0123456789");
			iframe = atoi(sFrame);
			sSymbol = docTitle.Mid(sFrame.GetLength()+2).SpanExcluding(")");
		}
//CString msg; msg.Format("%d %s", iframe, sSymbol); AfxMessageBox(msg);
		CFile file;
		if (!file.Open(fpath, CFile::modeRead | CFile::shareDenyWrite)) return 28001;
		hdf5.SetFile(&file);
		TErr err = 0;
		if (err = hdf5.ReadSuperBlock()) return err;
		if (err = hdf5.FindChildSymbol("exchange", -1)) return err;
		hdf5.MoveToChildTree();
		if (err = hdf5.FindChildSymbol(sSymbol, -1)) return err;
		int ientry = hdf5.m_iChildEntry;
		if (err = hdf5.GetDataObjHeader()) return err;
		if (iframe + nproc < 0) {
			iframe += nproc;
			while (true) {
				ientry--;
				if (ientry < 0) break;
				if (hdf5.FindChildSymbol("null", ientry)) break;
				if (hdf5.m_sChildTitle.Left(4) != "data") continue;
				if (hdf5.GetDataObjHeader()) break;
				iframe += (int)hdf5.m_plDataSize[0];
				if (iframe >= 0) break;
			}
			if (iframe < 0) {file.Close(); return 28001;}
		} else if (iframe + nproc < hdf5.m_plDataSize[0]) {
			iframe += nproc;
		} else {
			iframe = iframe + nproc - (int)hdf5.m_plDataSize[0];
			const int ientryOrg = ientry;
			bool bFound = false;
			while (true) {
				ientry++;
				if (hdf5.FindChildSymbol("null", ientry)) break;
				if (hdf5.m_sChildTitle.Left(4) != "data") continue;
				if (hdf5.GetDataObjHeader()) break;
				if (iframe < hdf5.m_plDataSize[0]) {bFound = true; break;}
				else {iframe -= (int)hdf5.m_plDataSize[0];}
			}
			if (!bFound) {hdf5.m_iChildEntry = ientryOrg; file.Close(); return 28001;}
		}
		if (err = ReadHDF5Frame(&file, &pPixel, &maxPixel, &iydim, &ixdim, &hdf5, iframe, ientry)) return err;
		InitContrast();
		UpdateView();
		CString title;
		title.Format("[%d(%s)] %s", iframe, hdf5.m_sChildTitle, fpath);
		this->SetTitle(title);
		file.Close();
		if (pv) pv->dlgPolygon.UpdateCurrentPolygon();
	} else {
		return 28001;
	}
	return 0;
}


void CGazoDoc::OnTomoAxis() 
{
	double dCenter, dGrad;
	BeginWaitCursor();
	GetAxis(0, &dCenter, &dGrad, dlgReconst.GetOffsetCTaxis(), 0, true);//111108
	EndWaitCursor();
}

void CGazoDoc::OnUpdateTomoAxis(CCmdUI* pCmdUI) 
{
	if (dlgReconst.m_hWnd) pCmdUI->Enable(true);
	else pCmdUI->Enable(false);
}


void CGazoDoc::OnTomoStat() 
{
	POSITION pos = GetFirstViewPosition();
	CGazoView* pv = (CGazoView*) GetNextView( pos );
	if (!pv) return;
	int ibcx, ibcy, ibsx, ibsy, iba = 0;
	int ibx0 = 0, iby0 = 0, ibx1 = ixdim-1, iby1 = iydim-1; 
	bool bBoxFlg = false;
	pv->GetBoxParams(&ibcx, &ibcy, &ibsx, &ibsy, &iba, &bBoxFlg);
	if (pv->bPolygonEnabled) {
		ibx0 = pv->iPolygonX[0]; iby0 = pv->iPolygonY[0]; 
		ibx1 = pv->iPolygonX[0]; iby1 = pv->iPolygonY[0]; 
		for (int i=1; i<CGAZOVIEW_NPOLYGON; i++) {
			ibx0 = (pv->iPolygonX[i] < ibx0) ? pv->iPolygonX[i] : ibx0;
			iby0 = (pv->iPolygonY[i] < iby0) ? pv->iPolygonY[i] : iby0;
			ibx1 = (pv->iPolygonX[i] > ibx1) ? pv->iPolygonX[i] : ibx1;
			iby1 = (pv->iPolygonY[i] > iby1) ? pv->iPolygonY[i] : iby1;
		}
	} else if (bBoxFlg) {
		ibx0 = ibcx - ibsx / 2;
		if (ibx0 < 0) ibx0 = 0; else if (ibx0 >= ixdim) ibx0 = ixdim - 1;
		ibx1 = ibx0 + ibsx - 1;
		if (ibx1 < 0) ibx1 = 0; else if (ibx1 >= ixdim) ibx1 = ixdim - 1;
		iby0 = ibcy - ibsy / 2;
		if (iby0 < 0) iby0 = 0; else if (iby0 >= iydim) iby0 = iydim - 1;
		iby1 = iby0 + ibsy - 1;
		if (iby1 < 0) iby1 = 0; else if (iby1 >= iydim) iby1 = iydim - 1;
		if (iba != 0) {AfxMessageBox("Reset box angle."); return;}
	} else {
		ibcx = ixdim/2; ibcy = iydim/2; ibsx = ixdim; ibsy = iydim;
	}
//	while (pos != NULL) {
//		CGazoView* pv = (CGazoView*) GetNextView( pos );
//		if (pv) {
//			pv->GetBoxParams(&ibcx, &ibcy, &ibsx, &ibsy, &iba, &bFlg);
//		}
//	}
	double sum = 0, sum2 = 0;
	int nsum = 0;
	int ipmin = -1, ipmax = -1;
	for (int i=ibx0; i<=ibx1; i++) {
		for (int j=iby0; j<=iby1; j++) {
			if (pv->bPolygonEnabled) {
				CPoint pnt(i, j);
				if (!pv->PointInPolygon(pnt)) continue;
			}
			int ip = pPixel[i + j * ixdim];
			if (bColor) {ip = ((ip & 0xff) + ((ip >> 8) & 0xff) + ((ip >> 16) & 0xff)) / 3;}
			if (ipmin < 0) {ipmin = ip; ipmax = ip;}
			else {
				ipmin = (ip < ipmin) ? ip : ipmin;
				ipmax = (ip > ipmax) ? ip : ipmax;
			}
			double pix = ip;
			sum += pix;
			sum2 += pix * pix;
			nsum++;
		}
	}
	sum /= nsum;
	sum2 /= nsum;
	double sigma = sqrt(sum2 - sum * sum);
	CString line, scr;
	line = this->GetPathName() + "\r\n";
	scr.Format("Pixel intensity\r\n Npixel\tE(x)\tE(x2)\tStdDev\tMax\tMin\r\n %d\t%.1f\t%.1f\t%.1f\t%d\t%d\r\n", 
				nsum, sum, sum2, sigma, ipmax, ipmin); 
	line += scr;
	if (pixDiv > 0) {
		sum = sum / pixDiv + pixBase;
		sigma = sigma / pixDiv;
		scr.Format("LAC\r\n Mean\tStdDev\r\n %.3f\t%.3f\r\n", sum, sigma); line += scr;
	}
	int iHisto[256];
	for (int i=0; i<=255; i++) {iHisto[i] = 0;}
	double dscale = (ipmax > 255) ? (ipmax / 255.) : 1.;
	ipmin - -1, ipmax = -1;
	for (int i=ibx0; i<=ibx1; i++) {
		for (int j=iby0; j<=iby1; j++) {
			if (pv->bPolygonEnabled) {
				CPoint pnt(i, j);
				if (!pv->PointInPolygon(pnt)) continue;
			}
			int ip = (int)(pPixel[i + j * ixdim] / dscale);
			ip = (ip > 0) ? ip : 0;
			ip = (ip <= 255) ? ip : 255;
			iHisto[ip]++;
			if (ipmin < 0) {ipmin = ip; ipmax = ip;}
			else {
				ipmin = (ip < ipmin) ? ip : ipmin;
				ipmax = (ip > ipmax) ? ip : ipmax;
			}
		}
	}
	line += "Histogram\r\n Intensity\tLAC\tRelative freq\tN\r\n";
	for (int i=ipmin; i<=ipmax; i++) {
		scr.Format(" %d\t%.3f\t%.3f\t%d\r\n", (int)(i * dscale), i * dscale / pixDiv + pixBase, (double)iHisto[i] / nsum, iHisto[i]);
		line += scr;
	}
	if (pv->bPolygonEnabled) {
		line += "Polygon vertex (x y)\r\n";
		for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
			scr.Format(" %d\t%d\r\n", pv->iPolygonX[i], pv->iPolygonY[i]);
			line += scr;
		}
	} else if (bBoxFlg) {
		line += "Box diagonal (x y)\r\n";
		scr.Format(" %d\t%d\r\n", ibx0, iby0);
		line += scr;
		scr.Format(" %d\t%d\r\n", ibx1, iby1);
		line += scr;
	} else {
		line += "Image size\r\n";
		scr.Format(" %d\t%d\r\n", ixdim, iydim);
		line += scr;
	}
	line += "\r\nIntesity matrix:\r\n";
	scr.Format(" origin:\t%d\t%d\r\n", ibx0, iby0);
	line += scr;
	for (int i=iby0; i<=iby1; i++) {
		for (int j=ibx0; j<=ibx1; j++) {
			if (pv->bPolygonEnabled) {
				CPoint pnt(i, j);
				if (!pv->PointInPolygon(pnt)) {
					if (bColor) scr = "x x x"; else scr = "x";
					if (j != ibx1) scr += "\t";
					line += scr;
					continue;
				}
			}
			int ip = pPixel[j + i * ixdim];
			if (bColor) {
				scr.Format("%3d %3d %3d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff);
			} else {
				scr.Format("%5d", ip);
			}
			if (j != ibx1) scr += "\t";
			line += scr;
			//double coeff = pix / pixDiv + pixBase;
		}
		line += "\r\n";
	}
	if (pixDiv > 0) {//if reconstructed image
		line += "\r\nLAC matrix:\r\n";
		scr.Format(" origin:\t%d\t%d\r\n", ibx0, iby0);
		line += scr;
		for (int i=iby0; i<=iby1; i++) {
			for (int j=ibx0; j<=ibx1; j++) {
				if (pv->bPolygonEnabled) {
					CPoint pnt(i, j);
					if (!pv->PointInPolygon(pnt)) {
						scr = "x";
						if (j != ibx1) scr += "\t";
						line += scr;
						continue;
					}
				}
				scr.Format("%.3f", (float)(pPixel[j + i * ixdim] / pixDiv + pixBase));
				if (j != ibx1) scr += "\t";
				line += scr;
			}
			line += "\r\n";
		}
	}
	CDlgMessage dlg;
	dlg.m_Msg = line;
	dlg.DoModal();
}

void CGazoDoc::OnUpdateTomoStat(CCmdUI* pCmdUI) 
{
	POSITION pos = GetFirstViewPosition();
	int ibcx, ibcy, ibsx, ibsy, iba = 0;
	bool bFlg = false;
	CGazoView* pv = (CGazoView*) GetNextView( pos );
	if (pv) {
		pv->GetBoxParams(&ibcx, &ibcy, &ibsx, &ibsy, &iba, &bFlg);
	}
	if (bFlg && (iba == 0)) pCmdUI->Enable(true);
	else if (pv) {
		if (pv->bPolygonEnabled) pCmdUI->Enable(true);
	}
	else pCmdUI->Enable(false);
}

void CGazoDoc::OnTomoLine() 
{
	POSITION pos = GetFirstViewPosition();
	int ix0, iy0, ix1, iy1;
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		if (pv) pv->GetLineParams(&ix0, &iy0, &ix1, &iy1);
	}
	double vx = ix1 - ix0;
	double vy = iy1 - iy0;
	const double vlen = sqrt(vx * vx + vy * vy);
	vx /= vlen; vy /= vlen;
	CString line = "Pixel\tRaw data\tLAC\r\n----------------------\r\n";
	CString scr;
	for (int i=0; i<vlen; i++) {
		double gx = ix0 + i * vx;
		double gy = iy0 + i * vy;
		if ((gx >= 0)&&(gy >= 0)&&(gx <= ixdim-1)&&(gy <= iydim-1)) {
			int igx = (int)gx;
			int igy = (int)gy;
			double dx = gx - igx;
			double dy = gy - igy;
			if (igx == ixdim-1) {igx--; dx = 1;}
			if (igy == iydim-1) {igy--; dy = 1;}
			//plane determined by least square fitting
			int is0 = pPixel[igx + igy * ixdim];
			int is1 = pPixel[(igx + 1) + igy * ixdim];
			int is2 = pPixel[igx + (igy + 1) * ixdim];
			int is3 = pPixel[(igx + 1) + (igy + 1) * ixdim];
			if (bColor) {
				is0 = ((is0 & 0xff) + ((is0 >> 8) & 0xff) + ((is0 >> 16) & 0xff)) / 3;
				is1 = ((is1 & 0xff) + ((is1 >> 8) & 0xff) + ((is1 >> 16) & 0xff)) / 3;
				is2 = ((is2 & 0xff) + ((is2 >> 8) & 0xff) + ((is2 >> 16) & 0xff)) / 3;
				is3 = ((is3 & 0xff) + ((is3 >> 8) & 0xff) + ((is3 >> 16) & 0xff)) / 3;
			}
			double ap = 0.5 * (is1 + is3 - is0 - is2);
			double bp = 0.5 * (is2 + is3 - is0 - is1);
			double cp = 0.25 * ((is0 + is1 + is2 + is3) - 2 * (ap + bp));
			//interpolated intensity
			int ipix = (int)(ap * dx + bp * dy + cp);
			if (pixDiv > 0)
				scr.Format("%5d\t%6d\t%.3f\r\n", i, ipix, ipix / pixDiv + pixBase);
			else
				scr.Format("%5d\t%6d\tN.D.\r\n", i, ipix);
			line += scr;
		} else {
			scr.Format("%5d\tN.D.\tN.D.\r\n", i);
			line += scr;
		}
	}
	scr.Format("Line: (%d,%d)-(%d,%d)", ix0, iy0, ix1, iy1);//210105
	line += scr;
	CDlgMessage dlg;
	dlg.m_Msg = line;
	dlg.DoModal();
}

void CGazoDoc::OnUpdateTomoLine(CCmdUI* pCmdUI) 
{
	POSITION pos = GetFirstViewPosition();
	bool bFlg = false;
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		if (pv) bFlg = pv->GetLineParams();
	}
	if (bFlg) pCmdUI->Enable(true);
	else pCmdUI->Enable(false);	
}

//void CGazoDoc::OnTomoProperty()
//{
//	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
//	pApp->dlgProperty.DoModal();
//	//CString line;
//	//line.Format("CPU:%d Memory:%d", dlgProperty.iCPU, dlgProperty.iMemory); AfxMessageBox(line);
//	//return;
//}

//void CGazoDoc::OnUpdateTomoProperty(CCmdUI *pCmdUI)
//{
//	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
//#ifndef _DEBUG
//	//if (dlgReconst.IsWindowVisible()) pCmdUI->Enable(false);
//	if (parentDoc) {
//		if (parentDoc->dlgReconst.m_hWnd) {pCmdUI->Enable(false); return;}
//	} else {
//		if (dlgReconst.m_hWnd) {pCmdUI->Enable(false); return;}
//	}
//	pCmdUI->Enable(true);
//#endif
//
//}

void CGazoDoc::OnTomoRefrac()
{
	//if (parentDoc) {parentDoc->OnTomoRefrac(); return;}
	TErr err = 0;
	//
	if (!dlgRefraction.m_hWnd) {
		TCHAR path_buffer[_MAX_PATH];
		_stprintf_s(path_buffer, _MAX_PATH, GetPathName());
		TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME];// TCHAR ext[_MAX_EXT];
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, NULL, 0);
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
		dataPath = path_buffer;
		dataPrefix = fnm;
		dataPrefix = dataPrefix.SpanExcluding("0123456789");
		//
		CString title = dataPath;
		if (dataPath.GetLength()) {
			_stprintf_s(path_buffer, _MAX_PATH, dataPath.Left(dataPath.GetLength()-1));
			_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnm, _MAX_FNAME, NULL, 0);
			title = fnm;
		}
		//
		if ( err = LoadLogFile(dlgReconst.m_bOffsetCT) ) {error.Log(err); return;}
		//CGazoApp* pApp = (CGazoApp*) AfxGetApp();
		//100315 if (pApp->prevPixelWidth > 0) dlgRefraction.m_PxSize = pApp->prevPixelWidth;
		//100315 if (!dlgRefraction.m_hWnd) dlgRefraction.Create(IDD_REFRACTION);
		dlgRefraction.Create(IDD_REFRACTION);
		//090216
		POSITION pos = GetFirstViewPosition();
		CGazoView* pv = (CGazoView*) GetNextView( pos );
		POINT coord;
		coord.x = 0; coord.y = 0;
		pv->ClientToScreen(&coord);
		dlgRefraction.SetWindowPos(&CWnd::wndTop, coord.x, coord.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//
		dlgRefraction.SetWindowText("Refraction correction " + title);
	}
	if (dlgRefraction.IsWindowVisible()) dlgRefraction.SetForegroundWindow();
	else dlgRefraction.ShowWindow(SW_SHOW);
}

void CGazoDoc::OnUpdateTomoRefrac(CCmdUI *pCmdUI)
{
	if (bDebug) pCmdUI->Enable(true);
	else pCmdUI->Enable(false);
}

void CGazoDoc::ShowRefracCorr(REFRAC_QUEUE* refq) {
	//130210==>
	const int imgx = this->ixdim;//refq->iXdim;
	const int imgy = this->iydim;//refq->iYdim;
	const int ndimxp = (int)((log((double)imgx) / LOG2)) + 1;
	const int ndimx = (int) pow((double)2, ndimxp);
	const int ndimyp = (int)((log((double)imgy) / LOG2)) + 1;
	const int ndimy = (int) pow((double)2, ndimyp);
	CCmplx* cPixel;
	try {cPixel = new CCmplx[ndimx * ndimy];}
	catch (CException* e) {e->Delete(); return;}
	for (int i=0; i<ndimx*ndimy; i++) {cPixel[i].Reset();}
	for (int i=0; i<imgx; i++) {
		for (int j=0; j<imgy; j++) {
			const int idx = i+j*imgx;
			//cPixel[i+j*ndimx].re = (float)(pPixel[idx]);
			cPixel[i+j*ndimx].re = (float)(pPixel[idx] / pixDiv + pixBase);
		}
	}
	//140612 refq->ifftx = ndimx;
	//140612 refq->iffty = ndimy;
	CFft fft2;
	fft2.Init2(ndimxp, -1, ndimyp, -1);
	fft2.FFT2Rev(cPixel);
	double tcpu = 0;
	//==>130212//////////
	/*130210
	CString fn = "";
	//read dark
	fn = refq->dataPath + "dark.img";
	CFile fimg;
	TErr err = 0;
	if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
		AfxMessageBox("Dark image not found");
		return;
	}
	int* iDark = NULL; int maxDark = 0; int ixDark, iyDark;
	err = ReadITEX(&fimg, &iDark, &maxDark, &iyDark, &ixDark);
	fimg.Close();
	if (err) {
		if (iDark) delete [] iDark;
		return;
	}
	//set file index
	int* iIncident0 = NULL; int maxIncident0 = 0; int ixIncident0, iyIncident0;
	int* iIncident1 = NULL; int maxIncident1 = 0; int ixIncident1, iyIncident1;
	const CString sframe = GetTitle().Mid(refq->itexFilePrefix.GetLength()).SpanExcluding(".");
	const int iframe = atoi(sframe);
	//find position in log file
	int iSample = -1;
	int iInc0pos = -1, iInc1pos = -1;
	for (int i=0; i<iLenSinogr; i++) {
		if (atoi(fname[i]) == iframe) {iSample = i; break;}
	}
	if ((iSample < 0)||((bInc[iSample] & CGAZODOC_BINC_SAMPLE) == 0)) {
		if (iDark) delete [] iDark;
		return;
	}
	//read incidents
	if ((iSample <= iInc0pos)||(iSample >= iInc1pos)) {
		//find incident
		for (int i=iSample; i<iLenSinogr; i++) {
			if ((bInc[i] & CGAZODOC_BINC_SAMPLE) == 0) {iInc1pos = i; break;}
		}
		for (int i=iSample; i>=0; i--) {
			if ((bInc[i] & CGAZODOC_BINC_SAMPLE) == 0) {iInc0pos = i; break;}
		}
		bool bIncErr = false;
		if (iInc0pos < 0) {
			if (iInc1pos >= 0) iInc0pos = iInc1pos;
			else bIncErr = true;
		}
		if (iInc1pos < 0) {
			if (iInc0pos >= 0) iInc1pos = iInc0pos;
			else bIncErr = true;
		}
		if (bIncErr) {
			if (iDark) delete [] iDark;
			return;
		}
		const int ideg = (int)(log10((double)iLenSinogr)) + 1;
		err = 0;
		fn = refq->dataPath + refq->itexFilePrefix + fname[iInc0pos].Right(ideg) + ".img";
		//CString line = "";
		//line += fn + "\r\n";
		if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			AfxMessageBox("Incident image not found");
			err++;
		} else {
			err += ReadITEX(&fimg, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0);
			fimg.Close();
		}
		fn = refq->dataPath + refq->itexFilePrefix + fname[iInc1pos].Right(ideg) + ".img";
		//line += fn + "\r\n";
		if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			AfxMessageBox("Incident image not found");
			err++;
		} else {
			err += ReadITEX(&fimg, &iIncident1, &maxIncident1, &iyIncident1, &ixIncident1);
			fimg.Close();
		}
		if (err) {
			if (iDark) delete [] iDark;
			if (iIncident0) delete [] iIncident0;
			if (iIncident1) delete [] iIncident1;
			return;
		}
		//AfxMessageBox(line);
	}
	//
	const double fexp0 = fexp[iInc0pos];
	const double fexp1 = fexp[iInc1pos];
	double fd = 0;
	if (fexp0 != fexp1) fd = (fexp[iSample] - fexp0) / (fexp1 - fexp0);
	//
	const int imgx = this->ixdim;//refq->iXdim;
	const int imgy = this->iydim;//refq->iYdim;
	const int ndimxp = (int)((log((double)imgx) / LOG2)) + 1;
	const int ndimx = (int) pow((double)2, ndimxp);
	const int ndimyp = (int)((log((double)imgy) / LOG2)) + 1;
	const int ndimy = (int) pow((double)2, ndimyp);
	CCmplx* cPixel;
	if ((cPixel = new CCmplx[ndimx * ndimy]) == NULL) return;
	for (int i=0; i<ndimx*ndimy; i++) {cPixel[i].Reset();}
	for (int i=0; i<imgx; i++) {
		for (int j=0; j<imgy; j++) {
			const int idx = i+j*imgx;
			double dIncident = iIncident0[idx] + fd * (iIncident1[idx] - iIncident0[idx]);
			double dSample = (pPixel[idx] - iDark[idx]) * 16384;
			if ((dSample < 0)||(dIncident < 2)) cPixel[i+j*ndimx].re = 0;
			else cPixel[i+j*ndimx].re = (float)(dSample / dIncident);
			//cPixel[i+j*ndimx].re = (float)(pPixel[idx] - iDark[idx]);
		}
	}
	if (iDark) delete [] iDark;
	if (iIncident0) delete [] iIncident0;
	if (iIncident1) delete [] iIncident1;
	//
	refq->ifftx = ndimx;
	refq->iffty = ndimy;
	CFft fft2;
	fft2.Init2(ndimxp, -1, ndimyp, -1);
	//
	struct _timeb tstruct; double tm0;
	_ftime_s( &tstruct );
	tm0 = tstruct.time + tstruct.millitm * 0.001;
	//
	RefracCorr(refq, &fft2, cPixel);
	//
	_ftime_s( &tstruct );
	double tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	*/
	//Generate View
	//
	CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
	CGazoDoc* pcd = pcv->GetDocument();
	if ((pcd->pPixel = new int[maxPixel]) == NULL) return;
	pcd->maxPixel = maxPixel;
	pcd->ixdim = imgx;
	pcd->iydim = imgy;
	pcd->parentDoc = this;
	pcd->SetModifiedFlag(TRUE);
	pcd->pixBase = pixBase;
	pcd->pixDiv = pixDiv;
	pcd->fCenter = fCenter;
	for (int i=0; i<imgx; i++) {
		for (int j=0; j<imgy; j++) {
			//pcd->pPixel[i+j*imgx] = (int)( cPixel[i+j*ndimx].Modulus() );
			pcd->pPixel[i+j*imgx] = (int)( log(cPixel[i+j*ndimx].Modulus2()) * 100);
		}
	}
	pcd->UpdateView(/*bInit=*/true);
	CString title;
	//fn.Format("%09d", iy);
	//fn = fsuffix + fn.Right((int)(log10((double)iLenSinogr)) + 1);
	title.Format("%s phase-retrieved / %.2f sec", GetTitle(), tcpu);
	pcd->SetTitle(title);
	pcd->dataPath = dataPath;
	pcd->dataPrefix = dataPrefix;
	//
	if (cPixel) delete [] cPixel;
	return;
}

void CGazoDoc::RefracCorr(REFRAC_QUEUE* refq, CFft* fft2, CCmplx* cPixel) {
	const int ndimx = refq->ifftx;
	const int ndimy = refq->iffty;
	fft2->FFT2Rev(cPixel);
	const double mu = refq->dLAC;
	const double deltaz0 = refq->ndz0;
	const double psize = refq->dPixelWidth * 0.0001;//cm
	const double px2 = psize * psize * ndimx * ndimx;//cm2
	const double py2 = psize * psize * ndimy * ndimy;//cm2
	for (int i=0; i<ndimx; i++) {
		int ni = i;
		if (ni > ndimx/2) ni = ndimx - i;
		const double pni = ni * ni / px2;
		for (int j=0; j<ndimy; j++) {
			int nj = j;
			if (nj > ndimy/2) nj = ndimy - j;
			cPixel[i + j*ndimx] *= (float)(mu / (deltaz0 * (pni + nj * nj / py2) + mu));
		}
	}
	fft2->FFT2(cPixel);
}

void CGazoDoc::BatchRefracCorr(REFRAC_QUEUE* refq) {
	CString fn = "";
	const int ideg = (int)(log10((double)iLenSinogr)) + 1;
	//read dark
	fn = refq->dataPath + "dark.img";
	CFile fimg;
	TErr err = 0;
	if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
		if (dlgRefraction.m_hWnd)	{
			dlgRefraction.GetDlgItem(IDC_REFR_STATUS)->SetWindowText("Dark image not found");
		}
		return;
	}
	int* iDark = NULL; int maxDark = 0; int ixDark, iyDark;
	err = ReadITEX(&fimg, &iDark, &maxDark, &iyDark, &ixDark);
	fimg.Close();
	if (err) {
		if (dlgRefraction.m_hWnd)	{
			dlgRefraction.GetDlgItem(IDC_REFR_STATUS)->SetWindowText("Dark image error");
		}
		if (iDark) delete [] iDark;
		return;
	}
	//threading
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	const int nCPU = (int)(pApp->dlgProperty.iCPU);
	//int nCPU = 1;
	for (int i=nCPU-1; i>=0; i--) {
		ri[i].hThread = NULL;
		ri[i].iStatus = RECONST_INFO_BUSY;
		ri[i].iStepSino = nCPU;
		ri[i].iLenSinogr = iLenSinogr;
		ri[i].bInc = bInc;
		ri[i].fdeg = fdeg;//100330
		ri[i].fexp = fexp;//100330
		ri[i].fFilter = (float*)((void*)refq);
		ri[i].nSinogr = (int*)((void*)fname);
		ri[i].iReconst = iDark;
		///
		void* pArg = (void*)(&(ri[i]));
		if (i) {
			ri[i].hThread = (unsigned int)_beginthreadex( NULL, 0, RefracCorrThread, pArg, 0, &(ri[i].threadID) );
		} else {
			RefracCorrThread(&(ri[i]));
		}
	}
	int ist = RECONST_INFO_IDLE;
	do {
		::ProcessMessage();
		ist = RECONST_INFO_IDLE;
		for (int i=nCPU-1; i>=0; i--) {
			if (ri[i].iStatus == RECONST_INFO_ERROR) {
				//AfxMessageBox("ERROR");
				ri[i].iStatus = RECONST_INFO_IDLE;
				dlgRefraction.iStatus = CDLGREFRAC_STOP;
			}
			ist |= ri[i].iStatus;
		}
	} while (ist != RECONST_INFO_IDLE);
	for (int i=nCPU-1; i>=0; i--) {if (ri[i].hThread) CloseHandle((HANDLE)(ri[i].hThread));}//120723
	if (!err) {
		if (dlgRefraction.m_hWnd)	{
			dlgRefraction.GetDlgItem(IDC_REFR_STATUS)->SetWindowText("Finished");
		}
	}
	if (iDark) delete [] iDark;
	return;
}


void CGazoDoc::OnTomoComment()
{
	CString sInfo;
	sInfo.Format("LAC = pixel / pixdiv + pixbase\r\n pixdiv = %f, pixbase = %f\r\nTIFF file comment:\r\n", pixDiv, pixBase);
	CDlgMessage dlg;
	dlg.m_Msg = sInfo + fileComment;
	dlg.DoModal();
//	AfxMessageBox(fileComment);
}

void CGazoDoc::OnTomoHorizcent()
{
	CDlgHorizcenter dlg;
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	if (GetDataPath().GetLength()) {
		_stprintf_s(path_buffer, _MAX_PATH, this->GetDataPath() + "centerList.txt");
	} else {
		_stprintf_s(path_buffer, _MAX_PATH, this->GetPathName());
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		_stprintf_s(fnm, _MAX_FNAME, "centerList");
		_stprintf_s(ext, _MAX_EXT, ".txt");
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
	}
	dlg.mOutpath = path_buffer;
	dlg.SetDoc(this);
	dlg.DoModal();
}

int ResolnListCompare( const void *arg1, const void *arg2 ) {
	if ( (*(struct CGZD_RESOLN_LIST*)arg1).dDistance2 > (*(struct CGZD_RESOLN_LIST*)arg2).dDistance2 ) return 1;
	else if ( (*(struct CGZD_RESOLN_LIST*)arg1).dDistance2 < (*(struct CGZD_RESOLN_LIST*)arg2).dDistance2 ) return -1;
	else return 0;
}

void CGazoDoc::OnTomographyResolutionReport()
{
	if ((ixdim <= 0)||(iydim <= 0)) return;

	POSITION pos = GetFirstViewPosition();
	CGazoView* pv = (CGazoView*) GetNextView( pos );
	int ixcent, iycent, imgx, imgy;
	int iangle = 0;
	bool bEnableBox = false;
	pv->GetBoxParams(&ixcent, &iycent, &imgx, &imgy, &iangle, &bEnableBox);
	if (!bEnableBox) {
		imgx = this->ixdim;
		imgy = this->iydim;
	}

	//const int imgx = this->ixdim;
	//const int imgy = this->iydim;
	const int ndimxp = (int)ceil((log((double)imgx) / LOG2));
	const int ndimyp = (int)ceil((log((double)imgy) / LOG2));
	const int ndimp = (ndimxp > ndimyp) ? ndimxp : ndimyp;
	const int ndimx = (int) pow((double)2, ndimp);
	const int ndimy = (int) pow((double)2, ndimp);
	const int ixoffset = (ndimx - imgx) / 2;
	const int iyoffset = (ndimy - imgy) / 2;
	const double dSmoothing = 2;

	const int istep = 5;
	const int iAxisOmit = 1;
	CCmplx* cPixel = NULL;
	struct CGZD_RESOLN_LIST* psResolnList = NULL;
	int iMaxResolnList = (ndimx/istep) * (ndimy/istep) / 2;
	try {
		cPixel = new CCmplx[ndimx * ndimy];
		psResolnList = new struct CGZD_RESOLN_LIST[iMaxResolnList];
	}
	catch (CException* e) {
		if (cPixel) delete [] cPixel;
		if (psResolnList) delete [] psResolnList;
		e->Delete(); return;
	}
	for (int i=0; i<iMaxResolnList; i++) {
		psResolnList[i].dDistance2 = 0;
		psResolnList[i].dLogMod[0] = 0; psResolnList[i].dLogMod[1] = 0; psResolnList[i].dLogMod[2] = 0;
	}
	const double pxd = (pixDiv > 0) ? pixDiv : 1;
	const int nmax = bColor ? 3 : 1;
	const double csa = cos(iangle * DEG_TO_RAD);
	const double sna = sin(iangle * DEG_TO_RAD);
	int iList;
	for (int n=0; n<nmax; n++) {
		for (int i=0; i<ndimx*ndimy; i++) {cPixel[i].Reset(); cPixel[i].im = -1;}
		double davg = 0;
		for (int i=0; i<imgx; i++) {
			for (int j=0; j<imgy; j++) {
				int ipix = 0;
				if (bEnableBox) {
					const int kxdim = this->ixdim;
					const int kydim = this->iydim;
					int ix = i - imgx / 2;
					int iy = j - imgy / 2;
					double gx = ix * csa - iy * sna + ixcent;
					double gy = ix * sna + iy * csa + iycent;
					if ((gx >= 0)&&(gy >= 0)&&(gx <= kxdim-1)&&(gy <= kydim-1)) {
						int igx = (int)gx;
						int igy = (int)gy;
						double dx = gx - igx;
						double dy = gy - igy;
						if ((fabs(dx) < 0.00001)&&(fabs(dy) < 0.00001)) {//090727
							ipix = pPixel[igx + igy * kxdim];
							if (bColor) ipix = ((ipix >> (n*8)) & 0xff);
						} else {
							//interpolated intensity
							if (igx == kxdim-1) {igx--; dx = 1;}
							if (igy == kydim-1) {igy--; dy = 1;}
							//plane determined by least square fitting
							int is0 = pPixel[igx + igy * kxdim];
							int is1 = pPixel[(igx + 1) + igy * kxdim];
							int is2 = pPixel[igx + (igy + 1) * kxdim];
							int is3 = pPixel[(igx + 1) + (igy + 1) * kxdim];
							if (bColor) {//170503
								is0 = ((is0 >> (n*8)) & 0xff);
								is1 = ((is1 >> (n*8)) & 0xff);
								is2 = ((is2 >> (n*8)) & 0xff);
								is3 = ((is3 >> (n*8)) & 0xff);
							}
							double ap = 0.5 * (is1 + is3 - is0 - is2);
							double bp = 0.5 * (is2 + is3 - is0 - is1);
							double cp = 0.25 * ((is0 + is1 + is2 + is3) - 2 * (ap + bp));
							ipix = (int)(ap * dx + bp * dy + cp);
						}
					}
				} else {
					const int idx = i+j*imgx;
					ipix = pPixel[idx];
					if (bColor) ipix = ((ipix >> (n*8)) & 0xff);
				}
				//if (bColor) cPixel[i+j*ndimx].re = (float)((ipix >> (n*8)) & 0xff);
				cPixel[i + ixoffset + (j + iyoffset) * ndimx].re = (float)(ipix);
				cPixel[i + ixoffset + (j + iyoffset) * ndimx].im = 0;
				davg += (float)(ipix);
			}
		}
		//set margin to the average: this is for minimizing streaks along the axes
		davg /= (imgx * imgy);
		for (int i=0; i<ndimx; i++) {
			for (int j=0; j<ndimy; j++) {
				if (cPixel[i+j*ndimx].im != 0) {
					cPixel[i+j*ndimx].re = (TCmpElmnt)davg;
				}
				cPixel[i+j*ndimx].im = 0;
			}
		}
		//smooth each end using gaussian to minimize the truncation error
		const int iwidth = (int)(ndimx * 0.01 * dSmoothing);
		for (int i=0; i<iwidth; i++) {
			for (int j=0; j<ndimy; j++) {
				int idist2 = (iwidth - i) * (iwidth - i);
				if (j < iwidth) idist2 += (iwidth - j) * (iwidth - j);
				else if (j >= ndimy-iwidth) idist2 += (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
				const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
				cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
			}
		}
		for (int i=ndimx-iwidth; i<ndimx; i++) {
			for (int j=0; j<ndimy; j++) {
				int idist2 = (ndimx-iwidth - i + 1) * (ndimx-iwidth - i + 1);
				if (j < iwidth) idist2 += (iwidth - j) * (iwidth - j);
				else if (j >= ndimy-iwidth) idist2 += (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
				const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
				cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
			}
		}
		for (int j=0; j<iwidth; j++) {
			for (int i=iwidth; i<ndimx-iwidth; i++) {
				int idist2 = (iwidth - j) * (iwidth - j);
				const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
				cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
			}
		}
		for (int j=ndimy-iwidth; j<ndimy; j++) {
			for (int i=iwidth; i<ndimx-iwidth; i++) {
				int idist2 = (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
				const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
				cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
			}
		}

		CFft fft2;
		fft2.Init2(ndimp, -1, ndimp, -1);
		//160727 fft2.FFT2Rev(cPixel);//peak at the origin
		fft2.FFT2(cPixel);

		const int ixorg = ndimx/2 - 1;
		const int iyorg = ndimy/2 - 1;
		iList = 0;
		bool bBreak = false;
		for (int ix=iAxisOmit+istep/2; ix<ndimx/2; ix+=istep) {
			for (int iy=iAxisOmit+istep/2; iy<ndimy/2; iy+=istep) {
				double dx = ix - (istep/2) + istep * 0.5; 
				double dy = iy - (istep/2) + istep * 0.5;
				double dDist2 = (dx * dx)/(ndimx * ndimx) + (dy * dy)/(ndimy * ndimy);
				//160826 if (dDist2 > 0.1) continue;
				if (dDist2 > 0.25) continue;
				double dsum = 0;
				int icount = 0;
				for (int jx=-1; jx<=1; jx+=2) {
					//jy is fixed as +1. jy=-1 will give the same resutls.
					for (int hx=0; hx<istep; hx++) {
						for (int hy=0; hy<istep; hy++) {
							const int kx = jx * (ix - (istep/2) + hx) + ixorg;
							if ((kx < 0)||(kx >= ndimx)) continue;
							const int ky = (iy - (istep/2) + hy) + iyorg;
							if ((ky < 0)||(ky >= ndimy)) continue;
							//160727 dsum += cPixel[kx + ky * ndimx].Modulus2() / (ndimx * ndimy);
							dsum += cPixel[kx + ky * ndimx].Modulus2() * (ndimx * ndimy);//the last multiplying is to keep consistency with the previous version.
							icount++;
						}
					}
					if (icount) dsum = log(dsum / icount);
					if (n == 0) psResolnList[iList].dDistance2 = dDist2;
					psResolnList[iList].dLogMod[n] = dsum;
					iList++;
					if (iList >= iMaxResolnList) {bBreak = true; break;}
					//CString msg;
					//msg.Format("%.10f, %.4f\n", (dx * dx)/(ndimx * ndimx) + (dy * dy)/(ndimy * ndimy), dsum);
					//flog.WriteString(msg);
				}//(int jx=-1; jx<=1; jx+=2)
				if (bBreak) break;
			}//(iy<ndimy/2)
			if (bBreak) break;
		}//(ix<ndimx/2)
	}//n
	qsort( (void *)psResolnList, (size_t)iList, sizeof(struct CGZD_RESOLN_LIST), ResolnListCompare );

	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	if (GetDataPath().GetLength()) {
		_stprintf_s(path_buffer, _MAX_PATH, this->GetDataPath() + "resolnPlot.csv");
	} else {
		_stprintf_s(path_buffer, _MAX_PATH, this->GetPathName());
		_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		CString sFnm = fnm;
		_stprintf_s(fnm, _MAX_FNAME, sFnm + "resolnPlot");
		_stprintf_s(ext, _MAX_EXT, ".csv");
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
	}
	CDlgResolnPlot dlg;
	dlg.m_sFileName = path_buffer;
	dlg.m_iMaxResolnList = iMaxResolnList;
	dlg.m_psResolnList = psResolnList;
	dlg.m_bColor = bColor;
	dlg.m_piDim[0] = imgx; dlg.m_piDim[1] = imgy; dlg.m_piDim[2] = ndimx; dlg.m_piDim[3] = ndimy;
	if (parentDoc) {
		if (parentDoc->dlgReconst.m_hWnd) dlg.m_dPixelWidth = parentDoc->dlgReconst.m_PixelWidth;
	} else {
		if (dlgReconst.m_hWnd) dlg.m_dPixelWidth = dlgReconst.m_PixelWidth;
	}
	dlg.DoModal();
	if (cPixel) delete [] cPixel;
	if (psResolnList) delete [] psResolnList;
	return;
}


void CGazoDoc::OnUpdateTomoFourier(CCmdUI *pCmdUI)
{
	//if (bDebug) pCmdUI->Enable(true);
	//else pCmdUI->Enable(false);
}

void CGazoDoc::OnTomoFourier()
{
	CDlgGeneral dlg;
	dlg.m_sTitle = "Fourier transform";
	dlg.m_sCaption1 = "Norm=0/log(norm)=1";
	dlg.m_sInput1 = "1";
	dlg.m_sCaption2 = "Full scale";
	dlg.m_sInput2.Format("%d", 1024 * 1024 * 8 - 1);
	dlg.m_sCaption3 = "Edge smoothing(%)";
	dlg.m_sInput3 = "2";
	if (dlg.DoModal() == IDCANCEL) return;
	const int iOutputLog = atoi(dlg.m_sInput1);
	if ((iOutputLog != 0)&&(iOutputLog != 1)) {AfxMessageBox("Invalid option"); return;}
	const int iFullScale = atoi(dlg.m_sInput2);
	const double dSmoothing = atof(dlg.m_sInput3);
	if ((dSmoothing < 0)&&(dSmoothing > 50)) {AfxMessageBox("Invalid smoothing width"); return;}

	const int imgx = this->ixdim;//refq->iXdim;
	const int imgy = this->iydim;//refq->iYdim;
	const int ndimxp = (int)ceil((log((double)imgx) / LOG2));
	const int ndimyp = (int)ceil((log((double)imgy) / LOG2));
	const int ndimp = (ndimxp > ndimyp) ? ndimxp : ndimyp;
	const int ndimx = (int) pow((double)2, ndimp);
	const int ndimy = (int) pow((double)2, ndimp);
	const int ixoffset = (ndimx - imgx) / 2;
	const int iyoffset = (ndimy - imgy) / 2;

	CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
	CGazoDoc* pcd = pcv->GetDocument();
	CCmplx* cPixel;
	try {
		cPixel = new CCmplx[ndimx * ndimy];
		pcd->pPixel = new int[ndimx * ndimy];
	}
	catch (CException* e) {
		if (cPixel) delete [] cPixel;
		if (pcd->pPixel) {delete [] pcd->pPixel; pcd->pPixel = NULL;}
		e->Delete();
		return;
	}
	pcd->maxPixel = ndimx * ndimy;
	pcd->ixdim = ndimx;
	pcd->iydim = ndimy;
	pcd->parentDoc = this;
	pcd->SetModifiedFlag(TRUE);
	pcd->pixBase = pixBase;
	pcd->pixDiv = pixDiv;
	pcd->fCenter = fCenter;
	pcd->SetTitle("Calculating..");
	pcd->bColor = bColor;

	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	if (pf) pf->m_wndStatusBar.SetPaneText(0, "Calculating..");

	const double pxd = (pixDiv > 0) ? pixDiv : 1;
	const int kmax = bColor ? 3 : 1;
	double dClrMax = 0, dClrMin = 255;
	for (int k=0; k<kmax; k++) {
		for (int i=0; i<ndimx*ndimy; i++) {cPixel[i].Reset(); cPixel[i].im = -1;}
		double davg = 0;
		for (int i=0; i<imgx; i++) {
			for (int j=0; j<imgy; j++) {
				const int idx = i+j*imgx;
				//cPixel[i+j*ndimx].re = (float)(pPixel[idx]);
				float fpix = (bColor) ? ((pPixel[idx] >> (k*8)) & 0xff) : (float)(pPixel[idx] / pxd + pixBase);
				cPixel[i + ixoffset + (j + iyoffset) * ndimx].re = fpix;
				cPixel[i + ixoffset + (j + iyoffset) * ndimx].im = 0;
				davg += fpix;
			}
		}
		//set margin to the average: this is for minimizing streaks along the axes
		davg /= (imgx * imgy);
		for (int i=0; i<ndimx; i++) {
			for (int j=0; j<ndimy; j++) {
				if (cPixel[i+j*ndimx].im != 0) {
					cPixel[i+j*ndimx].re = (TCmpElmnt)davg;
				}
				cPixel[i+j*ndimx].im = 0;
			}
		}
		//smooth each end using gaussian to minimize the truncation error
		const int iwidth = (int)(ndimx * 0.01 * dSmoothing);
		if (iwidth > 0) {
			for (int i=0; i<iwidth; i++) {
				for (int j=0; j<ndimy; j++) {
					int idist2 = (iwidth - i) * (iwidth - i);
					if (j < iwidth) idist2 += (iwidth - j) * (iwidth - j);
					else if (j >= ndimy-iwidth) idist2 += (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
					const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
					cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
				}
			}
			for (int i=ndimx-iwidth; i<ndimx; i++) {
				for (int j=0; j<ndimy; j++) {
					int idist2 = (ndimx-iwidth - i + 1) * (ndimx-iwidth - i + 1);
					if (j < iwidth) idist2 += (iwidth - j) * (iwidth - j);
					else if (j >= ndimy-iwidth) idist2 += (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
					const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
					cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
				}
			}
			for (int j=0; j<iwidth; j++) {
				for (int i=iwidth; i<ndimx-iwidth; i++) {
					int idist2 = (iwidth - j) * (iwidth - j);
					const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
					cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
				}
			}
			for (int j=ndimy-iwidth; j<ndimy; j++) {
				for (int i=iwidth; i<ndimx-iwidth; i++) {
					int idist2 = (ndimy-iwidth - j + 1) * (ndimy-iwidth - j + 1);
					const double blur = exp(-3. * idist2 / (iwidth * iwidth));//smooth the truncation down to its 5%
					cPixel[i+j*ndimx].re = (TCmpElmnt)((cPixel[i+j*ndimx].re - davg) * blur + davg);
				}
			}
		}//(iwidth > 0)

		CFft fft2;
		//fft2.Init2(ndimxp, 0, ndimyp, 0);
		fft2.Init2(ndimp, -1, ndimp, -1);
		//150528 fft2.FFT2Rev(cPixel);
		fft2.FFT2(cPixel);

		if (dClrMax == 0) {
			if (!bColor) dClrMin = SHRT_MAX;
			for (int i=0; i<ndimx; i++) {
				for (int j=0; j<ndimy; j++) {
					double dpix = cPixel[i+j*ndimx].Modulus();
					if (iOutputLog) dpix = (dpix <= 0) ? 0 : log(dpix);
					dClrMax = (dpix > dClrMax) ? dpix : dClrMax;
					dClrMin = (dpix < dClrMin) ? dpix : dClrMin;
				}
			}
			if (dClrMax == dClrMin) dClrMin--;
			if (!bColor) {
				pcd->pixBase = (float)dClrMin;
				pcd->pixDiv = (float)(iFullScale / (dClrMax - dClrMin));
			}
		}
		for (int i=0; i<ndimx; i++) {
			for (int j=0; j<ndimy; j++) {
				double dpix = cPixel[i+j*ndimx].Modulus();
				if (iOutputLog) dpix = (dpix <= 0) ? 0 : log(dpix);
				if (bColor) {
					dpix = (dpix - dClrMin) / (dClrMax - dClrMin) * 255;
					int ipix = (dpix > 255) ? 255 : ((dpix < 0) ? 0 : (int)(dpix));
					pcd->pPixel[i+j*ndimx] |= (ipix << (k*8));
				} else {
					unsigned __int64 uiPix = (unsigned __int64)((dpix - pcd->pixBase) * pcd->pixDiv);
					pcd->pPixel[i+j*ndimx] = (int)((uiPix < INT_MAX) ? uiPix : INT_MAX);
				}
				//150207 pcd->pPixel[i+j*ndimx] = (int)( cPixel[i+j*ndimx].re );
			}
		}
	}
	pcd->UpdateView(/*bInit=*/true);
	CString title;
	if (iOutputLog) title.Format("log(FT modulus) of %s", GetTitle());
	else title.Format("FT modulus of %s", GetTitle());
	pcd->SetTitle(title);
	pcd->dataPath = dataPath;
	pcd->dataPrefix = dataPrefix;
	//
	if (cPixel) delete [] cPixel;
	if (pf) pf->m_wndStatusBar.SetPaneText(0, "Finished");
	return;
}

void CGazoDoc::OnTomographyGaussianconvolution()
{
	CDlgGeneral dlg;
	dlg.m_sTitle = "PSF convolution";
	dlg.m_sCaption1 = "Sigma or radius";
	dlg.m_sInput1 = "1.699";
	dlg.m_sCaption2 = "Cutoff limit";
	dlg.m_sInput2 = "0.01";
	dlg.m_sCaption3 = "Gauss0/Circ1/Sqr2";
	dlg.m_sInput3 = "0";
	if (dlg.DoModal() == IDCANCEL) return;
	double sig = atof(dlg.m_sInput1);
	if (sig <= 0) {AfxMessageBox("Invalid sigma"); return;}
	double limit = atof(dlg.m_sInput2);
	if (limit <= 0.001) {AfxMessageBox("Too small cutoff"); return;}
	const int ifunc = atoi(dlg.m_sInput3);
	if ((ifunc < 0)||(ifunc > 2)) {AfxMessageBox("Invalid func selection"); return;}

	const double sig2 = sig * sig;
	int* iPixel1;
	const int ixydim = ixdim * iydim;
	try {iPixel1 = new int[ixydim];}
	catch (CException* e) {e->Delete(); return;}

	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	if (pf) pf->m_wndStatusBar.SetPaneText(0, "Calculating..");

	if (ifunc == 0) {//gaussian
		const double r2limit = -2.0 * sig2 * log(limit);
		const int irad = (int)(sqrt(r2limit));
		for (int i=0; i<ixdim; i++) {
			for (int j=0; j<iydim; j++) {
				double dIntens = 0, dIntens1 = 0, dIntens2 = 0;
				double bsum = 0;
				for (int m=-irad; m<=irad; m++) {
					for (int n=-irad; n<=irad; n++) {
						int im = i + m; int jn = j + n;
						if ((im < 0)||(im >= ixdim)||(jn < 0)||(jn >= iydim)) continue;
						const double blur = exp(-0.5 * (m * m + n * n) / sig2);
						if (blur < limit) continue;
						if (bColor) {
							int ip = pPixel[im + jn * ixdim];
							dIntens += blur * (ip & 0xff);
							dIntens1 += blur * ((ip >> 8) & 0xff);
							dIntens2 += blur * ((ip >> 16) & 0xff);
						} else {
							dIntens += blur * pPixel[im + jn * ixdim];
						}
						bsum += blur;
						//pPixel[im + jn * ixdim] += (int)(ipeak * blur);
					}
				}
				const int idx = i+j*ixdim;
				if (bsum > 0) {
					if (bColor) {
						iPixel1[idx] = (int)(dIntens / bsum) | ((int)(dIntens1 / bsum) << 8) |
										((int)(dIntens2 /bsum) << 16);
					} else {
						iPixel1[idx] = (int)(dIntens / bsum);
					}
				}
			}
		}
	} else if ((ifunc == 1)||(ifunc == 2)) {//circle or square aperture
		const int irad = (int)ceil(sig);
		const double sig2 = sig * sig;
		for (int i=0; i<ixdim; i++) {
			for (int j=0; j<iydim; j++) {
				double dIntens = 0, dIntens1 = 0, dIntens2 = 0;
				int isum = 0;
				for (int m=-irad; m<=irad; m++) {
					for (int n=-irad; n<=irad; n++) {
						if (ifunc == 1) {
							if (m * m + n * n > sig2) continue;
						}
						int im = i + m; int jn = j + n;
						if ((im < 0)||(im >= ixdim)||(jn < 0)||(jn >= iydim)) continue;
						if (bColor) {
							int ip = pPixel[im + jn * ixdim];
							dIntens += (ip & 0xff);
							dIntens1 += ((ip >> 8) & 0xff);
							dIntens2 += ((ip >> 16) & 0xff);
						} else {
							dIntens += pPixel[im + jn * ixdim];
						}
						isum++;
						//pPixel[im + jn * ixdim] += (int)(ipeak * blur);
					}
				}
				const int idx = i+j*ixdim;
				if (isum > 0) {
					if (bColor) {
						iPixel1[idx] = (int)(dIntens / isum) | ((int)(dIntens1 / isum) << 8) |
										((int)(dIntens2 / isum) << 16);
					} else {
						iPixel1[idx] = (int)(dIntens / isum);
					}
				}
			}
		}
	}//if (ifunc == ...)
	for (int i=0; i<ixydim; i++) {pPixel[i] = iPixel1[i];}
	if (iPixel1) delete [] iPixel1;
	if (pf) pf->m_wndStatusBar.SetPaneText(0, "Finished");
	UpdateView();
}

void CGazoDoc::OnAnalysisAddnoise()
{
	CDlgGeneral dlg;
	dlg.m_sTitle = "Add noise";
	dlg.m_sCaption1 = "Noise/int average";
	dlg.m_sInput1 = "0.2";
	//dlg.m_sCaption2 = "Cutoff limit";
	//dlg.m_sInput2 = "0.1";
	if (dlg.DoModal() == IDCANCEL) return;
	const double dNoise = atof(dlg.m_sInput1);
	if (dNoise <= 0) {AfxMessageBox("Invalid sigma"); return;}

	const int ixydim = ixdim * iydim;
	double dAverage = 0;
	srand(1);
	if (bColor) {
		for (int i=0; i<ixydim; i++) {
			int ip = pPixel[i];
			dAverage += ((ip & 0xff) + ((ip >> 8) & 0xff) + ((ip >> 16) & 0xff)) / 3;
		}
		dAverage /= ixydim;
		for (int i=0; i<ixydim; i++) {
			int iResult = 0;
			for (int j=0; j<3; j++) {
				int ip = (pPixel[i] >> (j*8)) & 0xff;
				ip += (int)(dAverage * dNoise * (rand()-RAND_MAX/2) / (RAND_MAX/2));
				if (ip < 0) ip = 0; else if (ip > 255) ip = 255;
				iResult |= (ip << (j*8));
			}
			pPixel[i] = iResult;
		}
	} else {
		for (int i=0; i<ixydim; i++) {dAverage += pPixel[i];}
		dAverage /= ixydim;
		for (int i=0; i<ixydim; i++) {
			pPixel[i] += (int)(dAverage * dNoise * (rand()-RAND_MAX/2) / (RAND_MAX/2));
			if (pPixel[i] < 0) pPixel[i] = 0;
			else if (pPixel[i] > 65535) pPixel[i] = 65535;
		}
	}
	UpdateView();
}

void CGazoDoc::OnUpdateAnalysisAddnoise(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
}

void CGazoDoc::OnAnalysisEnlarge()
{
	CDlgGeneral dlg;
	dlg.m_sTitle = "Enlarge/reduce image";
	dlg.m_sCaption1 = "Scale";
	dlg.m_sInput1 = "2.0";
	if (dlg.DoModal() == IDCANCEL) return;
	const double dscale = atof(dlg.m_sInput1);
	if (dscale <= 0.01) {AfxMessageBox("Invalid scale"); return;}
	const int imgx = this->ixdim;//refq->iXdim;
	const int imgy = this->iydim;//refq->iYdim;
	const int ndimx = (int)(imgx * dscale);
	const int ndimy = (int)(imgy * dscale);

	CGazoView* pcv = (CGazoView*)( ((CGazoApp*)AfxGetApp())->RequestNew() );
	CGazoDoc* pcd = pcv->GetDocument();
	try {
		pcd->pPixel = new int[ndimx * ndimy];
	}
	catch (CException* e) {
		if (pcd->pPixel) {delete [] pcd->pPixel; pcd->pPixel = NULL;}
		e->Delete();
		return;
	}
	pcd->maxPixel = ndimx * ndimy;
	pcd->ixdim = ndimx;
	pcd->iydim = ndimy;
	pcd->parentDoc = this;
	pcd->SetModifiedFlag(TRUE);
	pcd->pixBase = pixBase;
	pcd->pixDiv = pixDiv;
	pcd->fCenter = fCenter;
	pcd->SetTitle("Calculating..");
	pcd->bColor = bColor;

	const double pxd = (pixDiv > 0) ? pixDiv : 1;
	const int kmax = bColor ? 3 : 1;
	double dClrMax = 0, dClrMin = 255;
	for (int k=0; k<kmax; k++) {
		if (dscale >= 1) {
			for (int i=0; i<ndimx; i++) {
				for (int j=0; j<ndimy; j++) {
					const int iorg = (int)(i / dscale);
					const int jorg = (int)(j / dscale);
					const int idx = iorg + jorg * imgx;
					if (bColor) {
						int ipix = (pPixel[idx] >> (k*8)) & 0xff;
						pcd->pPixel[i+j*ndimx] |= (ipix << (k*8));
					} else {
						pcd->pPixel[i+j*ndimx] = pPixel[idx];
					}
				}
			}
		} else {
			for (int i=0; i<ndimx; i++) {
				for (int j=0; j<ndimy; j++) {
					const int iorg = (int)(i / dscale);
					const int jorg = (int)(j / dscale);
					int iend = (int)ceil((i+1) / dscale);
					int jend = (int)ceil((j+1) / dscale);
					iend = (iend > imgx) ? imgx : iend;
					jend = (jend > imgy) ? imgy : jend;
					//CString line; line.Format("%d %d %d %d", iorg, iend, jorg, jend); AfxMessageBox(line);
					int icount = 0;
					__int64 llsum = 0;
					for (int ix=iorg; ix<iend; ix++) {
						for (int iy=jorg; iy<jend; iy++) {
							const int idx = ix + iy * imgx;
							if (bColor) {
								llsum += (pPixel[idx] >> (k*8)) & 0xff;
							} else {
								llsum += pPixel[idx];
							}
							icount++;
						}
					}
					if (icount) llsum /= icount;
					if (bColor) pcd->pPixel[i+j*ndimx] |= ((llsum & 0xff) << (k*8));
					else pcd->pPixel[i+j*ndimx] = (int)llsum;
				}
			}
		}
	}
	pcd->UpdateView(/*bInit=*/true);
	CString title;
	title.Format("Scale-by-%.2f of %s", dscale, GetTitle());
	pcd->SetTitle(title);
	pcd->dataPath = dataPath;
	pcd->dataPrefix = dataPrefix;
	//
	return;
}

void CGazoDoc::OnAnalysisRadialprofile()
{
	const int imgx = this->ixdim;//refq->iXdim;
	const int imgy = this->iydim;//refq->iYdim;
	CString line;

	CDlgGeneral dlg;
	dlg.m_sTitle = "Radial profile";
	dlg.m_sCaption1 = "center X";
	dlg.m_sInput1.Format("%d", imgx / 2);
	dlg.m_sCaption2 = "center Y";
	dlg.m_sInput2.Format("%d", imgy / 2);
	dlg.m_sCaption3 = "Max radius";
	dlg.m_sInput3.Format("%d", (imgx < imgy) ? (imgx / 2) : (imgy / 2));
	dlg.m_sCaption4 = "Binning";
	dlg.m_sInput4 = "4";
	if (dlg.DoModal() == IDCANCEL) return;
	const int icenterx = atoi(dlg.m_sInput1);
	const int icentery = atoi(dlg.m_sInput2);
	const double dradmax = atof(dlg.m_sInput3);
	const int ibin = atoi(dlg.m_sInput4);
	if (ibin < 1) {AfxMessageBox("Invalid binning"); return;}
	if (dradmax < 0) {AfxMessageBox("Invalid maximum radius"); return;}


	static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = "txt";
	CString sFileName;
	CFileDialog fileDlg(TRUE, defaultExt, sFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) {
		AfxMessageBox("No file speciifed");
		return;
	}
	sFileName = fileDlg.GetPathName();
	FILE* fdata = NULL;
	errno_t errn = fopen_s(&fdata, sFileName, "wt");
	if (fdata == NULL) return;
	CStdioFile stdioData(fdata);
	for (int ix=0; ix<imgx; ix+=ibin) {
		for (int iy=0; iy<imgy; iy+=ibin) {
			double drx = ix + ibin * 0.5 - icenterx;
			double dry = iy + ibin * 0.5 - icentery;
			double drad = sqrt(drx * drx + dry * dry);
			if (drad > dradmax) continue;
			double dsum = 0;
			int icount = 0;
			for (int jx=0; jx<ibin; jx++) {
				if (ix + jx >= imgx) continue;
				for (int jy=0; jy<ibin; jy++) {
					if (iy + jy >= imgy) continue;
					int idx = (ix + jx) + (iy + jy) * imgx;
					dsum += pPixel[idx];
					icount++;
				}
			}
			if (icount == 0) continue;
			dsum /= icount;
			line.Format("%f %f %f\r\n", drad, dsum, (dsum / pixDiv + pixBase));
			stdioData.WriteString(line);
		}
	}
	fclose(fdata);
}

void CGazoDoc::OnMenuOverlay()
{
	/*
	TReal x1 = 1, y1 = 2;
	TReal x2 = 103, y2 = 4;
	TReal x3 = 5, y3 = 106;
	TReal x4 = 107, y4 = 108;
	TReal u1 = -15, v1 = 11;
	TReal u2 = 116, v2 = -12;
	TReal u3 = 17, v3 = 93;
	TReal u4 = 88, v4 = 114;
	//TReal u1 = 0, v1 = 0;
	//TReal u2 = 100, v2 = 0;
	//TReal u3 = 0, v3 = 100;
	//TReal u4 = 100, v4 = 100;
	//
	TReal prPoint[16];
	prPoint[0] = x1; prPoint[1] = y1;
	prPoint[2] = x2; prPoint[3] = y2;
	prPoint[4] = x3; prPoint[5] = y3;
	prPoint[6] = x4; prPoint[7] = y4;
	prPoint[8] = u1; prPoint[9] = v1;
	prPoint[10] = u2; prPoint[11] = v2;
	prPoint[12] = u3; prPoint[13] = v3;
	prPoint[14] = u4; prPoint[15] = v4;
	TReal prCoeff[8];
	TErr err = ProjTransformGetCoeff(prPoint, prCoeff);
	TReal a = prCoeff[0];
	TReal b = prCoeff[1];
	TReal c = prCoeff[2];
	TReal d = prCoeff[3];
	TReal e = prCoeff[4];
	TReal f = prCoeff[5];
	TReal g = prCoeff[6];
	TReal h = prCoeff[7];
	//
	TReal pu1 = (a*x1 + b*y1 + c) / (g*x1 + h*y1 + 1);
	TReal pv1 = (d*x1 + e*y1 + f) / (g*x1 + h*y1 + 1);
	TReal pu2 = (a*x2 + b*y2 + c) / (g*x2 + h*y2 + 1);
	TReal pv2 = (d*x2 + e*y2 + f) / (g*x2 + h*y2 + 1);
	TReal pu3 = (a*x3 + b*y3 + c) / (g*x3 + h*y3 + 1);
	TReal pv3 = (d*x3 + e*y3 + f) / (g*x3 + h*y3 + 1);
	TReal pu4 = (a*x4 + b*y4 + c) / (g*x4 + h*y4 + 1);
	TReal pv4 = (d*x4 + e*y4 + f) / (g*x4 + h*y4 + 1);
	CString msg = "", line;
	//line.Format("%f %f\r\n", g, h); msg += line;
	line.Format("(%f %f) (%f %f)\r\n", u1, v1, pu1, pv1); msg += line;
	line.Format("(%f %f) (%f %f)\r\n", u2, v2, pu2, pv2); msg += line;
	line.Format("(%f %f) (%f %f)\r\n", u3, v3, pu3, pv3); msg += line;
	line.Format("(%f %f) (%f %f)\r\n", u4, v4, pu4, pv4); msg += line;
	line.Format("err=%d\r\n", err); msg += line;
	AfxMessageBox(msg);
	return;//151014////
	*/
	//CDlgOverlay dlg;
	//dlg.DoModal();
	//return;
	if (!dlgOverlay.m_hWnd) {
		CString sPath = "Overlay";
		TCHAR* fileList = new TCHAR[MAX_FILE_DIALOG_LIST];
		_tcscpy_s(fileList, MAX_FILE_DIALOG_LIST, "");
		static char BASED_CODE defaultExt[] = ".tif";
		static char BASED_CODE szFilter[] = 
			"rec files (rec*.tif)|rec*.tif|ro files (ro*.tif)|ro*.tif|TIFF files (*.tif)|*.tif||";
		CFileDialog dlg(TRUE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter, NULL);
		dlg.m_ofn.nMaxFile = MAX_FILE_DIALOG_LIST;
		dlg.m_ofn.lpstrFile = (LPTSTR)fileList;
		if (dlg.DoModal() != IDOK) return;
		//
		izOverlay = 0;
		int nFiles = 0;
		TErr err = 0;
		ioydim = 0; ioxdim = 0;
		POSITION pos = dlg.GetStartPosition();
		while (pos) {
			TCHAR path_buffer[_MAX_PATH];
			_stprintf_s(path_buffer, _MAX_PATH, dlg.GetNextPathName(pos));
			TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME];
			TCHAR ext[_MAX_EXT];
			_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
			float fOverlayDiv = 0, fOverlayBase = 0;
			float fOverlayCenter = 0, fOverlayPixelWidth = 0;
			int ix = 0, iy = 0, iOverlayFilter = 0;
			if ((_tcscmp(ext, ".tif") == 0)||(_tcscmp(ext, ".TIF") == 0)) {
				CFile fimg;
				if (!fimg.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite)) {
					AfxMessageBox(path_buffer); err = 19001; break;
				}
				err = ReadTif(&fimg, &(ppOverlay[nFiles]), &(pMaxOverlay[nFiles]), &iy, &ix, 
								&fOverlayDiv, &fOverlayBase, &fOverlayCenter, &iOverlayFilter, &fOverlayPixelWidth);
				if ((ioxdim == 0)||(ioydim == 0)) {
					ioxdim = ix; ioydim = iy;
					sPath = dir;
					sPath.MakeReverse();
					sPath = sPath.Mid(2).SpanExcluding("\\");
					sPath.MakeReverse();
					sPath += "\\"; sPath += fnm;
					//_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, NULL);
					//sPath = path_buffer;
				} else if ((ioxdim != ix)||(ioydim != iy)) {
					AfxMessageBox("Image size unmatch"); err = 19003; break;
				}
				if (err) break;
			} else {
				AfxMessageBox("Not supported"); err = 19002; break;
			}
			nFiles++;
			if (nFiles >= CGAZODOC_MAXOVERLAY) break;
		}
		if (fileList) delete [] fileList;
		if (err) {AfxMessageBox("Error on file loading"); return;}
		izOverlay = nFiles;
		dlgOverlay.m_rx1 = 0; dlgOverlay.m_ry1 = 0;
		dlgOverlay.m_rx2 = ioxdim; dlgOverlay.m_ry2 = 0;
		dlgOverlay.m_rx3 = 0; dlgOverlay.m_ry3 = ioydim;
		dlgOverlay.m_rx4 = ioxdim; dlgOverlay.m_ry4 = ioydim;
		dlgOverlay.m_ru1 = 0; dlgOverlay.m_rv1 = 0; dlgOverlay.m_rw1 = nFiles / 2;
		dlgOverlay.m_ru2 = ioxdim; dlgOverlay.m_rv2 = 0; dlgOverlay.m_rw2 = nFiles / 2;
		dlgOverlay.m_ru3 = 0; dlgOverlay.m_rv3 = ioydim; dlgOverlay.m_rw3 = nFiles / 2;
		dlgOverlay.m_ru4 = ioxdim; dlgOverlay.m_rv4 = ioydim; dlgOverlay.m_rw4 = nFiles / 2;
		dlgOverlay.Create(IDD_OVERLAY);
		CString line; line.Format("%s - %d frames", sPath, izOverlay);
		dlgOverlay.SetWindowText(line);
	}
	if (dlgOverlay.IsWindowVisible()) dlgOverlay.SetForegroundWindow();
	else dlgOverlay.ShowWindow(SW_SHOW);
	dlgOverlay.UpdateGazoview();
	UpdateView();
}

void CGazoDoc::OnUpdateMenuOverlay(CCmdUI *pCmdUI)
{
	if (bColor) pCmdUI->Enable(false);
	else pCmdUI->Enable(true);
}



void CGazoDoc::OnAnalysisSubtract()
{//190708
	CDocTemplate* pDocTemplate = GetDocTemplate();
	if (!pDocTemplate) return;
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	if (pos == NULL) return;
	CGazoDoc* pd1 = NULL;
	CString sdoc = "";
	while (pd1 = (CGazoDoc*)pDocTemplate->GetNextDoc(pos)) {
		if (pd1 != this) sdoc += pd1->GetTitle() + "\r\n";
		if (pos == NULL) break;
	}
	if (sdoc.IsEmpty()) { AfxMessageBox("Open image first"); return; }
	CDlgFrameList dlg;
	dlg.m_sDocList = sdoc;
	if (dlg.DoModal() == IDCANCEL) return;
	if (dlg.iDocPos < 0) { AfxMessageBox("No image selected"); return; }
	pos = pDocTemplate->GetFirstDocPosition();
	if (pos == NULL) {AfxMessageBox("No images opened"); return;}
	pd1 = NULL;
	int i = 0;
	while (pd1 = (CGazoDoc*)pDocTemplate->GetNextDoc(pos)) {
		if (pd1 != this) {
			if (i == dlg.iDocPos) break;
		}
		if (pos == NULL) break;
		i++;
	}
	if (pd1 == NULL) { AfxMessageBox("No such image found"); return; }
	if ((ixdim != pd1->ixdim)||(iydim != pd1->iydim)) { AfxMessageBox("Image size does not match"); return; }
	if (bColor != pd1->bColor) { AfxMessageBox("Color mode does not match"); return; }
	//prepare window
	CGazoView* pcv = (CGazoView*)(((CGazoApp*)AfxGetApp())->RequestNew());
	CGazoDoc* pcd = pcv->GetDocument();
	try {
		pcd->pPixel = new int[ixdim * iydim];
	}
	catch (CException* e) {
		if (pcd->pPixel) { delete[] pcd->pPixel; pcd->pPixel = NULL; }
		e->Delete();
		return;
	}
	pcd->maxPixel = ixdim * iydim;
	pcd->ixdim = ixdim;
	pcd->iydim = iydim;
	pcd->parentDoc = this;
	pcd->SetModifiedFlag(TRUE);
	pcd->pixBase = pixBase;
	pcd->pixDiv = pixDiv;
	pcd->fCenter = fCenter;
	pcd->SetTitle("Calculating..");
	pcd->bColor = bColor;
	//subtract
	const int kmax = bColor ? 3 : 1;
	for (int k = 0; k < kmax; k++) {
		for (int i = 0; i < ixdim; i++) {
			for (int j = 0; j < iydim; j++) {
				if (bColor) {
					int ipix0 = (pPixel[i + j * ixdim] >> (k*8)) & 0xff;
					int ipix1 = (pd1->pPixel[i + j * ixdim] >> (k * 8)) & 0xff;
					pcd->pPixel[i + j * ixdim] |= ((ipix0 - ipix1) << (k * 8));
				} else {
					pcd->pPixel[i + j * ixdim] = pPixel[i + j * ixdim] - pd1->pPixel[i + j * ixdim];
				}
			}
		}
	}
	//show
	pcd->UpdateView(/*bInit=*/true);
	CString title;
	title.Format("{%s} - {%s}", GetTitle(), pd1->GetTitle());
	pcd->SetTitle(title);
	pcd->dataPath = dataPath;
	pcd->dataPrefix = dataPrefix;
	//
	return;
}


void CGazoDoc::OnUpdateAnalysisSubtract(CCmdUI *pCmdUI)
{//190708
	// TODO:ここにコマンド更新 UI ハンドラー コードを追加します。
}
