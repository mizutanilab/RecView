// CDlgCircleLasso.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "gazoView.h"
#include "gazoDoc.h"
//#include "pch.h"
#include "DlgCircleLasso.h"
#include "afxdialogex.h"


// CDlgCircleLasso ダイアログ

IMPLEMENT_DYNAMIC(CDlgCircleLasso, CDialogEx)

CDlgCircleLasso::CDlgCircleLasso(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CIRCLELASSO, pParent)
	, m_sCircleCurrent(_T(""))
{
	pv = NULL;
	sCircleList.Empty();
	//m_cBkBrs.CreateSolidBrush(RGB(0,0,0));
}

CDlgCircleLasso::~CDlgCircleLasso()
{
	//m_cBkBrs.DeleteObject();
}

void CDlgCircleLasso::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CIRCLE_CURRENT, m_sCircleCurrent);
	DDX_Control(pDX, IDC_CIRCLELIST, m_lbCircleList);
}


BEGIN_MESSAGE_MAP(CDlgCircleLasso, CDialogEx)
	ON_BN_CLICKED(IDC_CIRCLE_ADD, &CDlgCircleLasso::OnBnClickedCircleAdd)
	ON_BN_CLICKED(IDC_CIRCLE_DELETE, &CDlgCircleLasso::OnBnClickedCircleDelete)
	ON_BN_CLICKED(IDC_CIRCLE_RESET1, &CDlgCircleLasso::OnBnClickedCircleReset1)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CIRCLE_RESET2, &CDlgCircleLasso::OnBnClickedCircleReset2)
	ON_BN_CLICKED(IDC_CIRCLE_RESET3, &CDlgCircleLasso::OnBnClickedCircleReset3)
	ON_BN_CLICKED(IDC_CIRCLE_RESET4, &CDlgCircleLasso::OnBnClickedCircleReset4)
	ON_BN_CLICKED(IDC_CIRCLE_RESET5, &CDlgCircleLasso::OnBnClickedCircleReset5)
END_MESSAGE_MAP()


// CDlgCircleLasso メッセージ ハンドラー
void CDlgCircleLasso::SetView(CGazoView* pView) { if (pView) pv = pView; }

void CDlgCircleLasso::UpdateCurrentCircle() {
	if (!pv) return;
	if (!m_hWnd) return;
	CString line, sCoords = "";
	for (int i = 0; i < CGAZOVIEW_NPARAMCIRCLELASSO; i+=3) {
		if (i) line.Format("(%d %d %d)", pv->iCircleLasso[i], pv->iCircleLasso[i+1], pv->iCircleLasso[i+2]);
		else line.Format("(%d %d) %d", pv->iCircleLasso[i], pv->iCircleLasso[i + 1], pv->iCircleLasso[i + 2]);
		sCoords += line;
	}
	CGazoDoc* pd = pv->GetDocument();
	const CString fpath = pd->GetPathName();
	TCHAR path_buffer[_MAX_PATH];
	_stprintf_s(path_buffer, _MAX_PATH, fpath);
	TCHAR drive[_MAX_DRIVE]; TCHAR dir[_MAX_DIR]; TCHAR fnm[_MAX_FNAME]; TCHAR ext[_MAX_EXT];
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fnm, _MAX_FNAME, ext, _MAX_EXT);
	CString sFnm = fnm;
	sFnm.MakeReverse();
	sFnm = sFnm.SpanIncluding("0123456789");
	sFnm.MakeReverse();

	m_sCircleCurrent = sFnm + " " + sCoords;
	UpdateData(FALSE);
}
void CDlgCircleLasso::OnBnClickedCircleAdd()
{
	UpdateData();
	const CString sFrm = m_sCircleCurrent.SpanExcluding(" ");
	if (sFrm.IsEmpty()) return;
	int ipos = m_lbCircleList.FindString(0, sFrm);
	if (ipos != LB_ERR) m_lbCircleList.DeleteString(ipos);
	m_lbCircleList.AddString(m_sCircleCurrent);
	MakeCircleList();
}

void CDlgCircleLasso::OnBnClickedCircleDelete()
{
	int ipos = m_lbCircleList.GetCurSel();
	if (ipos != LB_ERR) m_lbCircleList.DeleteString(ipos);
	MakeCircleList();
}

BOOL CDlgCircleLasso::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_lbCircleList.SetHorizontalExtent(200);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CDlgCircleLasso::MakeCircleList() {
	CString sItem;
	sCircleList.Empty();
	const int icount = m_lbCircleList.GetCount();
	for (int i = 0; i < icount; i++) {
		m_lbCircleList.GetText(i, sItem);
		sCircleList += sItem + "\r\n";
	}
}

void CDlgCircleLasso::OnCancel()
{
	ShowWindow(SW_HIDE);
	MakeCircleList();
	//CDialog::OnCancel();
}

void CDlgCircleLasso::OnOK()
{
	if (!pv) return;
	pv->bCircleLassoEnabled = false;
	ShowWindow(SW_HIDE);
	MakeCircleList();
	pv->InvalidateRect(NULL, FALSE);

	//CDialog::OnOK();
}

void CDlgCircleLasso::CircleReset(int idx) {
	if (!pv) return;
	if (idx * 3 + 2 > CGAZOVIEW_NPARAMCIRCLELASSO - 1) return;
	pv->iCircleLasso[idx*3] = 0; pv->iCircleLasso[idx*3+1] = 0; pv->iCircleLasso[idx*3+2] = 50;
	pv->InvalidateRect(NULL, FALSE);
	UpdateCurrentCircle();
}

void CDlgCircleLasso::OnBnClickedCircleReset1() { CircleReset(1); }
void CDlgCircleLasso::OnBnClickedCircleReset2() { CircleReset(2); }
void CDlgCircleLasso::OnBnClickedCircleReset3() { CircleReset(3); }
void CDlgCircleLasso::OnBnClickedCircleReset4() { CircleReset(4); }
void CDlgCircleLasso::OnBnClickedCircleReset5() { CircleReset(5); }
