// DlgDialbox.cpp : 実装ファイル
//

#include "stdafx.h"
#include "DlgDialbox.h"
#include <process.h> //_beginthreadex

//161229 bluetooth lists
//#include <Winsock2.h>
#include <Ws2bth.h>
#include <BluetoothAPIs.h>

// CDlgDialbox ダイアログ

IMPLEMENT_DYNAMIC(CDlgDialbox, CDialog)

CDlgDialbox::CDlgDialbox(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDialbox::IDD, pParent)
{
	memset(&(ciDialbox.ovlp), 0, sizeof(ciDialbox.ovlp));
	ciDialbox.hEvent = INVALID_HANDLE_VALUE;
	ciDialbox.hComm = INVALID_HANDLE_VALUE;
	ciDialbox.hCommThread = INVALID_HANDLE_VALUE;
	ciDialbox.bActive = false;
	ciDialbox.hViewWnd = NULL;

	sCommConnected.Empty();
	sComPortList = EnumComPort();

	m_ucDialAction[0] = DIALBOX_SCROLLY;
	m_ucDialAction[3] = DIALBOX_SCROLLX;
	m_ucDialAction[1] = DIALBOX_MAG;
	m_ucDialAction[4] = DIALBOX_BRIGHTNESS;
	m_ucDialAction[2] = DIALBOX_FRAME;
	m_ucDialAction[5] = DIALBOX_FRAMEFAST;
	m_ucDialCCW[0] = 'Z';
	m_ucDialCW[0] = 'X';
	m_ucDialCCW[1] = 'A';
	m_ucDialCW[1] = 'S';
	m_ucDialCCW[2] = 'Q';
	m_ucDialCW[2] = 'W';
	m_ucDialCCW[3] = 'C';
	m_ucDialCW[3] = 'V';
	m_ucDialCCW[4] = 'D';
	m_ucDialCW[4] = 'F';
	m_ucDialCCW[5] = 'E';
	m_ucDialCW[5] = 'R';
	for (int i=0; i<DIALBOX_NDIALS; i++) {
		m_sDialCW[i].Empty();
		m_sDialCCW[i].Empty();
		m_sDialCW[i].AppendChar(m_ucDialCW[i]);
		m_sDialCCW[i].AppendChar(m_ucDialCCW[i]);
	}

	m_ucButtonAction[0] = DIALBOX_NOBUTTONACTION;
	m_ucButtonAction[1] = DIALBOX_OPENQUEUE;
	m_ucButtonRel[0] = 'T';
	m_ucButtonRel[1] = 'G';
	for (int i=0; i<DIALBOX_NBUTTONS; i++) {
		m_sButtonRel[i].Empty();
		m_sButtonRel[i].AppendChar(m_ucButtonRel[i]);
	}
}

CDlgDialbox::~CDlgDialbox()
{
	CloseDialboxBluetoothSPP();
}

void CDlgDialbox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIALBOX_COMPORT, m_ComPort);
	DDX_Control(pDX, IDC_DIALBOX_DIAL0ACT, m_cmbDialAction[0]);
	DDX_Control(pDX, IDC_DIALBOX_DIAL1ACT, m_cmbDialAction[1]);
	DDX_Control(pDX, IDC_DIALBOX_DIAL2ACT, m_cmbDialAction[2]);
	DDX_Control(pDX, IDC_DIALBOX_DIAL3ACT, m_cmbDialAction[3]);
	DDX_Control(pDX, IDC_DIALBOX_DIAL4ACT, m_cmbDialAction[4]);
	DDX_Control(pDX, IDC_DIALBOX_DIAL5ACT, m_cmbDialAction[5]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL0CCW, m_sDialCCW[0]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL0CW, m_sDialCW[0]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL1CCW, m_sDialCCW[1]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL1CW, m_sDialCW[1]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL2CCW, m_sDialCCW[2]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL2CW, m_sDialCW[2]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL3CCW, m_sDialCCW[3]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL3CW, m_sDialCW[3]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL4CCW, m_sDialCCW[4]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL4CW, m_sDialCW[4]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL5CCW, m_sDialCCW[5]);
	DDX_Text(pDX, IDC_DIALBOX_DIAL5CW, m_sDialCW[5]);
	DDX_Control(pDX, IDC_DIALBOX_BUTTON1ACT, m_cmbButtonAction[0]);
	DDX_Control(pDX, IDC_DIALBOX_BUTTON2ACT, m_cmbButtonAction[1]);
	DDX_Text(pDX, IDC_DIALBOX_BUTTON1REL, m_sButtonRel[0]);
	DDX_Text(pDX, IDC_DIALBOX_BUTTON2REL, m_sButtonRel[1]);
}


BEGIN_MESSAGE_MAP(CDlgDialbox, CDialog)
	ON_BN_CLICKED(IDC_DIALBOX_CONNECT, &CDlgDialbox::OnBnClickedDialboxConnect)
	ON_BN_CLICKED(IDC_DIALBOX_DISCONNECT, &CDlgDialbox::OnBnClickedDialboxDisconnect)

	ON_MESSAGE(WM_DIALBOX, OnDialbox)//161210

//	ON_CBN_SELCHANGE(IDC_DIALBOX_COMPORT, &CDlgDialbox::OnCbnSelchangeDialboxComport)
END_MESSAGE_MAP()

unsigned __stdcall SerialCommThread(void* pArg) {//161126
	COMM_INFO* pciDialbox = (COMM_INFO*)(pArg);
	SetCommMask(pciDialbox->hComm, EV_RXCHAR);
	while (1) {
		if (pciDialbox->bActive == false) break;
		DWORD dwEvent;
		WaitCommEvent(pciDialbox->hComm, &dwEvent, &(pciDialbox->ovlp));
		if ((dwEvent & EV_RXCHAR) == 0) continue;
		char cBuffer[COMM_BUFFER_LEN];
		memset(cBuffer, 0, COMM_BUFFER_LEN);
		DWORD dwLen = 0, dwError = 0, dwBytes = 0;
		if (ReadFile(pciDialbox->hComm, cBuffer, COMM_BUFFER_LEN, &dwLen, &(pciDialbox->ovlp)) == 0) {
			DWORD dwError = GetLastError();
			if (dwError == ERROR_IO_PENDING) {
				DWORD dwWaitLimit = GetTickCount() + 100;
				while(!GetOverlappedResult(pciDialbox->hComm, &(pciDialbox->ovlp), &dwBytes, FALSE)) {
					if(GetTickCount() > dwWaitLimit) break;
					if (pciDialbox->bActive == false) break;
				}
			}
		}
		if(dwError == 0 || dwError == ERROR_IO_PENDING) {
			if (dwLen) {
				SendMessage(pciDialbox->hViewWnd, WM_DIALBOX, (WPARAM)(&(cBuffer[0])), 0);
			}
		}
	}
	pciDialbox->bActive = false;
	return 0;
}

int CDlgDialbox::OpenDialboxBluetoothSPP(CString sCommPort) {
	//get event handle
	ciDialbox.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if( ciDialbox.hEvent == NULL ) return 16112601;
	memset(&(ciDialbox.ovlp), 0, sizeof(ciDialbox.ovlp));
	ciDialbox.ovlp.Offset = 0;
	ciDialbox.ovlp.OffsetHigh = 0;
	ciDialbox.ovlp.hEvent = ciDialbox.hEvent;

	//open port
	//LPCSTR portName = "\\\\.\\COM5";
	ciDialbox.hComm = CreateFile(sCommPort, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (ciDialbox.hComm == INVALID_HANDLE_VALUE) return 16112602;
	PurgeComm(ciDialbox.hComm, PURGE_RXCLEAR);

	//set params
	DCB dcb;
	memset(&dcb, 0, sizeof(DCB));
	GetCommState(ciDialbox.hComm, &dcb);
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	if (!SetCommState(ciDialbox.hComm, &dcb)) return 16112603;
	COMMTIMEOUTS timeout;
	memset(&timeout, 0, sizeof(COMMTIMEOUTS));
	timeout.ReadIntervalTimeout = MAXDWORD;
	if (!SetCommTimeouts(ciDialbox.hComm, &timeout)) return 16112604;

	//begin thread
	ciDialbox.bActive = true;
	ciDialbox.hViewWnd = this->m_hWnd;
	void* pArg = (void*)(&ciDialbox);
	unsigned int uiThreadID = 0;
	ciDialbox.hCommThread = (HANDLE)_beginthreadex( NULL, 0, SerialCommThread, pArg, 0, &uiThreadID );
	if (ciDialbox.hCommThread == INVALID_HANDLE_VALUE) return 16112605;

	sCommConnected = sCommPort.TrimLeft("\\.");

	return 0;
}

void CDlgDialbox::CloseDialboxBluetoothSPP(int iTimeout) {
	if ((iTimeout < 0)||(iTimeout > 100)) iTimeout = 100;

	if (ciDialbox.bActive) {
		DWORD dwState = STILL_ACTIVE;
		//SetCommMask(ciDialbox.hComm, EV_RXCHAR);
		do {
			ciDialbox.bActive = false;
			Sleep(100);
			GetExitCodeThread(ciDialbox.hCommThread, &dwState);
			if (iTimeout > 0) {
				iTimeout--;
				if (iTimeout == 0) break;
			}
		} while (dwState == STILL_ACTIVE);
		if (dwState == STILL_ACTIVE) {
			if (AfxMessageBox("Force disconnect the dialbox?", MB_OKCANCEL) == IDOK) {
				if (TerminateThread(ciDialbox.hCommThread, FALSE) == 0) {
					AfxMessageBox("Disconnection error.\r\nTerminate the process from Task Manager.");
				}
			}
		}
	}


	if (ciDialbox.hCommThread != INVALID_HANDLE_VALUE) {
		if (CloseHandle(ciDialbox.hCommThread)) ciDialbox.hCommThread = INVALID_HANDLE_VALUE;
	}
	if (ciDialbox.hComm != INVALID_HANDLE_VALUE) {
		PurgeComm(ciDialbox.hComm, PURGE_RXCLEAR);
		if (CloseHandle(ciDialbox.hComm)) ciDialbox.hComm = INVALID_HANDLE_VALUE;
	}
	if (ciDialbox.hEvent != INVALID_HANDLE_VALUE) {
		if (CloseHandle(ciDialbox.hEvent)) ciDialbox.hEvent = INVALID_HANDLE_VALUE;
	}
	sCommConnected.Empty();
}

// CDlgDialbox メッセージ ハンドラ

BOOL CDlgDialbox::OnInitDialog()
{
	CDialog::OnInitDialog();

	sComPortList = EnumComPort();
	bool bSelected = false;
	if (!sComPortList.IsEmpty()) {
		int ipos = 0;
		CString sPortName = sComPortList.Tokenize(_T("\r"), ipos);
		while (!sPortName.IsEmpty()) {
			CString sDeviceName = sComPortList.Tokenize(_T("\r"), ipos);
			int iItem = m_ComPort.AddString(sPortName + " (" + sDeviceName + ")");
			if (!bSelected) {
				if (sCommConnected.IsEmpty()) {
					if (sDeviceName.Find(BLUETOOTH_DEVICENAME_KEY) >= 0) {
						m_ComPort.SetCurSel(iItem);
						bSelected = true;
					}
				} else if (sCommConnected == sPortName) {
					m_ComPort.SetCurSel(iItem);
					bSelected = true;
				}
			}
			sPortName = sComPortList.Tokenize(_T("\r"), ipos);
		}
		if (!bSelected) m_ComPort.SetCurSel(0);
	}

	CString sAction[(DIALBOX_NACTION > DIALBOX_NBUTTONACTION) ? DIALBOX_NACTION : DIALBOX_NBUTTONACTION];
	sAction[DIALBOX_SCROLLX] = DIALBOX_SCROLLX_STR;//=0
	sAction[DIALBOX_SCROLLY] = DIALBOX_SCROLLY_STR;
	sAction[DIALBOX_MAG] = DIALBOX_MAG_STR;
	sAction[DIALBOX_FRAME] = DIALBOX_FRAME_STR;
	sAction[DIALBOX_FRAMEFAST] = DIALBOX_FRAMEFAST_STR;
	sAction[DIALBOX_CONTRAST] = DIALBOX_CONTRAST_STR;
	sAction[DIALBOX_BRIGHTNESS] = DIALBOX_BRIGHTNESS_STR;
	sAction[DIALBOX_NOACTION] = DIALBOX_NOACTION_STR;//=7
	for (int i=0; i<DIALBOX_NDIALS; i++) {
		for (int j=0; j<DIALBOX_NACTION; j++) {
			int iItem = m_cmbDialAction[i].AddString(sAction[j]);
			m_cmbDialAction[i].SetItemData(iItem, j);
			if (m_ucDialAction[i] == j) m_cmbDialAction[i].SetCurSel(iItem);
		}
	}

	sAction[DIALBOX_OPENQUEUE] = DIALBOX_OPENQUEUE_STR;
	sAction[DIALBOX_NOBUTTONACTION] = DIALBOX_NOBUTTONACTION_STR;
	for (int i=0; i<DIALBOX_NBUTTONS; i++) {
		for (int j=0; j<DIALBOX_NBUTTONACTION; j++) {
			int iItem = m_cmbButtonAction[i].AddString(sAction[j]);
			m_cmbButtonAction[i].SetItemData(iItem, j);
			if (m_ucButtonAction[i] == j) m_cmbButtonAction[i].SetCurSel(iItem);
		}
	}

	SetDlgItemText(IDC_DIALBOX_RX, "0");
	ciDialbox.hViewWnd = this->m_hWnd;
	EnableCtrl();
	return TRUE;	// return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

CString CDlgDialbox::GetBthComPort(CString sAddr) {
	CString sRtn = "";
	int iPort = 0;

	HKEY hKey1;
	DWORD dwNameLen1 = MAX_PATH;
	char pcName1[MAX_PATH + 1];

	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Enum\\BTHENUM", NULL, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey1);
	if (lResult != ERROR_SUCCESS) return sRtn;

	for (DWORD dwIndex1=0; ; dwIndex1++) {
		dwNameLen1 = MAX_PATH;
		lResult = RegEnumKeyEx( hKey1, dwIndex1, pcName1, &dwNameLen1, NULL, NULL, NULL, NULL);
		if (lResult == ERROR_NO_MORE_ITEMS) break;
		if (lResult != ERROR_SUCCESS) { continue;}
		HKEY hKey2;
		DWORD dwNameLen2 = MAX_PATH;
		char pcName2[MAX_PATH + 1];

		CString sSubKey;
		sSubKey.Format("SYSTEM\\CurrentControlSet\\Enum\\BTHENUM\\%s", pcName1);
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, NULL, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey2);
		if (lResult != ERROR_SUCCESS) {continue;}

		for (DWORD dwIndex2=0; ; dwIndex2++) {
			dwNameLen2 = MAX_PATH;
			lResult = RegEnumKeyEx(hKey2, dwIndex2, pcName2, &dwNameLen2, NULL, NULL, NULL, NULL);
			if (lResult == ERROR_NO_MORE_ITEMS) break;
			if (lResult != ERROR_SUCCESS) continue;

			sSubKey.Format("SYSTEM\\CurrentControlSet\\Enum\\BTHENUM\\%s\\%s", pcName1, pcName2);
			sSubKey.MakeUpper();

			if(sSubKey.Find(sAddr) < 0) continue;

			HKEY hKey3;
			char pcPort[100 + 1];
			DWORD dwLen = 100;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey + "\\Device Parameters", 0, KEY_READ, &hKey3) != ERROR_SUCCESS) continue;
			if (RegQueryValueEx(hKey3, "PortName", NULL, NULL, (LPBYTE)&pcPort, &dwLen) == ERROR_SUCCESS) {
				RegCloseKey(hKey3);
				pcPort[dwLen] = 0;
				CString sPort(pcPort);
				sPort.MakeUpper();
				if(sPort.Find("COM") >= 0) {
					sRtn = sPort;
//					sRtn += sSubKey + "\r\n" + sPort + "\r\n";
				}
			}
		}//for(dwIndex2++)
		RegCloseKey(hKey2);
    }//for(dwIndex1++)
    RegCloseKey(hKey1);
//	AfxMessageBox(sRtn + "\r\n" + sAddr);
	return sRtn;
}

CString CDlgDialbox::EnumComPort() {
	CString sRtn = "";
	//161229 list bluetooth com port

	BLUETOOTH_FIND_RADIO_PARAMS btFindRadioParams = {sizeof(BLUETOOTH_FIND_RADIO_PARAMS)};
	BLUETOOTH_RADIO_INFO btRadioInfo = {sizeof(BLUETOOTH_RADIO_INFO), 0};
	BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParams = {
		sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS), 
		TRUE,	//BOOL fReturnAuthenticated;
		TRUE,	//BOOL fReturnRemembered;
		TRUE,	//BOOL fReturnUnknown;
		TRUE,	//BOOL fReturnConnected;
		FALSE,	//BOOL fIssueInquiry;
		10,		//UCHAR cTimeoutMultiplier;
		NULL	//HANDLE hRadio;
	};

	BLUETOOTH_DEVICE_INFO btDeviceInfo = {sizeof(BLUETOOTH_DEVICE_INFO), 0};

	HANDLE hRadio = NULL;
	HBLUETOOTH_DEVICE_FIND hbtDeviceFind = NULL;

	HBLUETOOTH_RADIO_FIND hbtRadioFind = BluetoothFindFirstRadio(&btFindRadioParams, &hRadio);
	if (hbtRadioFind) {
		do {
			if (BluetoothGetRadioInfo(hRadio, &btRadioInfo) == ERROR_SUCCESS) {
				btDeviceSearchParams.hRadio = hRadio;
				ZeroMemory(&btDeviceInfo, sizeof(BLUETOOTH_DEVICE_INFO));
				btDeviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

				hbtDeviceFind = BluetoothFindFirstDevice(&btDeviceSearchParams, &btDeviceInfo);
				if (hbtDeviceFind) {
					do {
						CString sDeviceName(btDeviceInfo.szName);
						CString sAddr;
						sAddr.Format("%02x%02x%02x%02x%02x%02x", 
							btDeviceInfo.Address.rgBytes[5],btDeviceInfo.Address.rgBytes[4], btDeviceInfo.Address.rgBytes[3],
									btDeviceInfo.Address.rgBytes[2], btDeviceInfo.Address.rgBytes[1], btDeviceInfo.Address.rgBytes[0]);
						sAddr.MakeUpper();
						CString sComPort = GetBthComPort(sAddr);
						if (!sComPort.IsEmpty() && (sDeviceName.Find(BLUETOOTH_DEVICENAME_KEY) >= 0)) {
							sRtn += sComPort + "\r" + sDeviceName + "\r";
						}

					} while (BluetoothFindNextDevice(hbtDeviceFind, &btDeviceInfo));
				}
				BluetoothFindDeviceClose(hbtDeviceFind);
			}
			CloseHandle(hRadio);
		} while (BluetoothFindNextRadio(hbtRadioFind, &hRadio));
		BluetoothFindRadioClose(hbtRadioFind);
	}

	//list other serial ports
	HKEY hKey = NULL;
	TCHAR tcName[256];
	BYTE cData[256];
	DWORD dwNameBuffSize = sizeof(tcName);
	DWORD dwDataBuffSize = sizeof(cData);
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM" ), NULL, KEY_READ, &hKey)) {
		int i = 0;
		while (RegEnumValue(hKey, i, tcName, &dwNameBuffSize, NULL, NULL, cData, &dwDataBuffSize) == ERROR_SUCCESS) {
			if (!strstr(tcName, "BthModem")) {
				CString sPortName;
				sPortName.Format("%s\r%s\r", (char*)cData, tcName);
				sRtn += sPortName;
			}
			dwNameBuffSize = sizeof(tcName);
			dwDataBuffSize = sizeof(cData);
			i++;
		}
	}
	return sRtn;
}

void CDlgDialbox::OnBnClickedDialboxConnect()
{
	CString sComPort = "";
	const int iItem = m_ComPort.GetCurSel();
	if (iItem < 0) return;
	m_ComPort.GetLBText(iItem, sComPort);
	if (sComPort.IsEmpty()) return;
	int err = OpenDialboxBluetoothSPP("\\\\.\\" + sComPort.SpanExcluding(" "));
	if (err) {
		AfxMessageBox( 
			"Error on connecting the device.\r\n1. Is the selected device placed near your PC? Is it turned on?\r\n"
			"2. If this error repeatedly occurs even when the correct device is selected, "
			"go to the Bluetooth config window and remove the device and do pairing again."
		);
	}
	EnableCtrl();
}

void CDlgDialbox::EnableCtrl() {
	GetDlgItem(IDC_DIALBOX_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DIALBOX_DISCONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DIALBOX_COMPORT)->EnableWindow(TRUE);
	if (!sCommConnected.IsEmpty()) {
		GetDlgItem(IDC_DIALBOX_CONNECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DIALBOX_DISCONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DIALBOX_COMPORT)->EnableWindow(FALSE);
	}
}

void CDlgDialbox::OnBnClickedDialboxDisconnect()
{
	CloseDialboxBluetoothSPP(30);
	EnableCtrl();
}

void CDlgDialbox::OnOK()
{
	UpdateData();
	for (int i=0; i<DIALBOX_NDIALS; i++) {
		int iItem = m_cmbDialAction[i].GetCurSel();
		m_ucDialAction[i] = (char)m_cmbDialAction[i].GetItemData(iItem);
		m_ucDialCW[i] = m_sDialCW[i].GetAt(0);
		m_ucDialCCW[i] = m_sDialCCW[i].GetAt(0);
	}

	for (int i=0; i<DIALBOX_NBUTTONS; i++) {
		int iItem = m_cmbButtonAction[i].GetCurSel();
		m_ucButtonAction[i] = (char)m_cmbButtonAction[i].GetItemData(iItem);
		m_ucButtonRel[i] = m_sButtonRel[i].GetAt(0);
	}

	if (pv) {
		ciDialbox.hViewWnd = pv->m_hWnd;
	}
	CDialog::OnOK();
}

LRESULT CDlgDialbox::OnDialbox(WPARAM wParam, LPARAM lParam) {
	char cData = ((char*)wParam)[0];
	CString line; line.Format("%d '%c'", cData, cData);
	SetDlgItemText(IDC_DIALBOX_RX, line);
	return 0;
}

//void CDlgDialbox::OnCbnSelchangeDialboxComport()
//{
//	// TODO: ここにコントロール通知ハンドラ コードを追加します。
//}
