#pragma once
#include "afxwin.h"

#define CDLGPROPERTY_PROCTYPE_INTEL 0
#define CDLGPROPERTY_PROCTYPE_CUDA 1
#define CDLGPROPERTY_PROCTYPE_ATISTREAM 2

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
	void Init(int icpu, bool bsimd, 
		int iCudaCount, int iCudaBlock, int iCudaWarp,
		int iATIcount, int iATImaxwork, int iATIunitwork);
	CComboBox m_CPU;
	CComboBox m_Memory;
	int iCPU, iMemory;
	bool bSIMD;
	int iCUDA, iCUDAnblock, iCUDAwarpsize;
	int iATIstream, iATIstreamNwork, iATIstreamUnitwork;
protected:
	void EnableCtrl();
	int rMemory;
	int rCPU;
	int rCUDA, maxCUDA, rCUDAnblock, maxCUDAThreadsPerBlock;
	int r_ProcessorType;
	int rATIstream, maxATIstream, rATIstreamNwork, maxATIstreamNwork;
	BOOL rEnableSIMD;
	BOOL r_EnReport;
	BOOL r_UseCUDAFFT;
	BOOL r_EnFastSeek;

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
};
