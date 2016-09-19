#include "stdafx.h"
#include "ccmplx.h"
#include "cfft.h"
#include "gazo.h"
#include "DlgQueue.h"

//OpenCL template
#include "clReconst.h"

/*** GLOBALS ***/
cl_context          context;
cl_device_id        *devices;
int					nDevices;
cl_command_queue    commandQueue[ATISTREAM_MAXDEVICES];

cl_program program;

// This program uses only one kernel and this serves as a handle to it
cl_kernel  kernel;



//brief Releases program's resources 
void cleanupHost(void) {
    if(devices != NULL) {
        free(devices);
        devices = NULL;
    }
}

//brief Release OpenCL resources (Context, Memory etc.) 
void CLCleanup(void) {
    cl_int status;

    status = clReleaseKernel(kernel);
    if(status != CL_SUCCESS) {
		//AfxMessageBox("Error: In clReleaseKernel");
		//return 1; 
	}
    status = clReleaseProgram(program);
    if(status != CL_SUCCESS){
		//AfxMessageBox("Error: In clReleaseProgram");
		//return 1; 
	}
	//previously memfree here
	for (int i=0; i<nDevices; i++) {
	    status = clReleaseCommandQueue(commandQueue[i]);
		if(status != CL_SUCCESS) {
			//AfxMessageBox("Error: In clReleaseCommandQueue");
			//return 1;
		}
	}
    status = clReleaseContext(context);
    if(status != CL_SUCCESS) {
		//AfxMessageBox("Error: In clReleaseContext");
		//return 1;
	}
	cleanupHost();
	return;
}

TErr CLReconstMemAlloc(RECONST_INFO* ri, int idev) {
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
	const unsigned int mem_size_ifp = sizeof(cl_int) * idim_ifp;
	const unsigned int mem_size_igp = sizeof(cl_int) * ixdimp * DBPT_GINTP;
	//const unsigned int mem_size_p = sizeof(float2) * ndim;
	//const unsigned int mem_size_filt = sizeof(float) * ndim;
	//const unsigned int mem_size_strip = sizeof(short) * ixdim;
	//
	//OpenCL allocate device memory
    cl_int   status;
	//
	if ((ri->d_ifp != NULL)&&(mem_size_ifp > ri->max_d_ifp)) {
		if(clReleaseMemObject((cl_mem)ri->d_ifp) != CL_SUCCESS) return 30011;
		ri->d_ifp = NULL;
		ri->max_d_ifp = 0;
	}
	if (ri->d_ifp == NULL) {
		ri->d_ifp = clCreateBuffer(context, CL_MEM_READ_WRITE, mem_size_ifp, NULL, &status);
		if (status != CL_SUCCESS) return 30012;
		ri->max_d_ifp = mem_size_ifp;
	}
	//int* d_igp = NULL;
	if ((ri->d_igp != NULL)&&(mem_size_igp > ri->max_d_igp)) {
	    if (clReleaseMemObject((cl_mem)ri->d_igp) != CL_SUCCESS) return 30051;
		ri->d_igp = NULL;
		ri->max_d_igp = 0;
	}
	//cl_image_format cif;
	//cif.image_channel_data_type = CL_SIGNED_INT32;
	//cif.image_channel_order = CL_RGBA;
	//const size_t image_width = (ixdimp * DBPT_GINTP) / 4;
	if (ri->d_igp == NULL) {
		ri->d_igp = clCreateBuffer(context, CL_MEM_READ_WRITE, mem_size_igp, NULL, &status);
		//ri->d_igp = clCreateImage2D(context, CL_MEM_READ_ONLY, &cif, image_width, 1, 0, NULL, &status);
		if (status != CL_SUCCESS) return 30052;
		ri->max_d_igp = mem_size_igp;
	}
	return 0;
}

int initReconstCL(int* pDevices) {
	//
    cl_int status = 0;
    size_t deviceListSize;
	nDevices = 0;
	if (pDevices) *pDevices = 0;

    //Have a look at the available platforms and pick either
    //the AMD one if available or a reasonable default.

    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(status != CL_SUCCESS) {
        AfxMessageBox("Error: Getting Platforms. (clGetPlatformsIDs) 00");
        return 1;
    }
    
    if(numPlatforms > 0) {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if(status != CL_SUCCESS) {
            AfxMessageBox("Error: Getting Platform Ids. (clGetPlatformsIDs)");
            return 1;
        }
        for(unsigned int i=0; i < numPlatforms; ++i) {
            char pbuff[100];
            status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(pbuff), pbuff, NULL);
            if(status != CL_SUCCESS) {
                AfxMessageBox("Error: Getting Platform Info.(clGetPlatformInfo)");
                return 1;
            }
            //platform = platforms[i];
            //if(!strcmp(pbuff, "Advanced Micro Devices, Inc.")) break;
			//CString line, msg = pbuff;
			//line.Format(" %d/%d", i, numPlatforms);
			//AfxMessageBox(msg + line);
			if(strcmp(pbuff, "Advanced Micro Devices, Inc.") == 0) {
				platform = platforms[i];
				break;
			}
        }
        delete platforms;
    }

    if(NULL == platform) {
        AfxMessageBox("No ATI stream device found.");
        return 1;
    }

    //If we could find our platform, use it. Otherwise use just available platform.
    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	/////////////////////////////////////////////////////////////////
	// Create an OpenCL context
	/////////////////////////////////////////////////////////////////
    //100519 context = clCreateContextFromType(cps, CL_DEVICE_TYPE_CPU, NULL, NULL, &status);
    context = clCreateContextFromType(cps, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
    if(status != CL_SUCCESS) {  
		switch (status) {
			case CL_INVALID_PLATFORM: {AfxMessageBox("CL_INVALID_PLATFORM"); break;}
			case CL_INVALID_VALUE: {AfxMessageBox("CL_INVALID_VALUE"); break;}
			case CL_INVALID_DEVICE: {AfxMessageBox("CL_INVALID_DEVICE"); break;}
			case CL_DEVICE_NOT_AVAILABLE: {AfxMessageBox("CL_DEVICE_NOT_AVAILABLE"); break;}
			case CL_OUT_OF_HOST_MEMORY: {AfxMessageBox("CL_OUT_OF_HOST_MEMORY"); break;}
			case CL_DEVICE_NOT_FOUND: {AfxMessageBox("CL_DEVICE_NOT_FOUND"); break;}
			default: {AfxMessageBox("Error: clCreateContextFromType"); break;}
		}
		return 1; 
	}

    /* First, get the size of device list data */
    status = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize);
    if(status != CL_SUCCESS) {  
		AfxMessageBox("Error: Getting Context Info (device list size, clGetContextInfo)");
		return 1;
	}

	/////////////////////////////////////////////////////////////////
	// Detect OpenCL devices
	/////////////////////////////////////////////////////////////////
    devices = (cl_device_id *)malloc(deviceListSize);
	int iDevices = deviceListSize / sizeof(cl_device_id);
	if (iDevices > ATISTREAM_MAXDEVICES) iDevices = ATISTREAM_MAXDEVICES;
	//CString line; line.Format("%d", iDevices); AfxMessageBox(line);
	if(devices == 0) {
		AfxMessageBox("Error: No devices found.");
		return 1;
	}

    /* Now, get the device list data */
    status = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceListSize, devices, NULL);
    if(status != CL_SUCCESS) { 
		AfxMessageBox("Error: Getting Context Info (device list, clGetContextInfo)");
		return 1;
	}

	/////////////////////////////////////////////////////////////////
	// Create an OpenCL command queue
	/////////////////////////////////////////////////////////////////
	for (int i=0; i<iDevices; i++) {
	    commandQueue[i] = clCreateCommandQueue(context, devices[i], 0, &status);
		if(status != CL_SUCCESS) { 
			AfxMessageBox("Creating Command Queue. (clCreateCommandQueue)");
			return 1;
		}
	}

	//previously memalloc here

	/////////////////////////////////////////////////////////////////
	// Load CL file, build CL program object, create CL kernel object
	/////////////////////////////////////////////////////////////////

	std::string  sourceStr = "\
\
__kernel void reconstKernel(__constant int* igpm, __global int* ifp,\
                             const uint ixdimp, float fcos, float fsin,float foffset)\
{\
	int iy = get_global_id(1) + get_global_id(2) * get_global_size(1);\
	if (iy >= ixdimp) return;\
	const uint iyixdimp = iy * ixdimp;\
	const float fyisin = iy * fsin + foffset;\
	const int ixdimp2 = ixdimp << 4;\
	for (int ix=get_global_id(0); ix<ixdimp; ix+=get_global_size(0)) {\
		int ix0 = ( (int)(ix * fcos + fyisin) );\
		if ((ix0 >= 0)&&(ix0 < ixdimp2)) ifp[ix + iyixdimp] += igpm[ix0];\
	}\
}";

	//"ixdimp << 4" relates "#define DBPT_LOG2GINTP 4".

/*
	int ix = blockDim.x * blockIdx.x + threadIdx.x;
	if (ix >= ixdimp) return;
	int iy = blockIdx.y << 1;
	float fx1 = ix * fcos + iy * fsin + foffset; 
	int ixy = ix + ixdimp * iy;
	d_ifp[ixy] += tex1Dfetch(tex_igp, (int)(fx1));
	if (iy >= ixdimp - 1) return;
	d_ifp[ixy + ixdimp] += tex1Dfetch(tex_igp, (int)(fx1 + fsin));
*/

    const char * source    = sourceStr.c_str();
    size_t sourceSize[]    = { strlen(source) };
    program = clCreateProgramWithSource(context, 1, &source, sourceSize, &status);
	if(status != CL_SUCCESS) {
	  AfxMessageBox("Error: Loading Binary into cl_program (clCreateProgramWithBinary)");
	  return 1;
	}

    /* create a cl program executable for all the devices specified */
    //status = clBuildProgram(program, 1, devices, "-w", NULL, NULL);
    status = clBuildProgram(program, iDevices, devices, "-w", NULL, NULL);
	//Known bugs: this function gives several blinking windows.
	//
    if(status != CL_SUCCESS) { 
        if(status == CL_BUILD_PROGRAM_FAILURE) {
			AfxMessageBox("CL_BUILD_PROGRAM_FAILURE");
            cl_int logStatus;
            char * buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo (program, 
                devices[0], 
                CL_PROGRAM_BUILD_LOG, 
                buildLogSize, 
                buildLog, 
                &buildLogSize);
			if(logStatus != CL_SUCCESS) {
                AfxMessageBox("clGetProgramBuildInfo failed.");
				return 1;
			}
            buildLog = (char*)malloc(buildLogSize);
            if(buildLog == NULL) {
                AfxMessageBox("Failed to allocate host memory. (buildLog)");
				return 1;
            }
            memset(buildLog, 0, buildLogSize);

            logStatus = clGetProgramBuildInfo (program, 
                devices[0], 
                CL_PROGRAM_BUILD_LOG, 
                buildLogSize, 
                buildLog, 
                NULL);
			if(logStatus != CL_SUCCESS) {
				AfxMessageBox("clGetProgramBuildInfo failed.");
				return 1;
			}

            CString msg = " \n\t\t\tBUILD LOG";
            msg += " ************************************************\r\n";
            msg += buildLog;
            msg += "\r\n";
			msg += " ************************************************";
            free(buildLog);
			AfxMessageBox(msg);
		}
		AfxMessageBox("Error: Building Program (clBuildProgram)");
		return 1; 
	}

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "reconstKernel", &status);
    if(status != CL_SUCCESS) {  
		AfxMessageBox("Error: Creating Kernel from program. (clCreateKernel)");
		return 1;
	}

	nDevices = iDevices;
	if (pDevices) *pDevices = iDevices;
	return 0;
}

void CLReconstMemFree(RECONST_INFO* ri) {
	if (ri->d_ifp != NULL) {
		if (clReleaseMemObject(ri->d_ifp) == CL_SUCCESS) {
			ri->d_ifp = NULL;
			ri->max_d_ifp = 0;
		}
	}
	if (ri->d_igp != NULL) {
		if (clReleaseMemObject(ri->d_igp) == CL_SUCCESS) {
			ri->d_igp = NULL;
			ri->max_d_igp = 0;
		}
	}
}

void CLInitATIstreamDeviceInfo(int* iATIcount, int* iATImaxwork, int* iATIunitwork) {
	*iATIcount = 0; *iATImaxwork = ATISTREAM_MAXWORK; *iATIunitwork = ATISTREAM_UNITWORK;
	//return;//////
	if (initReconstCL(iATIcount)) {
		CLCleanup();
		return;
	}
	if (*iATIcount == 0) return;
	//
	size_t maxWorkGroupSize = 16777216;
	size_t iWorkSize;
	for (int i=0; i<*iATIcount; i++) {
		if (clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE,  
			sizeof(size_t), (void*)&iWorkSize, NULL) == CL_SUCCESS) {
				if (maxWorkGroupSize > iWorkSize) maxWorkGroupSize = iWorkSize;
		}
	}
	//
	//*iATIcount = 1;/////////////////
	//kernelごとにargumentsは異なるが、複数のdeviceが存在したときに
	//　同一のkernelオブジェクトに対して何度もclSetKernelArgをすることになる。
	//　同一のcontextに何度もメモリを確保することになる。
	//これで良いのか？
	return;
}

void CLReconstHost(RECONST_INFO* ri, int idev, bool bReport) {
	//if (initReconstCL(NULL) != 0) return;
	//-----OpenCL body-----//
	const int ixdim = ri->ixdim;
	const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
	const int iIntpDim = (int) pow((double)2, iZooming);
	const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
	const int ndim = (int) pow((double)2, ndimp);
	const float fCenter = (float)(ri->center);
	const int ixdimp = ixdim * iIntpDim;
	//
	if (CLReconstMemAlloc(ri, idev)) {
		AfxMessageBox("Out of OpenCL device memory.\r\n Close other dataset,\r\n or select on-board CPU from Tomography->Property menu.");
		ri->iStatus = RECONST_INFO_ERROR;
		return;
	}
	//prepare kernel
    cl_int   status;
	cl_uint maxDims;
    cl_event events[2];
    size_t globalThreads[3];
    size_t localThreads[3];
	size_t maxWorkGroupSize;
	size_t maxWorkItemSizes[3];
	//Query device capabilities. Maximum work item dimensions and the maximmum work item sizes
	status = clGetDeviceInfo(devices[idev], CL_DEVICE_MAX_WORK_GROUP_SIZE,  
								sizeof(size_t), (void*)&maxWorkGroupSize, NULL);
    if(status != CL_SUCCESS) {  
		AfxMessageBox("Error: Getting Device Info. (clGetDeviceInfo)");
		return;
	}
	
	status = clGetDeviceInfo(devices[idev],CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 
								sizeof(cl_uint), (void*)&maxDims, NULL);
    if(status != CL_SUCCESS) {  
		AfxMessageBox("Error: Getting Device Info. (clGetDeviceInfo)");
		return;
	}
	status = clGetDeviceInfo(devices[idev], CL_DEVICE_MAX_WORK_ITEM_SIZES, 
								sizeof(size_t)*maxDims, (void*)maxWorkItemSizes, NULL);
    if(status != CL_SUCCESS) {  
		AfxMessageBox("Error: Getting Device Info. (clGetDeviceInfo)");
		return;
	}
    globalThreads[0] = maxWorkItemSizes[0];
    localThreads[0]  = 256;
    globalThreads[1] = maxWorkItemSizes[1];
    localThreads[1]  = 1;
    globalThreads[2] = ixdimp / globalThreads[1] + 1;
    localThreads[2]  = 1;
	if((globalThreads[2] > maxWorkItemSizes[2]) || (localThreads[0] > maxWorkGroupSize)) {
		CString line;
		line.Format("%d %d\r\n", maxWorkItemSizes[0], maxWorkGroupSize);
		AfxMessageBox(line + "Unsupported: Device does not support requested number of work items.");
		return;
	}
    //Set appropriate arguments to the kernel
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&(ri->d_igp));
    if (status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (igp)"); return;}
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&(ri->d_ifp));
    if (status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (ifp)"); return; }
    //parameters
    status = clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *)&ixdimp);
    if(status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (ixdimp)"); return;}
	//
	CCmplx* p = NULL;
	int* igp = NULL;
	int* h_ifp = NULL;
	const int imargin = ixdimp;
	const int igpdim = (ixdimp + imargin * 2) * DBPT_GINTP;
	const int idim_ifp = ixdimp * ixdimp;
	try{
		p = new CCmplx[ndim];
		igp = new int[igpdim];
		h_ifp = new int[idim_ifp];
	}
	catch(CException* e) {
		e->Delete();
		if (igp) delete [] igp;
		if (p) delete [] p;
		if (h_ifp) delete [] h_ifp;
		ri->iStatus = RECONST_INFO_ERROR;
		return;
	}
	CFft fft;
	fft.Init1(ndimp, -1);
	memset(igp, 0, sizeof(int) * igpdim);
	int* igpm = (int*)( ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP );
	//
	//if (ixdimp & 0x01) idim_ifp += ixdimp;//making sure that iy is even. OK?
	const unsigned int mem_size_ifp = sizeof(int) * idim_ifp;
	const unsigned int mem_size_igp = sizeof(int) * ixdimp * DBPT_GINTP;
	//reset tomograph
	memset(h_ifp, 0, mem_size_ifp);
	if(clEnqueueWriteBuffer(commandQueue[idev], ri->d_ifp, CL_TRUE, 0,
							mem_size_ifp, h_ifp, 0, NULL, &events[0]) == CL_SUCCESS) { 
		if(clWaitForEvents(1, &events[0]) == CL_SUCCESS){
			if(clReleaseEvent(events[0]) != CL_SUCCESS) {AfxMessageBox("Error: Reset ifp"); return;}
		} else {AfxMessageBox("Error: Reset ifp"); return;}
	} else {AfxMessageBox("Error: Reset ifp"); return;}
	//
	const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
	int iCurrStep = 0;
	const int intcenter = (int)(ri->center);
	const int ixdimh = ixdimp / 2;
	const int ihoffset = ndim / 2 - 1 - ixdimh;
	int* ifp = ri->iReconst;
	const double center = ri->center;
	//const int icenter = (int)((ixdimh + center - (int)(center)) * DBPT_PNT);
	for (int i=(ri->iStartSino); i<(ri->iLenSinogr-1); i+=(ri->iStepSino)) {
		if (bReport) {
			if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
		}
		if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
		const int sidx = i * ri->iMultiplex + ri->iOffset;
		if (sidx >= ri->maxSinogrLen) break;
		(*(ri->nSinogr))++;
		//140611
		if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
			if (i & 1) {
				if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue;
			} else {
				if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue;
			}
		}

		short* iStrip = ri->iSinogr[sidx];
		//Deconvolution
		memset(p, 0, sizeof(CCmplx) * ndim);
		//120501==>
		//111206 codes from StdAfx.cpp
		const int idx0 = (0 - intcenter) * iIntpDim + (ndim / 2 - 1);
		const int idx1 = (ixdim-1 - intcenter) * iIntpDim + (ndim / 2 - 1) + 1;
		for (int m=0; m<idx0; m++) {p[m].re = iStrip[0];}
		for (int m=idx1; m<ndim; m++) {p[m].re = iStrip[ixdim-1];}
		//111206
		//==>120501
		//for (int k=0; k<ndim; k++) {p[k].Reset();}
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
		if (clEnqueueWriteBuffer(commandQueue[idev], ri->d_igp, CL_TRUE, 0,
								mem_size_igp, igpm, 0, NULL, &events[0]) == CL_SUCCESS){ 
			if (clWaitForEvents(1, &events[0]) == CL_SUCCESS){
				if (clReleaseEvent(events[0]) != CL_SUCCESS) continue;
			} else continue;
		} else continue;
		//const size_t image_width = (ixdimp * DBPT_GINTP) / 4;
		//size_t origin[] = {0, 0, 0};
		//size_t region[] = {image_width, 1, 1};
		//size_t row_pitch = image_width * 4 * 4;//RGBA (4 members) * int32 (4 bytes)
		//status = clEnqueueWriteImage(commandQueue, ri->d_igp, CL_TRUE, origin, region, row_pitch, 0,
		//						igpm, 0, NULL, &events[0]);
		//if (status != CL_SUCCESS) {
		//	switch (status) {
		//		case CL_INVALID_COMMAND_QUEUE: {AfxMessageBox("CL_INVALID_COMMAND_QUEUE"); break;}
		//		default: AfxMessageBox("Error: Enqueueing write image. (clEnqueueWriteImage)");
		//	}
		//	break;
		//}

		double theta = (ri->fdeg[i] + ri->fTiltAngle) * DEG_TO_RAD;
		const float fcos = (float)(cos(theta) * DBPT_GINTP);
		const float fsin = (float)(-sin(theta) * DBPT_GINTP);
		const float fcenter = (float)((ixdimh + center - (int)(center)) * DBPT_GINTP);
		const float foffset = fcenter - ixdimh * (fcos + fsin);
		//parameters
		status = clSetKernelArg(kernel, 3, sizeof(cl_float), (void *)&fcos);
		if(status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (cos)"); return;}
		status = clSetKernelArg(kernel, 4, sizeof(cl_float), (void *)&fsin);
		if(status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (sin)"); return;}
		status = clSetKernelArg(kernel, 5, sizeof(cl_float), (void *)&foffset);
		if(status != CL_SUCCESS) {AfxMessageBox("Error: Setting kernel argument. (offset)"); return;}
		/*/==> testcode
		for (int iy=0; iy<ixdimp; iy++) {
			for (int ix=0; ix<ixdimp; ix++) {
				int ix0 = ((int)(ix * fcos + iy * fsin + foffset));
				if (ix0 < 0) continue;
				if (ix0 >= ixdimp << DBPT_LOG2GINTP) continue;
				ifp[ix + iy * ixdimp] += igpm[ix0];
			}
		}
		continue;
		//==>end of testcode///*///
		//Enqueue a kernel run call.
		status = clEnqueueNDRangeKernel(commandQueue[idev], kernel, 3, NULL, 
										globalThreads, localThreads, 0, NULL, &events[0]);
		if (status != CL_SUCCESS) {
			switch (status) {
				case CL_INVALID_PROGRAM_EXECUTABLE: {AfxMessageBox("CL_INVALID_PROGRAM_EXECUTABLE"); break;}
				case CL_INVALID_COMMAND_QUEUE: {AfxMessageBox("CL_INVALID_COMMAND_QUEUE"); break;}
				case CL_INVALID_KERNEL: {AfxMessageBox("CL_INVALID_KERNEL"); break;}
				case CL_INVALID_CONTEXT: {AfxMessageBox("CL_INVALID_CONTEXT"); break;}
				case CL_INVALID_KERNEL_ARGS: {AfxMessageBox("CL_INVALID_KERNEL_ARGS"); break;}
				case CL_INVALID_WORK_DIMENSION: {AfxMessageBox("CL_INVALID_WORK_DIMENSION"); break;}
				case CL_INVALID_GLOBAL_WORK_SIZE: {AfxMessageBox("CL_INVALID_GLOBAL_WORK_SIZE"); break;}
				case CL_INVALID_WORK_GROUP_SIZE: {AfxMessageBox("CL_INVALID_WORK_GROUP_SIZE"); break;}
				case CL_INVALID_WORK_ITEM_SIZE: {AfxMessageBox("CL_INVALID_WORK_ITEM_SIZE"); break;}
				case CL_INVALID_GLOBAL_OFFSET: {AfxMessageBox("CL_INVALID_GLOBAL_OFFSET"); break;}
				case CL_OUT_OF_RESOURCES: {AfxMessageBox("CL_OUT_OF_RESOURCES"); break;}
				case CL_MEM_OBJECT_ALLOCATION_FAILURE: {AfxMessageBox("CL_MEM_OBJECT_ALLOCATION_FAILURE"); break;}
				case CL_INVALID_EVENT_WAIT_LIST: {AfxMessageBox("CL_INVALID_EVENT_WAIT_LIST"); break;}
				case CL_OUT_OF_HOST_MEMORY: {AfxMessageBox("CL_OUT_OF_HOST_MEMORY"); break;}
				default: AfxMessageBox("Error: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)");
			}
			break;
		}
		// wait for the kernel call to finish execution
		if (clWaitForEvents(1, &events[0]) != CL_SUCCESS) { 
			AfxMessageBox("Error: Waiting for kernel run to finish. (clWaitForEvents)");
			break;
		}
		if (clReleaseEvent(events[0]) != CL_SUCCESS) { 
			AfxMessageBox("Error: Release event object. (clReleaseEvent)");
			break;
		}
	}
    // copy results from device to host
	//Enqueue readBuffer
	if(clEnqueueReadBuffer(commandQueue[idev], ri->d_ifp, CL_TRUE, 0,
							mem_size_ifp, h_ifp, 0, NULL, &events[0]) != CL_SUCCESS){ 
		AfxMessageBox("Error: clEnqueueReadBuffer failed. (clEnqueueReadBuffer)");
	}
	if(clWaitForEvents(1, &events[0]) != CL_SUCCESS){ 
		AfxMessageBox("Error: Waiting for read buffer call to finish. (clWaitForEvents)");
	}
	if(clReleaseEvent(events[0]) != CL_SUCCESS){ 
		AfxMessageBox("Error: Release event object. (clReleaseEvent)");
	}
	for (int i=ixdimp * ixdimp - 1; i>=0; i--) {ifp[i] += h_ifp[i];}
	if (h_ifp) delete [] h_ifp;
	if (igp) delete [] igp;
	if (p) delete [] p;
	//-----end of OpenCL body-----//
	//CLReconstMemFree(ri);///////////////////
	//CLCleanup();
}

