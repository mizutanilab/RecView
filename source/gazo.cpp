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
//#include <cutil_inline.h>

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

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGazoApp クラスの構築

//global params
CError error;
//int iThreadStatus;
//CGazoDoc* pDocThread;

CGazoApp::CGazoApp()
 : m_mutex(FALSE, _T("RecView4"), NULL)//131013
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	iStatus = 0;
	sProgVersion = "RecView ";
	//sProgVersion = "RecView v5.4.0";
#ifdef _WIN64
	sProgVersion += "x64\r\n";
#endif
#ifndef _WIN64
	sProgVersion += "x86\r\n";
#endif
	sProgVersion += "Build: ";
	sProgVersion += __DATE__;
	sProgVersion += " / ";
	sProgVersion += __TIME__;


	//SYSTEM_INFO si;
	//GetSystemInfo(&si);
	//iAvailableCPU = (int)si.dwNumberOfProcessors * 2;
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	//const int iHyperThreading = (CPUInfo[3] & 0x10000000) / 0x10000000;
	const int iLogicalProcessorCount = (CPUInfo[1] & 0x00ff0000) / 0x00010000;
	bool bSIMD = false;
//160918	if ( (CPUInfo[3] & 0x00800000) && //MMX
//		 (CPUInfo[3] & 0x02000000) && //SSE
//		 (CPUInfo[3] & 0x04000000) && //SSE2
//		 (CPUInfo[2] & 0x00000001) )  //SSE3
	if ( (CPUInfo[3] & 0x00800000) && //MMX
		 (CPUInfo[3] & 0x02000000) && //SSE
		 (CPUInfo[3] & 0x04000000) ) //SSE2
		 bSIMD = true;
	//131019===>
	iAvailableCPU = ::GetProcessorCoreCount();
	if (iAvailableCPU <= 0) iAvailableCPU = iLogicalProcessorCount > 1 ? iLogicalProcessorCount : 1;
	//===>131019
	//
	int iCUDAcount = GetCudaDeviceCount();
	if (iCUDAcount == CUDA_ERROR_INSUFFICIENT_DRIVER) {
		iCUDAcount = 0;
		AfxMessageBox("INSUFFICIENT CUDA DRIVER\r\nCuda GPU cannot be used.");//131022
	}
	int iCUDAblock = GetCudaMaxThreadsPerBlock();
	int iCUDAwarp = GetCudaWarpSize();
	int iATIcount, iATImaxwork, iATIunitwork;
	CLInitATIstreamDeviceInfo(&iATIcount, &iATImaxwork, &iATIunitwork);//DO NOT call this func twice
	//
	dlgProperty.Init(iAvailableCPU, bSIMD, 
						iCUDAcount, iCUDAblock, iCUDAwarp, 
						iATIcount, iATImaxwork, iATIunitwork);
	//prevPixelWidth = -1;
	bShowBoxAxis = true;
	bDragScroll = false;

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
	CFileDialog dlg(TRUE, defaultExt, fn, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, NULL);
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
	// TODO: ここにコマンド ハンドラ コードを追加します。
	dlgProperty.DoModal();
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
	CDlgLsqfit dlg;
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
	if (error.Report().IsEmpty()) AfxMessageBox("No error.");
	else AfxMessageBox("Errors:\r\n" + error.Report());//120720
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
	} else {
		flog.Close();
	}
	//AfxMessageBox(path_buffer); return rtn;//////////////
	if (flog.Open(path_buffer, CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::typeText)) {
		flog.SeekToEnd();
		TCHAR tctime[26];
		_tctime_s(tctime, 26, &(tstruct.time));
		const CString stime = tctime;
		CString line;
		line.Format("LSQ fit [%s] %s\r\n", stime.Left(24), this->sProgVersion);
		flog.WriteString(line);
		line.Format(" Ref set: %s [%d files]\r\n Qry set: %s [%d files]\r\n", lq->m_RefList.SpanExcluding(_T("\r\n")), lq->nRefFiles, 
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
						CString msg; msg.Format("%f (%d %d %d) %.0f/%.0f\r\n", rlsq, ix, iy, iz, dilsq, (TReal)nlsq);
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
						if (i) ri[i].bMaster = false; else ri[i].bMaster = true;
						ri[i].iStatus = RECONST_INFO_IDLE;
						ri[i].i64result = 0;//double
						ri[i].i64sum = 0;//int
						if (iz + i > lq->m_ZHigh) continue;
						ri[i].iStatus = RECONST_INFO_BUSY;
						ri[i].max_d_ifp = lq->nRefFiles;
						ri[i].max_d_igp = lq->nQryFiles;
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
							CString msg; msg.Format("%f (%d %d %d) %.0f/%d\r\n", rlsq, ix, iy, iz+i, dilsq, nlsq);
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
	for (int i=0; i<iLsqList; i++) {if (dlg) dlg->m_Result += " " + *(pLsqList[i]);}
	_ftime_s( &tstruct );
	TReal tcpu = tstruct.time + tstruct.millitm * 0.001 - tm0;
	CString msg;
	TReal minlsq = 0; int mx = 0, my = 0, mz = 0;
	sscanf_s(*(pLsqList[0]), "%lf (%d %d %d)", &minlsq, &mx, &my, &mz);
	msg.Format(" Min: ref(0 0 0)=qry(%d %d %d) rmsd=%f", mx, my, mz, minlsq);
	const CString headerStr = "\r\n RMSD (dx dy dz) SumDiff/NSum\r\n";
	if (dlg) {
		if (dlg->bStarted) {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
			if (flog.m_hFile != CFile::hFileNull) {
				flog.WriteString(msg + headerStr);
				for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]));}
			}
			dlg->m_Result = msg + headerStr + dlg->m_Result;
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
				flog.WriteString(msg + headerStr);
				for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]));}
			}
			rtn.Format("(%d %d %d)%.2f", mx, my, mz, minlsq);
		} else {
			if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: aborted");
		}
	} else {
		if (pf) pf->m_wndStatusBar.SetPaneText(1, "Lsqfit: " + msg);
		if (flog.m_hFile != CFile::hFileNull) {
			flog.WriteString(msg + headerStr);
			for (int i=0; i<30; i++) {flog.WriteString(" " + *(pLsqList[i]));}
		}
	}
	if (flog.m_hFile != CFile::hFileNull) {
		flog.WriteString("---------------------------------------------------\r\n");
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

