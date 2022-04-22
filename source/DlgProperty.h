#pragma once
#include "afxwin.h"

#define CDLGPROPERTY_PROCTYPE_INTEL 0
#define CDLGPROPERTY_PROCTYPE_CUDA 1
#define CDLGPROPERTY_PROCTYPE_ATISTREAM 2
#define CDLGPROPERTY_PROCTYPE_ND -1

// CDlgProperty ダイアログ

class CDlgProperty : public CDialog
{
	DECLARE_DYNAMIC(CDlgProperty)

public:
	CDlgProperty(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgProperty();

// ダイアログ データ
	enum { IDD = IDD_PROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void Init(int icpu, bool bsimd, bool bavx2, bool bavx512, //220417
		int iCudaCount, int iCudaBlock, int iCudaWarp,
		int iATIcount, int iATImaxwork, int iATIunitwork, int iProcessorType = -1);
	CComboBox m_CPU;
	CComboBox m_Memory;
	int iCPU, iMemory;
	bool bSIMD, bAVX2, bAVX512;//220417
	int iCUDA, iCUDAnblock, iCUDAwarpsize;
	int iATIstream, iATIstreamNwork, iATIstreamUnitwork;
protected:
	void EnableCtrl();
	int rMemory;
	int rCPU;
	int rCUDA, maxCUDA, rCUDAnblock, maxCUDAThreadsPerBlock;
	int r_ProcessorType;
	int rATIstream, maxATIstream, rATIstreamNwork, maxATIstreamNwork;
	BOOL rEnableSIMD, rEnableAVX2, rEnableAVX512; //220417
	BOOL r_EnReport;
	BOOL r_UseCUDAFFT;
	BOOL r_EnFastSeek;
	BOOL r_EnCUDAStream;

protected:
	virtual void OnOK();
public:
	BOOL bEnableSIMD;
	int m_ProcessorType;
	afx_msg void OnBnClickedIntelcpu();
	afx_msg void OnBnClickedCudagpu();
protected:
	virtual void OnCancel();
public:
	CComboBox m_CUDA;
	CComboBox m_CUDAnblock;
	BOOL m_EnReport;
	BOOL bUseCUDAFFT;
	afx_msg void OnBnClickedPropAtistream();
	CComboBox m_ATIstream;
	CComboBox m_ATIstreamNwork;
	BOOL m_EnFastSeek;
	BOOL m_bEnableAVX2;
	BOOL m_bEnableAVX512; // 220417
	afx_msg void OnBnClickedPropSimd();
	afx_msg void OnBnClickedPropInfo();
	BOOL m_EnCUDAStream;
	afx_msg void OnBnClickedPropAvx2();
};
