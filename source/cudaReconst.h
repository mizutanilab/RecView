#if !defined(_CUDARECONST_H_)
#define _CUDARECONST_H_


//#include "cutil_inline.h"
#include "reconstinfo.h"

#define CUDA_ERROR_DEVICEINFO_MASK 0x0000ffff
#define CUDA_ERROR_INSUFFICIENT_DRIVER 0x00010000
#define CUDA_ERROR_INSUFFICIENT_COMPUTE_CAPABILITY 0x00020000
#define CUDA_ERROR_VIRTUAL_DEVICE_DETECTED 0x00040000
#define CUDA_ERROR_DEVICE_GETPROPERTY 0x00080000
#define CUDA_ERROR_DEVICE_GETCOUNT 0x00100000

//Host routine
extern "C" void CudaReconstHost(RECONST_INFO* ri, int idev, bool bReport);
extern "C" void CudaReconstMemFree(RECONST_INFO* ri);
extern "C" void CudaReconstHostFFT(RECONST_INFO* ri, int idev, bool bReport);
//extern "C" void CudaReconstHostLong(RECONST_INFO* ri, int idev);
extern "C" int GetCudaDeviceCount(int iMinComputeCapability);
extern "C" int GetCudaMaxThreadsPerBlock(int iDeviceCount);
extern "C" int GetCudaWarpSize(int iDeviceCount);
extern "C" int GetCudaDeviceName(int iDevice, char* pcName, int iszcName);
extern "C" int GetCudaDeviceComputingCapability(int iDevice, int* piMajor, int* piMinor);

extern "C" void CudaSinogramHost(RECONST_INFO* ri, int idev, bool bReport);

//CUDA
extern "C" void CudaBackProj(int ixdim, int iIntpDim, float center, double theta, int* d_ifp, int* d_igp);
extern "C" 
void CudaDeconvBackProj(int ixdim, int iIntpDim, int ndim, float center, double theta, 
				int* d_ifp, float* d_filt, short* d_strip, int* d_igp, float2* d_p, cufftHandle* fftplan);
//extern "C" 
//void CudaDeconvBackProjLong(int ixdim, int iIntpDim, int ndim, float center, float theta, 
//				long long* d_ifp, float* d_filt, short* d_strip, int* d_igp, float2* d_p, cufftHandle* fftplan);
extern "C" bool DBProjDlgCtrl(RECONST_INFO* ri, int iProgStep, int iSino, int* pCurrStep);

extern "C" 
void CudaSinogram(short* d_Strip, int ixmul, float t0);

extern "C" void CudaLsqfitMemAlloc(short** d_ppRefPixel, short** d_ppQryPixel, 
								   int* pMaxRefPixel, int* pMaxQryPixel, 
						short** ppRefPixel, short** ppQryPixel, int nRefFiles, int nQryFiles, int ixref,
						unsigned __int64** d_result);
extern "C" void CudaLsqfitMemFree(short** d_ppRefPixel, short** d_ppQryPixel, int nRefFiles, int nQryFiles, 
								  unsigned __int64* d_result);
extern "C" void CudaLsqfitHost(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry,
								int ix, int iy, unsigned __int64* ilsq, unsigned __int64* nlsq, 
								unsigned __int64* d_result, unsigned __int64* h_result);
extern "C" 
void CudaLsqfit(short* d_ref, short* d_qry, int ixref, int iyref, int ixqry, int iyqry,
					int ix, int iy, unsigned __int64* d_result);

//struct SCmplx {
//	float re;
//	float im;
//};
#endif //_CUDARECONST_H_
