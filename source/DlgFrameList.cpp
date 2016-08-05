// DlgFrameList.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgFrameList.h"
#include "gazoDoc.h"


// CDlgFrameList ダイアログ

IMPLEMENT_DYNAMIC(CDlgFrameList, CDialog)

CDlgFrameList::CDlgFrameList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFrameList::IDD, pParent)
{
	pd = NULL;
}

CDlgFrameList::~CDlgFrameList()
{
}

void CDlgFrameList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAMELIST_TREE, m_treeFrames);
}


BEGIN_MESSAGE_MAP(CDlgFrameList, CDialog)
END_MESSAGE_MAP()


// CDlgFrameList メッセージ ハンドラ

BOOL CDlgFrameList::OnInitDialog()
{
	CDialog::OnInitDialog();

	LONG lStyle = GetWindowLong(m_treeFrames.m_hWnd, GWL_STYLE);
	lStyle |= TVS_CHECKBOXES;
	SetWindowLong(m_treeFrames.m_hWnd, GWL_STYLE, lStyle);
	//TVS_CHECKBOXES style should not be enabled in the resource editor

	if (pd) {
		//HDF5: generate list from .h5 file
		//HIS: generate list from .his file
		//others: generate list from output.log
		if (pd->dataSuffix.MakeUpper() == ".H5") {
			m_treeFrames.InsertItem("not supported");
		} else if (pd->dataSuffix.MakeUpper() == ".HIS") {
			m_treeFrames.InsertItem("not supported");
		} else {
			//SPring-8 output.log
			const int iList = pd->iLenSinogr - 1;
			HTREEITEM hItem;
			for (int i=0; i<iList; i++) {
				CString sItem; sItem.Format("%s (%.2f)", pd->fname[i], pd->fdeg[i]);
				if (!pd->bInc[i]) sItem += " flat";
				hItem = m_treeFrames.InsertItem(sItem);
				m_treeFrames.SetItemData(hItem, i);
				m_treeFrames.SetCheck(hItem, TRUE);
			}
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgFrameList::OnOK()
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	UpdateData();
	HTREEITEM hItem = NULL;
	hItem = m_treeFrames.GetNextItem(TVGN_ROOT, TVGN_CHILD);
	int iCheck = m_treeFrames.GetCheck(hItem);
	CString line; line.Format("%d", iCheck); AfxMessageBox(line);

	CDialog::OnOK();
}
