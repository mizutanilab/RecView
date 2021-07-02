#if !defined(AFX_DLGQUEUE_H__DB29AD1C_3B04_4D2E_B82F_E7CB945E4E74__INCLUDED_)
#define AFX_DLGQUEUE_H__DB29AD1C_3B04_4D2E_B82F_E7CB945E4E74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cerror.h"
#include "general.h"

#define CDLGQUEUE_INITQUEUE_ALLOC 20
//#define CDLGQUEUE_MAXQUEUE 1000
#define CDLGQUEUE_IDLE 0
#define CDLGQUEUE_BUSY 1
#define CDLGQUEUE_STOP 2
#define CDLGQUEUE_PAUSE 4
#define CDLGQUEUE_FINAL 8

#define RQFLAGS_ANGINTP 1
#define RQFLAGS_ZERNIKE 2
#define RQFLAGS_DRIFTLIST 4
#define RQFLAGS_DRIFTPARAMS 8
#define RQFLAGS_USEONLYEVENFRAMES 16
#define RQFLAGS_USEONLYODDFRAMES 32
#define RQFLAGS_SKIPINITIALFLATSINHDF5 64
#define RQFLAGS_SINOGRAMKEPT 128

struct RECONST_QUEUE {
	bool bActive;
	int iLayer1; int iLayer2;
	double dCenter1; double dCenter2;
	double dPixelWidth;
	int iXdim;
	int iYdim;
	int iFilter;
	double dCutoff;
	double dOrder;
	int iInterpolation;
	//CString filePath;
	CString logFileName;
	CString dataPath;
	CString outFilePrefix;
	CString outFilePath;
	CString itexFilePrefix;
	CString itexFileSuffix;
	bool bOffsetCT;
	int iRawSinoXdim;
	int iSinoYdim;
	int iTrimWidth;
	//110920 bool bAngularIntp;
	unsigned int dReconFlags;
	float fTiltAngle;
	int drStart;
	int drEnd;
	double drX;
	double drY;
	BOOL drOmit;
	double dAxisInc;
	int iDatasetSel;
	//int iDatasetSize;
	//210618 int iLossFrameSet;
	unsigned __int64 ullLossFrameSet;//210618
	CString sDriftListPath;
	bool bReconOptionUpdated;
	CString sFramesToExclude;
	int iSampleFrameStart;
	int iSampleFrameEnd;
};

#define FQFLAGS_16BIT 1
#define FQFLAGS_OUTPUT_HISTG 2

struct FORMAT_QUEUE {
	bool bActive;
	double dLow;
	double dHigh;
	int iBoxCentX;
	int iBoxCentY;
	int iBoxSizeX;
	int iBoxSizeY;
	int iBoxAngle;
	BOOL bBoxEnabled;
	int iAverage;
	int iXdim;
	int iYdim;
	int nFiles;
	LPTSTR lpFileList;
	CString outFilePrefix;
	//180621 BOOL b16bit;
	unsigned int uiFlags;
	double dOspThreshold;
	int iOspDepth;
	CString sPolygonList;
};

struct REFRAC_QUEUE {
	int ifftx;
	int iffty;
	int iXdim;
	int iYdim;
	double dPixelWidth;//um
	double ndz0;//ndelta * z0
	double dLAC;//cm-1
	CString logFileName;
	CString dataPath;
	CString outFilePrefix;
	CString outFilePath;
	CString itexFilePrefix;
};

//120828===>
struct LSQFIT_QUEUE {
	bool bActive;
	int nRefFiles;
	int nQryFiles;
	int m_XLow;
	int m_XHigh;
	int m_YLow;
	int m_YHigh;
	int m_ZLow;
	int m_ZHigh;
	CString m_RefList;
	CString m_QryList;
	BOOL m_bMaxDiameter;//181214
};
//===>120828

class CGazoDoc;

// DlgQueue.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CDlgQueue �_�C�A���O

class CDlgQueue : public CDialog
{
friend class CGazoApp;
// �R���X�g���N�V����
public:
	CDlgQueue(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
	~CDlgQueue();
	void AddItem();
	TErr AddRecQueue(RECONST_QUEUE* rq);
	TErr AddFmtQueue(FORMAT_QUEUE* fq);
	TErr AddLsqfitQueue(LSQFIT_QUEUE* lq);
	TErr SaveQueue(CStdioFile* fp);
	TErr LoadQueue(CStdioFile* fp);

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CDlgQueue)
	enum { IDD = IDD_QUEUE };
	CListCtrl	m_QueueList;
	BOOL	m_FinalProc;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CDlgQueue)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	TErr AllocRecQueue();
	TErr AllocFmtQueue();
	TErr AllocLsqfitQueue();
	TErr InsertRecItem(int item, DWORD_PTR rqIdx, CString status);
	TErr InsertFmtItem(int item, DWORD_PTR fqIdx, CString status);
	TErr InsertLsqfitItem(int item, DWORD_PTR lqIdx, CString status);
	void EnableCtrl();
	RECONST_QUEUE* recQueue;
	int nRecQueue, maxRecQueue;
	FORMAT_QUEUE* fmtQueue;
	int nFmtQueue, maxFmtQueue;
	LSQFIT_QUEUE* lsqfitQueue;
	int nLsqfitQueue, maxLsqfitQueue;
	int iRclickedItem, iDragItem;
	int iStatus;
	CGazoDoc* pd;
	bool bIsSaved;

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CDlgQueue)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnRclickQueueList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopupqueueDel();
	afx_msg void OnBegindragQueueList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopupqueueUp();
	afx_msg void OnPopupqueueDown();
	afx_msg void OnQueueStop();
	afx_msg void OnQueueFinal();
	afx_msg void OnPopupqueueAppendstop();//201126
	afx_msg void OnPopupqueueAppendsleep();//201126
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQueuePause();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B


#endif // !defined(AFX_DLGQUEUE_H__DB29AD1C_3B04_4D2E_B82F_E7CB945E4E74__INCLUDED_)
