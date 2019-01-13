#include "stdafx.h"
#include "reconstinfo.h"
#include "cudaReconst.h"
#include "ccmplx.h"
#include "cfft.h"
#include "constants.h"
#include <helper_cuda.h>
#include <cufft.h>
#include <cuda.h>
#include <cuda_runtime.h>
//#include "cutil_math.h"

#include "gazo.h"
#include "DlgQueue.h"//RQFLAGS_

#include "cerror.h"//1207020
extern CError error;

TErr CudaReconstMemAlloc(RECONST_INFO* ri, int idev) {
	cudaSetDevice( idev );
	//
	const int ixdim = ri->ixdim;
	const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iIntpDim = (int) pow((double)2, iZooming);
	const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
	const int ndim = (int) pow((double)2, ndimp);
	const float fCenter = (float)(ri->center);
	const int ixdimp = ixdim * iIntpDim;
	//
	int idim_ifp = ixdimp * ixdimp;
	//if (ixdimp & 0x01) idim_ifp += ixdimp;//making sure that iy is even. OK?
	const unsigned int mem_size_ifp = sizeof(int) * idim_ifp;
	const unsigned int mem_size_p = sizeof(float2) * ndim;
	const unsigned int mem_size_filt = sizeof(float) * ndim;
	const unsigned int mem_size_strip = sizeof(short) * ixdim;
	const int igpdimy = ((ri->iLenSinogr - 1) - (ri->iStartSino) + (ri->iStepSino) - 1) / (ri->iStepSino);
	//090211 const unsigned int mem_size_igp = sizeof(int) * ixdimp;
	//190101 const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP * igpdimy;
	const unsigned int mem_size_px = sizeof(float) * ndim * igpdimy;
	//CUDA allocate device memory
	//int* d_ifp = NULL;
	if ((ri->d_ifp != NULL)&&(mem_size_ifp > ri->max_d_ifp)) {
	    if (cudaFree(ri->d_ifp) != cudaSuccess) return 30011;
		ri->d_ifp = NULL;
		ri->max_d_ifp = 0;
	}
	if (ri->d_ifp == NULL) {
		if (cudaMalloc((void**) &(ri->d_ifp), mem_size_ifp) != cudaSuccess) return 30012;
		ri->max_d_ifp = mem_size_ifp;
	}
	//short* d_strip = NULL;
	if ((ri->d_strip != NULL) && (mem_size_strip > ri->max_d_strip)) {
		if (cudaFree(ri->d_strip) != cudaSuccess) return 30041;
		ri->d_strip = NULL;
		ri->max_d_strip = 0;
	}
	if (ri->d_strip == NULL) {
		if (cudaMalloc((void**) &(ri->d_strip), mem_size_strip) != cudaSuccess) return 30042;
		ri->max_d_strip = mem_size_strip;
	}
	if ((ri->d_px != NULL) && (mem_size_px > ri->max_d_px)) {
		if (cudaFree(ri->d_px) != cudaSuccess) return 30021;
		ri->d_px = NULL;
		ri->max_d_px = 0;
	}
	if (ri->d_px == NULL) {
		if (cudaMalloc((void**) &(ri->d_px), mem_size_px) != cudaSuccess) return 30022;
		ri->max_d_px = mem_size_px;
	}
	//int* d_igp = NULL;
	if ((ri->d_igp != NULL)&&(mem_size_igp > ri->max_d_igp)) {
	    if (cudaFree(ri->d_igp) != cudaSuccess) return 30051;
		ri->d_igp = NULL;
		ri->max_d_igp = 0;
	}
	if (ri->d_igp == NULL) {
		if (cudaMalloc((void**) &(ri->d_igp), mem_size_igp) != cudaSuccess) return 30052;
		ri->max_d_igp = mem_size_igp;
	}
//	if ((ri->d_fcos != NULL) && (igpdimy * sizeof(float) > ri->max_d_fcos)) {//190107
//		if (cudaFree(ri->d_fcos) != cudaSuccess) return 30054;
//		if (ri->d_fsin) {
//			if (cudaFree(ri->d_fsin) != cudaSuccess) return 30054;
//		}
//		ri->d_fcos = NULL;
//		ri->d_fsin = NULL;
//		ri->max_d_fcos = 0;
//	}
//	if (ri->d_fcos == NULL) {
//		if (cudaMalloc((void**) &(ri->d_fcos), igpdimy * sizeof(float)) != cudaSuccess) return 30053;
//		if (cudaMalloc((void**) &(ri->d_fsin), igpdimy * sizeof(float)) != cudaSuccess) return 30053;
//		ri->max_d_fcos = igpdimy * sizeof(float);
//	}
#ifdef CUDAFFT
	//float2* d_p = NULL;
	if ((ri->d_p != NULL) && (mem_size_p > ri->max_d_p)) {
		if (cudaFree(ri->d_p) != cudaSuccess) return 30021;
		ri->d_p = NULL;
		ri->max_d_p = 0;
	}
	if (ri->d_p == NULL) {
		if (cudaMalloc((void**) &(ri->d_p), mem_size_p) != cudaSuccess) return 30022;
		ri->max_d_p = mem_size_p;
	}
	//float* d_filt = NULL;
	if ((ri->d_filt != NULL) && (mem_size_filt > ri->max_d_filt)) {
		if (cudaFree(ri->d_filt) != cudaSuccess) return 30031;
		ri->d_filt = NULL;
		ri->max_d_filt = 0;
	}
	if (ri->d_filt == NULL) {
		if (cudaMalloc((void**) &(ri->d_filt), mem_size_filt) != cudaSuccess) return 30032;
		ri->max_d_filt = mem_size_filt;
	}
	// FFT
	//cufftHandle fftplan;
	if ((ri->fftplan != NULL)&&(ri->ifftdim != ndim)) {
		if (CUFFT_SUCCESS != cufftDestroy(ri->fftplan)) return 30062;
		ri->fftplan = NULL;
		ri->ifftdim = 0;
	}
	if (ri->fftplan == NULL) {
		if (CUFFT_SUCCESS != cufftPlan1d(&(ri->fftplan), ndim, CUFFT_C2C, 1)) return 30072;
		//130923 CUFFT_SAFE_CALL(cufftPlan1d(&(ri->fftplan), ndim, CUFFT_C2C, 1) );
		ri->ifftdim = ndim;
	}
#endif
	//
	return 0;
}
void CudaReconstMemFree(RECONST_INFO* ri) {
	cudaSetDevice( ri->iStartSino );//100515
	//
	if (ri->d_ifp != NULL) cudaFree(ri->d_ifp);
	//130923 if (ri->d_ifp != NULL) cutilSafeCall(cudaFree(ri->d_ifp));
	ri->d_ifp = NULL;
	ri->max_d_ifp = 0;
	if (ri->d_strip != NULL) cudaFree(ri->d_strip);
	ri->d_strip = NULL;
	ri->max_d_strip = 0;
	if (ri->d_px != NULL) cudaFree(ri->d_px);
	ri->d_px = NULL;
	ri->max_d_px = 0;
	if (ri->d_igp != NULL) cudaFree(ri->d_igp);
	ri->d_igp = NULL;
 	ri->max_d_igp = 0;
//	if (ri->d_fcos != NULL) cudaFree(ri->d_fcos);//190107
//	ri->d_fcos = NULL;
//	if (ri->d_fsin != NULL) cudaFree(ri->d_fsin);
//	ri->d_fsin = NULL;
//	ri->max_d_fcos = 0;
#ifdef CUDAFFT
	if (ri->d_p != NULL) cudaFree(ri->d_p);
	ri->d_p = NULL;
	ri->max_d_p = 0;
	if (ri->d_filt != NULL) cudaFree(ri->d_filt);
	ri->d_filt = NULL;
	ri->max_d_filt = 0;
	if (ri->fftplan != NULL) cufftDestroy(ri->fftplan);
	ri->fftplan = NULL;
	ri->ifftdim = 0;
#endif
}

#ifdef CUDAFFT
void CudaReconstHostFFT(RECONST_INFO* ri, int idev, bool bReport) {
	if (cudaSuccess != cudaSetDevice( idev )) {
		AfxMessageBox("cudaSetDevice error"); return;
	}
	//-----CUDA body-----//
	const int ixdim = ri->ixdim;
	const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iIntpDim = (int) pow((double)2, iZooming);
	const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
	const int ndim = (int) pow((double)2, ndimp);
	const float fCenter = (float)(ri->center);
	const int ixdimp = ixdim * iIntpDim;
	const int imargin = 0;
	const int igpdimx = (ixdimp + imargin * 2) * DBPT_GINTP;
	const int intcenter = (int)(ri->center);
	int idim_ifp = ixdimp * ixdimp;
	const unsigned int mem_size_ifp = sizeof(int) * idim_ifp;
	//
	if (((ri->dReconFlags & RQFLAGS_SINOGRAMKEPT) == 0) || (ri->d_ifp == NULL) || (ri->max_d_ifp < mem_size_ifp)) {
		if (CudaReconstMemAlloc(ri, idev)) {
			AfxMessageBox("Out of CUDA device memory.\r\n Close other dataset,\r\n or select on-board CPU from Tomography->Property menu.");
			ri->iStatus = RECONST_INFO_ERROR;
			error.Log(28801);//120720
			return;
		}
		//if (ixdimp & 0x01) idim_ifp += ixdimp;//making sure that iy is even. OK?
		const unsigned int mem_size_filt = sizeof(float) * ndim;
		const unsigned int mem_size_strip = sizeof(short) * ixdim;
		cudaMemcpy(ri->d_filt, ri->fFilter, mem_size_filt, cudaMemcpyHostToDevice);
		//
		ri->iSinoCenter = intcenter;//190108
		for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
			int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
			if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
			//100315 const int sidx = ixdim * (i * ri->iMultiplex + ri->iOffset);
			//100315 if (sidx >= ri->maxLenSinogr) break;
			const int sidx = i * ri->iMultiplex + ri->iOffset;
			if (sidx >= ri->maxSinogrLen) break;
			(*(ri->nSinogr))++;
			//140611
			if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
				if (i & 1) { if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue; }
				else { if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue; }
			}
			if (cudaSuccess != cudaMemcpy(ri->d_strip, ri->iSinogr[sidx], mem_size_strip, cudaMemcpyHostToDevice)) {
				AfxMessageBox("cudaMemcpy error: d_strip");
				break;
			}
			//130923 cutilSafeCall(cudaMemcpy(ri->d_strip, ri->iSinogr[sidx], mem_size_strip, cudaMemcpyHostToDevice) );
			CudaDeconv(ixdim, iIntpDim, ndim, isino, fCenter,
				ri->d_filt, ri->d_strip, ri->d_igp, ri->d_p, &(ri->fftplan));
		}
		cudaDeviceSynchronize();
	}
	//reset tomograph
	if (cudaSuccess != cudaMemset(ri->d_ifp, 0, mem_size_ifp)) AfxMessageBox("cudaMemset error: ifp");
	const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
	int iCurrStep = 0;
	for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
		int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
		if (bReport && (isino % 20 == 0)) {
			if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
		}
		if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		const int sidx = i * ri->iMultiplex + ri->iOffset;
		if (sidx >= ri->maxSinogrLen) break;
		if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
			if (i & 1) { if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue; }
			else { if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue; }
		}
		CudaBackProj(ixdim, iIntpDim, fCenter, intcenter - ri->iSinoCenter,
			(ri->fdeg[i] + ri->fTiltAngle) * DEG_TO_RAD, ri->d_ifp, &(ri->d_igp[isino * igpdimx]));
	}
	// copy results from device to host
	int* ifp = ri->iReconst;//each iReconst is dedicated to the thread
	if (cudaSuccess != cudaMemcpy(ifp, ri->d_ifp, mem_size_ifp, cudaMemcpyDeviceToHost)) AfxMessageBox("cudaMemcpy error: ifp");
	//-----end of CUDA body-----//
}
#else
void CudaReconstHostFFT(RECONST_INFO* ri, int idev, bool bReport) {}
#endif

/*
void CudaReconstHost(RECONST_INFO* ri, int idev, bool bReport) {
	//AfxMessageBox("131014 CudaReconstHost");
	if (cudaSuccess != cudaSetDevice( idev )) {
		AfxMessageBox("cudaSetDevice error"); return;
	}
	//-----CUDA body-----//
	const int ixdim = ri->ixdim;
	const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iIntpDim = (int) pow((double)2, iZooming);
	const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
	const int ndim = (int) pow((double)2, ndimp);
	const float fCenter = (float)(ri->center);
	const int ixdimp = ixdim * iIntpDim;
	//
	if (CudaReconstMemAlloc(ri, idev)) {
		AfxMessageBox("Out of CUDA device memory.\r\n Close other dataset,\r\n or select on-board CPU from Tomography->Property menu.");
		ri->iStatus = RECONST_INFO_ERROR;
		error.Log(28801);//120720
		return;
	}
	CCmplx* p = NULL;
	int* igp = NULL;
	//181228 const int imargin = ixdimp;
	const int imargin = 0;
	const int igpdim = (ixdimp + imargin * 2) * DBPT_GINTP;
	try{
		p = new CCmplx[ndim];
		igp = new int[igpdim];
	}
	catch(CException* e) {
		e->Delete();
		if (igp) delete [] igp;
		if (p) delete [] p;
		ri->iStatus = RECONST_INFO_ERROR;
		error.Log(28802);//120720
		return;
	}
	CFft fft;
	TErr err;//120720
	if (err = fft.Init1(ndimp, -1)) error.Log(err);
	memset(igp, 0, sizeof(int) * igpdim);
	//for (int j=0; j<igpdim; j++) {igp[j] = 0;}
	int* igpm = (int*)( ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP );
	//
	int idim_ifp = ixdimp * ixdimp;
	//if (ixdimp & 0x01) idim_ifp += ixdimp;//making sure that iy is even. OK?
	const unsigned int mem_size_ifp = sizeof(int) * idim_ifp;
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//reset tomograph
	if (cudaSuccess != cudaMemset(ri->d_ifp, 0, mem_size_ifp)) AfxMessageBox("cudaMemset error");
	//130923 cutilSafeCall(cudaMemset(ri->d_ifp, 0, mem_size_ifp) );
	const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
	int iCurrStep = 0;
	const int intcenter = (int)(ri->center);
	const int ixdimh = ixdimp / 2;
	const int ihoffset = ndim / 2 - 1 - ixdimh;
	for (int i=(ri->iStartSino); i<(ri->iLenSinogr-1); i+=(ri->iStepSino)) {
		if (bReport) {
			if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
		}
		if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		const int sidx = i * ri->iMultiplex + ri->iOffset;
		if (sidx >= ri->maxSinogrLen) break;
		(*(ri->nSinogr))++;
		short* iStrip = ri->iSinogr[sidx];
		//140611
		if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES |  RQFLAGS_USEONLYODDFRAMES)) {
			if (i & 1) {
				if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue;
			} else {
				if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue;
			}
		}
		//Deconvolution
		memset(p, 0, sizeof(CCmplx) * ndim);
		//for (int k=0; k<ndim; k++) {p[k].Reset();}
			//111206
			const int idx0 = (0 - intcenter) * iIntpDim + (ndim / 2 - 1);
			const int idx1 = (ixdim-1 - intcenter) * iIntpDim + (ndim / 2 - 1) + 1;
			for (int m=0; m<idx0; m++) {p[m].re = iStrip[0];}
			for (int m=idx1; m<ndim; m++) {p[m].re = iStrip[ixdim-1];}
			//111206
		for (int k=0; k<ixdim; k++) {
			int idx = (k - intcenter) * iIntpDim + (ndim / 2 - 1);
			if (idx < 0) continue;
			if (idx >= ndim) break;
			p[idx].re = iStrip[k];
			//interpolation
			if (k == ixdim - 1) break;
			for (int j=1; j<iIntpDim; j++) {
				p[idx+j].re = (TCmpElmnt)
					(iStrip[k] * (iIntpDim - j) / iIntpDim + iStrip[k+1] * j / iIntpDim);
			}
		}
		fft.FFT1Rev(p);	
		for (int k=0; k<ndim; k++) {p[k] *= ri->fFilter[k];}
		fft.FFT1(p);
		//
		for (int j=0; j<ixdimp; j++) {
			const TCmpElmnt p0 = p[j + ihoffset].re * BACKPROJ_SCALE;
			const TCmpElmnt p1p0 = (j == ixdimp -1)? 
				0.0f : (p[j + ihoffset + 1].re - p[j + ihoffset].re) / DBPT_GINTP * BACKPROJ_SCALE;
			const int gidx = (j + imargin) * DBPT_GINTP;
			for (int k=0; k<DBPT_GINTP; k++) {igp[gidx + k] = (int)(p0 + p1p0 * k);}
		}
		//
		if (cudaSuccess != cudaMemcpy(ri->d_igp, igpm, mem_size_igp, cudaMemcpyHostToDevice)) AfxMessageBox("cudaMemcpy error");
		CudaBackProj(ixdim, iIntpDim, fCenter, 0, 
							(ri->fdeg[i] + ri->fTiltAngle) * DEG_TO_RAD, 
							ri->d_ifp, ri->d_igp);
	}
    // copy results from device to host
	int* ifp = ri->iReconst;
	int* h_ifp;
	//120720
	try {h_ifp = new int[idim_ifp];}
	catch(CException* e) {
		e->Delete();
		if (igp) delete [] igp;
		if (p) delete [] p;
		error.Log(28803);//120720
		return;
	}
	if (cudaSuccess != cudaMemcpy(h_ifp, ri->d_ifp, mem_size_ifp, cudaMemcpyDeviceToHost)) AfxMessageBox("cudaMemcpy error");
	for (int i=ixdimp * ixdimp - 1; i>=0; i--) {ifp[i] += h_ifp[i];}
	delete [] h_ifp;
	if (igp) delete [] igp;
	if (p) delete [] p;
	//-----end of CUDA body-----//
}*/

//190101
void CudaReconstHost(RECONST_INFO* ri, int idev, bool bReport) {
	//AfxMessageBox("131014 CudaReconstHost");
	if (cudaSuccess != cudaSetDevice(idev)) {
		AfxMessageBox("cudaSetDevice error"); return;
	}
	//-----CUDA body-----//
	const int ixdim = ri->ixdim;
	const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iIntpDim = (int)pow((double)2, iZooming);
	const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
	const int ndim = (int)pow((double)2, ndimp);
	const float fCenter = (float)(ri->center);
	const int ixdimp = ixdim * iIntpDim;
	//
	CCmplx* p = NULL;
	float* h_px = NULL;
	//int* igp = NULL;
	//float* h_fcos = NULL;
	//float* h_fsin = NULL;
	//181228 const int imargin = ixdimp;
	const int imargin = 0;
	const int igpdimx = (ixdimp + imargin * 2) * DBPT_GINTP;
	const int igpdimy = ((ri->iLenSinogr - 1) - (ri->iStartSino) + (ri->iStepSino) - 1) / (ri->iStepSino);
	const int igpdim = igpdimx * igpdimy;
	const int ipxdim = ndim * igpdimy;
	const int intcenter = (int)(ri->center);
	int idim_ifp = ixdimp * ixdimp;
	const unsigned int mem_size_ifp = sizeof(int) * idim_ifp;
	if ( ((ri->dReconFlags & RQFLAGS_SINOGRAMKEPT) == 0)||(ri->d_ifp == NULL)||(ri->max_d_ifp < mem_size_ifp) ) {
		if (CudaReconstMemAlloc(ri, idev)) {
			AfxMessageBox("Out of CUDA device memory.\r\n Close other dataset,\r\n or select on-board CPU from Tomography->Property menu.");
			ri->iStatus = RECONST_INFO_ERROR;
			error.Log(28801);//120720
			return;
		}
		try {
			p = new CCmplx[ndim];
			//igp = new int[igpdim];
			//h_fcos = new float[igpdimy];
			//h_fsin = new float[igpdimy];
		}
		catch (CException* e) {
			e->Delete();
			//if (igp) delete[] igp;
			if (p) delete[] p;
			//if (h_fcos) delete[] h_fcos;
			//if (h_fsin) delete[] h_fsin;
			ri->iStatus = RECONST_INFO_ERROR;
			error.Log(28802);//120720
			return;
		}
		//if (cudaHostAlloc(&igp, igpdim * sizeof(int), cudaHostAllocWriteCombined) != cudaSuccess) {
		//	error.Log(28803);
		//	return;
		//}
		if (cudaHostAlloc(&h_px, ipxdim * sizeof(float), cudaHostAllocWriteCombined) != cudaSuccess) {
			error.Log(28803);
			return;
		}
		CFft fft;
		TErr err;//120720
		if (err = fft.Init1(ndimp, -1)) error.Log(err);
		//memset(igp, 0, sizeof(int) * igpdim);
//		int* igpm = (int*)(((DWORD_PTR)igp) + imargin * sizeof(int) * DBPT_GINTP);
		//
		const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP * igpdimy;
		ri->iSinoCenter = intcenter;//190108
		const int ixdimh = ixdimp / 2;
		const int ihoffset = ndim / 2 - 1 - ixdimh;
		for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
			int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
			if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
			const int sidx = i * ri->iMultiplex + ri->iOffset;
			if (sidx >= ri->maxSinogrLen) break;
			(*(ri->nSinogr))++;
			short* iStrip = ri->iSinogr[sidx];
			//140611
			if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
				if (i & 1) {
					if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue;
				}
				else {
					if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue;
				}
			}
			//Deconvolution
			memset(p, 0, sizeof(CCmplx) * ndim);
			//for (int k=0; k<ndim; k++) {p[k].Reset();}
			//111206
			const int idx0 = (0 - intcenter) * iIntpDim + (ndim / 2 - 1);
			const int idx1 = (ixdim - 1 - intcenter) * iIntpDim + (ndim / 2 - 1) + 1;
			for (int m = 0; m < idx0; m++) { p[m].re = iStrip[0]; }
			for (int m = idx1; m < ndim; m++) { p[m].re = iStrip[ixdim - 1]; }
			//111206
			for (int k = 0; k < ixdim; k++) {
				int idx = (k - intcenter) * iIntpDim + (ndim / 2 - 1);
				if (idx < 0) continue;
				if (idx >= ndim) break;
				p[idx].re = iStrip[k];
				//interpolation
				if (k == ixdim - 1) break;
				for (int j = 1; j < iIntpDim; j++) {
					p[idx + j].re = (TCmpElmnt)
						(iStrip[k] * (iIntpDim - j) / iIntpDim + iStrip[k + 1] * j / iIntpDim);
				}
			}
			fft.FFT1Rev(p);
			for (int k = 0; k < ndim; k++) { p[k] *= ri->fFilter[k]; }
			fft.FFT1(p);
			//
			for (int k = 0; k < ndim; k++) { h_px[k + isino * ndim] = p[k].re; }
			//double theta = (ri->fdeg[i] + ri->fTiltAngle) * DEG_TO_RAD;
			//h_fcos[isino] = (float)(cos(theta) * DBPT_GINTP);
			//h_fsin[isino] = (float)(-sin(theta) * DBPT_GINTP);
		}
		const unsigned int mem_size_px = sizeof(float) * ipxdim;
		if (cudaSuccess != cudaMemcpy(ri->d_px, h_px, mem_size_px, cudaMemcpyHostToDevice)) AfxMessageBox("cudaMemcpy error: px");
		CudaSinoPx2igp(ndim, ixdimp, igpdimx, igpdimy, ri->d_igp, ri->d_px);
//		for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
//			int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
//			if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
//			for (int j = 0; j < ixdimp; j++) {
//				//const TCmpElmnt p0 = p[j + ihoffset].re * BACKPROJ_SCALE;
//				//const TCmpElmnt p1p0 = (j == ixdimp - 1) ?
//				//	0.0f : (p[j + ihoffset + 1].re - p[j + ihoffset].re) / DBPT_GINTP * BACKPROJ_SCALE;
//				const TCmpElmnt p0 = h_px[j + ihoffset + isino*ndim] * BACKPROJ_SCALE;
//				const TCmpElmnt p1p0 = (j == ixdimp - 1) ?
//					0.0f : (h_px[j + ihoffset + 1 + isino*ndim] - h_px[j + ihoffset + isino*ndim]) / DBPT_GINTP * BACKPROJ_SCALE;
//				const int gidx = (j + imargin) * DBPT_GINTP + isino * igpdimx;
//				for (int k = 0; k < DBPT_GINTP; k++) { igp[gidx + k] = (int)(p0 + p1p0 * k); }
//			}
//		}
		//memcpy
//		if (cudaSuccess != cudaMemcpy(ri->d_igp, igp, mem_size_igp, cudaMemcpyHostToDevice)) AfxMessageBox("cudaMemcpy error: igp");
		//if (cudaSuccess != cudaMemcpy(ri->d_fcos, h_fcos, igpdimy * sizeof(float), cudaMemcpyHostToDevice)) AfxMessageBox("cudaMemcpy error: fcos");
		//if (cudaSuccess != cudaMemcpy(ri->d_fsin, h_fsin, igpdimy * sizeof(float), cudaMemcpyHostToDevice)) AfxMessageBox("cudaMemcpy error: fsin");
		//if (h_fcos) delete[] h_fcos;
		//if (h_fsin) delete[] h_fsin;
		if (p) delete[] p;
		cudaFreeHost(h_px);
		//cudaFreeHost(igp);
	}//(ri->dReconFlags & RQFLAGS_SINOGRAMKEPT == 0)
	//reset tomograph
	if (cudaSuccess != cudaMemset(ri->d_ifp, 0, mem_size_ifp)) AfxMessageBox("cudaMemset error: ifp");
	//projection
	const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
	int iCurrStep = 0;
	for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
		int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
		if (bReport && (isino % 20 == 0)) {
			if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
		}
		if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		const int sidx = i * ri->iMultiplex + ri->iOffset;
		if (sidx >= ri->maxSinogrLen) break;
		if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
			if (i & 1) { if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue; }
			else { if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue; }
		}
////////TODO190113: examine delay by the following line. if it's acceptable, enable bReport in CUDA. Add the same line in CUDA-FFT
		if (isino % 20 == 0) cudaDeviceSynchronize();
//////////////////////////////////
		CudaBackProj( ixdim, iIntpDim, fCenter, intcenter - ri->iSinoCenter,
			(ri->fdeg[i] + ri->fTiltAngle) * DEG_TO_RAD, ri->d_ifp, &(ri->d_igp[isino * igpdimx]) );
		/*/141205==>
			float theta = (ri->fdeg[i] + ri->fTiltAngle) * (float)DEG_TO_RAD;
			const float fcos = (float)(cos(theta) * DBPT_GINTP);
			const float fsin = (float)(-sin(theta) * DBPT_GINTP);
			const float fcenter = (float)((ixdimh + (ri->center) - (int)(ri->center)) * DBPT_GINTP);
			const float foffset = fcenter - ixdimh * (fcos + fsin);
			const int ixdimpg = ixdimp << DBPT_LOG2GINTP;
//			int* ipgp = (int*)(((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP);
			int* ifp = ri->iReconst;
			for (int iy=0; iy<ixdimp; iy++) {
				for (int ix=0; ix<ixdimp; ix++) {
					int ix0 = (int)(ix * fcos + iy * fsin + foffset);
					if (ix0 < 0) continue;
					if (ix0 >= ixdimpg) continue;
					ifp[ix + iy * ixdimp] += igpm[ix0];
				}
			}
		//==>141205///*///
	}
	//CudaBackProj3(ixdim, iIntpDim, fCenter, ri->iStartSino, ri->iLenSinogr, ri->iStepSino, ri->fdeg, ri->fTiltAngle, ri->d_ifp, ri->d_igp);//slightly slow
	//CudaBackProj2(ixdim, iIntpDim, fCenter, igpdimx, igpdimy, ri->d_ifp, ri->d_igp, ri->d_fcos, ri->d_fsin);//slow
	// copy results from device to host
	int* ifp = ri->iReconst;//each iReconst is dedicated to the thread
	if (cudaSuccess != cudaMemcpy(ifp, ri->d_ifp, mem_size_ifp, cudaMemcpyDeviceToHost)) AfxMessageBox("cudaMemcpy error: ifp");
	//120720
	//int* h_ifp;
	//try { h_ifp = new int[idim_ifp]; }
	//catch (CException* e) {
	//	e->Delete();
	//	error.Log(28803);//120720
	//	return;
	//}
	//if (cudaSuccess != cudaMemcpy(h_ifp, ri->d_ifp, mem_size_ifp, cudaMemcpyDeviceToHost)) AfxMessageBox("cudaMemcpy error");
	//for (int i = ixdimp * ixdimp - 1; i >= 0; i--) { ifp[i] += h_ifp[i]; }
	//delete[] h_ifp;
	//-----end of CUDA body-----//
}

void CudaSinogramHost(RECONST_INFO* ri, int idev, bool bReport) {
	//This function gives correct sinograms, but affects the resulant tomograms by memory allocation of d_Strip.
	//Therefore, intel processors should be used for sinogram generation.
	cudaSetDevice( idev );
	const int isino = ri->maxSinogrLen;
	const int ixlen = ri->ixdim;
	const int iMultiplex = ri->iMultiplex;
	const int ixmul = ixlen * iMultiplex;
	short** iSinogr = ri->iSinogr;
	char* bInc = ri->bInc;
	float* fexp = ri->fexp;//100330 float* fexp = ri->fdeg;
	short* iDark = iSinogr[iMultiplex * (isino - 1)];
	short* iIncidentx = new short[ixlen];
	if (iIncidentx == NULL) {ri->iStatus = RECONST_INFO_ERROR; return;}
	//-----CUDA body-----//
	const unsigned int mem_size_array = sizeof(short) * ixmul;
	short* d_Strip = NULL;
	cudaMalloc((void**) &d_Strip, mem_size_array * 3);
	//130923 cutilSafeCall(cudaMalloc((void**) &d_Strip, mem_size_array * 3));
	//
	short* d_Incident = &(d_Strip[ixmul * 1]);
	//short* d_Incident1 = &(d_Strip[ixmul * 2]);
	//
	float t0;
	//121019
	int i0 = -1, i1 = -1;
	for (int i=0; i<isino-1; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) {i0 = i; break;}
	}
	for (int i=0; i<isino-1; i++) {
		if (!(bInc[i] & CGAZODOC_BINC_SAMPLE)) {i0 = i; i1 = -1; continue;}
		if (i1 < 0) {
			for (int j=i; j<isino-1; j++) {
				if (bInc[j] & CGAZODOC_BINC_SAMPLE) continue;
				i1 = j;
				break;
			}
			if (i1 < 0) {
				if (i0 >= 0) {i1 = i0;}
				else {ri->iStatus = RECONST_INFO_ERROR; break;}
			}
		}
		if ((i0 < 0)||(i1 < 0)) {ri->iStatus = RECONST_INFO_ERROR; return;}
		//100315 const int i0 = (int)(iIncident0 - iSinogr) / ixlen / iMultiplex;
		//const int i1 = (int)(iIncident1 - iSinogr) / ixlen / iMultiplex;
		t0 = fexp[i0];
		const float t1 = fexp[i1];
		const float ti = fexp[i];
		if (t0 == t1) t0 = 0;
		else t0 = (ti - t0) / (t1 - t0);
		for (int k=0; k<iMultiplex; k++) {
			short* iDark = iSinogr[iMultiplex * (isino - 1) + k];
			short* d_Dark = &(d_Strip[ixmul * 2]);
			cudaMemcpy(d_Dark, iDark, mem_size_array, cudaMemcpyHostToDevice);
			//130923 cutilSafeCall(cudaMemcpy(d_Dark, iDark, mem_size_array, cudaMemcpyHostToDevice) );
			//
			short* iIncident0 = iSinogr[iMultiplex * i0 + k];
			short* iIncident1 = iSinogr[iMultiplex * i1 + k];
			for (int j=0; j<ixlen; j++) {
				iIncidentx[j] = iIncident0[j] + (short)((iIncident1[j] - iIncident0[j] + 0.5) * t0) - iDark[j];
			}
			cudaMemcpy(d_Incident, iIncidentx, mem_size_array, cudaMemcpyHostToDevice);
			//130923 cutilSafeCall(cudaMemcpy(d_Incident, iIncidentx, mem_size_array, cudaMemcpyHostToDevice) );
			//cutilSafeCall(cudaMemcpy(d_Incident1, iIncident1, mem_size_array, cudaMemcpyHostToDevice) );
			//
			if (i % ri->iStepSino != ri->iStartSino) continue;
			short* iStrip = iSinogr[iMultiplex * i + k];
			cudaMemcpy(d_Strip, iStrip, mem_size_array, cudaMemcpyHostToDevice);
			//130923 cutilSafeCall(cudaMemcpy(d_Strip, iStrip, mem_size_array, cudaMemcpyHostToDevice) );
			CudaSinogram(d_Strip, ixlen, t0);
			cudaMemcpy(iStrip, d_Strip, mem_size_array, cudaMemcpyDeviceToHost);
			//130923 cutilSafeCall(cudaMemcpy(iStrip, d_Strip, mem_size_array, cudaMemcpyDeviceToHost) );
			//for (int j=0; j<ixmul; j++) {
			//	int iIncident = iIncident0[j] + (int)((iIncident1[j] - iIncident0[j] + 0.5) * t0) - iDark[j];
			//	if (iIncident <= 0) {iStrip[j] = 0; continue;}
			//	int iSample = iStrip[j] - iDark[j];
			//	if (iSample < SINOGRAM_PIXEL_MIN) {iStrip[j] = 0; continue;}//080623
			//	iStrip[j] = (short)(log((double)iIncident / (double)iSample) * LOG_SCALE + 0.5);
			//}
		}//k
	}
	//
	cudaFree(d_Strip);
	//130923 cutilSafeCall(cudaFree(d_Strip));
	delete [] iIncidentx;
	return;
}

void CudaLsqfitMemAlloc(short** d_ppRefPixel, short** d_ppQryPixel, int* pMaxRefPixel, int* pMaxQryPixel, 
						short** ppRefPixel, short** ppQryPixel, int nRefFiles, int nQryFiles, int ixref,
						unsigned __int64** d_result) {
	//cudaSetDevice( idev );
	for (int i=0; i<nRefFiles; i++) {
		const unsigned int mem_size = sizeof(short) * pMaxRefPixel[i];
		cudaMalloc((void**) &(d_ppRefPixel[i]), mem_size);
		cudaMemcpy(d_ppRefPixel[i], ppRefPixel[i], mem_size, cudaMemcpyHostToDevice);
		//130923 cutilSafeCall(cudaMalloc((void**) &(d_ppRefPixel[i]), mem_size));
		//130923 cutilSafeCall(cudaMemcpy(d_ppRefPixel[i], ppRefPixel[i], mem_size, cudaMemcpyHostToDevice) );
	}
	for (int i=0; i<nQryFiles; i++) {
		const unsigned int mem_size = sizeof(short) * pMaxQryPixel[i];
		cudaMalloc((void**) &(d_ppQryPixel[i]), mem_size);
		cudaMemcpy(d_ppQryPixel[i], ppQryPixel[i], mem_size, cudaMemcpyHostToDevice);
		//130923 cutilSafeCall(cudaMalloc((void**) &(d_ppQryPixel[i]), mem_size));
		//130923 cutilSafeCall(cudaMemcpy(d_ppQryPixel[i], ppQryPixel[i], mem_size, cudaMemcpyHostToDevice) );
	}
	const unsigned int mem_size = sizeof(unsigned __int64) * ixref * 2;
	cudaMalloc((void**) d_result, mem_size);
	//130923 cutilSafeCall(cudaMalloc((void**) d_result, mem_size));
}

void CudaLsqfitMemFree(short** d_ppRefPixel, short** d_ppQryPixel, int nRefFiles, int nQryFiles, 
					   unsigned __int64* d_result) {
	for (int i=0; i<nRefFiles; i++) {
		cudaFree(d_ppRefPixel[i]);
		//130923 cutilSafeCall(cudaFree(d_ppRefPixel[i]));
		d_ppRefPixel[i] = NULL;
	}
	for (int i=0; i<nQryFiles; i++) {
		cudaFree(d_ppQryPixel[i]);
		//130923 cutilSafeCall(cudaFree(d_ppQryPixel[i]));
		d_ppQryPixel[i] = NULL;
	}
	cudaFree(d_result);
	//130923 cutilSafeCall(cudaFree(d_result));
}

void CudaLsqfitHost(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry,
					int ix, int iy, unsigned __int64* ilsq, unsigned __int64* nlsq, 
					unsigned __int64* d_result, unsigned __int64* h_result) {
	//int* d_ref = d_ppRefPixel[jrz];
	//int* d_qry = d_ppQryPixel[jqz];
	//unsigned __int64* h_result = new unsigned __int64[ixref * 2];
	const unsigned int mem_size_result = sizeof(unsigned __int64) * ixref * 2;
	//cutilSafeCall(cudaMemset(d_result, 0, mem_size_result) );
	CudaLsqfit(d_ref, d_qry, ixref, iyref, ixqry, iyqry, ix, iy, d_result);
	cudaMemcpy(h_result, d_result, mem_size_result, cudaMemcpyDeviceToHost);
	//130923 cutilSafeCall(cudaMemcpy(h_result, d_result, mem_size_result, cudaMemcpyDeviceToHost) );
	for (int i=0; i<ixref; i++) {
		if (h_result[i + ixref] > iyref) continue;//this occurs in some datasets, but it's strange from coding. 
													//This prevents h_result[i] becoming too large.  130209
		*ilsq += h_result[i];
		*nlsq += h_result[i + ixref];
	}
}
