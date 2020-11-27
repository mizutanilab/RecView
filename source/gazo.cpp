// gazo.cpp : アプリケーション用クラスの機能定義を行います。
//

#include "stdafx.h"
#include "gazo.h"

#include <intrin.h>
#include <sys\timeb.h> //_timeb, _ftime
#include <process.h> //_beginthread
#include "MainFrm.h"
#include "ChildFrm.h"
#include "gazoDoc.h"
#include "gazoView.h"
#include "cerror.h"
#include "DlgLsqfit.h"
#include "DlgQueue.h"
#include "cxyz.h"//181213
//#include <cutil_inline.h>
#include "DlgMessage.h"

#include "cudaReconst.h"
#include "clReconst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGazoApp

BEGIN_MESSAGE_MAP(CGazoApp, CWinApp)
	//{{AFX_MSG_MAP(CGazoApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_QUEUE, OnUpdateTomoQueue)
	ON_COMMAND(IDM_TOMO_QUEUE, OnTomoQueue)
	//}}AFX_MSG_MAP
	// 標準のファイル基本ドキュメント コマンド
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN, CGazoApp::OnFileOpen)
	// 標準の印刷セットアップ コマンド
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(IDM_TOMO_PROPERTY, &CGazoApp::OnTomoProperty)
	ON_UPDATE_COMMAND_UI(IDM_TOMO_PROPERTY, &CGazoApp::OnUpdateTomoProperty)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW, &CGazoApp::OnUpdateWindowNew)
	ON_COMMAND(IDM_FILE_PREPFILES, &CGazoApp::OnFilePrepfiles)
	ON_COMMAND(IDM_TOMO_LSQFIT, &CGazoApp::OnTomoLsqfit)
	ON_COMMAND(IDM_FILE_CLOSEALL, &CGazoApp::OnFileCloseall)
	ON_COMMAND(IDM_TOMO_RENUMFILES, &CGazoApp::OnTomoRenumfiles)
	ON_COMMAND(IDM_VIEW_ERROR, &CGazoApp::OnViewError)
	ON_COMMAND(IDM_FILE_SAVEQUEUE, &CGazoApp::OnFileSavequeue)
	ON_COMMAND(IDM_FILE_LOADQUEUE, &CGazoApp::OnFileLoadqueue)
	ON_COMMAND(ID_APP_EXIT, &CGazoApp::OnAppExit)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_BOXAXISLABEL, &CGazoApp::OnUpdateViewBoxaxislabel)
	//ON_COMMAND(IDM_VIEW_BOXAXISLABEL, &CGazoApp::OnViewBoxaxislabel)
	ON_COMMAND(IDM_VIEW_DRAGSCROLL, &CGazoApp::OnViewDragscroll)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_DRAGSCROLL, &CGazoApp::OnUpdateViewDragscroll)

//	ON_COMMAND(ID_FILE_DIALBOX, &CGazoApp::OnFileDialbox)
//	ON_MESSAGE(WM_DIALBOX, OnDialbox)//161210

ON_COMMAND(IDM_VIEW_WHEELTOGO, &CGazoApp::OnViewWheeltogo)
ON_UPDATE_COMMAND_UI(IDM_VIEW_WHEELTOGO, &CGazoApp::OnUpdateViewWheeltogo)
ON_COMMAND(ID_TOOLBAR_QUEUE, &CGazoApp::OnTomoQueue)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGazoApp クラスの構築

//global params
CError error;
//int iThreadStatus;
//CGazoDoc* pDocThread;

#define CGAZOAPP_CUDA_ARCH 300
CGazoApp::CGazoApp()
 : m_mutex(FALSE, _T("RecView4"), NULL)//131013
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	iStatus = 0;
	sProgVersion = "RecView ";
#ifdef _WIN64
	sProgVersion += "x64";
#endif
#ifndef _WIN64
	sProgVersion += "x86";
#endif
#ifdef CUDAFFT
	sProgVersion += " with CUDA-FFT";
#endif
	sProgVersion += "\nBuild: ";
	sProgVersion += __DATE__;
	sProgVersion += " / ";
	sProgVersion += __TIME__;


	//SYSTEM_INFO si;
	//GetSystemInfo(&si);
	//iAvailableCPU = (int)si.dwNumberOfProcessors * 2;
	int CPUInfo[4];
	int iLogicalProcessorCount = 1;
	bool bSIMD = false;
	bool bAVX2 = false;//181222
	__cpuid(CPUInfo, 0);
	int idmax = CPUInfo[0];
	if (idmax >= 1) {
		__cpuid(CPUInfo, 1);
		//const int iHyperThreading = (CPUInfo[3] & 0x10000000) / 0x10000000;
		iLogicalProcessorCount = (CPUInfo[1] & 0x00ff0000) / 0x00010000;
		//160918	if ( (CPUInfo[3] & 0x00800000) && //MMX
		//		 (CPUInfo[3] & 0x02000000) && //SSE
		//		 (CPUInfo[3] & 0x04000000) && //SSE2
		//		 (CPUInfo[2] & 0x00000001) )  //SSE3
		if ((CPUInfo[3] & 0x00800000) && //MMX
			(CPUInfo[3] & 0x02000000) && //SSE
			(CPUInfo[3] & 0x04000000)) //SSE2
			bSIMD = true;
	}
	if (idmax >= 7) {
#ifdef _MSC_VER
#if _MSC_VER >=1910 //VS2017 or later
		__cpuidex(CPUInfo, 7, 0);
		if (CPUInfo[1] & (1 << 5)) bAVX2 = true;
#endif
#endif
	}
	sCPUname.Empty();
	__cpuid(CPUInfo, 0x80000000);
	if ((CPUInfo[0] & 0x7FFFFFFF) >= 4) {
		for (int k = 2; k <= 4; k++) {
			__cpuid(CPUInfo, 0x80000000 | k);
			for (int j = 0; j < 4; j++) {
				for (int i = 0; i < 4; i++) {
					char cid = CPUInfo[j] & 0xff;
					sCPUname += cid;
					CPUInfo[j] = CPUInfo[j] >> 8;
				}
			}
		}
	}
	sCPUname.TrimLeft();
	//131019===>
	iAvailableCPU = ::GetProcessorCoreCount();
	if (iAvailableCPU <= 0) iAvailableCPU = iLogicalProcessorCount > 1 ? iLogicalProcessorCount : 1;
	//===>131019
	CString msg = "", line;
	if (sCPUname.IsEmpty()) sCPUname = "Unknown x86/x64 processor";
	line.Format("x86/x64 CPU\r\n %s\r\n Number of cores: %d\r\n", sCPUname, iAvailableCPU); 
	msg += line;
	if (bSIMD) msg += " MMX+SSE+SSE2: detected\r\n";
	else msg += " MMX+SSE+SSE2: not detected\r\n";
	if (bAVX2) msg += " AVX2: detected\r\n";
	else msg += " AVX2: not detected\r\n";
	//memory
	MEMORYSTATUSEX memory;
	memory.dwLength = sizeof(memory);
	GlobalMemoryStatusEx(&memory);
	double dmem = (double)(memory.ullTotalPhys >> 20);//memory in Mbytes
	line.Format(" Memory: %.2fGB", dmem / 1024.); msg += line;
	error.Log(1, msg);
	//CPU/GPU scores
	double dCPUscore = 0;
	double dCUDAscore = 0;
	int iCUDAblock = 1024;
	int iCUDAwarp = 32;
	int iCUDAcount = 0;
	sCudaGPUname.Empty();
#ifdef _MSC_VER
	#if _MSC_VER >=1910 //VS2017 or later
		#ifdef _WIN64
			#define GZ_USE_CUDA
		#endif
	#else
		#define GZ_USE_CUDA
	#endif
#else
	#define GZ_USE_CUDA
#endif
#ifdef GZ_USE_CUDA
	//estimate CPU score
	//struct _timeb tstruct; double tm0;
	//_ftime_s(&tstruct);
	//tm0 = tstruct.time + tstruct.millitm * 0.001;
	//double tcpu = 0;
	//int icount = 1, isum = 0, ifp[100];
	//while (tcpu < 0.1) {
	//	isum += icount;
	//	for (double dx = 0; dx < icount; dx += 0.0001) { ifp[isum % 100] += isum; }
	//	_ftime_s(&tstruct);
	//	tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	//	icount *= 2;
	//}
	//dCPUscore = (tcpu * 270000. + cos(ifp[isum % 100])) / isum;//cos(ifp) is a dummy term to force calc.
	//msg.Format(" Estimated score: %.2f sec", dCPUscore);
	//error.Log(1, msg);
	double dCPUclock = 0;
	int ipos = sCPUname.Find("GHz");
	if (ipos >= 0) {
		dCPUclock = atof(sCPUname.Left(ipos).MakeReverse().SpanIncluding("0123456789. ").MakeReverse());
		dCPUscore = (dCPUclock > 0) ? 45. / (dCPUclock * iAvailableCPU) : 0;
		if (dCPUscore > 0) { msg.Format(" Estimated score: %.2f sec", dCPUscore); error.Log(1, msg);}
	}
	//CUDA GPUs (assumes homogeneous GPUs but detects incompatible ones)
	int iCUDAtotalcount = GetCudaDeviceCount(0);
	iCUDAtotalcount &= CUDA_ERROR_DEVICEINFO_MASK;
	char pcDeviceName[256];
	msg.Empty();
	for (int i = 0; i < iCUDAtotalcount; i++) {
		if (GetCudaDeviceName(i, pcDeviceName, 256) == 0) {
			int iMajor = 1, iMinor = 0, iCores = 0, iProcessors = 0, iClock = 0, iMemClock = 0;
			GetCudaDeviceComputingCapability(i, &iMajor, &iMinor);
			GetCudaNumberOfCores(i, &iCores, &iProcessors);
			GetCudaClockRate(i, &iClock, &iMemClock);
			line.Format("GPU%d: %s\r\n Compute capability: %d.%d\r\n Processors x cores: %d (%dx%d)\r\n Core clock: %.3f GHz\r\n Memory clock: %.3f GHz\r\n",
				i, pcDeviceName, iMajor, iMinor, iProcessors * iCores, iProcessors, iCores, iClock / 1000000., iMemClock / 1000000.);
		} else {
			line.Format("GPU%d: Unknown\r\n", i);
		}
		msg += line;
	}
	//The compute capability number is set in the Project-Property-CUDA C/C++ page
	//minimum number for CUDA Tk 10.0 is compute_30 (__CUDA_ARCH__ = 300)
	//though this directive seems "undefined".
	iCUDAcount = GetCudaDeviceCount(CGAZOAPP_CUDA_ARCH);
	if ((iCUDAcount & CUDA_ERROR_DEVICEINFO_MASK) == 0) msg += "NO CUDA DEVICE FOUND\r\n";
	if (iCUDAcount & CUDA_ERROR_INSUFFICIENT_DRIVER) msg += "INSUFFICIENT CUDA DRIVER\r\n";
	else if (iCUDAcount & CUDA_ERROR_DEVICE_GETCOUNT) msg += "ERROR IN DEVICE DETECTION\r\n";
	iCUDAcount &= CUDA_ERROR_DEVICEINFO_MASK;
	//
	int iSumCUDAcores = 0;
	for (int i = 0; i < iCUDAtotalcount; i++) {
		int iMajor = 1, iMinor = 0, iCores = 0, iProcessors = 0;
		pcDeviceName[0] = 0;
		GetCudaDeviceName(i, pcDeviceName, 256);
		if (GetCudaDeviceComputingCapability(i, &iMajor, &iMinor) == CUDA_ERROR_DEVICE_GETPROPERTY) {
			line.Format("ERROR IN DEVICE PROPERTY INQUIRY: GPU%d\r\n", i);
			msg += line;
		} else if (iMajor * 100 + iMinor * 10 < CGAZOAPP_CUDA_ARCH) {
			line.Format("LOW COMPUTE CAPABILITY: GPU%d(%s) CC=%d.%d\r\n", i, pcDeviceName, iMajor, iMinor);
			msg += line;
		} else {
			sCudaGPUname = pcDeviceName;
			if (GetCudaNumberOfCores(i, &iCores, &iProcessors) <= CUDA_ERROR_DEVICEINFO_MASK) iSumCUDAcores += iCores * iProcessors;
			int iblock = GetCudaMaxThreadsPerBlock(i);
			iCUDAblock = (iblock < iCUDAblock) ? iblock : iCUDAblock;
			int iwarp = GetCudaWarpSize(i);
			iCUDAwarp = (iwarp < iCUDAwarp) ? iwarp : iCUDAwarp;
		}
	}
	if (sCudaGPUname.IsEmpty()) sCudaGPUname = "Unknown CUDA GPU";
	dCUDAscore = (iSumCUDAcores > 0) ? 1700. / iSumCUDAcores : 0;
	if (dCUDAscore > 0) { line.Format("Total GPU score: %.2f sec\r\n", dCUDAscore); msg += line; }
	if (!msg.IsEmpty()) error.Log(1, msg);
	//msg.Format("Device:%d Block:%d Warp:%d", iCUDAcount, iCUDAblock, iCUDAwarp); error.Log(1, msg);
#endif
	//AMD GPU
	int iATIcount, iATImaxwork, iATIunitwork;
	CLInitATIstreamDeviceInfo(&iATIcount, &iATImaxwork, &iATIunitwork);//DO NOT call this func twice or more
	//How can we calc AMD processor score?
	//
	int iProcessorType = CDLGPROPERTY_PROCTYPE_INTEL;
	sProcessorSelectedOnInit = sCPUname;
	if (dCPUscore > 0) {
		if ((dCUDAscore > 0) && (dCUDAscore < dCPUscore)) {
			iProcessorType = CDLGPROPERTY_PROCTYPE_CUDA;
			sProcessorSelectedOnInit = sCudaGPUname;
		}
	}
	//
	dlgProperty.Init(iAvailableCPU, bSIMD, bAVX2, 
						iCUDAcount, iCUDAblock, iCUDAwarp, 
						iATIcount, iATImaxwork, iATIunitwork, iProcessorType);
	//prevPixelWidth = -1;
	bShowBoxAxis = true;
	bDragScroll = false;
	bWheelToGo = false;
}

CGazoApp::~CGazoApp() {
	if (dlgQueue.m_hWnd) dlgQueue.DestroyWindow();
	CLCleanup();
}
/////////////////////////////////////////////////////////////////////////////
// 唯一の CGazoApp オブジェクト

CGazoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGazoApp クラスの初期化

BOOL CGazoApp::InitInstance()
{
	//150101 hCursorRot = AfxGetApp()->LoadCursor(IDC_CURSOR_WHEEL);
	if (!dlgQueue.m_hWnd) dlgQueue.Create(IDD_QUEUE);
	//SYSTEM_INFO si;
	//GetSystemInfo(&si);
	//CString line; line.Format("%d", si.dwProcessorType); AfxMessageBox(line);

	AfxEnableControlContainer();

	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さく
	// したければ以下の特定の初期化ルーチンの中から不必要なもの
	// を削除してください。

#ifdef _AFXDLL
	//Enable3dControls();		// 共有 DLL の中で MFC を使用する場合にはここを呼び出してください。
#else
//	Enable3dControlsStatic();	// MFC と静的にリンクしている場合にはここを呼び出してください。
#endif

	// 設定が保存される下のレジストリ キーを変更します。
	// TODO: この文字列を、会社名または所属など適切なものに
	// 変更してください。
	SetRegistryKey(_T("GazoView RMizutani Application"));

	LoadStdProfileSettings(10);  // 標準の INI ファイルのオプションをローﾄﾞします (MRU を含む)

	// アプリケーション用のドキュメント テンプレートを登録します。ドキュメント テンプレート
	//  はドキュメント、フレーム ウィンドウとビューを結合するために機能します。

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_GAZOTYPE,
		RUNTIME_CLASS(CGazoDoc),
		RUNTIME_CLASS(CChildFrame), // カスタム MDI 子フレーム
		RUNTIME_CLASS(CGazoView));
	AddDocTemplate(pDocTemplate);

	// メイン MDI フレーム ウィンドウを作成
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// DDE、file open など標準のシェル コマンドのコマンドラインを解析します。
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;//not open a new document

	// コマンドラインでディスパッチ コマンドを指定します。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// メイン ウィンドウが初期化されたので、表示と更新を行います。
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	pMainFrame->m_wndStatusBar.SetPaneText(1, sProcessorSelectedOnInit);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard 仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
		// メッセージ ハンドラはありません。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CString m_sProgVersion;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
, m_sProgVersion(_T(""))
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_ABOUT_VER, m_sProgVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// メッセージ ハンドラはありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ダイアログを実行するためのアプリケーション コマンド
void CGazoApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.m_sProgVersion = sProgVersion;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGazoApp メッセージ ハンドラ

CView* CGazoApp::RequestNew() {
	OnFileNew();
	//CGazoDoc* pd = (CGazoDoc*) ((CMainFrame*) m_pMainWnd)->GetActiveDocument();
	CMDIFrameWnd *pFrame = 
							 (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// アクティブな MDI 子ウィンドウを取得する。 
	CMDIChildWnd *pChild = 
							 (CMDIChildWnd *) pFrame->GetActiveFrame();
	// または CMDIChildWnd *pChild = pFrame->MDIGetActive();

	// アクティブな MDI 子ウィンドウに結び付けられているアクティブなビューを取得する。
	//CGazoView *pView = (CGazoView *) pChild->GetActiveView();
	return pChild->GetActiveView();
	//return pView->GetDocument();
	//if (pd) AfxMessageBox(pd->GetTitle());
	//else AfxMessageBox("NULL");
	//CString line; line.Format("%d", pd); AfxMessageBox(line);
}

void CGazoApp::OnFileOpen() {
	CString fn = "";
	static char BASED_CODE defaultExt[] = ".img";
	static char BASED_CODE szFilter[] = 
		"Raw data (*.his;q*.img;*.h5)|*.his;q*.img;*.h5|rec files (rec*.tif)|rec*.tif|ro files (ro*.tif)|ro*.tif|ITEX files (*.img)|*.img|TIFF files (*.tif)|*.tif|Images (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp|All files (*.*)|*.*||";
	//CFileDialog dlg(TRUE, defaultExt, fn, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL, 0, FALSE);
	//190121 CFileDialog dlg(TRUE, defaultExt, fn, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter);
	CFileDialog dlg(TRUE, defaultExt, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter);
	if (dlg.DoModal() != IDOK) return;
	fn = dlg.GetPathName();
	CWinApp::OpenDocumentFile(fn);
}

void CGazoApp::SetBusy() {
	iStatus++;
	CMenu* pMenu = AfxGetMainWnd()->GetSystemMenu(FALSE);
	if (pMenu) {
		if (iStatus == 0) pMenu->EnableMenuItem(SC_CLOSE, MF_ENABLED);
		else pMenu->EnableMenuItem(SC_CLOSE,MF_GRAYED);
	}
}

void CGazoApp::SetIdle() {
	iStatus--;
	if (iStatus < 0) iStatus = 0;
	CMenu* pMenu = AfxGetMainWnd()->GetSystemMenu(FALSE);
	if (pMenu) {
		if (iStatus == 0) pMenu->EnableMenuItem(SC_CLOSE, MF_ENABLED);
		else pMenu->EnableMenuItem(SC_CLOSE,MF_GRAYED);
	}
}

bool CGazoApp::IsBusy() {
	if (iStatus) return true; else return false;
}

void CGazoApp::OnUpdateAppExit(CCmdUI* pCmdUI) 
{
	if (this->IsBusy()) {pCmdUI->Enable(false); return;}
	pCmdUI->Enable(true);	
}

void CGazoApp::OnUpdateTomoQueue(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(false); return;
	pCmdUI->Enable(true);
}

void CGazoApp::OnTomoQueue() 
{
	//if (!dlgQueue.m_hWnd) dlgQueue.Create(IDD_QUEUE);
	//dlgQueue.SetWindowText("Queue " + title);
	if (dlgQueue.IsWindowVisible()) dlgQueue.SetForegroundWindow();
	else dlgQueue.ShowWindow(SW_SHOW);
}

void CGazoApp::OnTomoProperty()
{
	const int iPrevProcType = dlgProperty.m_ProcessorType;
	if (dlgProperty.DoModal() == IDOK) {
		if (iPrevProcType != dlgProperty.m_ProcessorType) {
			CFrameWnd *pFrame = ((CMDIFrameWnd*)AfxGetMainWnd())->GetActiveFrame();
			if (pFrame) {
				CGazoDoc* pd0 = (CGazoDoc*)(pFrame->GetActiveDocument());
				if (pd0) {
					CDocTemplate* pdt = pd0->GetDocTemplate();
					if (pdt) {
						POSITION pos = pdt->GetFirstDocPosition();
						while (pos) {
							CGazoDoc* pd1 = (CGazoDoc*)pdt->GetNextDoc(pos);
							if (pd1) pd1->GPUMemFree(iPrevProcType);
						}
					}
				}
			}
		}
	}
	extern int blocksize;
	blocksize = dlgProperty.iCUDAnblock;
}

void CGazoApp::OnUpdateTomoProperty(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
	if (this->IsBusy()) pCmdUI->Enable(false);
	else pCmdUI->Enable(true);
	return;

	//if (parentDoc) {
	//	if (parentDoc->dlgReconst.m_hWnd) {pCmdUI->Enable(false); return;}
	//} else {
	//	if (dlgReconst.m_hWnd) {pCmdUI->Enable(false); return;}
	//}
	//pCmdUI->Enable(true);
}

void CGazoApp::OnUpdateWindowNew(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
	pCmdUI->Enable(false);
}

void CGazoApp::OnFilePrepfiles()
{
	static char BASED_CODE defaultExt[] = ".bat";
	static char BASED_CODE szFilter[] = 
		"Accompanied batch file (conv.bat)|conv.bat|Batch files (*.bat)|*.bat|All files (*.*)|*.*||";
	CFileDialog dlgFile(TRUE, defaultExt, "conv.bat", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) return;
	const CString fname = dlgFile.GetPathName();
	FILE* fconv = NULL;
	errno_t errn = fopen_s(&fconv, fname, "rt");
	if (errn) {AfxMessageBox("File not found."); return;}
	CStdioFile stdioConv(fconv);
	//
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fname);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
	const CString fpath = path_buffer;
	//AfxMessageBox(fname + "\r\n" + fpath);
	//check files
	TErr err = 0;
	CString fcheck = "";
	CString line = "";
	CString token[CGAZOAPP_CMD_MAXTOKEN];
	int iline = 0;
	while (stdioConv.ReadString(line)) {
		iline++;
		if (line.IsEmpty()) continue;
		const CString cmd = line.SpanExcluding("\t ");
		line = line.Mid(cmd.GetLength());
		line.TrimLeft();
		int narg = 0;
		int icmd = CGAZOAPP_CMD_NONE;
		if (cmd == "ren") {icmd = CGAZOAPP_CMD_REN; narg = 2;}
		else if (cmd == "copy") {icmd = CGAZOAPP_CMD_COPY; narg = 2;}
		else if (cmd == "img_ave") {icmd = CGAZOAPP_CMD_AVG; narg = CGAZOAPP_CMD_MAXTOKEN;}
		else continue;
		//get args
		for (int i=0; i<narg; i++) {
			token[i] = line.SpanExcluding("\t ");
			if (token[i].IsEmpty()) {narg = i; break;}
			line = line.Mid(token[i].GetLength());
			line.TrimLeft();
			if (line.IsEmpty()) {narg = i+1; break;}
		}
		CFile fimg;
		for (int i=0; i<narg-1; i++) {
			fcheck = token[i];
			if (fcheck.Left(1) == "q") continue;
			if (fimg.Open(fpath + fcheck, CFile::modeRead | CFile::shareDenyWrite)) {
				fimg.Close();
			} else {
				err = 23014;
				break;
			}
		}
		if (err) break;
	}
	if (err) {
		CString msg;
		msg.Format("ERROR. Line %d\r\nFile not found:\r\n ", iline);
		AfxMessageBox(msg + fcheck);
		return;
	}

	::CoInitialize(NULL);//initilaize COM
	IProgressDialog *pDlg = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER,
									IID_IProgressDialog, (void**)&pDlg);
	if (pDlg) {
		pDlg->SetTitle(L"Radiograph file preparation");
		//pDlg->SetAnimation(NULL, IDR_AVI);
		pDlg->SetLine(1, L"Radiograph file preparation", FALSE, NULL);
		pDlg->StartProgressDialog(NULL, NULL, PROGDLG_NORMAL | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
	}

	line = "";
	int istep = 0;
	bool bSkipErr = false;
	stdioConv.SeekToBegin();
	while (stdioConv.ReadString(line)) {
		if (line.IsEmpty()) continue;
		const CString cmd = line.SpanExcluding("\t ");
		line = line.Mid(cmd.GetLength());
		line.TrimLeft();
		int narg = 0;
		int icmd = CGAZOAPP_CMD_NONE;
		if (cmd == "ren") {icmd = CGAZOAPP_CMD_REN; narg = 2;}
		else if (cmd == "copy") {icmd = CGAZOAPP_CMD_COPY; narg = 2;}
		else if (cmd == "img_ave") {icmd = CGAZOAPP_CMD_AVG; narg = CGAZOAPP_CMD_MAXTOKEN;}
		//Dlg control
		istep++;
		if (pDlg) {
			//pDlg->SetProgress(icurr, imax);
			if (istep % 20 == 0) {
				CString msg2;
				switch (icmd) {
					case CGAZOAPP_CMD_REN: {msg2 = "Moving files..."; break;}
					case CGAZOAPP_CMD_COPY: {msg2 = "Copying files..."; break;}
					case CGAZOAPP_CMD_AVG: {msg2 = "Averaging images..."; break;}
					default: {msg2 = "Working...";}
				}
				pDlg->SetLine(2, (CStringW)msg2, FALSE, NULL);
				pDlg->SetLine(3, (CStringW)(token[0] + " ===> " + token[1]), FALSE, NULL);
				Sleep(1);
			}
			if( pDlg->HasUserCancelled() )  break;
		}
		//get args
		for (int i=0; i<narg; i++) {
			token[i] = line.SpanExcluding("\t ");
			if (token[i].IsEmpty()) {narg = i; break;}
			line = line.Mid(token[i].GetLength());
			line.TrimLeft();
			if (line.IsEmpty()) {narg = i+1; break;}
		}
		//commands
		const CString fn0 = fpath + token[0];
		const CString fn1 = fpath + token[1];
		BOOL bErr = TRUE;
		err = 0;
		switch (icmd) {
			case CGAZOAPP_CMD_REN: {
				bErr = ::MoveFile(fn0, fn1);
				break;}
			case CGAZOAPP_CMD_COPY: {
				bErr = ::CopyFile(fn0, fn1, TRUE);
				break;}
			case CGAZOAPP_CMD_AVG: {
				err = CalcAvgImage(fpath, token, narg);
				break;}
		}
		if ( ((bErr == 0)||(err != 0)) && (bSkipErr == false) ) {
			CString msg = "ERROR.\r\n", scr;
			if (err) {scr.Format(" reason# %d\r\n", err); msg += scr;}
			if (AfxMessageBox(msg + "Command:" + cmd + "\r\nFiles:\r\n " + fn0 + "\r\n " + fn1 + "\r\nContinue?", MB_YESNO)
					== IDNO) break;
			else bSkipErr = true;
		}
	}
	//
	pDlg->SetLine(2, (CStringW)"Finished.", FALSE, NULL);
	pDlg->SetLine(3, (CStringW)"", FALSE, NULL);
	fclose(fconv);
	if (pDlg) pDlg->StopProgressDialog();
	::CoUninitialize();//unload COM
}

TErr CGazoApp::CalcAvgImage(CString path, CString* files, int nfiles) {
	CFile fimg;
	int ixdim = 0;
	int iydim = 0;
	int* pData = NULL;
	int maxData = 0;
	int* pSum = NULL;
	TErr err = 0;
	int iavg = 0;
	CString comment = "";
	for (int i=0; i<nfiles-1; i++) {
		int ixprev = ixdim;
		int iyprev = iydim;
		const CString fn = path + files[i];
		if (fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			err = ReadITEX(&fimg, &pData, &maxData, &iydim, &ixdim, &comment);
			if (err) AfxMessageBox("ERROR in averaging. Unknown format:\r\n " + fn);
			fimg.Close();
		} else {
			AfxMessageBox("ERROR in averaging. File not found:\r\n " + fn);
		}
		if (ixprev && iyprev) {
			if ((ixprev != ixdim)||(iyprev != iydim)) {err = 23010; break;}
		}
		if (maxData <= 0) continue;
		if (!pSum) {
			pSum = new int[maxData];
			if (!pSum) {err =23012; break;}
			for (int j=0; j<maxData; j++) {pSum[j] = 0;}
		}
		for (int j=0; j<maxData; j++) {pSum[j] += pData[j];}
		iavg++;
	}
	if (err) {if (pData) delete [] pData; return err;}
	if (pSum == NULL) {if (pData) delete [] pData; return 23013;}
	//save
	for (int j=0; j<maxData; j++) {pSum[j] /= iavg;}
	const CString fn = path + files[nfiles-1];
	if (fimg.Open(fn, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite)) {
		err = WriteITEX(&fimg, pSum, iydim, ixdim, comment, 0, 0, 2);
		if (err) AfxMessageBox("ERROR in averaging. File output:\r\n " + fn);
		fimg.Close();
	}
	if (pData) delete [] pData;
	if (pSum) delete [] pSum;
	return err;
}

void CGazoApp::OnTomoLsqfit()
{
	CString sPathName[] = { "", "" };
	POSITION pos = GetFirstDocTemplatePosition();
	while (pos) {
		CDocTemplate* pTemplate = (CDocTemplate*)GetNextDocTemplate(pos);
		POSITION pos2 = pTemplate->GetFirstDocPosition();
		while (pos2) {
			CDocument* pd;
			if ((pd = pTemplate->GetNextDoc(pos2)) != NULL) {
				if (sPathName[0].IsEmpty()) sPathName[0] = pd->GetPathName();
				else if (sPathName[1].IsEmpty()) sPathName[1] = pd->GetPathName();
				else break;
			}
		}
	}
	const int nframes = 20;
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	for (int j = 0; j <= 1; j++) {
		_stprintf_s(path_buffer, _MAX_PATH, sPathName[j]);
		_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
		CString sfnm = fnm;
		CString sSuffix = sfnm.SpanExcluding("0123456789");
		const int ifmt = sfnm.GetLength() - sSuffix.GetLength();
		if (ifmt < 0) break;
		CString fmt; fmt.Format("%s%%0%dd", sSuffix, ifmt);
		int idx0 = atoi(sfnm.Mid(sSuffix.GetLength()));
		sPathName[j].Empty();
		for (int i = 0; i < nframes; i++) {
			sfnm.Format(fmt, i + idx0);
			_stprintf_s(fnm, _MAX_FNAME, sfnm);
			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fnm, ext);
			sPathName[j] += path_buffer;
			sPathName[j] += "\r\n";
		}
	}
	AfxMessageBox(sPathName[0] + "\r\n-----\r\n" + sPathName[1]);
	CDlgLsqfit dlg;
	dlg.m_RefList = sPathName[0];
	dlg.m_QryList = sPathName[1];
	dlg.nRefFiles = nframes;
	dlg.nQryFiles = nframes;
	dlg.UpdateNfiles();
	dlg.DoModal();
}

void CGazoApp::OnFileCloseall()
{
	if (AfxMessageBox("Close all files ?", MB_OKCANCEL) == IDCANCEL) return;
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	if (pFrame == NULL) return;
	CMDIChildWnd *pChild = NULL;
	while (pChild = (CMDIChildWnd *) pFrame->GetActiveFrame()) {
		if ((void*)pChild == (void*)pFrame) break;//if no child frames
		CGazoView *pv = (CGazoView *) pChild->GetActiveView();
		if (pv) {
			CGazoDoc* pd = pv->GetDocument();
			if (pd) {
				if (pd->SaveModified()) pd->OnCloseDocument();
			}
		}
	}
}

void CGazoApp::OnTomoRenumfiles()
{
	CDlgRenumFiles dlg;
	dlg.DoModal();
}

void CGazoApp::OnViewError()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	CDlgMessage dlg;
	if (error.Report().IsEmpty()) dlg.m_Msg = "No errors.";
	else dlg.m_Msg = error.Report();//120720
	dlg.DoModal();
}

int GazoAppLsqFitCompare( const void *arg1, const void *arg2 ) {
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

CString CGazoApp::Lsqfit(LSQFIT_QUEUE* lq, CDlgLsqfit* dlg, CDlgQueue* dqueue) {
	CString rtn = "ND";//this return value indicates aborted process
	const int nLsqList = (lq->m_XHigh - lq->m_XLow + 1) * (lq->m_YHigh - lq->m_YLow + 1) * (lq->m_ZHigh - lq->m_ZLow + 1);
	short** ppRefPixel = NULL;
	int* pMaxRefPixel = NULL;
	short** ppQryPixel = NULL;
	int* pMaxQryPixel = NULL;
	CString* sLsqList = NULL;
	try {
		ppRefPixel = new short*[lq->nRefFiles];
		pMaxRefPixel = new int[lq->nRefFiles];
		ppQryPixel = new short*[lq->nQryFiles];
		pMaxQryPixel = new int[lq->nQryFiles];
		sLsqList = new CString[nLsqList];
	}
	catch(CException* e) {
		e->Delete();
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		if (sLsqList) delete [] sLsqList;
		rtn = "Out of memory";
		return rtn;
	}
	for (int i=0; i<lq->nRefFiles; i++) {
		ppRefPixel[i] = NULL;
		pMaxRefPixel[i] = 0;
	}
	for (int i=0; i<lq->nQryFiles; i++) {
		ppQryPixel[i] = NULL;
		pMaxQryPixel[i] = 0;
	}
	for (int i=0; i<nLsqList; i++) {sLsqList[i].Empty();}

	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	CFile fp;
	int* ibuf = NULL;
	bool bError = false;
	//Reading reference image set
	CString str = lq->m_RefList;
	int iPos = 0;
	int ixref = -1, iyref = -1;
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: reading reference set");
	for (int i=0; i<lq->nRefFiles; i++) {
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			if (dlg) dlg->m_Result += "Not found: " + fn; 
			bError = true;
			break;
		}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			if (dlg) dlg->m_Result += "Unknown format: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (ixref < 0) {ixref = ixdim; iyref = iydim;}
		else if ((ixdim != ixref)||(iydim != iyref)) {
			if (dlg) dlg->m_Result += "Image size not matched: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		if (dlg) dlg->m_Result += " Ref: " + fn + "\r\n";
		fp.Close();
		try {ppRefPixel[i] = new short[nbuf];}
		catch(CException* e) {
			e->Delete();
			bError = true;
			rtn = "Out of memory";
			break;
		}
		pMaxRefPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppRefPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppRefPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppRefPixel[i])[j] = SHRT_MAX;
			else (ppRefPixel[i])[j] = (short)(absCoeff);
			//(ppRefPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		if (dlg) dlg->UpdateData(FALSE);
	}
	if (bError) {
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		if (sLsqList) delete [] sLsqList;
		return rtn;
	}
	//Reading query image set
	str = lq->m_QryList;
	iPos = 0;
	int ixqry = -1, iyqry = -1;
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: reading query set");
	for (int i=0; i<lq->nQryFiles; i++) {
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			if (dlg) {dlg->m_Result += "Not found: " + fn;} 
			bError = true;
			break;
		}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			if (dlg) dlg->m_Result += "Unknown format: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (ixqry < 0) {ixqry = ixdim; iyqry = iydim;}
		else if ((ixdim != ixqry)||(iydim != iyqry)) {
			if (dlg) dlg->m_Result += "Image size not matched: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		if (dlg) dlg->m_Result += " Qry: " + fn + "\r\n";
		fp.Close();
		try {ppQryPixel[i] = new short[nbuf];}
		catch(CException* e) {
			e->Delete();
			bError = true;
			rtn = "Out of memory";
			break;
		}
		pMaxQryPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppQryPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppQryPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppQryPixel[i])[j] = SHRT_MAX;
			else (ppQryPixel[i])[j] = (short)(absCoeff);
			//(ppQryPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		if (dlg) dlg->UpdateData(FALSE);
	}
	if (ibuf) delete [] ibuf;//131110
	if (bError) {
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		if (sLsqList) delete [] sLsqList;
		return rtn;
	}
	if (dlg) dlg->bStarted = true;
	if (dlg) dlg->EnableCtrl();
	int iLsqList = 0;
	RECONST_INFO ri[MAX_CPU];
	struct _timeb tstruct; double tm0;
	_ftime_s( &tstruct );
	tm0 = tstruct.time + tstruct.millitm * 0.001;
	//output logs
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	_stprintf_s(path_buffer, _MAX_PATH, lq->m_QryList.SpanExcluding(_T("\r\n")));
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "recviewlog", ".txt");
	CStdioFile flog;
	if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "0recviewlog", ".txt");
		if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "_recviewlog", ".txt");
		}
		else {
			flog.Close();
		}
	}
	else {
		flog.Close();
	}
	//AfxMessageBox(path_buffer); return rtn;//////////////
	if (flog.Open(path_buffer, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		flog.SeekToEnd();
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line;
		CString sVer = this->sProgVersion; sVer.Replace('\n', ' ');
		line.Format("LSQ fit [%s] %s\n", stime.Left(24), sVer);
		flog.WriteString(line);
		line.Format(" Ref set: %s [%d files]\n Qry set: %s [%d files]\n", lq->m_RefList.SpanExcluding(_T("\r\n")), lq->nRefFiles, 
										lq->m_QryList.SpanExcluding(_T("\r\n")), lq->nQryFiles);
		flog.WriteString(line);
		//if (dlg) flog.WriteString(dlg->m_Result);
	}
	//lsq fitting
	//const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	int nCPU = 1;
	if (this->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		nCPU = (int)(this->dlgProperty.iCUDA);
		short** d_ppRefPixel = NULL;
		short** d_ppQryPixel = NULL;
		unsigned __int64* h_result = NULL;
		try {
			d_ppRefPixel = new short*[lq->nRefFiles];
			d_ppQryPixel = new short*[lq->nQryFiles];
			h_result = new unsigned __int64[ixref * 2];
		}
		catch(CException* e) {
			e->Delete();
			rtn = "Out of memory";
			for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
			for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
			if (ppRefPixel) delete [] ppRefPixel;
			if (pMaxRefPixel) delete [] pMaxRefPixel;
			if (ppQryPixel) delete [] ppQryPixel;
			if (pMaxQryPixel) delete [] pMaxQryPixel;
			if (sLsqList) delete [] sLsqList;
			if (d_ppRefPixel) delete [] d_ppRefPixel;
			if (d_ppQryPixel) delete [] d_ppQryPixel;
			if (h_result) delete [] h_result;
			return rtn;
		}
		unsigned __int64* d_result = NULL;
		for (int i=0; i<lq->nRefFiles; i++) {d_ppRefPixel[i] = NULL;}
		for (int i=0; i<lq->nQryFiles; i++) {d_ppQryPixel[i] = NULL;}
		CudaLsqfitMemAlloc(d_ppRefPixel, d_ppQryPixel, pMaxRefPixel, pMaxQryPixel, 
						ppRefPixel, ppQryPixel, lq->nRefFiles, lq->nQryFiles, ixref, &d_result);
		for (int ix=lq->m_XLow; ix<=lq->m_XHigh; ix++) {
			for (int iy=lq->m_YLow; iy<=lq->m_YHigh; iy++) {
				for (int iz=lq->m_ZLow; iz<=lq->m_ZHigh; iz++) {
					unsigned __int64 nlsq = 0;
					unsigned __int64 ilsq = 0;
					for (int jrz=0; jrz<lq->nRefFiles; jrz++) {
						const int jqz = jrz + iz;
						if ((jqz < 0)||(jqz >= lq->nQryFiles)) continue;
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
						CString msg; msg.Format("%f (%d %d %d) %.0f/%.0f", rlsq, ix, iy, iz, dilsq, (TReal)nlsq);
						sLsqList[iLsqList++] = msg;
						if (iz == 0) {
							if (dlg) dlg->m_Result = "RMSD (dx dy dz) SumDiff/NSum\r\n" + msg;
							if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
						}
					}
					ProcessMessage();
					if (dlg) {if (dlg->bStarted == false) break;}
					if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				}
				if (dlg) dlg->UpdateData(FALSE);
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
			}
			if (dlg) {if (dlg->bStarted == false) break;}
			if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
		}
		CudaLsqfitMemFree(d_ppRefPixel, d_ppQryPixel, lq->nRefFiles, lq->nQryFiles, d_result);
		if (d_ppRefPixel) delete [] d_ppRefPixel;
		if (d_ppQryPixel) delete [] d_ppQryPixel;
		if (h_result) delete [] h_result;
	} else {//pApp->dlgProperty.m_ProcessorType
		nCPU = (int)(this->dlgProperty.iCPU);
		for (int ix=lq->m_XLow; ix<=lq->m_XHigh; ix++) {
			for (int iy=lq->m_YLow; iy<=lq->m_YHigh; iy++) {
				for (int iz=lq->m_ZLow; iz<=lq->m_ZHigh; iz+=nCPU) {
					for (int i=nCPU-1; i>=0; i--) {
						ri[i].hThread = NULL;
						ri[i].iStartSino = i;
						//if (i) ri[i].bMaster = false; else ri[i].bMaster = true;
						ri[i].iStatus = RECONST_INFO_IDLE;
						ri[i].i64result = 0;//double
						ri[i].i64sum = 0;//int
						if (iz + i > lq->m_ZHigh) continue;
						ri[i].iStatus = RECONST_INFO_BUSY;
						//190102 ri[i].max_d_ifp = lq->nRefFiles;
						//190102 ri[i].max_d_igp = lq->nQryFiles;
						ri[i].uiMaxRef = lq->nRefFiles;
						ri[i].uiMaxQry = lq->nQryFiles;
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
						if (iz + i > lq->m_ZHigh) continue;
						unsigned __int64 ilsq = ri[i].i64result;//__int64
						unsigned __int64 nlsq = ri[i].i64sum;//int
						if (nlsq) {
							TReal rlsq = sqrt(ilsq / (TReal)nlsq);
							TReal dilsq = (double)ilsq;
							//120624 CString msg; msg.Format("%d %d %d %f %.0f/%d\r\n", ix, iy, iz+i, rlsq, dilsq, nlsq);
							//120624 m_Result += msg;
							CString msg; msg.Format("%f (%d %d %d) %.0f/%lld", rlsq, ix, iy, iz+i, dilsq, nlsq);
							sLsqList[iLsqList++] = msg;
							if (dlg) dlg->m_Result = "RMSD (dx dy dz) SumDiff/NSum\r\n" + msg;
							if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
						}
					}
					ProcessMessage();
					if (dlg) {if (dlg->bStarted == false) break;}
					if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				}
				if (dlg) dlg->UpdateData(FALSE);
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
			}
			if (dlg) {if (dlg->bStarted == false) break;}
			if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
		}
	}
	//sort
	CString** pLsqList = NULL;
	try {pLsqList = new CString*[iLsqList];}
	catch(CException* e) {
		e->Delete();
		rtn = "Out of memory";
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		if (sLsqList) delete [] sLsqList;
		if (pLsqList) delete [] pLsqList;
		return rtn;
	}
	for (int i=0; i<iLsqList; i++) {pLsqList[i] = &(sLsqList[i]);}
	qsort( (void *)pLsqList, (size_t)iLsqList, sizeof(CString*), GazoAppLsqFitCompare );
	if (dlg) dlg->m_Result.Empty();
	for (int i=0; i<iLsqList; i++) {if (dlg) dlg->m_Result += " " + *(pLsqList[i]) + "\r\n";}
	_ftime_s( &tstruct );
	TReal tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	CString msg;
	TReal minlsq = 0; int mx = 0, my = 0, mz = 0;
	sscanf_s(*(pLsqList[0]), "%lf (%d %d %d)", &minlsq, &mx, &my, &mz);
	msg.Format(" Min: ref(0 0 0)=qry(%d %d %d) rmsd=%f", mx, my, mz, minlsq);
	const CString headerStr = " RMSD (dx dy dz) SumDiff/NSum";
	if (dlg) {
		if (dlg->bStarted) {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
			if (flog.m_hFile != CFile::hFileNull) {
				flog.WriteString(msg + "\n" + headerStr + "\n");
				for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]) + "\n");}
			}
			dlg->m_Result = msg +"\r\n" + headerStr + "\r\n" + dlg->m_Result;
			msg.Format("CPU=%fsec\r\n", tcpu);
			dlg->m_Result += msg;
			rtn.Format("(%d %d %d)%.2f", mx, my, mz, minlsq);
		} else {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: aborted");
		}
		dlg->bStarted = false;
	} else if (dqueue) {
		if (dqueue->iStatus != CDLGQUEUE_STOP) {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
			if (flog.m_hFile != CFile::hFileNull) {
				flog.WriteString(msg + "\n" + headerStr + "\n");
				for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]) + "\n");}
			}
			rtn.Format("(%d %d %d)%.2f", mx, my, mz, minlsq);
		} else {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: aborted");
		}
	} else {
		if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
		if (flog.m_hFile != CFile::hFileNull) {
			flog.WriteString(msg + headerStr);
			for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]) + "\n");}
		}
	}
	if (flog.m_hFile != CFile::hFileNull) {
		flog.WriteString("---------------------------------------------------\n");
		flog.Close();
	}
	//delete images
	for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
	if (ppRefPixel) delete [] ppRefPixel;
	if (pMaxRefPixel) delete [] pMaxRefPixel;
	for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
	if (ppQryPixel) delete [] ppQryPixel;
	if (pMaxQryPixel) delete [] pMaxQryPixel;
	if (sLsqList) delete [] sLsqList;
	if (pLsqList) delete [] pLsqList;
	return rtn;
}

CString CGazoApp::LsqfitMin(LSQFIT_QUEUE* lq, CDlgLsqfit* dlg, CDlgQueue* dqueue) {
	/*/
	double a[] = {-1, 2, 3,  2, 3, 4,  1, 1, 2};
	double b[] = {-1, 2, 3,  2, 3, 4,  1, 1, 2};
	int ierr = InvMatrix(a, 3, 1E-6);
	CString mmsg = "", mline;
	if (ierr) {mmsg.Format("%d", ierr); AfxMessageBox(mmsg); return "a";}
	mline.Format("%f %f %f\r\n%f %f %f\r\n%f %f %f\r\n---\r\n", a[0],a[1],a[2], a[3],a[4],a[5], a[6],a[7],a[8]);
	mmsg += mline;
	AfxMessageBox(mline); return "b";
	/*/
	CString rtn = "ND";//this return value indicates aborted process
	const int nLsqList = (lq->m_XHigh - lq->m_XLow + 1) * (lq->m_YHigh - lq->m_YLow + 1) * (lq->m_ZHigh - lq->m_ZLow + 1);
	short** ppRefPixel = NULL;
	int* pMaxRefPixel = NULL;
	short** ppQryPixel = NULL;
	int* pMaxQryPixel = NULL;
	try {
		ppRefPixel = new short*[lq->nRefFiles];
		pMaxRefPixel = new int[lq->nRefFiles];
		ppQryPixel = new short*[lq->nQryFiles];
		pMaxQryPixel = new int[lq->nQryFiles];
	}
	catch(CException* e) {
		e->Delete();
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		rtn = "Out of memory";
		return rtn;
	}
	for (int i=0; i<lq->nRefFiles; i++) {
		ppRefPixel[i] = NULL;
		pMaxRefPixel[i] = 0;
	}
	for (int i=0; i<lq->nQryFiles; i++) {
		ppQryPixel[i] = NULL;
		pMaxQryPixel[i] = 0;
	}

	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	CFile fp;
	int* ibuf = NULL;
	bool bError = false;
	if (dlg) dlg->bStarted = true;
	if (dlg) dlg->EnableCtrl();
	//Reading reference image set
	CString str = lq->m_RefList;
	int iPos = 0;
	int ixref = -1, iyref = -1;
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: reading reference set");
	for (int i=0; i<lq->nRefFiles; i++) {
		ProcessMessage();
		if (dlg) {if (dlg->bStarted == false) break;}
		if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			if (dlg) dlg->m_Result += "Not found: " + fn; 
			bError = true;
			break;
		}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			if (dlg) dlg->m_Result += "Unknown format: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (ixref < 0) {ixref = ixdim; iyref = iydim;}
		else if ((ixdim != ixref)||(iydim != iyref)) {
			if (dlg) dlg->m_Result += "Image size not matched: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		if (dlg) dlg->m_Result += " Ref: " + fn + "\r\n";
		fp.Close();
		try {ppRefPixel[i] = new short[nbuf];}
		catch(CException* e) {
			e->Delete();
			bError = true;
			rtn = "Out of memory";
			break;
		}
		pMaxRefPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppRefPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppRefPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppRefPixel[i])[j] = SHRT_MAX;
			else (ppRefPixel[i])[j] = (short)(absCoeff);
			//(ppRefPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		if (dlg) dlg->UpdateData(FALSE);
		ProcessMessage();
	}
	if (bError) {
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		return rtn;
	}
	//Reading query image set
	str = lq->m_QryList;
	iPos = 0;
	int ixqry = -1, iyqry = -1;
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: reading query set");
	for (int i=0; i<lq->nQryFiles; i++) {
		ProcessMessage();
		if (dlg) {if (dlg->bStarted == false) break;}
		if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
		CString fn= str.Tokenize(_T("\r\n"), iPos);
		if (fn.IsEmpty()) continue;
		if (!fp.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			if (dlg) {dlg->m_Result += "Not found: " + fn;} 
			bError = true;
			break;
		}
		float pixDiv = 0, pixBase = 0, fCenter = 0;
		float pw = 1;
		int iydim = 0, ixdim = 0, iFilter = 0;
		int nbuf = 0;
		if (ReadTif(&fp, &ibuf, &nbuf, &iydim, &ixdim, &pixDiv, &pixBase, 
								&fCenter, &iFilter, &pw)) {
			if (dlg) dlg->m_Result += "Unknown format: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (ixqry < 0) {ixqry = ixdim; iyqry = iydim;}
		else if ((ixdim != ixqry)||(iydim != iyqry)) {
			if (dlg) dlg->m_Result += "Image size not matched: " + fn;
			fp.Close();
			bError = true;
			break;
		}
		if (pixDiv < 0) {pixDiv = 0; pixBase = 0;}
		if (dlg) dlg->m_Result += " Qry: " + fn + "\r\n";
		fp.Close();
		try {ppQryPixel[i] = new short[nbuf];}
		catch(CException* e) {
			e->Delete();
			bError = true;
			rtn = "Out of memory";
			break;
		}
		pMaxQryPixel[i] = nbuf;
		for (int j=0; j<nbuf; j++) {
			float absCoeff = (ibuf[j] / pixDiv + pixBase) * 10;
			if (ibuf[j] == 0) (ppQryPixel[i])[j] = SHRT_MIN;
			else if (absCoeff < SHRT_MIN+1) (ppQryPixel[i])[j] = SHRT_MIN+1;
			else if (absCoeff > SHRT_MAX) (ppQryPixel[i])[j] = SHRT_MAX;
			else (ppQryPixel[i])[j] = (short)(absCoeff);
			//(ppQryPixel[i])[j] = (unsigned short)(ibuf[j]);
		}
		if (dlg) dlg->UpdateData(FALSE);
		ProcessMessage();
	}
	if (ibuf) delete [] ibuf;//131110
	if (bError) {
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		return rtn;
	}
	const int iMaxDaimeter = lq->m_XHigh;
	const int iMaxBinning = 64;
	struct _timeb tstruct;
	_ftime_s( &tstruct );
	//output logs
	TCHAR path_buffer[_MAX_PATH];//_MAX_PATH = 260, typically
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR];// TCHAR fnm[_MAX_FNAME];
	_stprintf_s(path_buffer, _MAX_PATH, lq->m_QryList.SpanExcluding(_T("\r\n")));
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "recviewlog", ".txt");
	CStdioFile flog;
	if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "0recviewlog", ".txt");
		if (!flog.Open(path_buffer, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, "_recviewlog", ".txt");
		}
		else {
			flog.Close();
		}
	}
	else {
		flog.Close();
	}
	//AfxMessageBox(path_buffer); return rtn;//////////////
	CString msgfn = "";
	if (flog.Open(path_buffer, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		flog.SeekToEnd();
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line;
		CString sVer = this->sProgVersion; sVer.Replace('\n', ' ');
		line.Format("LSQ fit [%s] %s\n", stime.Left(24), sVer);
		flog.WriteString(line);
		line.Format(" Ref set: %s [%d files]\n Qry set: %s [%d files]\n", lq->m_RefList.SpanExcluding(_T("\r\n")), lq->nRefFiles, 
										lq->m_QryList.SpanExcluding(_T("\r\n")), lq->nQryFiles);
		flog.WriteString(line);
		msgfn = line;
		line.Format(" Scan diameter: %d pixel\n Max binning: %d\n", iMaxDaimeter, iMaxBinning);
		flog.WriteString(line);
		//if (dlg) flog.WriteString(dlg->m_Result);
	}
	//lsq fitting
	if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: search");
	CXyz cDelta(0, 0, 0); short* psBinRefPixel = NULL; short* psBinQryPixel = NULL;
	try {
		psBinRefPixel = new short[ixref * iyref * lq->nRefFiles];
		psBinQryPixel = new short[ixqry * iyqry * lq->nQryFiles];
	}
	catch(CException* e) {
		e->Delete();
		for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
		for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
		if (ppRefPixel) delete [] ppRefPixel;
		if (pMaxRefPixel) delete [] pMaxRefPixel;
		if (ppQryPixel) delete [] ppQryPixel;
		if (pMaxQryPixel) delete [] pMaxQryPixel;
		return rtn;
	}
	msgfn.Replace("\n", "\r\n");
	if (dlg) dlg->m_Result = msgfn;
	const int imaxrad2 = iMaxDaimeter * iMaxDaimeter / 4;
	double dsummin = 0;
	CString msglog = " ";
	for (int ibin=iMaxBinning; ibin>=1; ibin/=2) {
		CString line; line.Format("Lsqfit: search binning%d", ibin);
		if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
		ProcessMessage();
		if (dlg) {if (dlg->bStarted == false) break;}
		if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
		int ibxref = ixref / ibin + ((ixref % ibin) ? 1 : 0);
		int ibyref = iyref / ibin + ((iyref % ibin) ? 1 : 0);
		int ibzref = lq->nRefFiles / ibin + ((lq->nRefFiles % ibin) ? 1 : 0);
		int ibxqry = ixqry / ibin + ((ixqry % ibin) ? 1 : 0);
		int ibyqry = iyqry / ibin + ((iyqry % ibin) ? 1 : 0);
		int ibzqry = lq->nQryFiles / ibin + ((lq->nQryFiles % ibin) ? 1 : 0);
		//ref binning
		if (ibin == 1) {
			for (int iz=0; iz<lq->nRefFiles; iz++) {
				ProcessMessage();
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				for (int ix=0; ix<ixref; ix++) {
					for (int iy=0; iy<iyref; iy++) {
						short spix = 0;
						int iradx = ix - ixref/2;
						int irady = iy - iyref/2;
						if (iradx * iradx + irady * irady <= imaxrad2) {
							spix = (ppRefPixel[iz])[iy * ixref + ix];
							if (spix == SHRT_MIN) spix = 0;
						}
						psBinRefPixel[iz * ibxref * ibyref + iy * ibxref + ix] = spix;
					}
				}
			}
		} else {
			for (int iz=0; iz<lq->nRefFiles; iz+=ibin) {
				ProcessMessage();
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				for (int ix=0; ix<ixref; ix+=ibin) {
					for (int iy=0; iy<iyref; iy+=ibin) {
						int isum = 0, icount = 0;
						for (int jx=0; jx<ibin; jx++) {
							if (ix + jx >= ixref) continue;
							for (int jy=0; jy<ibin; jy++) {
								if (iy + jy >= iyref) continue;
								int iradx = ix + jx - ixref/2;
								int irady = iy + jy - iyref/2;
								if (iradx * iradx + irady * irady > imaxrad2) continue;
								for (int jz=0; jz<ibin; jz++) {
									if (iz + jz >= lq->nRefFiles) continue;
									short spix = (ppRefPixel[iz + jz])[(iy + jy) * ixref + ix + jx];
									if (spix == SHRT_MIN) continue;
									isum += spix;
									icount++;
								}
							}
						}
						if (icount) isum /= icount;
						psBinRefPixel[iz/ibin * ibxref * ibyref + iy/ibin * ibxref + ix/ibin] = isum;
					}
				}
			}
		}
		//qry binning
		if (ibin == 1) {
			for (int iz=0; iz<lq->nQryFiles; iz++) {
				ProcessMessage();
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				for (int ix=0; ix<ixqry; ix++) {
					for (int iy=0; iy<iyqry; iy++) {
						short spix = 0;
						int iradx = ix - ixqry/2;
						int irady = iy - iyqry/2;
						if (iradx * iradx + irady * irady <= imaxrad2) {
							spix = (ppQryPixel[iz])[iy * ixqry + ix];
							if (spix == SHRT_MIN) spix = 0;
						}
						psBinQryPixel[iz * ibxqry * ibyqry + iy * ibxqry + ix] = spix;
					}
				}
			}
		} else {
			for (int iz=0; iz<lq->nQryFiles; iz+=ibin) {
				ProcessMessage();
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				for (int ix=0; ix<ixqry; ix+=ibin) {
					for (int iy=0; iy<iyqry; iy+=ibin) {
						int isum = 0, icount = 0;
						for (int jx=0; jx<ibin; jx++) {
							if (ix + jx >= ixqry) continue;
							for (int jy=0; jy<ibin; jy++) {
								if (iy + jy >= iyqry) continue;
								int iradx = ix + jx - ixqry/2;
								int irady = iy + jy - iyqry/2;
								if (iradx * iradx + irady * irady > imaxrad2) continue;
								for (int jz=0; jz<ibin; jz++) {
									if (iz + jz >= lq->nQryFiles) continue;
									short spix = (ppQryPixel[iz + jz])[(iy + jy) * ixqry + ix + jx];
									if (spix == SHRT_MIN) continue;
									isum += spix;
									icount++;
								}
							}
						}
						if (icount) isum /= icount;
						psBinQryPixel[iz/ibin * ibxqry * ibyqry + iy/ibin * ibxqry + ix/ibin] = isum;
					}
				}
			}
		}
		//minimization
		for (int jstep=0; jstep<10; jstep++) {
			ProcessMessage();
			if (dlg) {if (dlg->bStarted == false) break;}
			if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
			//grad
			CString line; line.Format("Lsqfit: search binning%d origin", ibin);
			if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
			CXyz cGrad(0, 0, 0);
			double dsum0 = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, CXyz(0, 0, 0),
											ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
			ProcessMessage();
			if (dlg) {if (dlg->bStarted == false) break;}
			if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
			if (dsum0 > 0) {
				line.Format("Lsqfit: search binning%d gradx", ibin);
				if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
				double dsumx = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, CXyz(1, 0, 0),
												ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
				line.Format("Lsqfit: search binning%d grady", ibin);
				if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
				double dsumy = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, CXyz(0, 1, 0),
												ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
				line.Format("Lsqfit: search binning%d gradz", ibin);
				if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
				double dsumz = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, CXyz(0, 0, 1),
												ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
				if (dsumx >= 0) cGrad.x = dsumx - dsum0;
				if (dsumy >= 0) cGrad.y = dsumy - dsum0;
				if (dsumz >= 0) cGrad.z = dsumz - dsum0;
				if (cGrad.Length2() > 0) cGrad.UnitLength();
			}
			//line search
			dsummin = (dsum0 > 0) ? dsum0 : 0; double dmin = 0;
			CString msg3 = "step", msg4 = " ";
			double diff[10];
			const double dstep = 0.5;
			const double dlimit = dstep * (10 - 1);
			double dx4 = 0, dx3 = 0, dx2 = 0, dx = 0, dx2y = 0, dxy = 0, dy = 0, dy2 = 0;
			for (int k=0; k<10; k++) {
				double dgrad = k * dstep;
				line; line.Format("Lsqfit: search binning%d grad%.1f", ibin, dgrad);
				if (pf) pf->m_wndStatusBar.SetPaneText(1, line);
				ProcessMessage();
				if (dlg) {if (dlg->bStarted == false) break;}
				if (dqueue) {if (dqueue->iStatus == CDLGQUEUE_STOP) break;}
				double dsumg = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, cGrad.X(-dgrad),
											ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
				diff[k] = (dsumg < 0) ? 0 : sqrt(dsumg);
				line.Format("%.1f\t", dgrad); msg3 += line;
				line.Format("%.2f\t", diff[k]); msg4 += line;
				dx4 += (dgrad * dgrad * dgrad * dgrad);
				dx3 += (dgrad * dgrad * dgrad);
				dx2 += (dgrad * dgrad);
				dx += dgrad;
				dx2y += (dgrad * dgrad * diff[k]);
				dxy += (dgrad * diff[k]);
				dy += diff[k];
				dy2 += (diff[k] * diff[k]);
//				if (dsumg < 0) continue;
//				if (dsumg < dsummin){dsummin = dsumg; dmin = dgrad;}
			}
			double a[9];
			a[0] = dx4; a[1] = dx3; a[2] = dx2;
			a[3] = dx3; a[4] = dx2; a[5] = dx;
			a[6] = dx2; a[7] = dx;  a[8] = 10;
			double dpeakx = 0, da = 0, devy = 0;
			if (InvMatrix(a, 3, 1E-6) == 0) {
				da = a[0]*dx2y + a[1]*dxy + a[2]*dy;
				double db = a[3]*dx2y + a[4]*dxy + a[5]*dy;
				double dc = a[6]*dx2y + a[7]*dxy + a[8]*dy;
				if (da > 0.01) {
					dpeakx = -db / (2 * da);
					//CString mmsg; mmsg.Format("%f %f %f %f", da, db, dc, dpeakx); AfxMessageBox(mmsg);
					if ((dpeakx >= 0) && (dpeakx <= dlimit)) {
						for (int k = 0; k < 10; k++) {//deviation from model
							double x = k * dstep;
							double d = da * x * x + db * x + dc;
							devy += (d - diff[k]) * (d - diff[k]);
						}
						//if ( sqrt(devy/10) < 0.5 * sqrt(dy2 / 10 -(dy * dy / 100)) ) {
						dsummin = GetImageDiff(psBinRefPixel, psBinQryPixel, ibin, cDelta, cGrad.X(-dpeakx),
							ibxref, ibyref, ibzref, ibxqry, ibyqry, ibzqry);
						dmin = dpeakx;
						cDelta.x -= cGrad.x * ibin * dmin;
						cDelta.y -= cGrad.y * ibin * dmin;
						cDelta.z -= cGrad.z * ibin * dmin;
						//					}
						devy = sqrt(devy / 10) / sqrt(dy2 / 10 - (dy * dy / 100));
					}
				}
			}
			CString msg2; 
			msg2.Format("Shift(x y z) RMSD  grad(x y z) x binning x step | peak 2ndOrder\n(%.1f %.1f %.1f) %.2f  (%.3f %.3f %.3f)x%dx%.1f | %.1f %.2f\n", 
				cDelta.x, cDelta.y, cDelta.z, (dsummin < 0) ? 0 : sqrt(dsummin), cGrad.x, cGrad.y, cGrad.z, ibin, dmin, dpeakx, da);
			msg2 += msg3 + "\n" + msg4 + "\n";
			msglog += msg2;
			if (dlg) {
				msg2.Replace("\n", "\r\n");
				dlg->m_Result += msg2;
				dlg->UpdateData(FALSE);
				ProcessMessage();
			}
			if (dmin < 0.2) break;
		}//jstep<5
	}//ibin
	if (psBinRefPixel) delete [] psBinRefPixel;
	if (psBinQryPixel) delete [] psBinQryPixel;
	//if (dlg) dlg->m_Result.Empty();
	CString msg;
	TReal minlsq = sqrt(dsummin); 
	msg.Format("ref(0 0 0)=qry(%.1f %.1f %.1f) rmsd=%.2f",  cDelta.x, cDelta.y, cDelta.z, minlsq);
	if (dlg) {
		if (dlg->bStarted) {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
			if (flog.m_hFile != CFile::hFileNull) {
				flog.WriteString(" Min: " + msg + "\n");
				flog.WriteString(msglog);
			}
			dlg->m_Result = msg + "\r\n--------------\r\n" + dlg->m_Result;
			ProcessMessage();
			rtn.Format("(%.1f %.1f %.1f)%.2f", cDelta.x, cDelta.y, cDelta.z, minlsq);
		} else {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: aborted");
		}
		dlg->bStarted = false;
	} else if (dqueue) {
		if (dqueue->iStatus != CDLGQUEUE_STOP) {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
			if (flog.m_hFile != CFile::hFileNull) {
				flog.WriteString(" Min: " + msg + "\n");
				flog.WriteString(msglog);
			}
			rtn.Format("(%.1f %.1f %.1f)%.2f", cDelta.x, cDelta.y, cDelta.z, minlsq);
		} else {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: aborted");
		}
	} else {
		if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
		if (flog.m_hFile != CFile::hFileNull) {
			flog.WriteString(" Min: " + msg + "\n");
			flog.WriteString(msglog);
		}
	}
	if (flog.m_hFile != CFile::hFileNull) {
		flog.WriteString("---------------------------------------------------\n");
		flog.Close();
	}
	//delete images
	for (int i=0; i<lq->nRefFiles; i++) {if (ppRefPixel[i]) delete [] ppRefPixel[i];}
	if (ppRefPixel) delete [] ppRefPixel;
	if (pMaxRefPixel) delete [] pMaxRefPixel;
	for (int i=0; i<lq->nQryFiles; i++) {if (ppQryPixel[i]) delete [] ppQryPixel[i];}
	if (ppQryPixel) delete [] ppQryPixel;
	if (pMaxQryPixel) delete [] pMaxQryPixel;
	return rtn;
}

unsigned __stdcall GetImageDiffThread(void* pArg) {
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) return 0;
	//
	short* psBinRefPixel = *(ri->ppRef);
	short* psBinQryPixel = *(ri->ppQry);
	//190102 int ibin = ri->max_d_ifp;
	int ibin = ri->iLsqBin;
	CXyz cDelta = *(ri->pcxyz1);
	CXyz cDisp = *(ri->pcxyz2);
	//190102 int ibxref = ri->max_d_igp;
	int ibxref = ri->iLsqBxref;
	int ibyref = ri->ixdim;
	int ibyref0 = ri->drStart;
	int ibyref1 = ri->drEnd;
	int ibzref = ri->iInterpolation;
	int ibxqry = ri->iLenSinogr;
	int ibyqry = ri->iMultiplex;
	int ibzqry = ri->iOffset;

	double dsum0 = 0; int icount = 0;
	const __int64 ibxyqry = ibxqry * ibyqry;
	const __int64 ibxyref = ibxref * ibyref;
	for (int iry=ibyref0; iry<ibyref1; iry++) {
	//for (int iry=0; iry<ibyref; iry++) {
		const int iqy = (int)(iry + cDelta.y/ibin + cDisp.y);
		const double dy = iry + cDelta.y/ibin + cDisp.y - iqy;
		if ((iqy < 0)||(iqy >= ibyqry)) continue;
		if (dy < 0) continue;
		for (int irz=0; irz<ibzref; irz++) {
			const int iqz = (int)(irz + cDelta.z/ibin + cDisp.z);
			const double dz = irz + cDelta.z/ibin + cDisp.z - iqz;
			if ((iqz < 0)||(iqz >= ibzqry)) continue;
			if (dz < 0) continue;
			const __int64 iqidx = iqz * ibxyqry + iqy * ibxqry;
			const __int64 iridx = irz * ibxyref + iry * ibxref;
			for (int irx=0; irx<ibxref; irx++) {
				const int iqx = (int)(irx + cDelta.x/ibin + cDisp.x);
				const double dx = irx + cDelta.x/ibin + cDisp.x - iqx;
				if ((iqx < 0)||(iqx >= ibxqry)) continue;
				if (dx < 0) continue;
				icount++;
				const int istepx = (iqx == ibxqry-1) ? 0 : 1;
				const int istepy = (iqy == ibyqry-1) ? 0 : ibxqry;
				short sa0 = psBinQryPixel[iqidx + iqx];
				short sa1 = psBinQryPixel[iqidx + iqx + istepx];
				short sa2 = psBinQryPixel[iqidx + iqx + istepy];
				short sa3 = psBinQryPixel[iqidx + iqx + istepx + istepy]; 
				double dpix0 = dx * (dy * sa3 + (1.0-dy) * sa1) + (1.0-dx) * (dy * sa2 + (1.0-dy) * sa0);
				const __int64 istepz = (iqz == ibzqry-1) ? 0 : ibxyqry;
				sa0 = psBinQryPixel[iqidx + istepz + iqx];
				sa1 = psBinQryPixel[iqidx + istepz + iqx + istepx];
				sa2 = psBinQryPixel[iqidx + istepz + iqx + istepy];
				sa3 = psBinQryPixel[iqidx + istepz + iqx + istepx + istepy]; 
				double dpix1 = dx * (dy * sa3 + (1.0-dy) * sa1) + (1.0-dx) * (dy * sa2 + (1.0-dy) * sa0);
				double dqrypix = dz * dpix1 + (1.0-dz) * dpix0;
				double diff = dqrypix - psBinRefPixel[iridx + irx];
				dsum0 += (diff * diff);
			}
		}
	}
	//if (icount) *(ri->pd1) = dsum0 / icount;
	//else *(ri->pd1) = -1;
	ri->i64sum = icount;
	ri->center = dsum0;
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

double CGazoApp::GetImageDiff(short* psBinRefPixel, short* psBinQryPixel, int ibin, CXyz cDelta, CXyz cDisp, 
							   int ibxref, int ibyref, int ibzref, int ibxqry, int ibyqry, int ibzqry) {
	double dsum0 = 0; int icount = 0;
	const __int64 ibxyqry = ibxqry * ibyqry;
	const __int64 ibxyref = ibxref * ibyref;
	/*
	for (int iry=0; iry<ibyref; iry++) {
		const int iqy = (int)(iry + cDelta.y/ibin + 0.5 + cDisp.y);
		if ((iqy < 0)||(iqy >= ibyqry)) continue;
		for (int irz=0; irz<ibzref; irz++) {
			const int iqz = (int)(irz + cDelta.z/ibin + 0.5 + cDisp.z);
			if ((iqz < 0)||(iqz >= ibzqry)) continue;
			const __int64 iqidx = iqz * ibxyqry + iqy * ibxqry;
			const __int64 iridx = irz * ibxyref + iry * ibxref;
			for (int irx=0; irx<ibxref; irx++) {
				int iqx = (int)(irx + cDelta.x/ibin + 0.5 + cDisp.x);
				if ((iqx < 0)||(iqx >= ibxqry)) continue;
				int iqry = psBinQryPixel[iqidx + iqx];
				if (iqry == SHRT_MIN) continue;
				int iref = psBinRefPixel[iridx + irx];
				if (iref == SHRT_MIN) continue;
				icount++;
				isum0 += ((iqry - iref) * (iqry - iref));
			}
		}
	}*/
	RECONST_INFO ri[MAX_CPU];
	const int nCPU = (int)(this->dlgProperty.iCPU);
	//const int nCPU=1;
	for (int i=nCPU-1; i>=0; i--) {
		ri[i].hThread = NULL;
		ri[i].iStartSino = i;
		//if (i) ri[i].bMaster = false; else ri[i].bMaster = true;
		ri[i].iStatus = RECONST_INFO_BUSY;
		//190102 ri[i].max_d_ifp = ibin;
		//190102 ri[i].max_d_igp = ibxref;
		ri[i].iLsqBin = ibin;
		ri[i].iLsqBxref = ibxref;
		ri[i].ixdim = ibyref;
		ri[i].drStart = (ibyref / nCPU) * i;
		ri[i].drEnd = (i == nCPU-1) ? ibyref : (ibyref / nCPU) * (i+1);
		ri[i].iInterpolation = ibzref;
		ri[i].iLenSinogr = ibxqry;
		ri[i].iMultiplex = ibyqry;
		ri[i].iOffset = ibzqry;
		ri[i].ppRef = &psBinRefPixel;
		ri[i].ppQry = &psBinQryPixel;
		ri[i].pcxyz1 = &cDelta;
		ri[i].pcxyz2 = &cDisp;
		ri[i].center = 0;//result
		ri[i].i64sum = 0;//result
		void* pArg = (void*)(&(ri[i]));
		if (i) {
			ri[i].hThread = (unsigned int)_beginthreadex( NULL, 0, GetImageDiffThread, pArg, 0, &(ri[i].threadID) );
		} else {
			GetImageDiffThread(&(ri[i]));
		}
	}//i
	int ist = RECONST_INFO_IDLE;
	do {
		ist = RECONST_INFO_IDLE;
		for (int i=nCPU-1; i>=0; i--) ist |= ri[i].iStatus;
	} while (ist != RECONST_INFO_IDLE);
	for (int i=nCPU-1; i>=0; i--) {
		if (ri[i].hThread) CloseHandle((HANDLE)(ri[i].hThread));
		dsum0 += ri[i].center;
		icount += (int)ri[i].i64sum;
	}
	/*
	for (int iry=0; iry<ibyref; iry++) {
		const int iqy = (int)(iry + cDelta.y/ibin + cDisp.y);
		const double dy = iry + cDelta.y/ibin + cDisp.y - iqy;
		if ((iqy < 0)||(iqy >= ibyqry)) continue;
		if (dy < 0) continue;
		for (int irz=0; irz<ibzref; irz++) {
			const int iqz = (int)(irz + cDelta.z/ibin + cDisp.z);
			const double dz = irz + cDelta.z/ibin + cDisp.z - iqz;
			if ((iqz < 0)||(iqz >= ibzqry)) continue;
			if (dz < 0) continue;
			const __int64 iqidx = iqz * ibxyqry + iqy * ibxqry;
			const __int64 iridx = irz * ibxyref + iry * ibxref;
			for (int irx=0; irx<ibxref; irx++) {
				const int iqx = (int)(irx + cDelta.x/ibin + cDisp.x);
				const double dx = irx + cDelta.x/ibin + cDisp.x - iqx;
				if ((iqx < 0)||(iqx >= ibxqry)) continue;
				if (dx < 0) continue;
				icount++;
				const int istepx = (iqx == ibxqry-1) ? 0 : 1;
				const int istepy = (iqy == ibyqry-1) ? 0 : ibxqry;
				short sa0 = psBinQryPixel[iqidx + iqx];
				short sa1 = psBinQryPixel[iqidx + iqx + istepx];
				short sa2 = psBinQryPixel[iqidx + iqx + istepy];
				short sa3 = psBinQryPixel[iqidx + iqx + istepx + istepy]; 
				double dpix0 = dx * (dy * sa3 + (1.0-dy) * sa1) + (1.0-dx) * (dy * sa2 + (1.0-dy) * sa0);
				const __int64 istepz = (iqz == ibzqry-1) ? 0 : ibxyqry;
				sa0 = psBinQryPixel[iqidx + istepz + iqx];
				sa1 = psBinQryPixel[iqidx + istepz + iqx + istepx];
				sa2 = psBinQryPixel[iqidx + istepz + iqx + istepy];
				sa3 = psBinQryPixel[iqidx + istepz + iqx + istepx + istepy]; 
				double dpix1 = dx * (dy * sa3 + (1.0-dy) * sa1) + (1.0-dx) * (dy * sa2 + (1.0-dy) * sa0);
				double dqrypix = dz * dpix1 + (1.0-dz) * dpix0;
				double diff = dqrypix - psBinRefPixel[iridx + irx];
				dsum0 += (diff * diff);
			}
		}
	}*/
	if (icount) return (dsum0 / icount);
	else return -1;
}

void CGazoApp::OnFileSavequeue()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	//Are there queues?
	int i = -1;
	bool bNoQueue = true;
	while ( (i = dlgQueue.m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
		if (dlgQueue.m_QueueList.GetItemText(i, 1) != "Finished") {bNoQueue = false; break;}
	}
	if (bNoQueue) {AfxMessageBox("No queue to save."); return;}
	//
	static char BASED_CODE defaultExt[] = ".que";
	static char BASED_CODE szFilter[] = "Queue files (*.que)|*.que|All files (*.*)|*.*||";
	CFileDialog dlgFile(FALSE, defaultExt, "recview.que", OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) return;
	const CString fname = dlgFile.GetPathName();
	FILE* fqueue = NULL;
	errno_t errn = fopen_s(&fqueue, fname, "wt");
	if (errn || (fqueue == NULL)) {AfxMessageBox("File open error."); return;}
	CStdioFile stdioQueue(fqueue);
	//
	dlgQueue.SaveQueue(&stdioQueue);
	//
	fclose(fqueue);
}

void CGazoApp::OnFileLoadqueue()
{
	static char BASED_CODE defaultExt[] = ".que";
	static char BASED_CODE szFilter[] = "Queue files (*.que)|*.que|All files (*.*)|*.*||";
	CFileDialog dlgFile(TRUE, defaultExt, "recview.que", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) return;
	const CString fname = dlgFile.GetPathName();
	FILE* fqueue = NULL;
	errno_t errn = fopen_s(&fqueue, fname, "rt");
	if (errn || (fqueue == NULL)) {AfxMessageBox("File open error."); return;}
	CStdioFile stdioQueue(fqueue);
	//
	dlgQueue.LoadQueue(&stdioQueue);
	//
	fclose(fqueue);
	//
	OnTomoQueue();//popup queue dialog to show loaded queues.
}

void CGazoApp::OnAppExit()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	//141231 only work from File-Exit menu
	CWinApp::OnAppExit();//this will call SaveAllModified and so on (?).
}

BOOL CGazoApp::SaveAllModified()
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	//AfxMessageBox("SaveAllModified");
	if (!dlgQueue.bIsSaved) {
		if (AfxMessageBox("Save remaining queues?", MB_YESNO) == IDYES) this->OnFileSavequeue();
	}
	return CWinApp::SaveAllModified();
}

void CGazoApp::OnUpdateViewBoxaxislabel(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
	pCmdUI->SetCheck(bShowBoxAxis);
}

//void CGazoApp::OnViewBoxaxislabel()
//{
//	// TODO: ここにコマンド ハンドラ コードを追加します。
//	if (bShowBoxAxis) bShowBoxAxis = false; else bShowBoxAxis = true;
//	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
//	if (pFrame == NULL) return;
//	CMDIChildWnd *pChild = NULL;
//	while (pChild = (CMDIChildWnd *) pFrame->GetActiveFrame()) {
//		if ((void*)pChild == (void*)pFrame) break;//if no child frames
//		CGazoView *pv = (CGazoView *) pChild->GetActiveView();
//		if (pv) pv->InvalidateRect(NULL, FALSE);
//	}
//}

void CGazoApp::OnViewDragscroll()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	if (bDragScroll) bDragScroll = false; else bDragScroll = true;
}

void CGazoApp::OnUpdateViewDragscroll(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
	pCmdUI->SetCheck(bDragScroll);
}


void CGazoApp::OnViewWheeltogo()
{
	// TODO: ここにコマンド ハンドラ コードを追加します。
	if (bWheelToGo) bWheelToGo = false; else bWheelToGo = true;
}

void CGazoApp::OnUpdateViewWheeltogo(CCmdUI *pCmdUI)
{
	// TODO: ここにコマンド更新 UI ハンドラ コードを追加します。
	pCmdUI->SetCheck(bWheelToGo);
}
