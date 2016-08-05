; CLW ÉtÉ@ÉCÉãÇÕ MFC ClassWizard ÇÃèÓïÒÇä‹ÇÒÇ≈Ç¢Ç‹Ç∑ÅB

[General Info]
Version=1
LastClass=CDlgHistogram
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "gazo.h"
LastPage=0

ClassCount=10
Class1=CGazoApp
Class2=CGazoDoc
Class3=CGazoView
Class4=CMainFrame

ResourceCount=11
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDR_GAZOTYPE
Class5=CChildFrame
Class6=CAboutDlg
Resource4=IDD_ABOUTBOX (âpåÍ (±“ÿ∂))
Resource5=IDD_QUEUE (âpåÍ (±“ÿ∂))
Resource6=IDR_MENU_POPUP (âpåÍ (±“ÿ∂))
Class7=CDlgReconst
Resource7=IDR_GAZOTYPE (âpåÍ (±“ÿ∂))
Class8=CDlgHistogram
Resource8=IDR_MAINFRAME (âpåÍ (±“ÿ∂))
Class9=CDlgQueue
Resource9=IDD_RECONST (âpåÍ (±“ÿ∂))
Resource10=IDD_HISTOGRAM (âpåÍ (±“ÿ∂))
Class10=CDlgMessage
Resource11=IDD_MESSAGE (âpåÍ (±“ÿ∂))

[CLS:CGazoApp]
Type=0
HeaderFile=gazo.h
ImplementationFile=gazo.cpp
Filter=N
LastObject=IDM_TOMO_QUEUE
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CGazoDoc]
Type=0
HeaderFile=gazoDoc.h
ImplementationFile=gazoDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CGazoDoc

[CLS:CGazoView]
Type=0
HeaderFile=gazoView.h
ImplementationFile=gazoView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=IDM_TOMO_LINE


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_TOOLBAR_FNFWD
BaseClass=CMDIFrameWnd
VirtualFilter=fWC


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M


[CLS:CAboutDlg]
Type=0
HeaderFile=gazo.cpp
ImplementationFile=gazo.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
CommandCount=8
Command8=ID_APP_ABOUT

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
CommandCount=8
Command8=ID_APP_ABOUT

[MNU:IDR_GAZOTYPE]
Type=1
Class=CGazoView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
CommandCount=21
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_WINDOW_NEW
Command18=ID_WINDOW_CASCADE
Command19=ID_WINDOW_TILE_HORZ
Command20=ID_WINDOW_ARRANGE
Command21=ID_APP_ABOUT

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
CommandCount=14
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE


[MNU:IDR_GAZOTYPE (âpåÍ (±“ÿ∂))]
Type=1
Class=CGazoView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=IDM_TOMO_RECONST
Command16=IDM_TOMO_HISTG
Command17=IDM_TOMO_QUEUE
Command18=IDM_TOMO_AXIS
Command19=IDM_TOMO_STAT
Command20=IDM_TOMO_LINE
Command21=ID_VIEW_TOOLBAR
Command22=ID_VIEW_STATUS_BAR
Command23=ID_WINDOW_NEW
Command24=ID_WINDOW_CASCADE
Command25=ID_WINDOW_TILE_HORZ
Command26=ID_WINDOW_ARRANGE
Command27=ID_APP_ABOUT
Command28=IDM_HLP_DEBUG
CommandCount=28

[MNU:IDR_MAINFRAME (âpåÍ (±“ÿ∂))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=IDM_TOMO_QUEUE
Command7=ID_VIEW_TOOLBAR
Command8=ID_VIEW_STATUS_BAR
Command9=ID_APP_ABOUT
Command10=IDM_HLP_DEBUG
CommandCount=10

[DLG:IDD_ABOUTBOX (âpåÍ (±“ÿ∂))]
Type=1
Class=CAboutDlg
ControlCount=7
Control1=IDC_STATIC,static,1342177283
Control2=IDOK,button,1342373889
Control3=IDC_STATIC,static,1342308480
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[TB:IDR_MAINFRAME (âpåÍ (±“ÿ∂))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_TOOLBAR_MAG
Command10=ID_TOOLBAR_MIN
Command11=ID_TOOLBAR_BRIGHT
Command12=ID_TOOLBAR_DARK
Command13=ID_TOOLBAR_CNTUP
Command14=ID_TOOLBAR_CNTDOWN
Command15=ID_TOOLBAR_FNFR
Command16=ID_TOOLBAR_FNREV
Command17=ID_TOOLBAR_FNSR
Command18=ID_TOOLBAR_FNSF
Command19=ID_TOOLBAR_FNFWD
Command20=ID_TOOLBAR_FNFF
CommandCount=20

[ACL:IDR_MAINFRAME (âpåÍ (±“ÿ∂))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_RECONST (âpåÍ (±“ÿ∂))]
Type=1
Class=CDlgReconst
ControlCount=29
Control1=IDC_RECONST_SHOW1,button,1342242817
Control2=IDOK,button,1342242816
Control3=IDCANCEL,button,1342242816
Control4=IDC_RECONST_SLICE1,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_RECONST_CENT1,edit,1350631552
Control8=IDC_RECONST_SLICE2,edit,1350631552
Control9=IDC_STATIC,static,1342308352
Control10=IDC_RECONST_CENT2,edit,1350631552
Control11=IDC_RECONST_QUEUE,button,1342242816
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_RECONST_PIXEL,edit,1350631552
Control15=IDC_STATIC,static,1342308352
Control16=IDC_RECONST_AUTO1,button,1342242816
Control17=IDC_RECONST_FILTER,combobox,1344339971
Control18=IDC_STATIC,static,1342308352
Control19=IDC_RECONST_AUTO2,button,1342242816
Control20=IDC_RECONST_SUFFIX,edit,1350631552
Control21=IDC_STATIC,static,1342308352
Control22=IDC_RECONST_SHOW2,button,1342242816
Control23=IDC_STATIC,static,1342308352
Control24=IDC_RECONST_CUTOFF,edit,1350631552
Control25=IDC_STATIC,static,1342308352
Control26=IDC_RECONST_ORDER,edit,1484849280
Control27=IDC_RECONST_STOP,button,1342242816
Control28=IDC_RECONST_PROGRESS,msctls_progress32,1350565888
Control29=IDC_RECONST_INTP,combobox,1344339971

[CLS:CDlgReconst]
Type=0
HeaderFile=DlgReconst.h
ImplementationFile=DlgReconst.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_RECONST_INTP

[DLG:IDD_HISTOGRAM (âpåÍ (±“ÿ∂))]
Type=1
Class=CDlgHistogram
ControlCount=34
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_HISTG_BITMAP,static,1342183438
Control4=IDC_HISTG_HSTMAX,static,1342308352
Control5=IDC_HISTG_HSTLOW,static,1342308352
Control6=IDC_HISTG_HSTHIGH,static,1342308352
Control7=IDC_HISTG_MAG,msctls_updown32,1342177312
Control8=IDC_HISTG_HSTUNIT,static,1342308352
Control9=IDC_HISTG_CURSLOW,edit,1350631552
Control10=IDC_HISTG_CURSHIGH,edit,1350631552
Control11=IDC_HISTG_QUEUE,button,1342242816
Control12=IDC_HISTG_PREVIEW,button,1342242819
Control13=IDC_STATIC,button,1342177287
Control14=IDC_HISTG_TRMCENTX,edit,1350631552
Control15=IDC_HISTG_TRMCENTY,edit,1350631552
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_HISTG_TRMSIZEX,edit,1350631552
Control20=IDC_HISTG_TRMSIZEY,edit,1350631552
Control21=IDC_STATIC,static,1342308352
Control22=IDC_HISTG_TRMANGLE,edit,1350631552
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_HISTG_FILEMSG,static,1342308352
Control26=IDC_HISTG_PREFIX,edit,1350631552
Control27=IDC_STATIC,static,1342308352
Control28=IDC_HISTG_GETPATH,button,1342242816
Control29=IDC_HISTG_PROGRESS,msctls_progress32,1350565888
Control30=IDC_HISTG_TRMAVRG,edit,1350631552
Control31=IDC_STATIC,static,1342308352
Control32=IDC_HISTG_ENTRIM,button,1342242819
Control33=IDC_HISTG_BOXMSG,static,1342308352
Control34=IDC_HISTG_16BIT,button,1342242819

[CLS:CDlgHistogram]
Type=0
HeaderFile=DlgHistogram.h
ImplementationFile=DlgHistogram.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_HISTG_CURSLOW

[DLG:IDD_QUEUE (âpåÍ (±“ÿ∂))]
Type=1
Class=CDlgQueue
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_QUEUE_LIST,SysListView32,1350631553
Control4=IDC_QUEUE_STOP,button,1342242816
Control5=IDC_QUEUE_FINAL,button,1342242819

[CLS:CDlgQueue]
Type=0
HeaderFile=DlgQueue.h
ImplementationFile=DlgQueue.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_QUEUE_FINAL

[MNU:IDR_MENU_POPUP (âpåÍ (±“ÿ∂))]
Type=1
Class=?
Command1=ID_POPUPQUEUE_DEL
Command2=ID_POPUPQUEUE_UP
Command3=ID_POPUPQUEUE_DOWN
CommandCount=3

[DLG:IDD_MESSAGE (âpåÍ (±“ÿ∂))]
Type=1
Class=CDlgMessage
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_MSG_TEXT,edit,1352734724

[CLS:CDlgMessage]
Type=0
HeaderFile=DlgMessage.h
ImplementationFile=DlgMessage.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CDlgMessage

