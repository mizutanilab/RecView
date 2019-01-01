// DlgPolygon.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgPolygon.h"
#include "gazoView.h"
#include "gazoDoc.h"


// CDlgPolygon ダイアログ

IMPLEMENT_DYNAMIC(CDlgPolygon, CDialog)

CDlgPolygon::CDlgPolygon(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPolygon::IDD, pParent)
	, m_sPolygonCurrent(_T(""))
{
	pv = NULL;
	sPolygonList.Empty();
}

CDlgPolygon::~CDlgPolygon()
{
}

void CDlgPolygon::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_POLYGON_CURRENT, m_sPolygonCurrent);
	DDX_Control(pDX, IDC_POLYGONLIST, m_lbPolygonList);
}


BEGIN_MESSAGE_MAP(CDlgPolygon, CDialog)
	ON_BN_CLICKED(IDC_POLYGON_ADD, &CDlgPolygon::OnBnClickedPolygonAdd)
	ON_BN_CLICKED(IDC_POLYGON_DELETE, &CDlgPolygon::OnBnClickedPolygonDelete)
END_MESSAGE_MAP()


// CDlgPolygon メッセージ ハンドラ
void CDlgPolygon::SetView(CGazoView* pView) {if (pView) pv = pView;}

void CDlgPolygon::UpdateCurrentPolygon() {
	if (!pv) return;
	if (!m_hWnd) return;
	CString line, sCoords = "";
	for (int i=0; i<CGAZOVIEW_NPOLYGON; i++) {
		line.Format("(%d %d)", pv->iPolygonX[i], pv->iPolygonY[i]);
		sCoords += line;
	}
	CGazoDoc* pd = pv->GetDocument();
	const CString fpath = pd->GetPathName();
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fpath);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	CString sFnm = fnm;
	sFnm.MakeReverse();
	sFnm = sFnm.SpanIncluding("0123456789");
	sFnm.MakeReverse();

	m_sPolygonCurrent = sFnm + " " + sCoords;
	UpdateData(FALSE);
}
void CDlgPolygon::OnBnClickedPolygonAdd()
{
	UpdateData();
	const CString sFrm = m_sPolygonCurrent.SpanExcluding(" ");
	if (sFrm.IsEmpty()) return;
	int ipos = m_lbPolygonList.FindString(0, sFrm);
	if (ipos != LB_ERR) m_lbPolygonList.DeleteString(ipos);
	m_lbPolygonList.AddString(m_sPolygonCurrent);
	MakePolygonList();
}

void CDlgPolygon::OnBnClickedPolygonDelete()
{
	int ipos = m_lbPolygonList.GetCurSel();
	if (ipos != LB_ERR) m_lbPolygonList.DeleteString(ipos);
	MakePolygonList();
}

BOOL CDlgPolygon::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_lbPolygonList.SetHorizontalExtent(700);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgPolygon::MakePolygonList() {
	CString sItem;
	sPolygonList.Empty();
	const int icount = m_lbPolygonList.GetCount();
	for (int i=0; i<icount; i++) {
		m_lbPolygonList.GetText(i, sItem);
		sPolygonList += sItem + "\r\n";
	}
}

void CDlgPolygon::OnCancel()
{
	ShowWindow(SW_HIDE);
	MakePolygonList();
	//CDialog::OnCancel();
}

void CDlgPolygon::OnOK()
{
	if (!pv) return;
	pv->bPolygonEnabled = false;
	ShowWindow(SW_HIDE);
	MakePolygonList();
	pv->InvalidateRect(NULL, FALSE);

	//CDialog::OnOK();
}
