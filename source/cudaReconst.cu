#if !defined( _CUDARECONST_CU_ )
#define _CUDARECONST_CU_

#include <math.h>
#include <helper_cuda.h>
#include <cufft.h>
//#include "cudaFFT.cu"

////////////////////////
//Tomographic reconstruction routines
typedef int BOOL;

#include "reconstinfo.h"
#include "cudaReconst.h"
#include "constants.h"
//#include "sm_12_atomic_functions.h"

texture<int, 1, cudaReadModeElementType> tex_igp;
int blocksize = CUDA_BLOCKSIZE;

__global__ void
projKernel9(int* d_ifp, int ixdimp, float fsin, float fcos, float foffset) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y;
	float fx1 = ix * fcos + iy * fsin + foffset;
	int ixy = ix + ixdimp * iy;
	d_ifp[ixy] += tex1Dfetch(tex_igp, (int)(fx1));
}

__global__ void
projAtomicKernel9(int* d_ifp, int ixdimp, float fsin, float fcos, float foffset) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y;
	float fx1 = ix * fcos + iy * fsin + foffset;
	int ixy = ix + ixdimp * iy;
	atomicAdd( &(d_ifp[ixy]), tex1Dfetch(tex_igp, (int)(fx1)) );
}

/*
__global__ void
projKernel9b(int* d_ifp, int ixdimp, float fsin, float fcos, float foffset) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	foffset += ix * fcos;
	for (int iy = 0; iy < ixdimp; iy++) {
		int ix1 = (int)(iy * fsin + foffset);
		int ixy = ix + ixdimp * iy;
		d_ifp[ixy] += tex1Dfetch(tex_igp, ix1);
	}
}

__global__ void
projKernel10(int* d_ifp, int* d_igp, int ixdimp, float* d_fcos, float* d_fsin, float fcenter, int iSinoDimX, int iSinoDimY) {
	//prepare shared mem
	extern __shared__ float s_fcos[];
	float* s_fsin = &(s_fcos[iSinoDimY]);
	const int icpyblock = (iSinoDimY + blockDim.x - 1) / blockDim.x;
	const int icpyfrom = threadIdx.x * icpyblock;
	const int icpyto = (threadIdx.x + 1) * icpyblock < iSinoDimY ? (threadIdx.x + 1) * icpyblock : iSinoDimY;
	for (int i = icpyfrom; i < icpyto; i++) {
		s_fcos[i] = d_fcos[i];
		s_fsin[i] = d_fsin[i];
	}
	__syncthreads();
	//params
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y;
	const int ixy = ix + ixdimp * iy;
	const int ixdimh = ixdimp / 2;
	if ((ix - ixdimh)*(ix - ixdimh) + (iy - ixdimh)*(iy - ixdimh) > ixdimh * ixdimh) return;
	//ix can be ix - ixdimh though this will change pixel values
	//ix -= ixdimh;
	//iy -= ixdimh;
	//if (ix * ix + iy * iy > ixdimh * ixdimh) return;
	//pixel sum
	int isum = 0;
	for (int i = 0; i < iSinoDimY; i++) {
		const float foffset = fcenter - ixdimh * (s_fcos[i] + s_fsin[i]);
		float fx1 = ix * s_fcos[i] + iy * s_fsin[i] + foffset;
		if ((fx1 >= 0)&&(fx1 < iSinoDimX)) isum += d_igp[(int)fx1 + iSinoDimX * i];
		//int ix1 = (int)(ix * d_fcos[i] + iy * d_fsin[i] + fcenter);//this will change pixel values
		//if ((ix1 >= 0)&&(ix1 < iSinoDimX)) isum += d_igp[ix1 + iSinoDimX * i];
	}
	d_ifp[ixy] = isum;
}
*/

//gazo.cpp, cudaReconstHost.cpp, and DlgProperty.cpp should also be rebuilt to switch on and off CUDAFFT.
#ifdef CUDAFFT
__global__ void
p2igpKernel(float2* d_p, int* d_igp, int ixdimp, int ihoffset, float fscale) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y;
	//const int gidx = ix * DBPT_GINTP;
	const int gidx = ix * DBPT_GINTP;
	const int pidx = ix + ihoffset;
	const float p0 = d_p[pidx].x * fscale;
	const float p1p0 = (ix == ixdimp -1)? 0.0f : (d_p[pidx + 1].x * fscale - p0) / DBPT_GINTP;
	d_igp[gidx + iy] = (int)(p0 + p1p0 * iy);
	//
	//for (int j=0; j<ixdimp; j++) {
	//	const TCmpElmnt p0 = p[j + ihoffset].re * BACKPROJ_SCALE;
	//	const TCmpElmnt p1p0 = (j == ixdimp -1)? 
	//		0.0f : (p[j + ihoffset + 1].re - p[j + ihoffset + 1].re) / DBPT_GINTP * BACKPROJ_SCALE;
	//	const int gidx = (j + imargin) * DBPT_GINTP;
	//	for (int k=0; k<DBPT_GINTP; k++) {igp[gidx + k] = (int)(p0 + p1p0 * k);}
	//}
}

__global__ void
filtKernel(float2* d_p, float* d_filt, int ndim) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ndim) return;
	d_p[ix].x *= d_filt[ix];
	d_p[ix].y *= d_filt[ix];
	//for (int k=0; k<ndim; k++) {d_p[k].x *= d_filt[k]; d_p[k].y *= d_filt[k];}
}

__global__ void
intpKernel(float2* d_p, short* d_strip, int ixdim, int ndim, int iIntpDim, float center) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdim) return;
	int iy = blockIdx.y;
	int idx = (ix - (int)center) * iIntpDim + (ndim / 2) + iy;
	if (idx < 0) return;
	if (idx >= ndim) return;
	if (iy == 0) { d_p[idx].x = d_strip[ix]; return; }
	//interpolation
	if (ix == ixdim - 1) return;
	d_p[idx].x = (float)(d_strip[ix] * (iIntpDim - iy) + d_strip[ix + 1] * iy) / iIntpDim;
//190120
//	int idx = (ix - (int)center) * iIntpDim + (ndim / 2);
//	if (idx < 0) return;
//	if (idx >= ndim) return;
//	int iy = blockIdx.y;
//	if (iy == 0) {d_p[idx].x = d_strip[ix]; return;}
//	//interpolation
//	if (ix == ixdim - 1) return;
//	d_p[idx+iy].x = (float)(d_strip[ix] * (iIntpDim - iy) + d_strip[ix+1] * iy) / iIntpDim;
	/*
	for (int k=0; k<ixdim; k++) {
		int idx = (k - (int)center) * iIntpDim + (ndim / 2);
		if (idx < 0) continue;
		if (idx >= ndim) break;
		d_p[idx].x = d_strip[k];
		//interpolation
		if (k == ixdim - 1) break;
		for (int j=1; j<iIntpDim; j++) {
			d_p[idx+j].x = (float)(d_strip[k] * (iIntpDim - j) / iIntpDim + d_strip[k+1] * j / iIntpDim);
		}
	}*/
}

extern "C" 
void CudaDeconv(int ixdim, int iIntpDim, int ndim, float center,   
				float* d_filt, short* d_strip, int* d_igp, float2* d_p, cufftHandle* fftplan) {
	//constants
	const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	const int ihoffset = ndim / 2 - ixdimh;
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
    dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil( ixdimp / (float)blocksize));
	dim3 dimGrid_ixdimp(gridsize);//0<=blockIdx.x<gridsize
	//
	//interpolation
    const unsigned int mem_size_p = sizeof(float2) * ndim;
	cudaMemset(d_p, 0, mem_size_p);
	//130923 cutilSafeCall(cudaMemset(d_p, 0, mem_size_p) );
	//090312 cudaMemset(d_p, 0, mem_size_p);
	const int gridsize_intp = (int)(ceil( ixdim / (float)blocksize));
	dim3 dimGrid_intp(gridsize_intp, iIntpDim);
	intpKernel<<< dimGrid_intp, dimBlock >>>(d_p, d_strip, ixdim, ndim, iIntpDim, center);
	//FFT-filter
	cufftExecC2C(*fftplan, (cufftComplex*)d_p, (cufftComplex*)d_p, CUFFT_FORWARD );
	//
	const int gridsize_ndim = (int)(ceil( ndim / (float)blocksize));
	dim3 dimGrid_ndim(gridsize_ndim);
	filtKernel<<< dimGrid_ndim, dimBlock >>>(d_p, d_filt, ndim);
	//
	cufftExecC2C(*fftplan, (cufftComplex*)d_p, (cufftComplex*)d_p, CUFFT_INVERSE );
	//
	float fscale = (float)BACKPROJ_SCALE / ndim;
	dim3 dimGrid_p2igp(gridsize, DBPT_GINTP);
	p2igpKernel<<< dimGrid_p2igp, dimBlock >>>(d_p, d_igp, ixdimp, ihoffset, fscale);
}
#endif

extern "C"
void CudaBackProjStream(int ixdimp, float center, int iCenterOffset, int iIntpDim, double theta, int* d_ifp, int* d_igp, cudaStream_t stream) {
	//constants
	//const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
	dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil(ixdimp / (float)blocksize));
	//igp texture
	textureReference* texRefPtr;
	if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, &tex_igp)) return;
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<int>();
	tex_igp.addressMode[0] = cudaAddressModeClamp;
	tex_igp.filterMode = cudaFilterModePoint;
	tex_igp.normalized = false;    // access with integer texture coordinates
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//
	if (cudaSuccess != cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp)) return;
	//params
	const float fcos = (float)(cos(theta) * DBPT_GINTP);
	const float fsin = (float)(-sin(theta) * DBPT_GINTP);
	//210105 const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
	//a possible revision related to the bug fix of 210105 in the corresponding intel routine
	const float fcenter = (iIntpDim == 1) ? (ixdimh + center - (int)(center)) * DBPT_GINTP :
											(ixdimh + (center - (int)(center)) * iIntpDim) * DBPT_GINTP;
	const float foffset = fcenter - ixdimh * (fcos + fsin) + DBPT_GINTP * iCenterOffset;
	//Kernel
	int iydim = ixdimp;
	dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
	projAtomicKernel9 << < dimGrid, dimBlock, 0, stream >> > (d_ifp, ixdimp, fsin, fcos, foffset);//no delays
	//projKernel9 << < dimGrid, dimBlock, 0, stream >> > (d_ifp, ixdimp, fsin, fcos, foffset);
}

__global__ void
px2igpKernelStream(float* d_px, int* d_igp, int ixdimp) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	const int iy = blockIdx.y;
	const int gidx = ix * DBPT_GINTP;
	//const int pidx = (ix + ihoffset < 0) ? 0 : ((ix + ihoffset >= ndim)? ndim-1 : ix + ihoffset);
	const float p0 = d_px[ix] * BACKPROJ_SCALE;
	const float p1p0 = (ix == ixdimp - 1) ? 0.0f : (d_px[ix + 1] * BACKPROJ_SCALE - p0) / DBPT_GINTP;
	d_igp[gidx + iy] = (int)(p0 + p1p0 * iy);
}
extern "C" void CudaSinoPx2igpStream(int ixdimp, int* d_igp, float* d_px, cudaStream_t stream) {
	dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil(ixdimp / (float)blocksize));
	dim3 dimGrid_p2igp(gridsize, DBPT_GINTP);
	px2igpKernelStream << < dimGrid_p2igp, dimBlock, 0, stream >> > (d_px, d_igp, ixdimp);
}

/*
__global__ void
px2igpKernel(float* d_px, int* d_igp, int ixdimp, int ihoffset, int ndim, int igpdimx) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	const int isino = blockIdx.z;
	const int iy = blockIdx.y;
	const int gidx = ix * DBPT_GINTP + isino * igpdimx;
	const int pidx = ix + ihoffset + isino * ndim;
	const float p0 = d_px[pidx] * BACKPROJ_SCALE;
	const float p1p0 = (ix == ixdimp - 1) ? 0.0f : (d_px[pidx + 1] * BACKPROJ_SCALE - p0) / DBPT_GINTP;//should be revised accroding to Stream version
	d_igp[gidx + iy] = (int)(p0 + p1p0 * iy);

//	for (int i = (ri->iStartSino); i < (ri->iLenSinogr - 1); i += (ri->iStepSino)) {
//		int isino = (i - (ri->iStartSino)) / (ri->iStepSino);
//		if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
//		for (int j = 0; j < ixdimp; j++) {
//			const TCmpElmnt p0 = h_px[j + ihoffset + isino * ndim] * BACKPROJ_SCALE;
//			const TCmpElmnt p1p0 = (j == ixdimp - 1) ?
//				0.0f : (h_px[j + ihoffset + 1 + isino * ndim] - h_px[j + ihoffset + isino * ndim]) / DBPT_GINTP * BACKPROJ_SCALE;
//			const int gidx = (j + imargin) * DBPT_GINTP + isino * igpdimx;
//			for (int k = 0; k < DBPT_GINTP; k++) { igp[gidx + k] = (int)(p0 + p1p0 * k); }
//		}
//	}
}

extern "C" void CudaSinoPx2igp(int ndim, int ixdimp, int igpdimx, int igpdimy, int* d_igp, float* d_px) {
	const int ixdimh = ixdimp / 2;
	const int ihoffset = ndim / 2 - 1 - ixdimh;
	dim3 dimBlock(blocksize, 1, 1);
	const int gridsize = (int)(ceil(ixdimp / (float)blocksize));
	dim3 dimGrid_p2igp(gridsize, DBPT_GINTP, igpdimy);
	px2igpKernel << < dimGrid_p2igp, dimBlock >> > (d_px, d_igp, ixdimp, ihoffset, ndim, igpdimx);
}

extern "C" 
void CudaBackProj3(int ixdim, int iIntpDim, float center, int iStartSino, int iLenSinogr, int iStepSino, float* pfdeg, float ftilt, int* d_ifp, int* d_igp) {
	//constants
	const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	const int imargin = 0;
	const int igpdimx = (ixdimp + imargin * 2) * DBPT_GINTP;
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
    dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil( ixdimp / (float)blocksize));
	//igp texture
	textureReference* texRefPtr;
	if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, &tex_igp)) return;
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<int>();
	tex_igp.addressMode[0] = cudaAddressModeClamp;
	tex_igp.filterMode = cudaFilterModePoint;
	tex_igp.normalized = false;    // access with integer texture coordinates
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//
	for (int i = iStartSino; i < iLenSinogr - 1; i += iStepSino) {
		const int isino = (i - iStartSino) / iStepSino;
		const double theta = (pfdeg[i] + ftilt) * DEG_TO_RAD; 

		if (cudaSuccess != cudaBindTexture(0, texRefPtr, &(d_igp[isino * igpdimx]), &channelDesc, mem_size_igp)) return;
		//params
		const float fcos = (float)(cos(theta) * DBPT_GINTP);
		const float fsin = (float)(-sin(theta) * DBPT_GINTP);
		const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
		const float foffset = fcenter - ixdimh * (fcos + fsin);
		//Kernel
		int iydim = ixdimp;
		dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
		projKernel9 << < dimGrid, dimBlock >> > (d_ifp, ixdimp, fsin, fcos, foffset);
		//Kernel9b: slow
		//dim3 dimGrid9b(gridsize, 1);// 0<=blockIdx.x<gridsize
		//projKernel9b << < dimGrid9b, dimBlock >> > (d_ifp, ixdimp, fsin, fcos, foffset);
	}

	cudaUnbindTexture(tex_igp);
}

//190101
extern "C"
void CudaBackProj2(int ixdim, int iIntpDim, float center, int iSinoDimX, int iSinoDimY, int* d_ifp, int* d_igp, float* d_fcos, float* d_fsin) {
	//constants
	const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	//
	//const int iSinoDimX = (ixdimp + imargin * 2) * DBPT_GINTP;
	//const int iSinoDimY = ((ri->iLenSinogr - 1) - (ri->iStartSino) + (ri->iStepSino) - 1) / (ri->iStepSino);
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
	dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil(ixdimp / (float)blocksize));
	//params
	const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
	//Kernel
	const int iydim = ixdimp;
	const int shared_mem_size = iSinoDimY * sizeof(float) * 2;
	dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
	projKernel10 << < dimGrid, dimBlock, shared_mem_size >> > (d_ifp, d_igp, ixdimp, d_fcos, d_fsin, fcenter, iSinoDimX, iSinoDimY);
}

extern "C"
void CudaBackProj(int ixdim, int iIntpDim, float center, int iCenterOffset, double theta, int* d_ifp, int* d_igp) {
	//constants
	const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
	dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil(ixdimp / (float)blocksize));
	//igp texture
	textureReference* texRefPtr;
	if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, &tex_igp)) return;
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<int>();
	tex_igp.addressMode[0] = cudaAddressModeClamp;
	tex_igp.filterMode = cudaFilterModePoint;
	tex_igp.normalized = false;    // access with integer texture coordinates
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//
	if (cudaSuccess != cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp)) return;
	//params
	const float fcos = (float)(cos(theta) * DBPT_GINTP);
	const float fsin = (float)(-sin(theta) * DBPT_GINTP);
	const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
	const float foffset = fcenter - ixdimh * (fcos + fsin) + DBPT_GINTP * iCenterOffset;
	//Kernel
	int iydim = ixdimp;
	dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
	projKernel9 << < dimGrid, dimBlock >> > (d_ifp, ixdimp, fsin, fcos, foffset);
	//Kernel9b: slow
	//dim3 dimGrid9b(gridsize, 1);// 0<=blockIdx.x<gridsize
	//projKernel9b << < dimGrid9b, dimBlock >> > (d_ifp, ixdimp, fsin, fcos, foffset);
}
*/

__global__ void
sinoKernel(short* d_Dark, short* d_Incident, short* d_Strip, int ixmul, float t0) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixmul) return;
	if (d_Incident[ix] <= 0) {d_Strip[ix] = 0; return;}
	int iSample = d_Strip[ix] - d_Dark[ix];
	if (iSample < SINOGRAM_PIXEL_MIN) {d_Strip[ix] = 0; return;}
	d_Strip[ix] = (short)(log(d_Incident[ix] / (double)iSample) * LOG_SCALE + 0.5);
	//for (int j=0; j<ixmul; j++) {
	//	int d_Incident = d_Incident0[j] + (int)((d_Incident1[j] - d_Incident0[j] + 0.5) * t0) - d_Dark[j];
	//	if (d_Incident <= 0) {d_Strip[j] = 0; continue;}
	//	int iSample = d_Strip[j] - d_Dark[j];
	//	if (iSample < SINOGRAM_PIXEL_MIN) {d_Strip[j] = 0; continue;}
	//	d_Strip[j] = (short)(log((double)d_Incident / (double)iSample) * LOG_SCALE + 0.5);
	//}
}

extern "C" 
void CudaSinogram(short* d_Strip, int ixmul, float t0) {
    dim3 dimBlock(blocksize, 1);
	const int gridsize_ixmul = (int)(ceil( ixmul / (float)blocksize));
	dim3 dimGrid_ixmul(gridsize_ixmul);
	short* d_Incident = &(d_Strip[ixmul]);
	short* d_Dark = &(d_Strip[ixmul * 2]);
	//normal
	sinoKernel<<< dimGrid_ixmul, dimBlock >>>(d_Dark, d_Incident, d_Strip, ixmul, t0);
}

__global__ void
lsqfitKernel(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry, int ix, int iy, 
					unsigned __int64* d_result) {
	int jrx = blockDim.x * blockIdx.x + threadIdx.x;
	if (jrx >= ixref) return;
	const int jqx = jrx + ix;
	if ((jqx < 0)||(jqx >= ixqry)) return;
	//unsigned __int64 nlsqsum = 0;
	int nlsqsum = 0;
	unsigned __int64 ilsqsum = 0;
	for (int jry=0; jry<iyref; jry++) {
		const int jqy = jry + iy;
		if ((jqy < 0)||(jqy >= iyqry)) continue;
		int dr = d_ref[jry * ixref + jrx];
		if (dr == SHRT_MIN) continue;
		int dq = d_qry[jqy * ixqry + jqx];
		if (dq == SHRT_MIN) continue;
		//130207 unsigned __int64 idiff = dr - dq;
		__int64 idiff = dr - dq;
		ilsqsum += idiff * idiff;
		nlsqsum++;
	}
	d_result[jrx] = ilsqsum;
	d_result[jrx + ixref] = nlsqsum;
	//atomicAdd(&(d_result[0]), ilsqsum);
	//atomicAdd(&(d_result[1]), nlsqsum);
}

extern "C" 
void CudaLsqfit(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry,
					int ix, int iy, unsigned __int64* d_result) {
    dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil( ixref / (float)blocksize));
	dim3 dimGrid(gridsize);
	lsqfitKernel<<< dimGrid, dimBlock >>>(d_ref, d_qry, ixref, iyref, ixqry, iyqry, ix, iy, d_result);
}

extern "C" int GetCudaDeviceCount(int iMinComputeCapability) {
    int deviceCount;
	cudaError_t cerr = cudaGetDeviceCount(&deviceCount);
	if (cerr == cudaErrorNoDevice) {
		return 0;
	} else if (cerr == cudaErrorInsufficientDriver) {
		return CUDA_ERROR_INSUFFICIENT_DRIVER;
	} else if (cerr != cudaSuccess) {
		return CUDA_ERROR_DEVICE_GETCOUNT;
	}
	//130923 cutilSafeCall(cudaGetDeviceCount(&deviceCount));

    //detect virtual device or low "compute capability" 181226
	//const int iMinComputeCapability = __CUDA_ARCH__;
	//The compute capability number is set in the Project-Property-CUDA C/C++ page
	//minimum number for CUDA Tk 10.0 is compute_30 (__CUDA_ARCH__ = 300)
	for (int i=0; i<deviceCount; i++) {
		cudaDeviceProp deviceProp;
		if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, i)) { return i | CUDA_ERROR_DEVICE_GETPROPERTY; }
		if (deviceProp.major == 9999 && deviceProp.minor == 9999) { return i | CUDA_ERROR_VIRTUAL_DEVICE_DETECTED; }//virtual device
		else if (deviceProp.major * 100 + deviceProp.minor * 10 < iMinComputeCapability) { return i | CUDA_ERROR_INSUFFICIENT_COMPUTE_CAPABILITY; }//low "compute capability"
	}

//	if (deviceCount) {
//	    cudaDeviceProp deviceProp;
//		if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, 0)) return 0;
//		//130923 cutilSafeCall(cudaGetDeviceProperties(&deviceProp, 0));
//	    if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
//			//comment out the follwing line to enable virtual device
//			deviceCount = 0;
//		}
//	}
    return deviceCount;
}

extern "C" int GetCudaDeviceName(int iDevice, char* pcName, int iszcName) {//181226
	const int isz = (iszcName < 256) ? iszcName : 256;
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	if (deviceProp.major == 9999 && deviceProp.minor == 9999) { return CUDA_ERROR_VIRTUAL_DEVICE_DETECTED; }//virtual device
	for (int i = 0; i < isz; i++) {
		char c = deviceProp.name[i];
		pcName[i] = c;
		if (c == 0) break;
	}
	return 0;
}

extern "C" int GetCudaDeviceComputingCapability(int iDevice, int* piMajor, int* piMinor) {//181226
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	if (deviceProp.major == 9999 && deviceProp.minor == 9999) { return CUDA_ERROR_VIRTUAL_DEVICE_DETECTED; }//virtual device
	if (piMajor) *piMajor = deviceProp.major;
	if (piMinor) *piMinor = deviceProp.minor;
	return 0;
}

extern "C" int GetCudaMaxThreadsPerBlock(int iDevice) {
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	return deviceProp.maxThreadsPerBlock;
}

extern "C" int GetCudaWarpSize(int iDevice) {
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	return deviceProp.warpSize;
}

extern "C" int GetCudaNumberOfCores(int iDevice, int* piCores, int* piProcessors) {//190110
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	if (deviceProp.major == 9999 && deviceProp.minor == 9999) { return CUDA_ERROR_VIRTUAL_DEVICE_DETECTED; }//virtual device
	//220422 if (piCores) *piCores = _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor);
	if (piCores) {//220422
		if ((deviceProp.major == 8) && (deviceProp.minor == 6)) *piCores = 128;//GA102-7 has 64 FP32 + 64 FP/INT32 = 128 cores per SM (NVIDIA Ampere GA102 GPU Architecture, 2021)
		else *piCores = _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor);
	}
	if (piProcessors) *piProcessors = deviceProp.multiProcessorCount;
	return 0;
}

extern "C" int GetCudaClockRate(int iDevice, int* piClockRate, int* piMemRate) {//190115
	cudaDeviceProp deviceProp;
	if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, iDevice)) { return CUDA_ERROR_DEVICE_GETPROPERTY; }
	if (deviceProp.major == 9999 && deviceProp.minor == 9999) { return CUDA_ERROR_VIRTUAL_DEVICE_DETECTED; }//virtual device
	if (piClockRate) *piClockRate = deviceProp.clockRate;
	if (piMemRate) *piMemRate = deviceProp.memoryClockRate;
	return 0;
}

/*
deviceQuery.cu

There is 1 device supporting CUDA

Device 0: "Quadro FX 3700"
  Major revision number:                         1
  Minor revision number:                         1
  Total amount of global memory:                 536870912 bytes
  Number of multiprocessors:                     14
  Number of cores:                               112
  Total amount of constant memory:               65536 bytes
  Total amount of shared memory per block:       16384 bytes
  Total number of registers available per block: 8192
  Warp size:                                     32
  Maximum number of threads per block:           512
  Maximum sizes of each dimension of a block:    512 x 512 x 64
  Maximum sizes of each dimension of a grid:     65535 x 65535 x 1
  Maximum memory pitch:                          262144 bytes
  Texture alignment:                             256 bytes
  Clock rate:                                    1.25 GHz
  Concurrent copy and execution:                 No

Test PASSED
*/

#endif //_CUDARECONST_CU_