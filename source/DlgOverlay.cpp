// DlgOverlay.cpp : 実装ファイル
//

#include "stdafx.h"
#include "gazo.h"
#include "DlgOverlay.h"
#include "gazoDoc.h"
#include "gazoView.h"
#include "cxyz.h"


// CDlgOverlay ダイアログ

IMPLEMENT_DYNAMIC(CDlgOverlay, CDialog)

CDlgOverlay::CDlgOverlay(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgOverlay::IDD, pParent)
	, m_rx1(0)
	, m_ry1(0)
	, m_rx2(100)
	, m_ry2(0)
	, m_rx3(0)
	, m_ry3(100)
	, m_rx4(100)
	, m_ry4(100)
	, m_ru1(0)
	, m_rv1(0)
	, m_ru2(100)
	, m_rv2(0)
	, m_ru3(0)
	, m_rv3(100)
	, m_ru4(100)
	, m_rv4(100)
	, m_bMove1(TRUE)
	, m_bMove2(TRUE)
	, m_bMove3(TRUE)
	, m_bMove4(TRUE)
	, m_rw1(0)
	, m_rw2(0)
	, m_rw3(0)
	, m_rw4(0)
	, m_bMovew1(FALSE)
	, m_bMovew2(FALSE)
	, m_bMovew3(FALSE)
	, m_bMovew4(FALSE)
{
	pd = NULL;
}

CDlgOverlay::~CDlgOverlay()
{
}

void CDlgOverlay::SetDoc(CGazoDoc* pDoc) {if (pDoc) pd = pDoc;}

void CDlgOverlay::UpdateGazoview() {
	if (!pd) return;
	POSITION pos = pd->GetFirstViewPosition();
	while (pos != NULL) {
		CGazoView* pv = (CGazoView*) pd->GetNextView( pos );
		if (pv) {
			TReal prPoint[16];
			prPoint[0] = m_rx1; prPoint[1] = m_ry1;
			prPoint[2] = m_rx2; prPoint[3] = m_ry2;
			prPoint[4] = m_rx3; prPoint[5] = m_ry3;
			prPoint[6] = m_rx4; prPoint[7] = m_ry4;
			prPoint[8] = m_ru1; prPoint[9] = m_rv1;
			prPoint[10] = m_ru2; prPoint[11] = m_rv2;
			prPoint[12] = m_ru3; prPoint[13] = m_rv3;
			prPoint[14] = m_ru4; prPoint[15] = m_rv4;
			TErr err = ProjTransformGetCoeff(prPoint, pv->pdOverlayCoeff);
			pd->UpdateView();
			break;
		}
	}
}

void CDlgOverlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OVLYX1, m_rx1);
	DDX_Text(pDX, IDC_OVLYY1, m_ry1);
	DDX_Text(pDX, IDC_OVLYX2, m_rx2);
	DDX_Text(pDX, IDC_OVLYY2, m_ry2);
	DDX_Text(pDX, IDC_OVLYX3, m_rx3);
	DDX_Text(pDX, IDC_OVLYY3, m_ry3);
	DDX_Text(pDX, IDC_OVLYX4, m_rx4);
	DDX_Text(pDX, IDC_OVLYY4, m_ry4);
	DDX_Text(pDX, IDC_OVLYU1, m_ru1);
	DDX_Text(pDX, IDC_OVLYV1, m_rv1);
	DDX_Text(pDX, IDC_OVLYU2, m_ru2);
	DDX_Text(pDX, IDC_OVLYV2, m_rv2);
	DDX_Text(pDX, IDC_OVLYU3, m_ru3);
	DDX_Text(pDX, IDC_OVLYV3, m_rv3);
	DDX_Text(pDX, IDC_OVLYU4, m_ru4);
	DDX_Text(pDX, IDC_OVLYV4, m_rv4);
	DDX_Check(pDX, IDC_MOVE1, m_bMove1);
	DDX_Check(pDX, IDC_MOVE2, m_bMove2);
	DDX_Check(pDX, IDC_MOVE3, m_bMove3);
	DDX_Check(pDX, IDC_MOVE4, m_bMove4);
	DDX_Text(pDX, IDC_OVLYW1, m_rw1);
	DDX_Text(pDX, IDC_OVLYW2, m_rw2);
	DDX_Text(pDX, IDC_OVLYW3, m_rw3);
	DDX_Text(pDX, IDC_OVLYW4, m_rw4);
	DDX_Check(pDX, IDC_OVLY_CHKW1, m_bMovew1);
	DDX_Check(pDX, IDC_OVLY_CHKW2, m_bMovew2);
	DDX_Check(pDX, IDC_OVLY_CHKW3, m_bMovew3);
	DDX_Check(pDX, IDC_OVLY_CHKW4, m_bMovew4);
}


BEGIN_MESSAGE_MAP(CDlgOverlay, CDialog)
	ON_BN_CLICKED(IDC_OVLYUP, &CDlgOverlay::OnBnClickedOvlyup)
	ON_BN_CLICKED(IDC_OVLY_APPLY, &CDlgOverlay::OnBnClickedOvlyApply)
	ON_BN_CLICKED(IDC_OVLYDOWN, &CDlgOverlay::OnBnClickedOvlydown)
	ON_BN_CLICKED(IDC_OVLYLEFT, &CDlgOverlay::OnBnClickedOvlyleft)
	ON_BN_CLICKED(IDC_OVLYRIGHT, &CDlgOverlay::OnBnClickedOvlyright)
	ON_BN_CLICKED(IDC_OVLYCW, &CDlgOverlay::OnBnClickedOvlyCW)
	ON_BN_CLICKED(IDC_OVLYCCW, &CDlgOverlay::OnBnClickedOvlyCCW)
	ON_BN_CLICKED(IDC_OVLYLARGE, &CDlgOverlay::OnBnClickedOvlylarge)
	ON_BN_CLICKED(IDC_OVLYSMALL, &CDlgOverlay::OnBnClickedOvlysmall)
	ON_BN_CLICKED(IDC_OVLY_WUP, &CDlgOverlay::OnBnClickedOvlyWup)
	ON_BN_CLICKED(IDC_OVLY_WDOWN, &CDlgOverlay::OnBnClickedOvlyWdown)
END_MESSAGE_MAP()


// CDlgOverlay メッセージ ハンドラ

void CDlgOverlay::OnBnClickedOvlyup()
{
	UpdateData();
	if (m_bMove1) m_ry1--;
	if (m_bMove2) m_ry2--;
	if (m_bMove3) m_ry3--;
	if (m_bMove4) m_ry4--;
	UpdateData(FALSE);
	UpdateGazoview();
}

void CDlgOverlay::OnBnClickedOvlyApply()
{
	UpdateData();
	UpdateGazoview();
}

void CDlgOverlay::OnBnClickedOvlydown()
{
	UpdateData();
	if (m_bMove1) m_ry1++;
	if (m_bMove2) m_ry2++;
	if (m_bMove3) m_ry3++;
	if (m_bMove4) m_ry4++;
	UpdateData(FALSE);
	UpdateGazoview();
}

void CDlgOverlay::OnBnClickedOvlyleft()
{
	UpdateData();
	if (m_bMove1) m_rx1--;
	if (m_bMove2) m_rx2--;
	if (m_bMove3) m_rx3--;
	if (m_bMove4) m_rx4--;
	UpdateData(FALSE);
	UpdateGazoview();
}

void CDlgOverlay::OnBnClickedOvlyright()
{
	UpdateData();
	if (m_bMove1) m_rx1++;
	if (m_bMove2) m_rx2++;
	if (m_bMove3) m_rx3++;
	if (m_bMove4) m_rx4++;
	UpdateData(FALSE);
	UpdateGazoview();
}

void CDlgOverlay::OnBnClickedOvlyCW() {Rotation(-1.0);}
void CDlgOverlay::OnBnClickedOvlyCCW() {Rotation(1.0);}

void CDlgOverlay::Rotation(double dDeg) {
	UpdateData();
	const TReal gx = (m_rx1 + m_rx2 + m_rx3 + m_rx4) / 4.;
	const TReal gy = (m_ry1 + m_ry2 + m_ry3 + m_ry4) / 4.;
	const double dRot = dDeg * DEG_TO_RAD;
	TReal tx = cos(dRot) * (m_rx1 - gx) + sin(dRot) * (m_ry1 - gy);
	TReal ty = -sin(dRot) * (m_rx1 - gx) + cos(dRot) * (m_ry1 - gy);
	m_rx1 = tx + gx; m_ry1 = ty + gy;
	tx = cos(dRot) * (m_rx2 - gx) + sin(dRot) * (m_ry2 - gy);
	ty = -sin(dRot) * (m_rx2 - gx) + cos(dRot) * (m_ry2 - gy);
	m_rx2 = tx + gx; m_ry2 = ty + gy;
	tx = cos(dRot) * (m_rx3 - gx) + sin(dRot) * (m_ry3 - gy);
	ty = -sin(dRot) * (m_rx3 - gx) + cos(dRot) * (m_ry3 - gy);
	m_rx3 = tx + gx; m_ry3 = ty + gy;
	tx = cos(dRot) * (m_rx4 - gx) + sin(dRot) * (m_ry4 - gy);
	ty = -sin(dRot) * (m_rx4 - gx) + cos(dRot) * (m_ry4 - gy);
	m_rx4 = tx + gx; m_ry4 = ty + gy;
	UpdateData(FALSE);
	UpdateGazoview();
}


void CDlgOverlay::OnBnClickedOvlylarge() {Magnify(1.01);}

void CDlgOverlay::OnBnClickedOvlysmall() {Magnify(1./1.01);}

void CDlgOverlay::Magnify(double dScale) {
	UpdateData();
	const TReal gx = (m_rx1 + m_rx2 + m_rx3 + m_rx4) / 4.;
	const TReal gy = (m_ry1 + m_ry2 + m_ry3 + m_ry4) / 4.;
	m_rx1 = (m_rx1 - gx) * dScale + gx;
	m_ry1 = (m_ry1 - gx) * dScale + gx;
	m_rx2 = (m_rx2 - gx) * dScale + gx;
	m_ry2 = (m_ry2 - gx) * dScale + gx;
	m_rx3 = (m_rx3 - gx) * dScale + gx;
	m_ry3 = (m_ry3 - gx) * dScale + gx;
	m_rx4 = (m_rx4 - gx) * dScale + gx;
	m_ry4 = (m_ry4 - gx) * dScale + gx;
	UpdateData(FALSE);
	UpdateGazoview();
}


void CDlgOverlay::OnBnClickedOvlyWup() {Zshift(1);}
void CDlgOverlay::OnBnClickedOvlyWdown() {Zshift(-1);}

void CDlgOverlay::Zshift(int iDelta) {
	UpdateData();
	int iCode = 0;
	if (m_bMovew1) {m_rw1 += iDelta; iCode |= 1;}
	if (m_bMovew2) {m_rw2 += iDelta; iCode |= 2;}
	if (m_bMovew3) {m_rw3 += iDelta; iCode |= 4;}
	if (m_bMovew4) {m_rw4 += iDelta; iCode |= 8;}
	CXyz cx1 = CXyz(m_ru1, m_rv1, m_rw1);
	CXyz cx2 = CXyz(m_ru2, m_rv2, m_rw2);
	CXyz cx3 = CXyz(m_ru3, m_rv3, m_rw3);
	CXyz cx4 = CXyz(m_ru4, m_rv4, m_rw4);
	CXyz cPlaneNorm;
	double dPlaneSec = 0;
	switch (iCode) {
		default: {return;}
		case 0: {return;}
		case 14: {}
		case 1: {
			cPlaneNorm = ((cx2 - cx4) * (cx1 - cx4) + (cx1 - cx4) * (cx3 - cx4)).X(0.5);
			if (fabs(cPlaneNorm.z) < 1E-6) return;
			dPlaneSec = cPlaneNorm.X(cx4);
			m_rw2 = (dPlaneSec - (cPlaneNorm.x * cx2.x + cPlaneNorm.y * cx2.y)) / cPlaneNorm.z;
			m_rw3 = (dPlaneSec - (cPlaneNorm.x * cx3.x + cPlaneNorm.y * cx3.y)) / cPlaneNorm.z;
			break;}
		case 13: {}
		case 2: {
			cPlaneNorm = ((cx1 - cx3) * (cx2 - cx3) + (cx2 - cx3) * (cx4 - cx3)).X(0.5);
			if (fabs(cPlaneNorm.z) < 1E-6) return;
			dPlaneSec = cPlaneNorm.X(cx3);
			m_rw1 = (dPlaneSec - (cPlaneNorm.x * cx1.x + cPlaneNorm.y * cx1.y)) / cPlaneNorm.z;
			m_rw4 = (dPlaneSec - (cPlaneNorm.x * cx4.x + cPlaneNorm.y * cx4.y)) / cPlaneNorm.z;
			break;}
		case 11: {}
		case 4: {
			cPlaneNorm = ((cx1 - cx2) * (cx3 - cx2) + (cx3 - cx2) * (cx4 - cx2)).X(0.5);
			if (fabs(cPlaneNorm.z) < 1E-6) return;
			dPlaneSec = cPlaneNorm.X(cx2);
			m_rw1 = (dPlaneSec - (cPlaneNorm.x * cx1.x + cPlaneNorm.y * cx1.y)) / cPlaneNorm.z;
			m_rw4 = (dPlaneSec - (cPlaneNorm.x * cx4.x + cPlaneNorm.y * cx4.y)) / cPlaneNorm.z;
			break;}
		case 7: {}
		case 8: {
			cPlaneNorm = ((cx2 - cx1) * (cx4 - cx1) + (cx4 - cx1) * (cx3 - cx1)).X(0.5);
			if (fabs(cPlaneNorm.z) < 1E-6) return;
			dPlaneSec = cPlaneNorm.X(cx1);
			m_rw2 = (dPlaneSec - (cPlaneNorm.x * cx2.x + cPlaneNorm.y * cx2.y)) / cPlaneNorm.z;
			m_rw3 = (dPlaneSec - (cPlaneNorm.x * cx3.x + cPlaneNorm.y * cx3.y)) / cPlaneNorm.z;
			break;}
		case 3: {break;}
		case 5: {break;}
		case 10: {break;}
		case 12: {break;}
		case 15: {break;}
	}
	UpdateData(FALSE);
	UpdateGazoview();
}

