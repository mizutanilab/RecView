// DlgGeneral.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgGeneral.h"


// CDlgGeneral ダイアログ

IMPLEMENT_DYNAMIC(CDlgGeneral, CDialog)

CDlgGeneral::CDlgGeneral(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgGeneral::IDD, pParent)
	, m_sCaption1(_T(""))
	, m_sInput1(_T(""))
	, m_sCaption2(_T(""))
	, m_sInput2(_T(""))
	, m_sCaption3(_T(""))
	, m_sInput3(_T(""))
{
	m_sTitle = _T("");
}

CDlgGeneral::~CDlgGeneral()
{
}

void CDlgGeneral::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CAPTION1, m_sCaption1);
	DDX_Text(pDX, IDC_INPUT1, m_sInput1);
	DDX_Text(pDX, IDC_CAPTION2, m_sCaption2);
	DDX_Text(pDX, IDC_INPUT2, m_sInput2);
	DDX_Text(pDX, IDC_CAPTION3, m_sCaption3);
	DDX_Text(pDX, IDC_INPUT3, m_sInput3);
}


BEGIN_MESSAGE_MAP(CDlgGeneral, CDialog)
END_MESSAGE_MAP()


// CDlgGeneral メッセージ ハンドラ

BOOL CDlgGeneral::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ここに初期化を追加してください
	if (!m_sTitle.IsEmpty()) SetWindowText(m_sTitle);
	if (m_sCaption3.IsEmpty()) GetDlgItem(IDC_INPUT3)->EnableWindow(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
