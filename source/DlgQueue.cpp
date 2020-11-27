// DlgQueue.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgQueue.h"
#include "cerror.h"
#include "gazoDoc.h"
#include "MainFrm.h"
//201126
#include "DlgGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CError error;

/////////////////////////////////////////////////////////////////////////////
// CDlgQueue ダイアログ


CDlgQueue::CDlgQueue(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgQueue::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgQueue)
	m_FinalProc = FALSE;
	//}}AFX_DATA_INIT
	recQueue = NULL;
	nRecQueue = 0;
	maxRecQueue = 0;
	fmtQueue = NULL;
	nFmtQueue = 0;
	maxFmtQueue = 0;
	lsqfitQueue = NULL;
	nLsqfitQueue = 0;
	maxLsqfitQueue = 0;
	iRclickedItem = -1;
	iDragItem = -1;
	pd = NULL;
	bIsSaved = true;
}

CDlgQueue::~CDlgQueue() {
	if (recQueue) delete [] recQueue;
	for (int i=0; i<maxFmtQueue; i++) {
		LPTSTR tlp = fmtQueue[i].lpFileList;
		if (tlp) delete [] tlp;
	}
	if (fmtQueue) delete [] fmtQueue;
	if (lsqfitQueue) delete [] lsqfitQueue;
}

TErr CDlgQueue::AddRecQueue(RECONST_QUEUE* rq) {
	//if (nRecQueue >= CDLGQUEUE_MAXQUEUE) return 22405;
	TErr err = AllocRecQueue();
	if (err) return err;
	int idx = nRecQueue;
	for (int i=0; i<nRecQueue; i++) {if (!recQueue[i].bActive) {idx = i; break;}}
	recQueue[idx] = *rq;
	int item = m_QueueList.GetItemCount();
	if (err = InsertRecItem(item, idx, "Hold")) return err;
	recQueue[idx].bActive = true;
	bIsSaved = false;
	if (idx == nRecQueue) nRecQueue++;
	EnableCtrl();
//AfxMessageBox(recQueue[idx].dataPath + recQueue[idx].itexFilePrefix + recQueue[idx].itexFileSuffix);
	return 0;
}

TErr CDlgQueue::InsertRecItem(int item, DWORD_PTR rqIdx, CString status) {
	RECONST_QUEUE* rq = &(recQueue[rqIdx]);
	if (m_QueueList.InsertItem(item, "Reconst") < 0) return 22403;
	if (!m_QueueList.SetItemData(item, rqIdx)) return 22403;
	CString desc;
	desc.Format("%s Set %d / Layer %d-%d / Center %.1f-%.1f / Tilt %.1f",
		rq->dataPath, rq->iDatasetSel, rq->iLayer1, rq->iLayer2, rq->dCenter1, rq->dCenter2, rq->fTiltAngle);
	if (!m_QueueList.SetItemText(item, 2, desc)) return 22403;
	if (!m_QueueList.SetItemText(item, 1, status)) return 22403;
	////
	//if (rq->dataPath.Right(1) == "G") m_QueueList.SetItemText(item, 1, "Processing");
	////
	//if (!m_QueueList.SetItemText(item, 1, "Process")) return 22403;
	return 0;
}

TErr CDlgQueue::AllocRecQueue() {
	if (recQueue) {
		if (nRecQueue < maxRecQueue) return 0;
		RECONST_QUEUE* tqueue = recQueue;
		recQueue = new RECONST_QUEUE[maxRecQueue * 2];
		if (!recQueue) {recQueue = tqueue; return 22402;}
		for (int i=0; i<maxRecQueue; i++) {recQueue[i] = tqueue[i];}
		for (int i=maxRecQueue; i<maxRecQueue * 2; i++) {recQueue[i].bActive = false;}
		maxRecQueue *= 2;
		delete [] tqueue;
	} else {
		recQueue = new RECONST_QUEUE[CDLGQUEUE_INITQUEUE_ALLOC];
		if (!recQueue) return 22401;
		maxRecQueue = CDLGQUEUE_INITQUEUE_ALLOC;
		for (int i=0; i<maxRecQueue; i++) {recQueue[i].bActive = false;}
	}
	return 0;
}

TErr CDlgQueue::AddFmtQueue(FORMAT_QUEUE* fq) {
	//if (nFmtQueue >= CDLGQUEUE_MAXQUEUE) return 22405;
	TErr err = AllocFmtQueue();
	if (err) return err;
	int idx = nFmtQueue;
	for (int i=0; i<nFmtQueue; i++) {if (!fmtQueue[i].bActive) {idx = i; break;}}
	LPTSTR tlp = fmtQueue[idx].lpFileList;
	fmtQueue[idx] = *fq;
	if (!tlp) tlp = new TCHAR[MAX_FILE_DIALOG_LIST];
	if (!tlp) return 22405;
	fmtQueue[idx].lpFileList = tlp;
	if (fq->lpFileList) {
		for (int i=0; i<MAX_FILE_DIALOG_LIST; i++) {tlp[i] = fq->lpFileList[i];}
	}
	int item = m_QueueList.GetItemCount();
	if (err = InsertFmtItem(item, idx, "Hold")) return err;
	fmtQueue[idx].bActive = true;
	bIsSaved = false;
	if (idx == nFmtQueue) nFmtQueue++;
	EnableCtrl();
	return 0;
}

TErr CDlgQueue::InsertFmtItem(int item, DWORD_PTR fqIdx, CString status) {
	FORMAT_QUEUE* fq = &(fmtQueue[fqIdx]);
	if (m_QueueList.InsertItem(item, "OutputImage") < 0) return 22404;
	if (!m_QueueList.SetItemData(item, fqIdx)) return 22404;
	//120803 get the first file name
	CString sfinput = "";
	if (fq->nFiles > 1) {//141230
		for (int i=0; i<MAX_FILE_DIALOG_LIST; i++) {
			if (fq->lpFileList[i] == NULL) {
				sfinput = &(fq->lpFileList[i + 1]);
				break;
			}
		}
	}
	CString desc;
	desc.Format("%s / %s (%d files) / LAC %.2f-%.2f",
		fq->lpFileList, sfinput, fq->nFiles, fq->dLow, fq->dHigh);
	if (fq->bBoxEnabled) {
		CString line;
		line.Format(" / Box (%d,%d) %dx%d", fq->iBoxCentX, fq->iBoxCentY, fq->iBoxSizeX, fq->iBoxSizeY);
		desc += line;
	}
	if (fq->iOspDepth > 0) {
		CString line;
		line.Format(" / Cap %d LAC>%.1f", fq->iOspDepth, fq->dOspThreshold);
		desc += line;
	}
	if (!m_QueueList.SetItemText(item, 2, desc)) return 22404;
	if (!m_QueueList.SetItemText(item, 1, status)) return 22403;
	return 0;
}

TErr CDlgQueue::AllocFmtQueue() {
	if (fmtQueue) {
		if (nFmtQueue < maxFmtQueue) return 0;
		FORMAT_QUEUE* tqueue = fmtQueue;
		fmtQueue = new FORMAT_QUEUE[maxFmtQueue * 2];
		if (!fmtQueue) {fmtQueue = tqueue; return 22402;}
		for (int i=0; i<maxFmtQueue; i++) {fmtQueue[i] = tqueue[i];}
		for (int i=maxFmtQueue; i<maxFmtQueue * 2; i++) {
			fmtQueue[i].lpFileList = NULL;
			fmtQueue[i].bActive = false;
		}
		maxFmtQueue *= 2;
		//lpFileList
		delete [] tqueue;
	} else {
		fmtQueue = new FORMAT_QUEUE[CDLGQUEUE_INITQUEUE_ALLOC];
		if (!fmtQueue) return 22401;
		//lpFileList
		maxFmtQueue = CDLGQUEUE_INITQUEUE_ALLOC;
		for (int i=0; i<maxFmtQueue; i++) {
			fmtQueue[i].lpFileList = NULL;
			fmtQueue[i].bActive = false;
		}
	}
	return 0;
}

void CDlgQueue::EnableCtrl() {
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_QUEUE_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_QUEUE_FINAL)->EnableWindow(TRUE);
	GetDlgItem(IDC_QUEUE_PAUSE)->EnableWindow(TRUE);
	//180614 if (iStatus & CDLGQUEUE_PAUSE) GetDlgItem(IDOK)->SetWindowText("Resume");
	//else GetDlgItem(IDOK)->SetWindowText("Pause");
	if (iStatus & CDLGQUEUE_BUSY) {
		//GetDlgItem(IDOK)->SetWindowText("Pause");
		GetDlgItem(IDOK)->SetWindowText("Resume");
		return;
	}
	GetDlgItem(IDC_QUEUE_PAUSE)->EnableWindow(FALSE);
	//
	GetDlgItem(IDOK)->SetWindowText("Start");
	GetDlgItem(IDC_QUEUE_STOP)->EnableWindow(FALSE);
	//GetDlgItem(IDC_QUEUE_FINAL)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	const int nitem = m_QueueList.GetItemCount();
	for (int i=0; i<nitem; i++) {
		CString status = m_QueueList.GetItemText(i, 1);
		if ((status == "Hold")||(status == "Aborted")) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			break;
		}
	}
}

void CDlgQueue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgQueue)
	DDX_Control(pDX, IDC_QUEUE_LIST, m_QueueList);
	DDX_Check(pDX, IDC_QUEUE_FINAL, m_FinalProc);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgQueue, CDialog)
	//{{AFX_MSG_MAP(CDlgQueue)
	ON_NOTIFY(NM_RCLICK, IDC_QUEUE_LIST, OnRclickQueueList)
	ON_COMMAND(ID_POPUPQUEUE_DEL, OnPopupqueueDel)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_QUEUE_LIST, OnBegindragQueueList)
	ON_COMMAND(ID_POPUPQUEUE_UP, OnPopupqueueUp)
	ON_COMMAND(ID_POPUPQUEUE_DOWN, OnPopupqueueDown)
	ON_BN_CLICKED(IDC_QUEUE_STOP, OnQueueStop)
	ON_BN_CLICKED(IDC_QUEUE_FINAL, OnQueueFinal)
	ON_COMMAND(ID_POPUPQUEUE_APPENDSTOP, OnPopupqueueAppendstop)
	ON_COMMAND(ID_POPUPQUEUE_APPENDSLEEP, OnPopupqueueAppendsleep)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_QUEUE_PAUSE, &CDlgQueue::OnBnClickedQueuePause)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgQueue メッセージ ハンドラ

BOOL CDlgQueue::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString header = "Type";
	int idx = m_QueueList.InsertColumn(0, header, LVCFMT_LEFT, 60, 0);
	header = "Status";
	idx = m_QueueList.InsertColumn(1, header, LVCFMT_LEFT, 60, 1);
	header = "Description";
	idx = m_QueueList.InsertColumn(2, header, LVCFMT_LEFT, 370, 1);
	m_QueueList.SetExtendedStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	EnableCtrl();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDlgQueue::OnCancel() 
{
	ShowWindow(SW_HIDE);
	//DestroyWindow();
	
	//CDialog::OnCancel();
}

void CDlgQueue::OnBnClickedQueuePause()
{
	int iQueue = -1;
	CString sQueueStatus = "Processing";
	while ( (iQueue = m_QueueList.GetNextItem(iQueue, LVNI_ALL)) >= 0) {
		sQueueStatus = m_QueueList.GetItemText(iQueue, 1);
		if (sQueueStatus == "Processing") break;
	}
	if (iQueue >= 0) m_QueueList.SetItemText(iQueue, 1, "Pause");
	GetDlgItem(IDC_QUEUE_PAUSE)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	iStatus |= CDLGQUEUE_PAUSE;
	while (iStatus & CDLGQUEUE_BUSY) {
		::ProcessMessage();
		if (!(iStatus & CDLGQUEUE_PAUSE)) {
			GetDlgItem(IDC_QUEUE_PAUSE)->EnableWindow(TRUE);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			break;
		}
	}
	if (iQueue >= 0) m_QueueList.SetItemText(iQueue, 1, "Processing");
	return;
}

void CDlgQueue::OnOK() 
{
	if (iStatus & CDLGQUEUE_PAUSE) {
		iStatus &= ~CDLGQUEUE_PAUSE;
		return;
	}
	//180614
	//if (iStatus & CDLGQUEUE_BUSY) {
	//	GetDlgItem(IDC_QUEUE_RESUME)->EnableWindow(TRUE);
	//	GetDlgItem(IDOK)->EnableWindow(FALSE);
	//	iStatus |= CDLGQUEUE_PAUSE;
	//	while (iStatus & CDLGQUEUE_BUSY) {
	//		::ProcessMessage();
	//		if (!(iStatus & CDLGQUEUE_PAUSE)) {
	//			GetDlgItem(IDC_QUEUE_RESUME)->EnableWindow(FALSE);
	//			GetDlgItem(IDOK)->EnableWindow(TRUE);
	//			break;
	//		}
	//	}
	//	return;
	//}
	//131019===>
	int i = -1;
	CString sFirstQueueStatus = "Hold";
	while ( (i = m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
		sFirstQueueStatus = m_QueueList.GetItemText(i, 1);
		if ((sFirstQueueStatus == "Hold") || (sFirstQueueStatus == "Aborted")) break;
	}
	if (i < 0) return;
	const int iFirstQueue = i;
	m_QueueList.SetItemText(iFirstQueue, 1, "Waiting");
	//===>131019

	iStatus = CDLGQUEUE_BUSY;
	CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	pApp->SetBusy();
	EnableCtrl();
	const int nitem = m_QueueList.GetItemCount();
	TErr err = 0; CString msg;
	/*/130202 test code ===>
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	for (;;) {
		pf->m_wndToolBar.OnUpdateCmdUI(pf, TRUE);
		::ProcessMessage();
		if (iStatus == CDLGQUEUE_STOP) break;
	}
	pApp->SetIdle();
	iStatus = CDLGQUEUE_IDLE;
	EnableCtrl();
	return;
	///*///===>130202

	//131019===>
	//mutex
	CSingleLock slock(&(pApp->m_mutex), FALSE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	while (!slock.IsLocked()) {
		::ProcessMessage();
		if (this->iStatus == CDLGQUEUE_STOP) break;
		slock.Lock(100);
	}
//	GetDlgItem(IDOK)->EnableWindow(TRUE);
	m_QueueList.SetItemText(iFirstQueue, 1, sFirstQueueStatus);
	i = -1;
	//===>131019

	CGazoDoc doc;//Bringing doc into the while loop causes out of memory error though memory leaks were not deteced.
	while ( (i = m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
	//for (int i=0; i<nitem; i++) {
		if (this->iStatus == CDLGQUEUE_STOP) break;//131019
		//141229 CString mode = m_QueueList.GetItemText(i, 0);
		//141229 DWORD_PTR idata = m_QueueList.GetItemData(i);
		CString status = m_QueueList.GetItemText(i, 1);
		if (!(status == "Hold") && !(status == "Aborted")) continue;
		m_QueueList.SetItemText(i, 1, "Processing");
		const DWORD_PTR idx = m_QueueList.GetItemData(i);
		doc.ClearAll();
		CString lsqfitResult = "";
		if (m_QueueList.GetItemText(i, 0) == "Reconst") {
			RECONST_QUEUE* rq = &(recQueue[idx]);
			doc.SetQueueMode(true);
			doc.SetLogPath(rq->logFileName);
			doc.SetDataPath(rq->dataPath);
			if (rq->itexFileSuffix.MakeUpper() == ".H5") {//160628
				doc.SetDataSuffix(rq->itexFileSuffix);
				doc.SetDataPrefix(rq->itexFilePrefix);
			}
			//CFile file;
			//if (!file.Open(rq->filePath, CFile::modeRead | CFile::shareDenyWrite)) {
			//	m_QueueList.SetItemText(i, 1, "Error");
			//	continue;
			//}
			//doc.ReadFile(&file);
			//file.Close();
			////doc.SetPathName(rq->filePath, FALSE);
			doc.SetDimension(rq->iXdim, rq->iYdim);
			pd = &doc;
			if (err = doc.LoadLogFile((BOOL)(rq->bOffsetCT))) {
				msg.Format("Err%d", err);
				m_QueueList.SetItemText(i, 1, msg);
				error.Log(err);
				pd = NULL;
				continue;
			}
			if (err = doc.BatchReconst(rq)) {
				msg.Format("Err%d", err);
				m_QueueList.SetItemText(i, 1, msg);
				error.Log(err);
				pd = NULL;
				continue;
			}
			//120726 doc.DeleteAll();
			pd = NULL;
		} else if (m_QueueList.GetItemText(i, 0) == "OutputImage") {
			FORMAT_QUEUE* fq = &(fmtQueue[idx]);
			//120723 CGazoDoc doc;
			pd = &doc;
			CString rtn = "";
			if (err = doc.OutputImageInBox(fq, NULL, &rtn)) {
				msg.Format("Err%d", err);
				m_QueueList.SetItemText(i, 1, msg);
				error.Log(err);
			}
			//120726 doc.DeleteAll();
			pd = NULL;
			if (!rtn.IsEmpty()) {
				CString desc = m_QueueList.GetItemText(i, 2);
				m_QueueList.SetItemText(i, 2, rtn + " " + desc);
			}
		} else if (m_QueueList.GetItemText(i, 0) == "Lsqfit") {
			CString desc = m_QueueList.GetItemText(i, 2);
			LSQFIT_QUEUE* lq = &(lsqfitQueue[idx]);
			if (lq->m_bMaxDiameter) lsqfitResult = pApp->LsqfitMin(lq, NULL, this);//181214
			else lsqfitResult = pApp->Lsqfit(lq, NULL, this);
			if (desc.Left(3) == "ND ") desc = desc.Mid(3);
			m_QueueList.SetItemText(i, 2, lsqfitResult + " " + desc);
		} else if (m_QueueList.GetItemText(i, 0) == "Stop") {//201126
			m_QueueList.SetItemText(i, 1, "Finished");
			break;
		} else if (m_QueueList.GetItemText(i, 0) == "Sleep") {//201126
			CString desc = m_QueueList.GetItemText(i, 2);
			unsigned __int64 ullDur = m_QueueList.GetItemData(i);
			CTime cTime = CTime::GetCurrentTime();
			m_QueueList.SetItemText(i, 2, desc + cTime.Format(" started: %m/%d %H:%M:%S"));
			const time_t tTime0 = cTime.GetTime();
			unsigned __int64 ullDiff = 0;
			do {
				::ProcessMessage();
				Sleep(10);
				CTime ctCurr = CTime::GetCurrentTime();
				ullDiff = (unsigned __int64)(ctCurr.GetTime() - tTime0);
				if ((doc.dlgReconst.iStatus == CDLGRECONST_STOP)||(this->iStatus == CDLGQUEUE_STOP)) {
					m_QueueList.SetItemText(i, 2, desc);
					break;
				}
			} while (ullDiff < ullDur);
			m_QueueList.SetItemText(i, 1, "Finished");
		}
		//status control
		if (doc.dlgReconst.iStatus == CDLGRECONST_STOP) {
			//this->iStatus = CDLGQUEUE_STOP;
			m_QueueList.SetItemText(i, 1, "Aborted");
			break;
		} else if (this->iStatus == CDLGQUEUE_STOP) {//131019
			m_QueueList.SetItemText(i, 1, "Aborted");
			break;
		} else if (lsqfitResult == "ND") {
			m_QueueList.SetItemText(i, 1, "Aborted");
			break;
		} else {
			m_QueueList.SetItemText(i, 1, "Finished");
		}
		if (m_FinalProc) break;
		//if (iStatus & CDLGQUEUE_FINAL) break;
	}
	doc.DeleteAll();//120726
	pApp->SetIdle();
	iStatus = CDLGQUEUE_IDLE;
	slock.Unlock();//131019
	EnableCtrl();

	//update bIsSaved
	i = -1;
	bIsSaved = true;
	while ( (i = m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
		if (m_QueueList.GetItemText(i, 1) != "Finished") {bIsSaved = false; break;}
	}

	//CDialog::OnOK();
}

void CDlgQueue::OnQueueStop() 
{
	if (AfxMessageBox("Abort current process?", MB_OKCANCEL) == IDCANCEL) return;
	if (pd) pd->dlgReconst.iStatus = CDLGRECONST_STOP;
	iStatus = CDLGQUEUE_STOP;
}

void CDlgQueue::OnRclickQueueList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint point, treePoint;
	::GetCursorPos(&point);
	treePoint = point;
	m_QueueList.ScreenToClient(&treePoint);
	UINT nflg;
	iRclickedItem = m_QueueList.HitTest(treePoint, &nflg);
	const int nitem = m_QueueList.GetItemCount();
	int iFirstHold = 0;
	for (int i=0; i<nitem; i++) {
		CString status = m_QueueList.GetItemText(i, 1);
		if ((status == "Hold")||(status == "Aborted")) {
			iFirstHold = i; break;
		}
	}
	//CString msg; msg.Format("201126 %d", iFirstHold); AfxMessageBox(msg);
	bool bProcess = false, bFirst = false, bEnd = false;
	POSITION pos = m_QueueList.GetFirstSelectedItemPosition();
	while (pos) {
		int item = m_QueueList.GetNextSelectedItem(pos);
		if (item == iFirstHold) bFirst = true;
		if (item == nitem - 1) bEnd = true;
		CString status = m_QueueList.GetItemText(item, 1);
		if ((status != "Hold")&&(status != "Aborted")) bProcess = true;
	}
	//GetParentFrame()->ActivateFrame();
	//UpdateWindow();
	//return;
	CMenu menu;
	if (menu.LoadMenu(IDR_MENU_POPUP)) {
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		pPopup->EnableMenuItem(ID_POPUPQUEUE_DEL, MF_BYCOMMAND | MF_GRAYED);
		pPopup->EnableMenuItem(ID_POPUPQUEUE_UP, MF_BYCOMMAND | MF_GRAYED);
		pPopup->EnableMenuItem(ID_POPUPQUEUE_DOWN, MF_BYCOMMAND | MF_GRAYED);
		pPopup->EnableMenuItem(ID_POPUPQUEUE_APPENDSTOP, MF_BYCOMMAND | MF_GRAYED);//201126
		pPopup->EnableMenuItem(ID_POPUPQUEUE_APPENDSLEEP, MF_BYCOMMAND | MF_GRAYED);//201126
		if (iRclickedItem >= 0) {
			if (!bProcess) {
				pPopup->EnableMenuItem(ID_POPUPQUEUE_DEL, MF_BYCOMMAND | MF_ENABLED);
				if (!bFirst) pPopup->EnableMenuItem(ID_POPUPQUEUE_UP, MF_BYCOMMAND | MF_ENABLED);
				if (!bEnd) pPopup->EnableMenuItem(ID_POPUPQUEUE_DOWN, MF_BYCOMMAND | MF_ENABLED);
				if (!bEnd) pPopup->EnableMenuItem(ID_POPUPQUEUE_APPENDSTOP, MF_BYCOMMAND | MF_ENABLED);//201126
				if (!bEnd) pPopup->EnableMenuItem(ID_POPUPQUEUE_APPENDSLEEP, MF_BYCOMMAND | MF_ENABLED);//201126
			}
		}
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
														point.x, point.y, this); // use this dialog for cmds
//													point.x, point.y, AfxGetMainWnd()); // use main window for cmds
	}
	
	*pResult = 0;
}

void CDlgQueue::OnPopupqueueAppendstop() {//201126
	const int nitem = m_QueueList.GetItemCount();
	int ipos = nitem;
	if (iRclickedItem >= 0) {
		LVITEM item;
		for (int i = nitem - 1; i >= 0; i--) {
			item.iItem = i;
			item.mask = LVIF_STATE;
			item.stateMask = LVIS_SELECTED;
			m_QueueList.GetItem(&item);
			if (item.state & LVIS_SELECTED) {
				ipos = i+1; break;
			}
		}
	}
	if (m_QueueList.InsertItem(ipos, "Stop") < 0) return;
	if (!m_QueueList.SetItemData(ipos, 0)) return;
	//CString desc;
	//desc.Format("Stop");
	if (!m_QueueList.SetItemText(ipos, 2, "")) return;
	if (!m_QueueList.SetItemText(ipos, 1, "Hold")) return;
	bIsSaved = false;
	EnableCtrl();
	//AfxMessageBox("Appendstop");
}

void CDlgQueue::OnPopupqueueAppendsleep() {//201126
	const int nitem = m_QueueList.GetItemCount();
	int ipos = nitem;
	if (iRclickedItem >= 0) {
		LVITEM item;
		for (int i = nitem - 1; i >= 0; i--) {
			item.iItem = i;
			item.mask = LVIF_STATE;
			item.stateMask = LVIS_SELECTED;
			m_QueueList.GetItem(&item);
			if (item.state & LVIS_SELECTED) {
				ipos = i + 1; break;
			}
		}
	}
	CDlgGeneral dlg;
	dlg.m_sCaption1 = "Duration (sec)";
	dlg.m_sInput1 = "3600";
	if (dlg.DoModal() == IDCANCEL) {
		if (IsWindowVisible()) SetForegroundWindow();
		else ShowWindow(SW_SHOW);
		return;
	}
	const int idur = atoi(dlg.m_sInput1);
	if (idur <= 0) return;
	if (m_QueueList.InsertItem(ipos, "Sleep") < 0) return;
	if (!m_QueueList.SetItemData(ipos, idur)) return;
	CString desc;
	const int isec = idur % 60;
	int imin = idur / 60;
	int ihour = imin / 60;
	imin = imin % 60;
	desc.Format("%d sec (%d:%d:%d)", idur, ihour, imin, isec);
	if (!m_QueueList.SetItemText(ipos, 2, desc)) return;
	if (!m_QueueList.SetItemText(ipos, 1, "Hold")) return;
	bIsSaved = false;
	EnableCtrl();
	if (IsWindowVisible()) SetForegroundWindow();
	else ShowWindow(SW_SHOW);
	//AfxMessageBox("Appendstop");
}

void CDlgQueue::OnPopupqueueDel()
{
	if (iRclickedItem < 0) return;
	if (AfxMessageBox("Delete selected queue(s)?", MB_OKCANCEL) == IDCANCEL) return;
	const int nitem = m_QueueList.GetItemCount();
	LVITEM item;
	for (int i=nitem-1; i>=0; i--) {
		item.iItem = i;
		item.mask = LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		m_QueueList.GetItem(&item);
		if (item.state & LVIS_SELECTED) {
			DWORD_PTR idx = m_QueueList.GetItemData(i);
			CString mode = m_QueueList.GetItemText(i, 0);
			if (mode == "OutputImage") {
				//idx -= CDLGQUEUE_MAXQUEUE;
				if ((idx >= 0)&&(idx < (DWORD)nFmtQueue)) fmtQueue[idx].bActive = false;
			} else if (mode == "Reconst") {
				if ((idx >= 0)&&(idx < (DWORD)nRecQueue)) recQueue[idx].bActive = false;
			} else if (mode == "Lsqfit") {
				if ((idx >= 0)&&(idx < (DWORD)nLsqfitQueue)) lsqfitQueue[idx].bActive = false;
			}
			m_QueueList.DeleteItem(i);
		}
	}
	//update bIsSaved
	int i = -1;
	bIsSaved = true;
	while ( (i = m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
		if (m_QueueList.GetItemText(i, 1) != "Finished") {bIsSaved = false; break;}
	}
	//
	if (IsWindowVisible()) SetForegroundWindow();
	else ShowWindow(SW_SHOW);
}

void CDlgQueue::OnBegindragQueueList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	iDragItem = pNMListView->iItem;////????
	//CString line; line.Format("%d", iDragItem); AfxMessageBox(line);
	
	*pResult = 0;
}

void CDlgQueue::OnPopupqueueUp() 
{
	if (iRclickedItem < 0) return;
	const int nitem = m_QueueList.GetItemCount();
	int iFirstHold = 0;
	for (int i=0; i<nitem; i++) {
		CString status = m_QueueList.GetItemText(i, 1);
		if ((status == "Hold")||(status == "Aborted")) {
			iFirstHold = i; break;
		}
	}
	LVITEM item;
	int iDeletedItem = -1;
	for (int i=iFirstHold + 1; i<nitem; i++) {
		//if (m_QueueList.GetItemText(i, 1) == "Hold") continue;
		item.iItem = i;
		item.mask = LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		m_QueueList.GetItem(&item);
		if (item.state & LVIS_SELECTED) {
			DWORD_PTR idata = m_QueueList.GetItemData(i);
			CString status = m_QueueList.GetItemText(i, 1);
			CString mode = m_QueueList.GetItemText(i, 0);
			if (mode == "Reconst") {//rec item
				m_QueueList.DeleteItem(i);
				InsertRecItem(i-1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i-1;
			} else if (mode == "OutputImage") {
				m_QueueList.DeleteItem(i);
				InsertFmtItem(i-1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i-1;
			} else if (mode == "Lsqfit") {
				m_QueueList.DeleteItem(i);
				InsertLsqfitItem(i-1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i-1;
			} else if (mode == "Stop") {//201126
				m_QueueList.DeleteItem(i);
				if (m_QueueList.InsertItem(i - 1, mode) >= 0) {
					m_QueueList.SetItemData(i - 1, idata);
					m_QueueList.SetItemText(i - 1, 2, "");//comment
					m_QueueList.SetItemText(i - 1, 1, status);
				}
				if (iDeletedItem < 0) iDeletedItem = i - 1;
			} else if (mode == "Sleep") {//201126
				CString desc = m_QueueList.GetItemText(i, 2);
				m_QueueList.DeleteItem(i);
				if (m_QueueList.InsertItem(i - 1, mode) >= 0) {
					m_QueueList.SetItemData(i - 1, idata);
					m_QueueList.SetItemText(i - 1, 2, desc);
					m_QueueList.SetItemText(i - 1, 1, status);
				}
				if (iDeletedItem < 0) iDeletedItem = i - 1;
			}
		}
	}
	//
	//item.iItem = iRclickedItem;
	//item.mask = LVIF_STATE;
	if (iDeletedItem >= 0) {
		item.state = LVIS_FOCUSED;
		item.stateMask = LVIS_FOCUSED;
		m_QueueList.SetItemState(iDeletedItem, &item);
	}
	bIsSaved = false;
}

void CDlgQueue::OnPopupqueueDown() 
{
	if (iRclickedItem < 0) return;
	const int nitem = m_QueueList.GetItemCount();
	int iFirstHold = 0;
	for (int i=0; i<nitem; i++) {
		CString status = m_QueueList.GetItemText(i, 1);
		if ((status == "Hold")||(status == "Aborted")) {
			iFirstHold = i; break;
		}
	}
	LVITEM item;
	int iDeletedItem = -1;
	for (int i=nitem-2; i>=iFirstHold; i--) {
		//if (m_QueueList.GetItemText(i, 1) == "Hold") continue;
		item.iItem = i;
		item.mask = LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		m_QueueList.GetItem(&item);
		if (item.state & LVIS_SELECTED) {
			DWORD_PTR idata = m_QueueList.GetItemData(i);
			CString status = m_QueueList.GetItemText(i, 1);
			CString mode = m_QueueList.GetItemText(i, 0);
			if (mode == "Reconst") {//rec item
				m_QueueList.DeleteItem(i);
				InsertRecItem(i+1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i+1;
			} else if (mode == "OutputImage") {
				m_QueueList.DeleteItem(i);
				InsertFmtItem(i+1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i+1;
			} else if (mode == "Lsqfit") {
				m_QueueList.DeleteItem(i);
				InsertLsqfitItem(i+1, idata, status);
				if (iDeletedItem < 0) iDeletedItem = i+1;
			} else if (mode == "Stop") {//201126
				m_QueueList.DeleteItem(i);
				if (m_QueueList.InsertItem(i + 1, mode) >= 0) {
					m_QueueList.SetItemData(i + 1, idata);
					m_QueueList.SetItemText(i + 1, 2, "");//comment
					m_QueueList.SetItemText(i + 1, 1, status);
				}
				if (iDeletedItem < 0) iDeletedItem = i + 1;
			} else if (mode == "Sleep") {//201126
				CString desc = m_QueueList.GetItemText(i, 2);
				m_QueueList.DeleteItem(i);
				if (m_QueueList.InsertItem(i + 1, mode) >= 0) {
					m_QueueList.SetItemData(i + 1, idata);
					m_QueueList.SetItemText(i + 1, 2, desc);
					m_QueueList.SetItemText(i + 1, 1, status);
				}
				if (iDeletedItem < 0) iDeletedItem = i + 1;
			}
		}
	}
	//
	if (iDeletedItem >= 0) {
		item.state = LVIS_FOCUSED;
		item.stateMask = LVIS_FOCUSED;
		m_QueueList.SetItemState(iDeletedItem, &item);
	}
	bIsSaved = false;
}


void CDlgQueue::OnQueueFinal() 
{
	UpdateData();
	//if (m_FinalProc) AfxMessageBox("final");
}

TErr CDlgQueue::AddLsqfitQueue(LSQFIT_QUEUE* lq) {
	TErr err = AllocLsqfitQueue();
	if (err) return err;
	int idx = nLsqfitQueue;
	for (int i=0; i<nLsqfitQueue; i++) {if (!lsqfitQueue[i].bActive) {idx = i; break;}}
	lsqfitQueue[idx] = *lq;
	int item = m_QueueList.GetItemCount();
	if (err = InsertLsqfitItem(item, idx, "Hold")) return err;
	lsqfitQueue[idx].bActive = true;
	bIsSaved = false;
	if (idx == nLsqfitQueue) nLsqfitQueue++;
	EnableCtrl();
	return 0;
}

TErr CDlgQueue::InsertLsqfitItem(int item, DWORD_PTR lqIdx, CString status) {
	LSQFIT_QUEUE* lq = &(lsqfitQueue[lqIdx]);
	if (m_QueueList.InsertItem(item, "Lsqfit") < 0) return 22403;
	if (!m_QueueList.SetItemData(item, lqIdx)) return 22403;
	CString desc;
	desc.Format("%s [%d] + %s [%d]", lq->m_RefList.SpanExcluding(_T("\r\n")), lq->nRefFiles, 
										lq->m_QryList.SpanExcluding(_T("\r\n")), lq->nQryFiles);
	if (!m_QueueList.SetItemText(item, 2, desc)) return 22403;
	if (!m_QueueList.SetItemText(item, 1, status)) return 22403;
	return 0;
}

TErr CDlgQueue::AllocLsqfitQueue() {
	if (lsqfitQueue) {
		if (nLsqfitQueue < maxLsqfitQueue) return 0;
		LSQFIT_QUEUE* tqueue = lsqfitQueue;
		lsqfitQueue = new LSQFIT_QUEUE[maxLsqfitQueue * 2];
		if (!lsqfitQueue) {lsqfitQueue = tqueue; return 22402;}
		for (int i=0; i<maxLsqfitQueue; i++) {lsqfitQueue[i] = tqueue[i];}
		for (int i=maxLsqfitQueue; i<maxLsqfitQueue * 2; i++) {lsqfitQueue[i].bActive = false;}
		maxLsqfitQueue *= 2;
		delete [] tqueue;
	} else {
		lsqfitQueue = new LSQFIT_QUEUE[CDLGQUEUE_INITQUEUE_ALLOC];
		if (!lsqfitQueue) return 22401;
		maxLsqfitQueue = CDLGQUEUE_INITQUEUE_ALLOC;
		for (int i=0; i<maxLsqfitQueue; i++) {lsqfitQueue[i].bActive = false;}
	}
	return 0;
}

TErr CDlgQueue::SaveQueue(CStdioFile* fp) {
	TErr err = 0;
	if (fp == NULL) return 22411;
	//fp->WriteString("test");
	CString line;
	int i = -1;
	while ( (i = m_QueueList.GetNextItem(i, LVNI_ALL)) >= 0) {
		const CString status = m_QueueList.GetItemText(i, 1);
		if (!(status == "Hold") && !(status == "Aborted")) continue;
		const CString mode = m_QueueList.GetItemText(i, 0);
		const CString desc = m_QueueList.GetItemText(i, 2);
		line.Format("%s %s\r\n", mode, desc);
		fp->WriteString(line);
		const DWORD_PTR idx = m_QueueList.GetItemData(i);
		if (mode == "Reconst") {
			RECONST_QUEUE* rq = &(recQueue[idx]);
			//rq->bActive; this must be set TRUE when loading
			//line.Format(" %\r\n", rq->); fp->WriteString(line);
			//if (rq->) fp->WriteString(" TRUE\r\n"); else fp->WriteString(" FALSE\r\n");
			if (rq->bOffsetCT) fp->WriteString("bOffsetCT TRUE\r\n");
			else fp->WriteString("bOffsetCT FALSE\r\n");
			if (rq->bReconOptionUpdated) fp->WriteString("bReconOptionUpdated TRUE\r\n");
			else fp->WriteString("bReconOptionUpdated FALSE\r\n");
			line.Format("dataPath %s\r\n", rq->dataPath); fp->WriteString(line);
			line.Format("dCenter1 %lf\r\n", rq->dCenter1); fp->WriteString(line);
			line.Format("dCenter2 %lf\r\n", rq->dCenter2); fp->WriteString(line);
			line.Format("dAxisInc %lf\r\n", rq->dAxisInc); fp->WriteString(line);
			line.Format("dCutoff %lf\r\n", rq->dCutoff); fp->WriteString(line);
			line.Format("dOrder %lf\r\n", rq->dOrder); fp->WriteString(line);
			line.Format("dPixelWidth %lf\r\n", rq->dPixelWidth); fp->WriteString(line);
			line.Format("dReconFlags %u\r\n", rq->dReconFlags); fp->WriteString(line);
			line.Format("drEnd %d\r\n", rq->drEnd); fp->WriteString(line);
			if (rq->drOmit) fp->WriteString("drOmit TRUE\r\n"); else fp->WriteString("drOmit FALSE\r\n");
			line.Format("drStart %d\r\n", rq->drStart); fp->WriteString(line);
			line.Format("drX %lf\r\n", rq->drX); fp->WriteString(line);
			line.Format("drY %lf\r\n", rq->drY); fp->WriteString(line);
			line.Format("fTiltAngle %f\r\n", rq->fTiltAngle); fp->WriteString(line);
			line.Format("iDatasetSel %d\r\n", rq->iDatasetSel); fp->WriteString(line);
			line.Format("iFilter %d\r\n", rq->iFilter); fp->WriteString(line);
			line.Format("iInterpolation %d\r\n", rq->iInterpolation); fp->WriteString(line);
			line.Format("iLayer1 %d\r\n", rq->iLayer1); fp->WriteString(line);
			line.Format("iLayer2 %d\r\n", rq->iLayer2); fp->WriteString(line);
			line.Format("iLossFrameSet %d\r\n", rq->iLossFrameSet); fp->WriteString(line);
			line.Format("iSinoXdim %d\r\n", rq->iRawSinoXdim); fp->WriteString(line);
			line.Format("iSinoYdim %d\r\n", rq->iSinoYdim); fp->WriteString(line);
			line.Format("itexFilePrefix %s\r\n", rq->itexFilePrefix); fp->WriteString(line);
			line.Format("itexFileSuffix %s\r\n", rq->itexFileSuffix); fp->WriteString(line);
			line.Format("iTrimWidth %d\r\n", rq->iTrimWidth); fp->WriteString(line);
			line.Format("iXdim %d\r\n", rq->iXdim); fp->WriteString(line);
			line.Format("iYdim %d\r\n", rq->iYdim); fp->WriteString(line);
			line.Format("logFileName %s\r\n", rq->logFileName); fp->WriteString(line);
			line.Format("outFilePath %s\r\n", rq->outFilePath); fp->WriteString(line);
			line.Format("outFilePrefix %s\r\n", rq->outFilePrefix); fp->WriteString(line);
			line.Format("sDriftListPath %s\r\n", rq->sDriftListPath); fp->WriteString(line);
			line.Format("sFramesToExclude %s\r\n", rq->sFramesToExclude); fp->WriteString(line);
			line.Format("iSampleFrameStart %d\r\n", rq->iSampleFrameStart); fp->WriteString(line);
			line.Format("iSampleFrameEnd %d\r\n", rq->iSampleFrameEnd); fp->WriteString(line);
		} else if (mode == "OutputImage") {
			FORMAT_QUEUE* fq = &(fmtQueue[idx]);
			//line.Format(" %\r\n", fq->); fp->WriteString(line);
			//if (fq->) fp->WriteString(" TRUE\r\n"); else fp->WriteString(" FALSE\r\n");
			line.Format("nFiles %d\r\n", fq->nFiles); fp->WriteString(line);
			LPTSTR lpFile = fq->lpFileList;
			//180622 line.Format("lpFileList %s", lpFile);
			line.Format("%s", lpFile);
			if (line.Find(' ') >= 0) line = "lpFileList \"" + line + "\"";
			else line = "lpFileList " + line;
			bool bNull = false;
			if (fq->nFiles > 1) {//multiple selection
				bNull = false;
				int idx = 0;
				for (int j=0; j<MAX_FILE_DIALOG_LIST; j++) {
					if (lpFile[j] == NULL) {
						if (bNull) break;
						bNull = true; idx++; CString sfname = (char*)(&(lpFile[j + 1])); line += " " + sfname;
						//AfxMessageBox(fname);
						if (idx >= fq->nFiles) break;
					} else {
						bNull = false;
					}
				}
			}
			//AfxMessageBox(line);
			fp->WriteString(line + "\r\n");
			if (fq->uiFlags & FQFLAGS_16BIT) fp->WriteString("b16bit TRUE\r\n"); else fp->WriteString("b16bit FALSE\r\n");
			if (fq->uiFlags & FQFLAGS_OUTPUT_HISTG) fp->WriteString("bOutHistg TRUE\r\n"); else fp->WriteString("bOutHistg FALSE\r\n");
			//if (fq->b16bit) fp->WriteString("b16bit TRUE\r\n"); else fp->WriteString("b16bit FALSE\r\n");
			if (fq->bBoxEnabled) fp->WriteString("bBoxEnabled TRUE\r\n"); else fp->WriteString("bBoxEnabled FALSE\r\n");
			line.Format("dHigh %lf\r\n", fq->dHigh); fp->WriteString(line);
			line.Format("dLow %lf\r\n", fq->dLow); fp->WriteString(line);
			line.Format("dOspThreshold %lf\r\n", fq->dOspThreshold); fp->WriteString(line);
			line.Format("iAverage %d\r\n", fq->iAverage); fp->WriteString(line);
			line.Format("iBoxAngle %d\r\n", fq->iBoxAngle); fp->WriteString(line);
			line.Format("iBoxCentX %d\r\n", fq->iBoxCentX); fp->WriteString(line);
			line.Format("iBoxCentY %d\r\n", fq->iBoxCentY); fp->WriteString(line);
			line.Format("iBoxSizeX %d\r\n", fq->iBoxSizeX); fp->WriteString(line);
			line.Format("iBoxSizeY %d\r\n", fq->iBoxSizeY); fp->WriteString(line);
			line.Format("iOspDepth %d\r\n", fq->iOspDepth); fp->WriteString(line);
			line.Format("iXdim %d\r\n", fq->iXdim); fp->WriteString(line);
			line.Format("iYdim %d\r\n", fq->iYdim); fp->WriteString(line);
			line.Format("outFilePrefix %s\r\n", fq->outFilePrefix); fp->WriteString(line);
			int ipos = 0;
			if (!fq->sPolygonList.IsEmpty()) {
				for (;;) {
					const CString sEntry = fq->sPolygonList.Tokenize(_T("\r\n"), ipos);
					if (sEntry.IsEmpty()) break;
					line.Format("sPolygonList %s\r\n", sEntry); fp->WriteString(line);
				}
			}
		} else if (mode == "Lsqfit") {
			LSQFIT_QUEUE* lq = &(lsqfitQueue[idx]);
			//lq->bActive; this must be set TRUE when loading
			//if (lq->) fp->WriteString(" TRUE\r\n"); else fp->WriteString(" FALSE\r\n");
			//line.Format(" %\r\n", lq->); fp->WriteString(line);
			line.Format("nRefFiles %d\r\n", lq->nRefFiles); fp->WriteString(line);
			line.Format("m_RefList\r\n%s\r\n", lq->m_RefList); fp->WriteString(line);
			line.Format("nQryFiles %d\r\n", lq->nQryFiles); fp->WriteString(line);
			line.Format("m_QryList\r\n%s\r\n", lq->m_QryList); fp->WriteString(line);
			line.Format("m_XLow %d\r\n", lq->m_XLow); fp->WriteString(line);
			line.Format("m_XHigh %d\r\n", lq->m_XHigh); fp->WriteString(line);
			line.Format("m_YLow %d\r\n", lq->m_YLow); fp->WriteString(line);
			line.Format("m_YHigh %d\r\n", lq->m_YHigh); fp->WriteString(line);
			line.Format("m_ZLow %d\r\n", lq->m_ZLow); fp->WriteString(line);
			line.Format("m_ZHigh %d\r\n", lq->m_ZHigh); fp->WriteString(line);
			if (lq->m_bMaxDiameter) fp->WriteString("bMaxDiameter TRUE\r\n"); else fp->WriteString("bMaxDiameter FALSE\r\n");
		} else if (mode == "Stop") {//201126
			//
		} else if (mode == "Sleep") {//201126
			const int idur = m_QueueList.GetItemData(i);
			line.Format("Duration %d\r\n", idur); fp->WriteString(line);
		}
		fp->WriteString("End\r\n");
	}
	if (!err) bIsSaved = true;
	return err;
}

TErr CDlgQueue::LoadQueue(CStdioFile* fp) {
	TErr err = 0;
	//AfxMessageBox("141230 LoadQueue");
	if (fp == NULL) return 22411;
	CString line;
	CString msg = "";
	int i = -1;
	while (fp->ReadString(line)) {
		CString mode = line.SpanExcluding(" \t");
		if (mode == "Reconst") {
			RECONST_QUEUE rq;
			while (fp->ReadString(line)) {
				CString cmd = line.SpanExcluding(" \t\r\n");
				CString param = line.Mid(cmd.GetLength());
				param.TrimLeft(); param.TrimRight();
				//AfxMessageBox(cmd + "/" + param + "/");
				if (cmd == "End") {break;
				} else if (cmd == "bOffsetCT") {if (param == "TRUE") rq.bOffsetCT = TRUE; else if (param == "FALSE") rq.bOffsetCT = FALSE; else msg += line;
				} else if (cmd == "bReconOptionUpdated") {if (param == "TRUE") rq.bReconOptionUpdated = TRUE; else if (param == "FALSE") rq.bReconOptionUpdated = FALSE; else msg += line;
				} else if (cmd == "dataPath") {rq.dataPath = param;
				} else if (cmd == "dCenter1") {if (sscanf_s(param, "%lf", &(rq.dCenter1)) < 1) msg += line;
				} else if (cmd == "dCenter2") {if (sscanf_s(param, "%lf", &(rq.dCenter2)) < 1) msg += line;
				} else if (cmd == "dAxisInc") {if (sscanf_s(param, "%lf", &(rq.dAxisInc)) < 1) msg += line;
				} else if (cmd == "dCutoff") {if (sscanf_s(param, "%lf", &(rq.dCutoff)) < 1) msg += line;
				} else if (cmd == "dOrder") {if (sscanf_s(param, "%lf", &(rq.dOrder)) < 1) msg += line;
				} else if (cmd == "dPixelWidth") {if (sscanf_s(param, "%lf", &(rq.dPixelWidth)) < 1) msg += line;
				} else if (cmd == "dReconFlags") {if (sscanf_s(param, "%u", &(rq.dReconFlags)) < 1) msg += line;
				} else if (cmd == "drEnd") {if (sscanf_s(param, "%d", &(rq.drEnd)) < 1) msg += line;
				} else if (cmd == "drOmit") {if (param == "TRUE") rq.drOmit = TRUE; else if (param == "FALSE") rq.drOmit = FALSE; else msg += line;
				} else if (cmd == "drStart") {if (sscanf_s(param, "%d", &(rq.drStart)) < 1) msg += line;
				} else if (cmd == "drX") {if (sscanf_s(param, "%lf", &(rq.drX)) < 1) msg += line;
				} else if (cmd == "drY") {if (sscanf_s(param, "%lf", &(rq.drY)) < 1) msg += line;
				} else if (cmd == "fTiltAngle") {if (sscanf_s(param, "%f", &(rq.fTiltAngle)) < 1) msg += line;
				} else if (cmd == "iDatasetSel") {if (sscanf_s(param, "%d", &(rq.iDatasetSel)) < 1) msg += line;
				} else if (cmd == "iFilter") {if (sscanf_s(param, "%d", &(rq.iFilter)) < 1) msg += line;
				} else if (cmd == "iInterpolation") {if (sscanf_s(param, "%d", &(rq.iInterpolation)) < 1) msg += line;
				} else if (cmd == "iLayer1") {if (sscanf_s(param, "%d", &(rq.iLayer1)) < 1) msg += line;
				} else if (cmd == "iLayer2") {if (sscanf_s(param, "%d", &(rq.iLayer2)) < 1) msg += line;
				} else if (cmd == "iLossFrameSet") {if (sscanf_s(param, "%d", &(rq.iLossFrameSet)) < 1) msg += line;
				} else if (cmd == "iSinoXdim") {if (sscanf_s(param, "%d", &(rq.iRawSinoXdim)) < 1) msg += line;
				} else if (cmd == "iSinoYdim") {if (sscanf_s(param, "%d", &(rq.iSinoYdim)) < 1) msg += line;
				} else if (cmd == "itexFilePrefix") {rq.itexFilePrefix = param;
				} else if (cmd == "itexFileSuffix") {rq.itexFileSuffix = param;
				} else if (cmd == "iTrimWidth") {if (sscanf_s(param, "%d", &(rq.iTrimWidth)) < 1) msg += line;
				} else if (cmd == "iXdim") {if (sscanf_s(param, "%d", &(rq.iXdim)) < 1) msg += line;
				} else if (cmd == "iYdim") {if (sscanf_s(param, "%d", &(rq.iYdim)) < 1) msg += line;
				} else if (cmd == "logFileName") {rq.logFileName = param;
				} else if (cmd == "outFilePath") {rq.outFilePath = param;
				} else if (cmd == "outFilePrefix") {rq.outFilePrefix = param;
				} else if (cmd == "sDriftListPath") {rq.sDriftListPath = param;
				} else if (cmd == "sFramesToExclude") {rq.sFramesToExclude = " " + param; //AfxMessageBox("=" + rq.sFramesToExclude + "=");
				} else if (cmd == "iSampleFrameStart") {if (sscanf_s(param, "%d", &(rq.iSampleFrameStart)) < 1) msg += line;
				} else if (cmd == "iSampleFrameEnd") {if (sscanf_s(param, "%d", &(rq.iSampleFrameEnd)) < 1) msg += line;
				} else {msg += line;
				}
			}
			if (err = AddRecQueue(&rq)) break;
		} else if (mode == "OutputImage") {
			//FORMAT_QUEUE* fq = &(fmtQueue[idx]);
			FORMAT_QUEUE fq;
			fq.uiFlags = 0;
			fq.lpFileList = new TCHAR[MAX_FILE_DIALOG_LIST];
			//180622 _tcscpy_s(fq.lpFileList, MAX_FILE_DIALOG_LIST, "");
			memset(fq.lpFileList, 0, MAX_FILE_DIALOG_LIST);
			fq.sPolygonList.Empty();
			while (fp->ReadString(line)) {
				CString cmd = line.SpanExcluding(" \t\r\n");
				CString param = line.Mid(cmd.GetLength());
				param.TrimLeft(); param.TrimRight();
				int iFileCount = 0;
				//AfxMessageBox(cmd + "/" + param + "/");
				if (cmd == "End") {break;
				} else if (cmd == "nFiles") {if (sscanf_s(param, "%d", &(fq.nFiles)) < 1) msg += line;
				//} else if (cmd == "b16bit") {if (param == "TRUE") fq.b16bit = TRUE; else if (param == "FALSE") fq.b16bit = FALSE; else msg += line;
				} else if (cmd == "b16bit") {if (param == "TRUE") fq.uiFlags |= FQFLAGS_16BIT; else if (param != "FALSE") msg += line;
				} else if (cmd == "bOutHistg") {if (param == "TRUE") fq.uiFlags |= FQFLAGS_OUTPUT_HISTG; else if (param != "FALSE") msg += line;
				} else if (cmd == "bBoxEnabled") {if (param == "TRUE") fq.bBoxEnabled = TRUE; else if (param == "FALSE") fq.bBoxEnabled = FALSE; else msg += line;
				} else if (cmd == "dHigh") {if (sscanf_s(param, "%lf", &(fq.dHigh)) < 1) msg += line;
				} else if (cmd == "dLow") {if (sscanf_s(param, "%lf", &(fq.dLow)) < 1) msg += line;
				} else if (cmd == "dOspThreshold") {if (sscanf_s(param, "%lf", &(fq.dOspThreshold)) < 1) msg += line;
				} else if (cmd == "iAverage") {if (sscanf_s(param, "%d", &(fq.iAverage)) < 1) msg += line;
				} else if (cmd == "iBoxAngle") {if (sscanf_s(param, "%d", &(fq.iBoxAngle)) < 1) msg += line;
				} else if (cmd == "iBoxCentX") {if (sscanf_s(param, "%d", &(fq.iBoxCentX)) < 1) msg += line;
				} else if (cmd == "iBoxCentY") {if (sscanf_s(param, "%d", &(fq.iBoxCentY)) < 1) msg += line;
				} else if (cmd == "iBoxSizeX") {if (sscanf_s(param, "%d", &(fq.iBoxSizeX)) < 1) msg += line;
				} else if (cmd == "iBoxSizeY") {if (sscanf_s(param, "%d", &(fq.iBoxSizeY)) < 1) msg += line;
				} else if (cmd == "iOspDepth") {if (sscanf_s(param, "%d", &(fq.iOspDepth)) < 1) msg += line;
				} else if (cmd == "iXdim") {if (sscanf_s(param, "%d", &(fq.iXdim)) < 1) msg += line;
				} else if (cmd == "iYdim") {if (sscanf_s(param, "%d", &(fq.iYdim)) < 1) msg += line;
				} else if (cmd == "outFilePrefix") {fq.outFilePrefix = param;
				} else if (cmd == "sPolygonList") {fq.sPolygonList += param + "\r\n";
				} else if (cmd == "lpFileList") {
					//180622
					CString fname;
					if ((param.GetAt(0) == '"')&&(param.GetLength() > 1)) {
						fname = "\"" + param.Mid(1).SpanExcluding("\"") + "\"";
					} else {
						fname = param.SpanExcluding(" \t");
					}
					if (fname.IsEmpty()) {msg += line; continue;} else iFileCount = 1;
					int idx = fname.GetLength();
					_tcscpy_s(fq.lpFileList, idx+1, fname);
					param = param.Mid(idx); param.TrimLeft();
					while (param.GetLength()) {
						fname = param.SpanExcluding(" \t");
						if (fname.IsEmpty()) break;
						iFileCount++;
						const int ilen = fname.GetLength();
						_tcscpy_s(&(fq.lpFileList[idx+1]), ilen+1, fname);
						param = param.Mid(ilen); param.TrimLeft();
						idx += (ilen + 1);
					}
				} else {msg += line;
				}
			}
			/*/test code
			LPTSTR lpFile = fq.lpFileList;
			line.Format("lpFileList %s", lpFile);
			bool bNull = false;
			if (fq.nFiles > 1) {//multiple selection
				bNull = false;
				int idx = 0;
				for (int j=0; j<MAX_FILE_DIALOG_LIST; j++) {
					if (lpFile[j] == NULL) {
						if (bNull) break;
						bNull = true; idx++; CString sfname = (char*)(&(lpFile[j + 1])); line += "," + sfname;
						//AfxMessageBox(fname);
						if (idx >= fq.nFiles) break;
					} else {
						bNull = false;
					}
				}
			}
			AfxMessageBox("fileList\r\n" + line);
			//end of test code///*///
			if (err = AddFmtQueue(&fq)) break;
			if (fq.lpFileList) delete [] fq.lpFileList;
		} else if (mode == "Lsqfit") {
			LSQFIT_QUEUE lq;
			while (fp->ReadString(line)) {
				CString cmd = line.SpanExcluding(" \t\r\n");
				CString param = line.Mid(cmd.GetLength());
				param.TrimLeft(); param.TrimRight();
				//AfxMessageBox(cmd + "/" + param + "/");
				if (cmd == "End") {break;
				} else if (cmd == "nRefFiles") {if (sscanf_s(param, "%d", &(lq.nRefFiles)) < 1) msg += line;
				} else if (cmd == "nQryFiles") {if (sscanf_s(param, "%d", &(lq.nQryFiles)) < 1) msg += line;
				} else if (cmd == "m_XLow") {if (sscanf_s(param, "%d", &(lq.m_XLow)) < 1) msg += line;
				} else if (cmd == "m_XHigh") {if (sscanf_s(param, "%d", &(lq.m_XHigh)) < 1) msg += line;
				} else if (cmd == "m_YLow") {if (sscanf_s(param, "%d", &(lq.m_YLow)) < 1) msg += line;
				} else if (cmd == "m_YHigh") {if (sscanf_s(param, "%d", &(lq.m_YHigh)) < 1) msg += line;
				} else if (cmd == "m_ZLow") {if (sscanf_s(param, "%d", &(lq.m_ZLow)) < 1) msg += line;
				} else if (cmd == "m_ZHigh") {if (sscanf_s(param, "%d", &(lq.m_ZHigh)) < 1) msg += line;
				} else if (cmd == "bMaxDiameter") {if (param == "TRUE") lq.m_bMaxDiameter = TRUE; else if (param == "FALSE") lq.m_bMaxDiameter = FALSE; else msg += line;
				} else if (cmd == "m_RefList") {
					for (int j=0; j<lq.nRefFiles; j++) {
						if (!fp->ReadString(line)) break;
						lq.m_RefList += (line + "\r\n");
					}
				} else if (cmd == "m_QryList") {
					for (int j=0; j<lq.nQryFiles; j++) {
						if (!fp->ReadString(line)) break;
						lq.m_QryList += (line + "\r\n");
					}
				} else if (!cmd.IsEmpty()) {msg += line;
				}
			}
			if (err = AddLsqfitQueue(&lq)) break;
		} else if (mode == "Stop") {//201126
			while (fp->ReadString(line)) {
				CString cmd = line.SpanExcluding(" \t\r\n");
				//CString param = line.Mid(cmd.GetLength());
				//param.TrimLeft(); param.TrimRight();
				//AfxMessageBox(cmd + "/" + param + "/");
				if (cmd == "End") {
					break;
				} else if (!cmd.IsEmpty()) {
					msg += line;
				}
			}
			int item = m_QueueList.GetItemCount();
			if (m_QueueList.InsertItem(item, "Stop") >= 0) {
				m_QueueList.SetItemData(item, 0);
				m_QueueList.SetItemText(item, 2, "");
				m_QueueList.SetItemText(item, 1, "Hold");
				//EnableCtrl();
			}
		} else if (mode == "Sleep") {//201126
			int idur = 0;
			while (fp->ReadString(line)) {
				CString cmd = line.SpanExcluding(" \t\r\n");
				CString param = line.Mid(cmd.GetLength());
				param.TrimLeft(); param.TrimRight();
				//AfxMessageBox(cmd + "/" + param + "/");
				if (cmd == "End") {
					break;
				} else if (cmd == "Duration") {
					if (sscanf_s(param, "%d", &idur) < 1) msg += line;
				} else if (!cmd.IsEmpty()) {
					msg += line;
				}
			}
			//add queue
			//const int idur = m_QueueList.GetItemData(i);
			//line.Format("Duration %d\r\n", idur); fp->WriteString(line);
			int item = m_QueueList.GetItemCount();
			if (m_QueueList.InsertItem(item, "Sleep") >= 0) {
				m_QueueList.SetItemData(item, idur);
				CString desc;
				const int isec = idur % 60;
				int imin = idur / 60;
				int ihour = imin / 60;
				imin = imin % 60;
				desc.Format("%d sec (%d:%d:%d)", idur, ihour, imin, isec);
				m_QueueList.SetItemText(item, 2, desc);
				m_QueueList.SetItemText(item, 1, "Hold");
				//EnableCtrl();
			}
		}
	}
	bIsSaved = false;
	if (!msg.IsEmpty()) AfxMessageBox("Following lines were ignored.\r\n" + msg);
	return err;
}
