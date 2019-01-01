#if !defined(_CLRECONST_H_)
#define _CLRECONST_H_

//#include <vector_types.h>
//#include "cutil_inline.h"
#include "reconstinfo.h"

//OpenCL template
//#ifndef TEMPLATE_H_
//#define TEMPLATE_H_

#include <CL/cl.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

void CLCleanup(void);// Releases OpenCL resources (Context, Memory etc.)

void CLReconstHost(RECONST_INFO* ri, int idev, bool bReport);
void CLInitATIstreamDeviceInfo(int* iATIcount, int* iATImaxwork, int* iATIunitwork);
//TErr CLReconstMemAlloc(RECONST_INFO* ri, int idev);
void CLReconstMemFree(RECONST_INFO* ri);

extern "C" bool DBProjDlgCtrl(RECONST_INFO* ri, int iProgStep, int iSino, int* pCurrStep);

#endif //_CLRECONST_H_
