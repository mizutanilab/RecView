#include "stdafx.h"
#include "general.h"

extern "C" int GetCudaDeviceCount() {
	return 0;
}

extern "C" TErr CudaReconstMemAlloc(RECONST_INFO* ri, int idev) {
	return 0;
}

extern "C" void CudaReconstMemFree(RECONST_INFO* ri) {
}

extern "C" void CudaReconstHostFFT(RECONST_INFO* ri, int idev, bool bReport) {
}

extern "C" void CudaSinogramHost(RECONST_INFO* ri, int idev, bool bReport) {
}

extern "C" void CudaReconstHost(RECONST_INFO* ri, int idev, bool bReport, bool bEnStream) {
}

extern "C" void CudaLsqfitMemAlloc(short** d_ppRefPixel, short** d_ppQryPixel, 
								   int* pMaxRefPixel, int* pMaxQryPixel, 
						short** ppRefPixel, short** ppQryPixel, int nRefFiles, int nQryFiles, int ixref,
						unsigned __int64** d_result) {
}
extern "C" void CudaLsqfitMemFree(short** d_ppRefPixel, short** d_ppQryPixel, int nRefFiles, int nQryFiles, 
					   unsigned __int64* d_result) {
}
extern "C" void CudaLsqfitHost(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry,
					int ix, int iy, unsigned __int64* ilsq, unsigned __int64* nlsq, 
					unsigned __int64* d_result, unsigned __int64* h_result) {
}

int blocksize = CUDA_BLOCKSIZE;

extern "C" int GetCudaMaxThreadsPerBlock() {
	return CUDA_BLOCKSIZE;
}

extern "C" int GetCudaWarpSize() {
	return CUDA_WARPSIZE;
}

#define CUDA_ERROR_DEVICE_GETPROPERTY 0x00080000

extern "C" int GetCudaDeviceName(int iDevice, char* pcName, int iszcName) {//181226
	return CUDA_ERROR_DEVICE_GETPROPERTY;
}

extern "C" int GetCudaDeviceComputingCapability(int iDevice, int* piMajor, int* piMinor) {//181226
	return CUDA_ERROR_DEVICE_GETPROPERTY;
}
extern "C" int GetCudaNumberOfCores(int iDevice, int* piCores, int* piProcessors) {
	return CUDA_ERROR_DEVICE_GETPROPERTY;
}

extern "C" int GetCudaClockRate(int iDevice, int* piClockRate) {
	return CUDA_ERROR_DEVICE_GETPROPERTY;
}

extern "C" cudaError_t cudaFreeHost(void* ptr) { return cudaSuccess; }//190108

extern "C" cudaError_t cudaHostAlloc(void** ptr, size_t size, unsigned int flags) { return cudaSuccess; }//190108

extern "C" cudaError_t cudaMallocHost(int** ptr, size_t size) { return cudaSuccess; }//190110

extern "C" cudaError_t cudaStreamDestroy(cudaStream_t stream) { return cudaSuccess; }//190707
