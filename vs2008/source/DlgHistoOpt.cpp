// DlgHistoOpt.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgHistoOpt.h"


// CDlgHistoOpt ダイアログ

IMPLEMENT_DYNAMIC(CDlgHistoOpt, CDialog)

CDlgHistoOpt::CDlgHistoOpt(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgHistoOpt::IDD, pParent)
	, m_Prefix(_T("ro"))
	, m_TrimAvrg(1)
	, m_16bit(FALSE)
	, m_RemoveDepth(0)
	, m_RemoveLAC(0)
{

}

CDlgHistoOpt::~CDlgHistoOpt()
{
}

void CDlgHistoOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HISTG_PREFIX, m_Prefix);
	DDX_Text(pDX, IDC_HISTG_TRMAVRG, m_TrimAvrg);
	DDX_Check(pDX, IDC_HISTG_16BIT, m_16bit);
	DDX_Text(pDX, IDC_HISTOPT_RMDEPTH, m_RemoveDepth);
	DDX_Text(pDX, IDC_HISTOPT_RMLAC, m_RemoveLAC);
}


BEGIN_MESSAGE_MAP(CDlgHistoOpt, CDialog)
END_MESSAGE_MAP()


// CDlgHistoOpt メッセージ ハンドラ
