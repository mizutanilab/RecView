/*	CFileDlgFolderSelect.h		v1	8/6/2009
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"
#include "gazo.h"
#include "CFileDlgFolderSelect.h"

#if !defined( _CFILEDLGFOLDERSELECT_CPP_ )
#define _CFILEDLGFOLDERSELECT_CPP_

CFileDlgFolderSelect::CFileDlgFolderSelect(BOOL bOpenFileDialog, LPCTSTR lpszDefExt,
	LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd) {
	m_prevFolder = _T("");
}

void CFileDlgFolderSelect::OnFolderChange() {
	CString strFolderPath = GetFolderPath();
	if (m_prevFolder != strFolderPath) m_prevFolder = strFolderPath;
	else {
		CPoint point;
		GetCursorPos(&point);
		CRect rect;
		GetParent()->GetDlgItem(IDOK)->GetWindowRect(&rect);
		//if cursor is on the "Open" button
		if (point.x >= rect.left && point.x <= rect.right &&
			point.y >= rect.top && point.y <= rect.bottom) {
			//set folder name
			strncpy(m_ofn.lpstrFile, (LPCTSTR)strFolderPath, m_ofn.nMaxFile);
			if (OnFileNameOK() == FALSE) {
				//close dialog
				CDialog *pParent = (CDialog *)GetParent();
				if (pParent != NULL) pParent->EndDialog(IDOK);
			}
		}
	}
	CFileDialog::OnFolderChange();
}



#endif // _CFILEDLGFOLDERSELECT_CPP_
