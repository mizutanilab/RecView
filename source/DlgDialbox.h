#pragma once
#include "afxwin.h"
#include "resource.h"

#define DIALBOX_NDIALS 6
#define DIALBOX_NBUTTONS 2
#define WM_DIALBOX (WM_APP + 0x100)
#define COMM_BUFFER_LEN 80
#define BLUETOOTH_DEVICENAME_KEY "BTdials"

#define DIALBOX_NACTION 8
#define DIALBOX_SCROLLX 0
#define DIALBOX_SCROLLY 1
#define DIALBOX_MAG 2
#define DIALBOX_FRAME 3
#define DIALBOX_FRAMEFAST 4
#define DIALBOX_CONTRAST 5
#define DIALBOX_BRIGHTNESS 6
#define DIALBOX_NOACTION 7
#define DIALBOX_SCROLLX_STR "Scroll X"
#define DIALBOX_SCROLLY_STR "Scroll Y"
#define DIALBOX_MAG_STR "Zoom"
#define DIALBOX_FRAME_STR "Frame"
#define DIALBOX_FRAMEFAST_STR "Frame fast"
#define DIALBOX_CONTRAST_STR "Contrast"
#define DIALBOX_BRIGHTNESS_STR "Brightness"
#define DIALBOX_NOACTION_STR "No action"

#define DIALBOX_NBUTTONACTION 2
#define DIALBOX_OPENQUEUE 0
#define DIALBOX_NOBUTTONACTION 1
#define DIALBOX_OPENQUEUE_STR "Open queue"
#define DIALBOX_NOBUTTONACTION_STR "No action"

unsigned __stdcall SerialCommThread(void* pArg);
struct COMM_INFO {
	HANDLE hEvent;
	HANDLE hComm;
	HANDLE hCommThread;
	OVERLAPPED ovlp;
	bool bActive;
	HWND hViewWnd;
};

// CDlgDialbox ダイアログ

class CDlgDialbox : public CDialog
{
	DECLARE_DYNAMIC(CDlgDialbox)

public:
	CDlgDialbox(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgDialbox();

// ダイアログ データ
	enum { IDD = IDD_DIALBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int OpenDialboxBluetoothSPP(CString sCommPort);
	void CloseDialboxBluetoothSPP(int iTimeout = 10);

	CComboBox m_ComPort;
	CView* pv;
	CString sCommConnected;
	//CString sDialboxMsg[DIALBOX_NDIALS];
	COMM_INFO ciDialbox;

	void EnableCtrl();

	afx_msg void OnBnClickedDialboxConnect();
	afx_msg void OnBnClickedDialboxDisconnect();
	CComboBox m_cmbDialAction[DIALBOX_NDIALS];
	BYTE m_ucDialCCW[DIALBOX_NDIALS];
	BYTE m_ucDialCW[DIALBOX_NDIALS];
	unsigned char m_ucDialAction[DIALBOX_NDIALS];
	CString m_sDialCCW[DIALBOX_NDIALS];
	CString m_sDialCW[DIALBOX_NDIALS];

	CComboBox m_cmbButtonAction[DIALBOX_NBUTTONS];
	BYTE m_ucButtonRel[DIALBOX_NBUTTONS];
	unsigned char m_ucButtonAction[DIALBOX_NBUTTONS];
	CString m_sButtonRel[DIALBOX_NBUTTONS];

private:
	CString GetBthComPort(CString sAddr);
	CString EnumComPort();
	CString sComPortList;

protected:
	virtual void OnOK();
	LRESULT OnDialbox(WPARAM wParam, LPARAM lParam);
//public:
//	afx_msg void OnCbnSelchangeDialboxComport();
};
