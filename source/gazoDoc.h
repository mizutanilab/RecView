// gazoDoc.h : CGazoDoc �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAZODOC_H__4E875408_7C47_4F97_BB25_07A9FB9273C1__INCLUDED_)
#define AFX_GAZODOC_H__4E875408_7C47_4F97_BB25_07A9FB9273C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "DlgReconst.h"
#include "DlgHistogram.h"
#include "cfft.h"
#include "DlgQueue.h"
#include "DlgRefraction.h"
#include "DlgOverlay.h"
#include "DlgProperty.h" //CDLGPROPERTY_PROCTYPE_ND
#include "chdf5.h"
#include <atlimage.h> //CImage

#define CGAZODOC_FILE_FWD 20
#define CGAZODOC_FILE_FASTFWD 200
#define CGAZODOC_FILE_REV -20
#define CGAZODOC_FILE_FASTREV -200
#define CGAZODOC_FILE_JUMPFWD 10000
#define CGAZODOC_FILE_JUMPREV -10000

#define CGAZODOC_STATUS_RESET 0
#define CGAZODOC_STATUS_FILEFWD 1
#define CGAZODOC_STATUS_FILEREV 2

#define CGAZODOC_MAXOVERLAY 500

#define CGAZODOC_FLAG_HDF5STEPSCAN 1

class CMainFrame;

struct CGZD_RESOLN_LIST {
	double dDistance2;
	double dLogMod[3];
};

class CGazoDoc : public CDocument
{
friend class CDlgReconst;
friend class CDlgFrameList;
friend class CMainFrame;
friend class CGazoView;

public:
	CGazoDoc();
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	DECLARE_DYNCREATE(CGazoDoc)

// �A�g���r���[�g
public:

// �I�y���[�V����
public:

//�I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CGazoDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CGazoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void DeleteAll();
	void InitAll();
	void ClearAll();
	void GPUMemFree(int iProcessorType = CDLGPROPERTY_PROCTYPE_ND, bool bCudaDeviceReset = false);//190124
	void GetDimension(int* ix, int* iy);
	void SetDimension(int ix, int iy);
	CString GetLogPath();
	void SetLogPath(CString arg);
	CString GetDataPath();
	void SetDataPath(CString arg);
	CString GetDataPrefix();
	void SetDataPrefix(CString arg);
	CString GetDataSuffix();
	void SetDataSuffix(CString arg);
	int GetSinogramYdim(BOOL bOffsetCT);
	void SetQueueMode(bool arg);
	void SetDispLevel(int iLow, int iHigh);
	void UpdateView(bool bInit = false);
	//float GetCenter(int iLayer);
	TErr GetAxis(int iTargetSlice, double* pCenter, double* pGrad, int iAxisOffset, 
					int iDatasetSel, bool bReport = false);
	TErr LoadLogFile(BOOL bOffsetCT = FALSE);
	TErr SetConvList(CString sDataPath, CString sFilePrefix, CString sFileSuffix, 
						int iDatasetSel, CMainFrame* pf = NULL);
	int CountFrameFromConvBat(CString sDataPath = "");
	void ShowTomogram(RECONST_QUEUE* rq, int iy, double fc, CGazoDoc* pdTarget = NULL);
	void ShowSinogram(RECONST_QUEUE* rq, int iy, double fc);
	TErr BatchReconst(RECONST_QUEUE* rq);
	void EnableSystemMenu(bool bEnable);
	TErr OutputImageInBox(FORMAT_QUEUE* fq, CProgressCtrl* progress = NULL, CString* psMsg = NULL);
	TErr ReadFile(CFile* fp);//160803
	TErr AllocPixelBuf(int ix, int iy);
	void ResetSinogram();
	void ShowRefracCorr(REFRAC_QUEUE* refq);
	void BatchRefracCorr(REFRAC_QUEUE* refq);
	void RefracCorr(REFRAC_QUEUE* refq, CFft* fft2, CCmplx* cPixel);
	void AdjBrightness(int iAdj);
	void AdjContrast(int iAdj);
	bool PointInPolygon(int ix, int iy, int* piPolygonX, int* piPolygonY);
	TErr GetPolygon(CString sSliceNumber, CString sPolygonList, int* piPolygonX, int* piPolygonY);
	//
	int* pPixel;
	float pixDiv, pixBase;
	int iDispLow, iDispHigh;//150102
	int* ppOverlay[CGAZODOC_MAXOVERLAY];
	bool bCmdLine;
	int maxHisFrame;
	//210618 int iLossFrameSet;//120715
	unsigned __int64 ullLossFrameSet;//210618
	//bool bUnderCalc;
	//int iThreadStatus;
	bool bDebug;
	CDlgReconst dlgReconst;
	CDlgRefraction dlgRefraction;
	//CDlgProperty dlgProperty;
	unsigned int uiDocStatus;
	CDlgHistogram dlgHist;//150102
	CDlgOverlay dlgOverlay;//151012
	CHDF5 hdf5;//160521
	CImage m_cimage;
	bool bColor;

protected:
	//void InitializeView();
	void InitContrast();
	TErr GenerateSinogram(RECONST_QUEUE* rq, int iLayer, double center, 
											double deltaCent = 0, int iMultiplex = 1);
	TErr DeconvBackProj(RECONST_QUEUE* rq, double center, int iMultiplex, int iOffset,
											int* nSinogr, double* tcpu, float* pixelBase, float* pixelDiv);
	//void Deconvolution(short* strip, CCmplx* p, int ndim, int iIntpDim, double center);
	TErr SetFilter(RECONST_QUEUE* rq, int ndim);
	void CheckDispLimit();
	TErr ProceedImage(int nproc = 0);
	void OutputCorrelationPlotData();
	void RemoveSurface(int* pixIn, FORMAT_QUEUE* fq, int kxdim, int kydim, float tpixDiv, float tpixBase, int* piRemoved);
	TErr CalcAvgFromHis(CString path, CString fnhis, CString* files, int nfiles, int iDatasetSel);

	double dAxisCenter;

private:
	TErr LoadLogFileAlloc(DWORD ilen);
	TErr SetFramesToExclude();

	int ixdim, iydim;
	int maxPixel;
	int ioxdim, ioydim;
	int pMaxOverlay[CGAZODOC_MAXOVERLAY];
	int izOverlay;
	CString fileComment;
	//int iBrightness, iContrast;
	//150102 int iDispLow, iDispHigh;
	int iPixelMax, iPixelMin;
protected:
	int iLenSinogr;
private:
	int numOfSampleSinogr;
	short** iSinogr;
	int maxSinogrLen;
	int maxSinogrWidth;
	short** ofSinogr;
	int maxOfSinogrLen;
	int maxOfSinogrWidth;
	int iCurrSinogr;
	int iCurrTrim;
	int maxImageEntry;
protected:
	CString* fname;
	float* fdeg;
	char* bInc;
private:
	float* fexp;
	CFft fft;
	CGazoDoc* parentDoc;
	float fCenter;
	CString logPath;
	CString dataPath;
	CString dataPrefix;
protected:
	CString dataSuffix;
private:
	int* iReconst; int nReconst, maxReconst;
	float* fFilter; int maxFilter;
	//150102 CDlgHistogram dlgHist;
	RECONST_INFO ri[MAX_CPU];
	bool bFromQueue;
	double dAxisGrad;
	CString* convList;
	unsigned int maxConvList;
	int iFramePerDataset;
	int nDarkFrame;
	int m_iFlag;
	CString m_sFramesToExclude;
	__int64 m_lHDF5DataSize0;
	CString m_sHisAvgFiles;//201125

// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CGazoDoc)
	afx_msg void OnToolbarBright();
	afx_msg void OnToolbarDark();
	afx_msg void OnToolbarCntdown();
	afx_msg void OnToolbarCntup();
	afx_msg void OnUpdateToolbarBright(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbarDark(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbarCntdown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbarCntup(CCmdUI* pCmdUI);
	afx_msg void OnTomoReconst();
	afx_msg void OnHlpDebug();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTomoHistg(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTomoReconst(CCmdUI* pCmdUI);
	afx_msg void OnTomoHistg();
	afx_msg void OnToolbarFnfwdOne();
	afx_msg void OnToolbarFnrevOne();
	afx_msg void OnToolbarFnfwd();
	afx_msg void OnToolbarFnff();
	afx_msg void OnToolbarFnrev();
	afx_msg void OnToolbarFnfr();
	afx_msg void OnToolbarFnPause();
	afx_msg void OnUpdateToolbarFnPause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHlpDebug(CCmdUI* pCmdUI);
	afx_msg void OnTomoAxis();
	afx_msg void OnUpdateTomoAxis(CCmdUI* pCmdUI);
	afx_msg void OnToolbarFnsf();
	afx_msg void OnUpdateToolbarFnsf(CCmdUI* pCmdUI);
	afx_msg void OnToolbarFnsr();
	afx_msg void OnUpdateToolbarFnsr(CCmdUI* pCmdUI);
	afx_msg void OnTomoStat();
	afx_msg void OnUpdateTomoStat(CCmdUI* pCmdUI);
	afx_msg void OnTomoLine();
	afx_msg void OnUpdateTomoLine(CCmdUI* pCmdUI);
	//manually
	afx_msg void OnToolbarFnJumpR();//120803
	afx_msg void OnToolbarFnJumpF();//120803
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
//	afx_msg void OnTomoProperty();
//	afx_msg void OnUpdateTomoProperty(CCmdUI *pCmdUI);
	afx_msg void OnTomoRefrac();
	afx_msg void OnUpdateTomoRefrac(CCmdUI *pCmdUI);
//	afx_msg void OnTomoLsqfit();
	afx_msg void OnTomoComment();
	afx_msg void OnTomoHorizcent();
	afx_msg void OnTomographyResolutionReport();
	afx_msg void OnTomoFourier();
	afx_msg void OnUpdateTomoFourier(CCmdUI *pCmdUI);
	afx_msg void OnTomographyGaussianconvolution();
	afx_msg void OnAnalysisAddnoise();
	afx_msg void OnUpdateAnalysisAddnoise(CCmdUI *pCmdUI);
	afx_msg void OnMenuOverlay();
	afx_msg void OnUpdateMenuOverlay(CCmdUI *pCmdUI);
	afx_msg void OnAnalysisEnlarge();
	afx_msg void OnAnalysisRadialprofile();
	afx_msg void OnAnalysisSubtract();
	afx_msg void OnUpdateAnalysisSubtract(CCmdUI *pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_GAZODOC_H__4E875408_7C47_4F97_BB25_07A9FB9273C1__INCLUDED_)
