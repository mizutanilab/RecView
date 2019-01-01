/*	CFileDlgFolderSelect.h		v1	8/6/2009
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#if !defined( _CFILEDLGFOLDERSELECT_H_ )
#define _CFILEDLGFOLDERSELECT_H_

class CFileDlgFolderSelect : public CFileDialog
{
public:
	CFileDlgFolderSelect(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
	virtual void OnFolderChange();
private:
	CString m_prevFolder;
};

#endif // _CFILEDLGFOLDERSELECT_H_
