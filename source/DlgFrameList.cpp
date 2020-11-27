// DlgFrameList.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgFrameList.h"
#include "gazoDoc.h"
#include "chdf5.h"
//201125
#include "MainFrm.h"

// CDlgFrameList ダイアログ

IMPLEMENT_DYNAMIC(CDlgFrameList, CDialog)

CDlgFrameList::CDlgFrameList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFrameList::IDD, pParent)
{
	pd = NULL;
	m_sFramesToExclude.Empty();
	m_piRevList = NULL;
	m_lHDF5DataSize[0] = 0;
	m_lHDF5DataSize[1] = 0;
	m_lHDF5DataSize[2] = 0;
	m_dwSelectedFrame = 0;
	m_sDocList.Empty();//190708
	iDocPos = -1;//190708
}

CDlgFrameList::~CDlgFrameList()
{
	if (m_piRevList) delete [] m_piRevList;
}

void CDlgFrameList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAMELIST_TREE, m_treeFrames);
}

BEGIN_MESSAGE_MAP(CDlgFrameList, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_FRAMELIST_TREE, &CDlgFrameList::OnNMClickFramelistTree)
	ON_NOTIFY(NM_SETFOCUS, IDC_FRAMELIST_TREE, &CDlgFrameList::OnNMSetfocusFramelistTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FRAMELIST_TREE, &CDlgFrameList::OnTvnSelchangedFramelistTree)
END_MESSAGE_MAP()


// CDlgFrameList メッセージ ハンドラ

BOOL CDlgFrameList::OnInitDialog()
{
	CDialog::OnInitDialog();

	LONG lStyle = GetWindowLong(m_treeFrames.m_hWnd, GWL_STYLE);
	lStyle |= TVS_CHECKBOXES;
	SetWindowLong(m_treeFrames.m_hWnd, GWL_STYLE, lStyle);
	//TVS_CHECKBOXES style should not be enabled in the resource editor
//	const int isino = pd->iLenSinogr;
//	CString line; line.Format("%d", isino); AfxMessageBox(line);

	if (pd) {
		if (pd->dataSuffix.MakeUpper() == ".H5") {
			//current frame
			CString docTitle = pd->GetTitle();
			m_dwSelectedFrame = 0;
			if (docTitle.GetAt(0) == '[') {
				const CString sFrame = docTitle.Mid(1).SpanIncluding("0123456789");
				m_dwSelectedFrame = atoi(sFrame);
			}
			//HDF5: generate list from .h5 file
			CHDF5 hdf5;
			CString fn = pd->dataPath + pd->dataPrefix + pd->dataSuffix;
			CFile fhdf5;
			if (!fhdf5.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
				fhdf5.Close(); m_treeFrames.InsertItem("file open error"); return TRUE;
			}
			//get lDataSize0
			hdf5.SetFile(&fhdf5);
			TErr err = 0;
			CString sErr; 
			if (err = hdf5.ReadSuperBlock(NULL)) {sErr.Format("HDF5 error %d", err); m_treeFrames.InsertItem(sErr); return TRUE;}
			if (err = hdf5.FindChildSymbol("exchange", -1, NULL)) {sErr.Format("HDF5 error %d", err); m_treeFrames.InsertItem(sErr); return TRUE;}
			hdf5.MoveToChildTree();
//			__int64 lDataSize[3], lDataSizeTotal = 0;
			const CString psSymbol[] = {"data", "data_dark", "data_white"};
			for (int i=0; i<3; i++) {
				if (err = hdf5.FindChildSymbol(psSymbol[i], -1, NULL)) {sErr.Format("HDF5 error %d", err); m_treeFrames.InsertItem(sErr); return TRUE;}
				if (hdf5.m_sChildTitle.Left(psSymbol[i].GetLength()) != psSymbol[i]) {m_treeFrames.InsertItem("HDF5 not found: " + psSymbol[i]); return TRUE;}
				if (err = hdf5.GetDataObjHeader(NULL)) {sErr.Format("HDF5 error %d", err); m_treeFrames.InsertItem(sErr); return TRUE;}
				m_lHDF5DataSize[i] = hdf5.m_plDataSize[0];
			}
			//
			DWORD ipos = 0;
			err = ReadHDF5Theta(&fhdf5, &hdf5, NULL, &ipos);
			if (err) {//fly scan
				//m_treeFrames.InsertItem("fly scan");
			}
			fhdf5.Close();
			const int idigit = (int)log10((double)m_lHDF5DataSize[0] + 3) + 1;
			CString fmt; fmt.Format(" %%0%dd", idigit);
			CString fmts[3];
			fmts[0].Format(" s%%0%dd", idigit);
			fmts[1].Format(" b%%0%dd", idigit);
			fmts[2].Format(" w%%0%dd", idigit);
			int icount = 0;
			HTREEITEM hItem;
			for (int i=0; i<3; i++) {
				for (int j=0; j<m_lHDF5DataSize[i]; j++) {
					CString sItem;
					sItem.Format(fmt, j);
					if (j == 0) sItem += "[" + psSymbol[i] + "]";
					if (i == 0) {
						int idx = j - pd->dlgReconst.m_iDlgFL_SampleFrameStart + 1;
						if ((idx >= 0)&&(idx < pd->iLenSinogr - 1)) {
							if (pd->bInc[idx] & CGAZODOC_BINC_SAMPLE) {
								CString line; line.Format(" (%.2f)", pd->fdeg[idx]); sItem += line;
							}
						}
						if (j == pd->dlgReconst.m_iDlgFL_SampleFrameStart) sItem += " start";
						else if (j == pd->dlgReconst.m_iDlgFL_SampleFrameEnd) sItem += " end";
					}
					CString sFrmNum; sFrmNum.Format(fmt, j);
					CString sFrmNums; sFrmNums.Format(fmts[i], j);
					BOOL bCheck = TRUE;
					if (i) {if (m_sFramesToExclude.Find(sFrmNums) >= 0) bCheck = FALSE;}
					else if ((m_sFramesToExclude.Find(sFrmNum) >= 0)||(m_sFramesToExclude.Find(sFrmNums) >= 0)) bCheck = FALSE;
					hItem = m_treeFrames.InsertItem(sItem);
					m_treeFrames.SetItemData(hItem, icount++);
					m_treeFrames.SetCheck(hItem, bCheck);
					//vs2010 if ((i == 0)&&(j < pd->dlgReconst.m_iDlgFL_SampleFrameStart)) m_treeFrames.SetItemStateEx(hItem, TVIS_EX_DISABLED);
				}
			}
			//sErr.Format("nframes %lld", lDataSize0); m_treeFrames.InsertItem(sErr);
		} else if (pd->dataSuffix.MakeUpper() == ".HIS") {
			//current frame
			CString docTitle = pd->GetTitle();
			m_dwSelectedFrame = 0;
			if (docTitle.GetAt(0) == '[') {
				m_dwSelectedFrame = atoi(docTitle.Mid(1).SpanIncluding("0123456789"));
			}
//			CString msg; msg.Format("%d", m_dwSelectedFrame); AfxMessageBox(msg);
			//HIS: generate list from .his file
			const int iList = pd->iFramePerDataset;
			if (m_piRevList) delete [] m_piRevList;
			try {m_piRevList = new int[iList];}
			catch(CException* e) {
				e->Delete();
				m_treeFrames.InsertItem("out of memory error");
				return TRUE;
			}
			if (pd->iFramePerDataset < 0) {
				m_treeFrames.InsertItem("frame counting error");
				return TRUE;
			}
			//setconvlist
			TCHAR path_buffer[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
			_stprintf_s(path_buffer, _MAX_PATH, pd->GetPathName());
			_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
			pd->SetConvList(path_buffer, fnm, ext, pd->dlgReconst.m_iDatasetSel);
			for (int i=0; i<iList; i++) {m_piRevList[i] = INT_MIN;}
			const int iConv = pd->maxConvList;
			if (pd->convList) {
				for (int i=0; i<iConv; i++) {
					int cidx = i;
					if (pd->convList[cidx].GetAt(0) == 'q') {//point to another image
							cidx = atoi(pd->convList[cidx].Mid(1)) - 1;
							if ((cidx < 0)||(cidx > iConv)) continue;
					}
					if (pd->convList[cidx].GetAt(0) == 'r') {
						int ihisFrame = atoi(pd->convList[cidx].Mid(1)) - 1;
						if (pd->dlgReconst.m_nDataset > 1) {
							if (pd->nDarkFrame > 0) {//if dark frames were taken only in the first set.
								ihisFrame = ((ihisFrame - pd->nDarkFrame) % (pd->iFramePerDataset - pd->nDarkFrame));
							} else {//if dark frames were taken at every dataset.
								ihisFrame = (ihisFrame % pd->iFramePerDataset);
							}
						}
						if ((ihisFrame >= 0)&&(ihisFrame < iList)) m_piRevList[ihisFrame] = cidx;
					}
				}
			}
			if (!pd->m_sHisAvgFiles.IsEmpty()) {//201125
				//uncheck flat and dark files which are not found in conv.bat
				//this is not necessary for the recon calc (CalcAvgFromHis reads convList) but needed for consistent displaying
				//CString msg = "";
				int curPos = 0;
				CString resToken = pd->m_sHisAvgFiles.Tokenize(_T(" "), curPos);
				while (resToken != _T("")) {
					int idx = atoi(resToken.Mid(1).SpanExcluding(".")) - 1;
					//CString line; line.Format("%d/", idx); msg += line;
					if ((idx < iList) && (idx >= 0)) m_piRevList[idx] = -1;//-1 indicates frame is included
					resToken = pd->m_sHisAvgFiles.Tokenize(_T(" "), curPos);
				}
				//AfxMessageBox(msg);
			}//201125
//			CString msg = "revList161112\r\n", line;
//			for (int i=iConv-50; i<iConv; i++) {line.Format("%d %s\r\n", i, pd->convList[i]); msg += line;}
//			for (int i=0; i<80; i++) {line.Format("(%d %d) ", i, m_piRevList[i]); msg += line;}
//			for (int i=iList-50; i<iList; i++) {line.Format("(%d %d) ", i, m_piRevList[i]); msg += line;}
//			AfxMessageBox(msg);
			//format string
			const int idigit = (int)log10((double)iList) + 1;
			CString fmt; fmt.Format(" %%0%dd", idigit);
			CString fmt2; fmt2.Format(" h%%0%dd", idigit);
			CString fmt3; fmt3.Format(" s%%0%dd", idigit);
			HTREEITEM hItem;
			for (int i=0; i<iList; i++) {
				int iframe = i;
				if (pd->nDarkFrame > 0) {//if dark frames were taken only in the first set.
					iframe += (pd->iFramePerDataset - pd->nDarkFrame) * pd->dlgReconst.m_iDatasetSel;
				} else {//if dark frames were taken at every dataset.
					iframe += pd->iFramePerDataset * pd->dlgReconst.m_iDatasetSel;
				}
				CString sItem;
				sItem.Format(fmt, iframe);
				//201125 BOOL bCheck = TRUE;
				BOOL bCheck = FALSE;//201125 default FALSE
				int iRev = m_piRevList[i];
				if (iRev >= 0) {
					CString line; line.Format(" (%.2f)", pd->fdeg[iRev]); sItem += line;
					//if (!(pd->bInc[i] & CGAZODOC_BINC_SAMPLE)) sItem += " flat";
					CString sFrmNum; sFrmNum.Format(fmt, iRev);
					CString sFrmNum3; sFrmNum3.Format(fmt3, iRev);
					bCheck = ((m_sFramesToExclude.Find(sFrmNum) >= 0) || (m_sFramesToExclude.Find(sFrmNum3) >= 0)) ? FALSE : TRUE;
					if (iRev == pd->dlgReconst.m_iDlgFL_SampleFrameStart) sItem += " start";
					else if (iRev == pd->dlgReconst.m_iDlgFL_SampleFrameEnd) sItem += " end";
				} else {
					//uncheck files which are not found in conv.bat
					if (iRev == -1) bCheck = TRUE;//201125
					//overwrite with exclude list
					CString sFrmNum; sFrmNum.Format(fmt2, i);
					//201125 bCheck = (m_sFramesToExclude.Find(sFrmNum) >= 0) ? FALSE : TRUE;
					if (m_sFramesToExclude.Find(sFrmNum) >= 0) bCheck = FALSE;//201125
				}
				hItem = m_treeFrames.InsertItem(sItem);
				m_treeFrames.SetItemData(hItem, i);
				m_treeFrames.SetCheck(hItem, bCheck);
//				if (i == m_dwSelectedFrame) m_treeFrames.SelectItem(hItem);
			}
		} else {
			//current frame
			TCHAR path_buffer[_MAX_PATH];
			_stprintf_s(path_buffer, _MAX_PATH, pd->GetPathName());
			TCHAR fnm[_MAX_FNAME];
			_tsplitpath_s( path_buffer, NULL, 0, NULL, 0, fnm, _MAX_FNAME, NULL, 0);
			CString filename = fnm;
			const CString fprefix = filename.SpanExcluding("0123456789.");
			m_dwSelectedFrame = atoi(filename.Mid(fprefix.GetLength())) - 1;//file name starts from q0001
			//others: generate list from output.log
			//SPring-8 output.log
			const int iList = pd->iLenSinogr - 1;
			const int idigit = (int)log10((double)iList) + 1;
			CString fmt; fmt.Format(" %%0%dd", idigit);
			CString fmt3; fmt3.Format(" s%%0%dd", idigit);
			HTREEITEM hItem;
			for (int i=0; i<iList; i++) {
				CString sItem; sItem.Format("%s (%.2f)", pd->fname[i], pd->fdeg[i]);
				if (!(pd->bInc[i] & CGAZODOC_BINC_SAMPLE)) sItem += " flat";
				else {
					if (i == pd->dlgReconst.m_iDlgFL_SampleFrameStart) sItem += " start";
					else if (i == pd->dlgReconst.m_iDlgFL_SampleFrameEnd) sItem += " end";
				}
				hItem = m_treeFrames.InsertItem(sItem);
				m_treeFrames.SetItemData(hItem, i);
				CString sFrmNum; sFrmNum.Format(fmt, i);
				CString sFrmNum3; sFrmNum3.Format(fmt3, i);
				BOOL bCheck = ((m_sFramesToExclude.Find(sFrmNum) >= 0)||(m_sFramesToExclude.Find(sFrmNum3) >= 0)) ? FALSE : TRUE;
				m_treeFrames.SetCheck(hItem, bCheck);
			}
		}
	} else if (!m_sDocList.IsEmpty()) {//190708
		int ipos = 0, i = 0;
		HTREEITEM hItem;
		for (;;) {
			CString sItem = m_sDocList.Tokenize(_T("\r\n"), ipos);
			if (sItem.IsEmpty()) break;
			hItem = m_treeFrames.InsertItem(sItem);
			m_treeFrames.SetItemData(hItem, i);
			m_treeFrames.SetCheck(hItem, FALSE);
			i++;
		}
		this->SetWindowText("Select image");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgFrameList::OnOK()
{
	UpdateData();
	if (pd) {
		HTREEITEM hItem = m_treeFrames.GetNextItem(TVGN_ROOT, TVGN_CHILD);
		m_sFramesToExclude = " ";//set blank char no to be overwritten in SetFramesToExclude()
		const int iList = pd->iLenSinogr - 1;
		const int idigit = (int)log10((double)iList) + 1;
		CString fmt;
		fmt.Format(" %%0%dd", idigit);
		if (pd->dataSuffix.MakeUpper() == ".H5") {
			//HDF5
			CString fmt0; fmt0.Format(" s%%0%dd", idigit);
			CString fmt1; fmt1.Format(" b%%0%dd", idigit);
			CString fmt2; fmt2.Format(" w%%0%dd", idigit);
			while (hItem) {
				if (!m_treeFrames.GetCheck(hItem)) {
					int idata = m_treeFrames.GetItemData(hItem);
					CString sFrmNum;
					if (idata < m_lHDF5DataSize[0]) {
						if ((idata >= pd->dlgReconst.m_iDlgFL_SampleFrameStart)&&(idata <= pd->dlgReconst.m_iDlgFL_SampleFrameEnd)) {
							sFrmNum.Format(fmt, idata);
						} else {
							sFrmNum.Format(fmt0, idata);
						}
					} else if (idata < m_lHDF5DataSize[0] + m_lHDF5DataSize[1]) {
						sFrmNum.Format(fmt1, idata - m_lHDF5DataSize[0]);
					} else {
						sFrmNum.Format(fmt2, idata - m_lHDF5DataSize[0] - m_lHDF5DataSize[1]);
					}
					m_sFramesToExclude += sFrmNum;
				}
				hItem = m_treeFrames.GetNextItem(hItem, TVGN_NEXT);
			}
		} else if ((pd->dataSuffix.MakeUpper() == ".HIS")&&(m_piRevList)) {
			//HIS
			CString fmt2; fmt2.Format(" h%%0%dd", idigit);
			CString fmt3; fmt3.Format(" s%%0%dd", idigit);
			while (hItem) {
				if (!m_treeFrames.GetCheck(hItem)) {
					int idata = m_treeFrames.GetItemData(hItem);
					CString sFrmNum;
					if (m_piRevList[idata] >= 0) {
						if ((m_piRevList[idata] >= pd->dlgReconst.m_iDlgFL_SampleFrameStart) &&
							(m_piRevList[idata] <= pd->dlgReconst.m_iDlgFL_SampleFrameEnd)) sFrmNum.Format(fmt, m_piRevList[idata]);
						else sFrmNum.Format(fmt3, m_piRevList[idata]);
					}
					else sFrmNum.Format(fmt2, idata);
					m_sFramesToExclude += sFrmNum;
				}
				hItem = m_treeFrames.GetNextItem(hItem, TVGN_NEXT);
			}
		} else {
			//others
			CString fmt3; fmt3.Format(" s%%0%dd", idigit);
			while (hItem) {
				if (!m_treeFrames.GetCheck(hItem)) {
					CString sFrmNum;
					int idata = m_treeFrames.GetItemData(hItem);
					if ((idata >= pd->dlgReconst.m_iDlgFL_SampleFrameStart)&&
						(idata <= pd->dlgReconst.m_iDlgFL_SampleFrameEnd)) sFrmNum.Format(fmt, idata);
					else sFrmNum.Format(fmt3, idata);
					//CString sFrmNum; sFrmNum.Format(fmt, m_treeFrames.GetItemData(hItem));
					m_sFramesToExclude += sFrmNum;
				}
				hItem = m_treeFrames.GetNextItem(hItem, TVGN_NEXT);
			}
		}
		if (pd->bDebug) AfxMessageBox(m_sFramesToExclude);
	} else if (!m_sDocList.IsEmpty()) {//190708
		HTREEITEM hItem = m_treeFrames.GetNextItem(TVGN_ROOT, TVGN_CHILD);
		while (hItem) {
			if (m_treeFrames.GetCheck(hItem)) {
				iDocPos = m_treeFrames.GetItemData(hItem);
				break;
			}
			hItem = m_treeFrames.GetNextItem(hItem, TVGN_NEXT);
		}
	}

	CDialog::OnOK();
}

void CDlgFrameList::OnNMClickFramelistTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	//201125==>
	if (!pd) return;
	const int iList = pd->iFramePerDataset;
	NMTREEVIEW *pNMTree = (NMTREEVIEW*)pNMHDR;
	TVHITTESTINFO ht = { 0 };
	DWORD pos = ::GetMessagePos();
	ht.pt.x = LOWORD(pos);
	ht.pt.y = HIWORD(pos);
	::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1);
	m_treeFrames.HitTest(&ht);
	CMainFrame* pf = (CMainFrame*)AfxGetMainWnd();

	if (TVHT_ONITEMSTATEICON & ht.flags)
	{
		int idata = m_treeFrames.GetItemData(ht.hItem);
		if ((idata >= 0)&&(idata < iList)) {
			if (m_piRevList[idata] == INT_MIN) {
				m_treeFrames.SetCheck(ht.hItem, TRUE);//this disables check
				if (pf) pf->m_wndStatusBar.SetPaneText(0, "Not listed in conv.bat");
			}
		}
	}
	//==>201125
	*pResult = 0;
}

void CDlgFrameList::OnNMSetfocusFramelistTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	*pResult = 0;
}

void CDlgFrameList::OnTvnSelchangedFramelistTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	DWORD dwData = m_treeFrames.GetItemData(pNMTreeView->itemNew.hItem);
//	CString msg; msg.Format("%d==>%d", m_dwSelectedFrame, dwData); AfxMessageBox(msg);
	if (pd) {
		if (pd->dataSuffix.MakeUpper() == ".HIS") {
			if (pd->iLossFrameSet >= 0) {//161231
				if (pd->iLossFrameSet == pd->dlgReconst.m_iDatasetSel) {
					if ((int)(dwData - pd->nDarkFrame) > (pd->iFramePerDataset - pd->nDarkFrame) / 2) {dwData--;}
				} else if (pd->iLossFrameSet < pd->dlgReconst.m_iDatasetSel) {dwData--;}
			}
			if (pd->nDarkFrame > 0) {//if dark frames were taken only in the first set.
				dwData += (pd->iFramePerDataset - pd->nDarkFrame) * pd->dlgReconst.m_iDatasetSel;
			} else {//if dark frames were taken at every dataset.
				dwData += pd->iFramePerDataset * pd->dlgReconst.m_iDatasetSel;
			}
		}
		pd->ProceedImage((int)dwData - m_dwSelectedFrame);
	}
	m_dwSelectedFrame = dwData;

	*pResult = 0;
}
