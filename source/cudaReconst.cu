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


//160804 kernel4 was rrevised but not tested
__global__ void
projKernel4(int* d_ifp, int ixdimp, float fsin, float fcos, float foffset) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int ixy = ix;
	for (int iy=0; iy<ixdimp; iy++) {
		float fx1 = ix * fcos + iy * fsin + foffset; 
		d_ifp[ixy] += tex1Dfetch(tex_igp, (int)(fx1));
		ixy += ixdimp;
	}
}

__global__ void
projKernel8f(int* d_ifp, int ixdimp, float fsin, float fcos, float foffset) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y << 1;
	float fx1 = ix * fcos + iy * fsin + foffset; 
	int ixy = ix + ixdimp * iy;
	d_ifp[ixy] += tex1Dfetch(tex_igp, (int)(fx1));
	if (iy >= ixdimp - 1) return;
	d_ifp[ixy + ixdimp] += tex1Dfetch(tex_igp, (int)(fx1 + fsin));
}

__global__ void
p2igpCopyKernel(float2* d_p, int* d_igp, int ixdimp, int ihoffset, float fscale) {
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	//without DBPT_GINTP:
	//d_igp[ix] = (int)(d_p[ix + ihoffset].x * fscale);
	//for (int j=0; j<ixdimp; j++) {d_igp[j] = (int)(d_p[j + ihoffset].x * fscale);}
	//
	//with DBPT_GINTP
	const int gidx = ix * DBPT_GINTP;
	int iy = blockIdx.y;
	if (iy == 0) {d_igp[gidx] = (int)(d_p[ix + ihoffset].x * fscale); return;}
	const float p1p0 = (ix == ixdimp -1)? 
		0.0f : (d_p[ix + ihoffset + 1].x - d_p[ix + ihoffset].x) / DBPT_GINTP * fscale;
	d_igp[gidx + iy] = (int)(d_p[ix + ihoffset].x * fscale + p1p0 * iy);
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
	int idx = (ix - (int)center) * iIntpDim + (ndim / 2);
	if (idx < 0) return;
	if (idx >= ndim) return;
	int iy = blockIdx.y;
	if (iy == 0) {d_p[idx].x = d_strip[ix]; return;}
	//interpolation
	if (ix == ixdim - 1) return;
	d_p[idx+iy].x = (float)(d_strip[ix] * (iIntpDim - iy) + d_strip[ix+1] * iy) / iIntpDim;
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
void CudaDeconvBackProj(int ixdim, int iIntpDim, int ndim, float center, float theta,  
				int* d_ifp, float* d_filt, short* d_strip, int* d_igp, float2* d_p, cufftHandle* fftplan) {
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
	//igp texture
	textureReference* texRefPtr;
	//131011 cudaGetTextureReference((const textureReference **)&texRefPtr, "tex_igp");
	if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, &tex_igp)) return;
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<int>();
	//with DBPT_GINTP
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//without DBPT_GINTP
    //const unsigned int mem_size_igp = sizeof(int) * ixdimp;
	cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp);
	//130923 cutilSafeCall( cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp));
	//090312 cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp);
	tex_igp.addressMode[0] = cudaAddressModeClamp;
	tex_igp.filterMode = cudaFilterModePoint;
	tex_igp.normalized = false;    // access with integer texture coordinates
	//
	//interpolation
    const unsigned int mem_size_p = sizeof(float2) * ndim;
	cudaMemset(d_p, 0, mem_size_p);
	//130923 cutilSafeCall(cudaMemset(d_p, 0, mem_size_p) );
	//090312 cudaMemset(d_p, 0, mem_size_p);
	const int gridsize_intp = (int)(ceil( ixdim / (float)blocksize));
	dim3 dimGrid_intp(gridsize_intp, iIntpDim);
	intpKernel<<< dimGrid_intp, dimBlock >>>(d_p, d_strip, ixdim, ndim, iIntpDim, center);
	cudaThreadSynchronize();
	//FFT-filter
	cufftExecC2C(*fftplan, (cufftComplex*)d_p, (cufftComplex*)d_p, CUFFT_FORWARD );
	//
	const int gridsize_ndim = (int)(ceil( ndim / (float)blocksize));
	dim3 dimGrid_ndim(gridsize_ndim);
	filtKernel<<< dimGrid_ndim, dimBlock >>>(d_p, d_filt, ndim);
	cudaThreadSynchronize();
	//
	cufftExecC2C(*fftplan, (cufftComplex*)d_p, (cufftComplex*)d_p, CUFFT_INVERSE );
	//
	float fscale = (float)BACKPROJ_SCALE / ndim;
	dim3 dimGrid_p2igp(gridsize, DBPT_GINTP);
	p2igpCopyKernel<<< dimGrid_p2igp, dimBlock >>>(d_p, d_igp, ixdimp, ihoffset, fscale);
	//without DBPT_GINTP
	//p2igpCopyKernel<<< dimGrid_ixdimp, dimBlock >>>(d_p, d_igp, ixdimp, ihoffset, fscale);
	cudaThreadSynchronize();
	//params
	const float fcos = (float)(cos(theta) * DBPT_GINTP);
	const float fsin = (float)(-sin(theta) * DBPT_GINTP);
	const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
	const float foffset = fcenter - ixdimh * (fcos + fsin);
	//Kernels
	//shared memory must not be used since its use causes device driver crash
	//
	//Kernel4 for emulation mode
	//projKernel4<<< dimGrid_ixdimp, dimBlock >>>(d_ifp, ixdimp, fsin, fcos, foffset);
	//
	//Kernel8f
	int iydim = (ixdimp >> 1) + (ixdimp & 0x01);
	dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
	projKernel8f<<< dimGrid, dimBlock >>>(d_ifp, ixdimp, fsin, fcos, foffset);
	//
	//cudaThreadSynchronize();
}

extern "C" 
void CudaBackProj(int ixdim, int iIntpDim, float center, float theta, int* d_ifp, int* d_igp) {
	//constants
	const int ixdimp = ixdim * iIntpDim;
	const int ixdimh = ixdimp / 2;
	//
	//kernel parameters
	//const int blocksize = CUDA_BLOCKSIZE;// ==> blockDim.x;
    dim3 dimBlock(blocksize, 1);
	const int gridsize = (int)(ceil( ixdimp / (float)blocksize));
	//igp texture
	textureReference* texRefPtr;
	//131011 if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, "tex_igp")) return;
	if (cudaSuccess != cudaGetTextureReference((const textureReference **)&texRefPtr, &tex_igp)) return;
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<int>();
	//with DBPT_GINTP
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	if (cudaSuccess != cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp)) return;
	//130923 cutilSafeCall( cudaBindTexture(0, texRefPtr, d_igp, &channelDesc, mem_size_igp));
	tex_igp.addressMode[0] = cudaAddressModeClamp;
	tex_igp.filterMode = cudaFilterModePoint;
	tex_igp.normalized = false;    // access with integer texture coordinates
	//params
	const float fcos = (float)(cos(theta) * DBPT_GINTP);
	const float fsin = (float)(-sin(theta) * DBPT_GINTP);
	const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
	const float foffset = fcenter - ixdimh * (fcos + fsin);
	//
	//Kernel4 for emulation mode
	//dim3 dimGrid_ixdimp(gridsize);
	//projKernel4<<< dimGrid_ixdimp, dimBlock >>>(d_ifp, ixdimp, fsin, fcos, foffset);
	//
	//Kernel8f
	int iydim = (ixdimp >> 1) + (ixdimp & 0x01);
	dim3 dimGrid(gridsize, iydim);// 0<=blockIdx.x<gridsize, 0<=blockIdx.y<iydim
	projKernel8f<<< dimGrid, dimBlock >>>(d_ifp, ixdimp, fsin, fcos, foffset);
}

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

extern "C" int GetCudaDeviceCount() {
    int deviceCount;
	cudaError_t cerr = cudaGetDeviceCount(&deviceCount);
	if (cerr == cudaErrorNoDevice) {
		return 0;
	} else if (cerr == cudaErrorInsufficientDriver) {
		return CUDA_ERROR_INSUFFICIENT_DRIVER;
	} else if (cerr != cudaSuccess) {
		return 0;
	}
	//130923 cutilSafeCall(cudaGetDeviceCount(&deviceCount));

    //detect virtual device
    if (deviceCount) {
	    cudaDeviceProp deviceProp;
		if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, 0)) return 0;
		//130923 cutilSafeCall(cudaGetDeviceProperties(&deviceProp, 0));
	    if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
			//comment out the follwing line to enable virtual device
			deviceCount = 0;
		}
	}
    return deviceCount;
}

extern "C" int GetCudaMaxThreadsPerBlock() {
	int iCUDAblock = 65536;
    int iDeviceCount = GetCudaDeviceCount();
	if (iDeviceCount == CUDA_ERROR_INSUFFICIENT_DRIVER) iDeviceCount = 0;
    //detect virtual device
	cudaDeviceProp deviceProp;
    if (iDeviceCount) {
		if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, 0)) return CUDA_BLOCKSIZE;
		//130923 cutilSafeCall(cudaGetDeviceProperties(&deviceProp, 0));
	    if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
		    iDeviceCount = 0; iCUDAblock = CUDA_BLOCKSIZE;
		}
		for (int i=0; i<iDeviceCount; i++) {
			if (deviceProp.maxThreadsPerBlock < iCUDAblock) iCUDAblock = deviceProp.maxThreadsPerBlock;
		}
	}
	return iCUDAblock;
}

extern "C" int GetCudaWarpSize() {
	int iCUDAwarp = CUDA_WARPSIZE;
    int iDeviceCount = GetCudaDeviceCount();
	if (iDeviceCount == CUDA_ERROR_INSUFFICIENT_DRIVER) iDeviceCount = 0;
    //detect virtual device
	cudaDeviceProp deviceProp;
    if (iDeviceCount) {
		if (cudaSuccess != cudaGetDeviceProperties(&deviceProp, 0)) return CUDA_WARPSIZE;
		//130923 cutilSafeCall(cudaGetDeviceProperties(&deviceProp, 0));
	    if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
		    iDeviceCount = 0;
		}
		for (int i=0; i<iDeviceCount; i++) {
			if (deviceProp.warpSize < iCUDAwarp) iCUDAwarp = deviceProp.warpSize;
		}
	}
	return iCUDAwarp;
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