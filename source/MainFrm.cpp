// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "gazo.h"

#include "MainFrm.h"
#include "gazoView.h"
#include "gazoDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_DIALBOX, &CMainFrame::OnFileDialbox)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DIALBOX, OnDialbox)//161210
	ON_COMMAND(ID_ACCKEY_A, &CMainFrame::OnAcckeyA)
	ON_COMMAND(ID_ACCKEY_S, &CMainFrame::OnAcckeyS)
	ON_COMMAND(ID_ACCKEY_Z, &CMainFrame::OnAcckeyZ)
	ON_COMMAND(ID_ACCKEY_X, &CMainFrame::OnAcckeyX)
	ON_COMMAND(ID_ACCKEY_Q, &CMainFrame::OnAcckeyQ)
	ON_COMMAND(ID_ACCKEY_W, &CMainFrame::OnAcckeyW)
	ON_COMMAND(ID_ACCKEY_E, &CMainFrame::OnAcckeyE)
	ON_COMMAND(ID_ACCKEY_R, &CMainFrame::OnAcckeyR)
	ON_COMMAND(ID_ACCKEY_D, &CMainFrame::OnAcckeyD)
	ON_COMMAND(ID_ACCKEY_F, &CMainFrame::OnAcckeyF)
	ON_COMMAND(ID_ACCKEY_C, &CMainFrame::OnAcckeyC)
	ON_COMMAND(ID_ACCKEY_V, &CMainFrame::OnAcckeyV)
	ON_COMMAND(ID_ACCKEY_T, &CMainFrame::OnAcckeyT)
	ON_COMMAND(ID_ACCKEY_G, &CMainFrame::OnAcckeyG)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	// TODO: この位置にメンバの初期化処理コードを追加してください。
	dlgDialbox.pv = (CView*)this;	
	//161225 Accelerator
	mfrAccel = LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_MAINFRAME));
}

CMainFrame::~CMainFrame()
{
	dlgDialbox.CloseDialboxBluetoothSPP();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // 作成に失敗
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // 作成に失敗
	}
	//190119
	UINT nID, nStyle;
	int cxWidth;
	CDC* pDC = m_wndStatusBar.GetDC();
	CFont *pOldFont = pDC->SelectObject(m_wndStatusBar.GetFont());
	CSize size1 = pDC->GetTextExtent(CString('-', 110));
	CSize size2 = pDC->GetTextExtent("Center (0000 0000) Size (0000 0000) Tilt 000");
	pDC->SelectObject(pOldFont);
	m_wndStatusBar.ReleaseDC(pDC);
	m_wndStatusBar.GetPaneInfo(1, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(1, nID, nStyle, size1.cx);
	m_wndStatusBar.GetPaneInfo(2, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(2, nID, nStyle, size2.cx);

	// TODO: ツール バーをドッキング可能にしない場合は以下の３行を削除
	//       してください。
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: この位置で CREATESTRUCT cs を修正して、Window クラスやスタイルを
	//       修正してください。

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

void CMainFrame::OnFileDialbox()
{
	dlgDialbox.DoModal();
}

LRESULT CMainFrame::OnDialbox(WPARAM wParam, LPARAM lParam) {
	CMDIChildWnd *pChild = (CMDIChildWnd*)this->GetActiveFrame();
	CGazoView* pv = (CGazoView*)pChild->GetActiveView();
	if (!pv) return 0;
	CGazoDoc* pd = (CGazoDoc*)(pv->GetDocument());

	char* pcBuffer = (char*)wParam;
	char cCurrent;
	int iCont = 1;

	for (int j=0; j<COMM_BUFFER_LEN; j++) {
		cCurrent = pcBuffer[j];
		if (cCurrent == 0) break;
		if (j < COMM_BUFFER_LEN-1) {
			if (pcBuffer[j+1] == cCurrent) {iCont++; continue;}
		}
		bool bProcessed = false;
		for (int i=0; i<DIALBOX_NDIALS; i++) {
			int iStep = iCont;
			if (cCurrent == dlgDialbox.m_ucDialCCW[i]) iStep = -iCont;
			else if (cCurrent == dlgDialbox.m_ucDialCW[i]) iStep = iCont;
			else continue;
			bool bInvalidate = true;
			switch (dlgDialbox.m_ucDialAction[i]) {
				case DIALBOX_SCROLLX: {
					int ipos = pv->GetScrollPos(SB_HORZ);
					pv->SetScrollPos(SB_HORZ, ipos - iStep);
					break;}
				case DIALBOX_SCROLLY: {
					int ipos = pv->GetScrollPos(SB_VERT);
					pv->SetScrollPos(SB_VERT, ipos - iStep);
					break;}
				case DIALBOX_MAG: {
					if (iStep > 0) pv->OnToolbarMag(); else pv->OnToolbarMin();
					break;}
				case DIALBOX_FRAME: {
					if (pd) pd->ProceedImage(iStep);
					break;}
				case DIALBOX_FRAMEFAST: {
					if (pd) pd->ProceedImage(iStep * 20);
					break;}
				case DIALBOX_CONTRAST: {
					if (pd) pd->AdjContrast(iStep * 10);
					break;}
				case DIALBOX_BRIGHTNESS: {
					if (pd) pd->AdjBrightness(-iStep * 10);
					break;}
				case DIALBOX_NOACTION: {
					bInvalidate = false;
					break;}
			}//switch (dlgDialbox.m_ucDialAction[i])
			if (bInvalidate) pv->InvalidateRect(NULL, FALSE);
			bProcessed = true;
			break;
		}//for (int i=0; i<DIALBOX_NDIALS; i++)
		if (bProcessed) {iCont = 1; continue;}

		for (int i=0; i<DIALBOX_NBUTTONS; i++) {
			if (cCurrent != dlgDialbox.m_ucButtonRel[i]) continue;
			int iStep = iCont;
			bool bInvalidate = true;
			switch (dlgDialbox.m_ucButtonAction[i]) {
				case DIALBOX_OPENQUEUE: {
					CGazoApp* pApp = (CGazoApp*) AfxGetApp();
					pApp->OnTomoQueue();
					break;}
				case DIALBOX_NOBUTTONACTION: {
					bInvalidate = false;
					break;}
			}
			if (bInvalidate) pv->InvalidateRect(NULL, FALSE);
		}
		iCont = 1;
	}//for (int j=0; j<COMM_BUFFER_LEN; j++)

	return 0;
}

void CMainFrame::OnAcckeyA() {char pcBuffer[] = {'A', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyS() {char pcBuffer[] = {'S', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyZ() {char pcBuffer[] = {'Z', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyX() {char pcBuffer[] = {'X', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyQ() {char pcBuffer[] = {'Q', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyW() {char pcBuffer[] = {'W', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyE() {char pcBuffer[] = {'E', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyR() {char pcBuffer[] = {'R', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyD() {char pcBuffer[] = {'D', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyF() {char pcBuffer[] = {'F', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyC() {char pcBuffer[] = {'C', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyV() {char pcBuffer[] = {'V', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyG() {char pcBuffer[] = {'G', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}
void CMainFrame::OnAcckeyT() {char pcBuffer[] = {'T', 0}; OnDialbox(((WPARAM)pcBuffer), 0);}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (TranslateAccelerator(this->GetSafeHwnd(), mfrAccel, pMsg)) return TRUE;
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}
