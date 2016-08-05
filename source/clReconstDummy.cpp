#include "stdafx.h"
#include "general.h"

void CLInitATIstreamDeviceInfo(int* iATIcount, int* iATImaxwork, int* iATIunitwork) {
	*iATIcount = 0; *iATImaxwork = ATISTREAM_MAXWORK; *iATIunitwork = ATISTREAM_UNITWORK;
	return;
}

void CLCleanup(void) {}

void CLReconstMemFree(RECONST_INFO* ri) {}

void CLReconstHost(RECONST_INFO* ri, int idev, bool bReport) {}

