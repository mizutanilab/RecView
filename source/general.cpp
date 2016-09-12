#include "stdafx.h"



#if !defined( _GENERAL_CPP_ )

#define _GENERAL_CPP_
#include <math.h>
#include <time.h> // clock_t
#include <process.h> //_beginthread
#include <afx.h> // CString, LPCTSTR
#include "ccmplx.h"
#include "cfft.h"
#include "MainFrm.h"
#include "gazoDoc.h"
#include "DlgReconst.h"
#include "gazo.h"
#include "cudaReconst.h"
#include "DlgQueue.h"
#include "clReconst.h"
#include "chdr5.h" //160521 HDR5

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD); //GetProcAddress

#define ANSI
#ifdef ANSI
#endif


void ProcessMessage() {
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
	}
	//130203 update toolbar status
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	pf->m_wndToolBar.OnUpdateCmdUI(pf, TRUE);
}

void SleepSecond(TReal sec) {
	clock_t start = clock();
	while ((float)(clock() - start)/CLOCKS_PER_SEC < sec) {continue;}
}

DWORD GetProcessorCoreCount() {
    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD dwProcessorCoreCount = 0;
    DWORD byteOffset = 0;

    glpi = (LPFN_GLPI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32")),
                            "GetLogicalProcessorInformation");
    if (NULL == glpi) return 0;//GetLogicalProcessorInformation is not supported.

    while (!done) {
        DWORD rc = glpi(buffer, &returnLength);
        if (FALSE == rc) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (buffer) free(buffer);
                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
                if (NULL == buffer) return 0;//Error: Allocation failure
            } else {//TEXT("\nError %d\n"), GetLastError();
                return 0;
            }
        } else {
            done = TRUE;
        }
    }

    ptr = buffer;
    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
        if (ptr->Relationship == RelationProcessorCore) dwProcessorCoreCount++;
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }
    free(buffer);
	return dwProcessorCoreCount;
}

COLORREF HSBtoRGB(double H, double S, double B) {
	double r, g, b;
	H = (int)H % 360;//Hue 0-359
	S *= 255; B *= 255;
	S = (S < 0)? 0 : (S > 255)? 255 : S;//Saturation 0-255
	B = (B < 0)? 0 : (B > 255)? 255 : B;//Brightness 0-255
	double max = B;
	double min = max - S * max / 255;
	if (H < 60) {
		r = max;
		g = min + H * (max - min) / 60;
		b = min;
	} else if (H < 120) {
		r = max - (H - 60) * (max - min) / 60;
		g = max;
		b = min;
	} else if (H < 180){
		r = min;
		g = max;
		b = min + (H - 120) * (max - min) / 60;
	} else if (H < 240){
		r = min;
		g = max - (H - 180) * (max - min) / 60;
		b = max;
	} else if (H < 300){
		r = min + (H - 240) * (max - min) / 60;
		g = min;
		b = max;
	} else {//Hue < 360
		r = max;
		g = min;
		b = max - (H - 300) * (max - min) / 60;
	}
	return RGB((int)r, (int)g, (int)b);
}

TErr ReadBmp(CString filePath, int** buffer, int* pMaxBuffer, int* prevH, int* prevW,
											 char* paletteBlue, char* paletteGreen, char* paletteRed, int maxPalette) {
		//filePath = fileDlg.GetNextPathName(pos);
	TErr err = 0;
	FILE* file = NULL;
	errno_t errn = fopen_s(&file, filePath, "rb" );
	if (errn) return 18101;
	//080105 CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	//080105 pf->m_wndStatusBar.SetPaneText(0,"Reading " + filePath);
	//header identifier
	char carg[81];
	if (fread( carg, sizeof( char ), 2, file ) != 2) {err = 18101; fclose(file); return err;}
	if (strncmp( carg, "BM" , 2 ) != 0) {err = 18102; fclose(file); return err;}
	//file size
	int fileSize;
	if (fread( &fileSize, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//reserved x 2
	short iReserved;
	if (fread( &iReserved, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iReserved, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//offset
	int offset;
	if (fread( &offset, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//size of infoheader
	int headerSize;
	if (fread( &headerSize, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (headerSize != 40) {err = 18103; fclose(file); return err;}
	//pixel width and height
	int width, height;
	if (fread( &width, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &height, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if ((width == 0)||(height == 0)) {err = 18106; fclose(file); return err;}
	if (*prevH) {
		if ((*prevH != height)||(*prevW != width)) {err = 18106; fclose(file); return err;}
	} else {
		*prevH = height; *prevW = width;
	}
	//iPlanes, iBitCount, iCompression
	short iPlanes, iBitCount; int iCompression;
	if (fread( &iPlanes, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iBitCount, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iCompression, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if ((iBitCount != 8)||(iCompression != 0)) {err = 18103; fclose(file); return err;}
	//iSizeImage, iXPixPerMeter, iYPixPerMeter, iClrUsed, iClrImportant
	int iSizeImage, iXPixPerMeter, iYPixPerMeter, iClrUsed, iClrImportant;
	if (fread( &iSizeImage, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iXPixPerMeter, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iYPixPerMeter, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iClrUsed, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fread( &iClrImportant, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//if size inquiry only
	if (!buffer) {
		*pMaxBuffer = width * height * 8 / iBitCount;
		fclose(file);
		return err;
	}
	//palette
	if (iClrUsed > maxPalette) {err = 18105; fclose(file); return err;}
	for (int i=0; i<iClrUsed; i++) {
		//B G R
		if (fread( &(paletteBlue[i]), sizeof( char ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
		if (fread( &(paletteGreen[i]), sizeof( char ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
		if (fread( &(paletteRed[i]), sizeof( char ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
		//reserved
		if (fread( carg, sizeof( char ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	}
	if (err) {fclose(file); return err;}
	// read data
	if (*pMaxBuffer < width * height) {
		if (*buffer) delete [] (*buffer);
		*buffer = NULL;
	}
	if (!(*buffer)) {
		if ((*buffer = new int[width * height]) == NULL) {err = 18105; fclose(file); return err;}
		*pMaxBuffer = width * height;
		for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	}
	char cReserved[4];
	int iline = (width * iBitCount) / 8;
	int iskip = iline % 4;
	if (iskip) iskip = 4 - iskip;
	//for (i=0; i<height; i++) {//061215
	for (int i=height-1; i>=0; i--) {
		for (int j=0; j<width; j++) {
			char dens;
			if (fread(&dens, sizeof(char), 1, file) != 1) {err = 18107; fclose(file); return err;}
			(*buffer)[width * i + j] += (unsigned char)dens;
			//buffer[width * i + j] = (unsigned char)dens;
		}
		if (err) return err;
		if (iskip) {
			if (fread(cReserved, sizeof(char), iskip, file) != (unsigned)iskip) {err = 18108; fclose(file); return err;}
		}
		if (err) return err;
	}
	fclose(file);
	return err;
	//if (err) break;
	/*CString line = "";
	line.Format("fileSize %d offset %d\r\nheaderSize %d width %d height %d\r\niPlanes %d iBitCount %d iCompression %d\r\niSizeImage %d iXPixPerMeter %d iYPixPerMeter %d iClrUsed %d iClrImportant %d\r\n",
		fileSize, offset, headerSize, width, height, iPlanes, iBitCount, iCompression, iSizeImage, iXPixPerMeter, iYPixPerMeter, iClrUsed, iClrImportant);
	CString scr;
	for (i=0; i<iClrUsed; i+=16) {
		scr.Format("%d %d %d\r\n", paletteBlue[i], paletteGreen[i], paletteRed[i]);
		line += scr;
	}
	unsigned int bmax = 0;
	for (i=0; i<height; i+=30) {
		for (int j=0; j<width; j+=10) {
			unsigned int ichar = buffer[width * i + j];
			if (ichar > bmax) bmax = ichar;
			if (ichar < 10) line += "-";
			else if (ichar < 20) line += "1"; else if (ichar < 30) line += "2";
			else if (ichar < 40) line += "3";	else if (ichar < 50) line += "4";
			else if (ichar < 60) line += "5";	else if (ichar < 70) line += "6";
			else if (ichar < 80) line += "7";	else if (ichar < 90) line += "8";
			else if (ichar < 100) line += "9"; else if (ichar < 110) line += "A";
			else if (ichar < 120) line += "B"; else if (ichar < 130) line += "C";
			else if (ichar < 140) line += "D"; else if (ichar < 150) line += "E";
			else if (ichar < 160) line += "F"; else if (ichar < 170) line += "G";
			else if (ichar < 180) line += "H"; else if (ichar < 190) line += "I";
			else if (ichar < 200) line += "J"; else if (ichar < 210) line += "K";
			else if (ichar < 220) line += "L"; else if (ichar < 230) line += "M";
			else if (ichar < 240) line += "N"; else if (ichar < 250) line += "P";
			else line += "Q";
		}
		line += "\r\n";
	}
	scr.Format("max %d", bmax); line += scr;
	AfxMessageBox(line);
	return err;
	///*///
}
TErr WriteBmpTrueColor(CString filePath, COLORREF* buffer, int height, int width) {
	TErr err = 0;
	FILE* file = NULL;
	errno_t errn = fopen_s(&file, filePath, "wb" );
	if (errn) return 18100;
	////////
	//sizeof(char) = 1 bytes
	//sizeof(short) = 2 bytes
	//sizeof(int) = 4 bytes
	////////
	//header identifier
	char carg[81];
	strcpy_s(carg, "BM" );
	if (fwrite( carg, sizeof( char ), 2, file ) != 2) {err = 18101; fclose(file); return err;}
	//file size
	int fileSize = 0;/////
	if (fwrite( &fileSize, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//reserved x 2
	short iReserved = 0;
	if (fwrite( &iReserved, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iReserved, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//offset
	int offset = 54;
	if (fwrite( &offset, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//size of infoheader
	int headerSize = 40;
	if (fwrite( &headerSize, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//pixel width and height
	if (fwrite( &width, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &height, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//iPlanes, iBitCount, iCompression
	short iPlanes = 1;
	short iBitCount = 24; //24: true color
	int iCompression = 0; //0: no compression
	if (fwrite( &iPlanes, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iBitCount, sizeof( short ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iCompression, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//iSizeImage, iXPixPerMeter, iYPixPerMeter, iClrUsed, iClrImportant
	int iSizeImage = 0, iXPixPerMeter = 0, iYPixPerMeter = 0, iClrUsed = 0, iClrImportant = 0;
	if (fwrite( &iSizeImage, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iXPixPerMeter, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iYPixPerMeter, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iClrUsed, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	if (fwrite( &iClrImportant, sizeof( int ), 1, file ) != 1) {err = 18101; fclose(file); return err;}
	//write data
	char cReserved[4], rgb[3];
	int iline = (width * iBitCount) / 8;
	int iskip = iline % 4;
	if (iskip) iskip = 4 - iskip;
	//for (int i=0; i<height; i++) {
	for (int i=height-1; i>=0; i--) {
		for (int j=0; j<width; j++) {
			int r1 = (int)(buffer[width * i + j]);
			rgb[0] = (char)((r1 >> 16) & 255);//blue
			rgb[1] = (char)((r1 >> 8) & 255);//green
			rgb[2] = (char)(r1 & 255);//red
			//rgb[0] = 127; rgb[1] = 0; rgb[2] = 0;
			if (fwrite(rgb, sizeof(char), 3, file) != 3) {err = 18107; fclose(file); return err;}
		}
		if (err) return err;
		if (iskip) {
			if (fwrite(cReserved, sizeof(char), iskip, file) != (unsigned)iskip) {err = 18108; fclose(file); return err;}
		}
		if (err) return err;
	}
	fclose(file);
	return err;
}

void ConvertToLittleEndian(void* param, rsize_t bytes, char* carg) {
	if (bytes > READTIF_CARG_SIZE) return;
	memcpy_s(carg, bytes, param, bytes);
	for (unsigned int i=0; i<bytes/2; i++) {
		char ctmp = carg[i];
		carg[i] = carg[bytes-1-i];
		carg[bytes-1-i] = ctmp;
	}
	memcpy_s(param, bytes, carg, bytes);
}

TErr ReadTif(CFile* fp, int** buffer, int* pMaxBuffer, int* prevH, int* prevW,
						 float* pixDiv, float* pixBase, float* fCenter, int* iFilter, float* fPixelWidth,
						 int* nSino) {
	TErr err = 0;
	//byte order
	char carg[READTIF_CARG_SIZE];
	unsigned char uchar[4];
	bool bLittleEndian = true;
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18121; return err;}
	if (strncmp( carg, "II" , 2 ) == 0) bLittleEndian = true;
	else if (strncmp( carg, "MM" , 2 ) == 0) bLittleEndian = false;
	else {err = 18122; return err;}
	//BigEdian is now supported
	//if (!bLittleEndian) {err = 18123; return err;}
	//confirm identifier
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18124; return err;}
	if (bLittleEndian) {
		if ((carg[0] != 42)||(carg[1] != 0)) {err = 18125; return err;}
	} else {
		if ((carg[1] != 42)||(carg[0] != 0)) {err = 18125; return err;}
	}
	//offset
	unsigned long offset = 0;
	if (fp->Read(&offset, sizeof(long)) != sizeof(long)) {err = 18126; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&offset, sizeof(int), carg);
	if ((unsigned long)(fp->Seek(offset, CFile::begin)) != offset) {err = 18127; return err;}
	short iDirEnt = 0, iIFD, itype; unsigned long inum;
	if (fp->Read(&iDirEnt, sizeof(short)) != sizeof(short)) {err = 18128; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&iDirEnt, sizeof(short), carg);
	int iImageWidth = -1, iImageLength = -1, iBitsPerSample = 8, iCompression = 1, 
		iPhotometric = 1, iRowsPerStrip = -1, iResolutionUnit = 1, iSamplesPerPixel = 1,
		iPlanarConfiguration = 1;
	int oStripOffsets = -1, oStripByteCounts = -1, oXResolution = -1, oYResolution = -1;
	int nStripOffsets = 0, nStripByteCounts = 0;
	int oArtist = -1, oImageDescription = -1;
	int nArtist = 0, nImageDescription = 0;
	short sizeofStripOffsets = sizeof(long);
	short sizeofStripByteCounts = sizeof(long);
	for (int i=0; i<iDirEnt; i++) {
		if (fp->Read(&iIFD, sizeof(short)) != sizeof(short)) {err = 18129; return err;}
		if (fp->Read(&itype, sizeof(short)) != sizeof(short)) {err = 18130; return err;}
		if (fp->Read(&inum, sizeof(unsigned long)) != sizeof(unsigned long)) {err = 18131; return err;}
		if (!bLittleEndian) {
			ConvertToLittleEndian(&iIFD, sizeof(short), carg);
			ConvertToLittleEndian(&itype, sizeof(short), carg);
			ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		for (int j=0; j<4; j++) {
			if (fp->Read(&(uchar[j]), sizeof(char)) != sizeof(char)) {err = 18132; return err;}
		}
		int iarg0 = 0, iarg1 = 0; bool bIndirect = false;
		switch (itype) {
			case TIF_BYTE: {}
			case TIF_ASCII: {
				if (inum > 4) bIndirect = true;
				break;}
			case TIF_SHORT: {
				if (inum > 2) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256; iarg1 = uchar[2] + uchar[3] * 256;}
					else {iarg0 = uchar[0] * 256 + uchar[1]; iarg1 = uchar[2] * 256 + uchar[3];}
				}
				break;}
			case TIF_LONG: {
				if (inum > 1) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;}
					else {iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;}
				}
				break;}
			case TIF_RATIONAL: {
				bIndirect = true;
				break;}
			default: {
				err = 18133; return err;
				break;}
		}
		if (bIndirect) {
			if (bLittleEndian) iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;
			else iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;
		}
		switch (iIFD) {
			case TIF_IMAGEWIDTH: {iImageWidth = iarg0; break;}
			case TIF_IMAGELENGTH: {iImageLength = iarg0; break;}
			case TIF_BITSPERSAMPLE: {iBitsPerSample = iarg0; break;}
			case TIF_COMPRESSION: {iCompression = iarg0; break;}
			case TIF_PHOTOMETRIC: {iPhotometric = iarg0; break;}
			case TIF_ROWSPERSTRIP: {iRowsPerStrip = iarg0; break;}
			case TIF_RESOLUTIONUNIT: {iResolutionUnit = iarg0; break;}
			case TIF_STRIPOFFSETS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripOffsets = sizeof(short);}
				else {err = 18134; return err;}
				oStripOffsets = iarg0; 
				nStripOffsets = inum; 
				break;}
			case TIF_STRIPBYTECOUNTS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripByteCounts = sizeof(short);}
				else {err = 18135; return err;}
				oStripByteCounts = iarg0; 
				nStripByteCounts = inum; 
				break;}
			case TIF_XRESOLUTION: {oXResolution = iarg0; break;}
			case TIF_YRESOLUTION: {oYResolution = iarg0; break;}
			case TIF_SAMPLESPERPIXEL: {iSamplesPerPixel = iarg0; break;}
			case TIF_PLANARCONFIGURATION: {iPlanarConfiguration = iarg0; break;}
			case TIF_ARTIST: {oArtist = iarg0; nArtist = inum; break;}
			case TIF_IMAGEDESCRIPTION: {oImageDescription = iarg0; nImageDescription = inum; break;}
			case TIF_SOFTWARE: {}//NOP
			case TIF_DATETIME: {}//NOP
			default: break;
		}
	}
	//unsupported formats
	if (iCompression != 1) {err = 18111; return err;}
	if (iSamplesPerPixel != 1) {err = 18112; return err;}
	if (iPlanarConfiguration != 1) {err = 18113; return err;}
	if (iPhotometric > 1) {err = 18101; return err;}
	if ((iBitsPerSample != 8)&&(iBitsPerSample != 16)) {err = 18101; return err;}
	//if (iBitsPerSample != 8) {err = 18101; return err;}
	//irregular values
	if (iRowsPerStrip <= 0) {err = 18114; return err;}
	if (oStripOffsets <= 0) {err = 18115; return err;}
	//pixel width and height
	const int width = iImageWidth;
	const int height = iImageLength;
	int nstrip = height / iRowsPerStrip; if (height % iRowsPerStrip) nstrip++;
	if ((width <= 0)||(height <= 0)) {err = 18106; return err;}
	if (*prevH) {
		if ((*prevH != height)||(*prevW != width)) return WARN_READIMAGE_SIZECHANGE;//160803
	} else {
		*prevH = height; *prevW = width;
	}
	if (!buffer) {//if size inquiry only
		//*pMaxBuffer = width * height * 8 / iBitsPerSample;
		*pMaxBuffer = width * height;
		return err;
	}
	//
	if (*pMaxBuffer < width * height) {
		if (*buffer) delete [] (*buffer);
		*buffer = NULL;
	}
	if (!(*buffer)) {
		try {*buffer = new int[width * height];}
		catch (CException* e) {e->Delete(); err = 18105; return err;}
		//if ((*buffer = new int[width * height]) == NULL) {err = 18105; return err;}
		*pMaxBuffer = width * height;
		for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	}
	//strip offsets and strip byte counts
	int iprev = -1; bool bStream = true;
	unsigned int ifirst = 0; unsigned int icount = 0;
	unsigned long stoffset = 0;
	for (int i=0; i<nstrip; i++) {
		if (nStripOffsets == 1) {//121013
			inum = oStripOffsets;
			//CString msg; msg.Format("121013-1 %d %d", nStripOffsets, inum); AfxMessageBox(msg);
		} else {
			stoffset = oStripOffsets + sizeofStripOffsets * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18136; return err;}
			if ((short)fp->Read(&inum, sizeofStripOffsets) != sizeofStripOffsets) {err = 18137; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		if (iprev < 0) {
			ifirst = inum;
		} else {
			if (iBitsPerSample == 8) {
				if ((int)inum - iprev != (int)icount) {bStream = false; break;}
			} else if (iBitsPerSample == 16) {
				if ((int)inum - iprev != (int)icount) {bStream = false; break;}
			}
		}
		iprev = inum;
		if (nStripByteCounts == 1) {//121013
			inum = oStripByteCounts;
			//CString msg; msg.Format("121013-2 %d %d", nStripByteCounts, inum); AfxMessageBox(msg);
		} else {
			stoffset = oStripByteCounts + sizeofStripByteCounts * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18138; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18139; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		icount = inum;
	}
	//Data
	if (bStream) {
		if (nStripByteCounts == 1) {//121013
			inum = oStripByteCounts;
			//CString msg; msg.Format("121013-3 %d %d", nStripByteCounts, inum); AfxMessageBox(msg);
		} else {
			if ((int)(fp->Seek(oStripByteCounts, CFile::begin)) != oStripByteCounts) {err = 18140; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18141; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		icount = inum;
		if ((unsigned long)(fp->Seek(ifirst, CFile::begin)) != ifirst) {err = 18142; return err;}
	}// else {AfxMessageBox("bStream false");}//////
	unsigned char* dens = NULL;
	unsigned short* sdens = NULL;
	if (iBitsPerSample == 8) {
		dens = new unsigned char[width];
		if (!dens) {err = 18143; return err;}
	} else if (iBitsPerSample == 16) {
		sdens = new unsigned short[width];
		if (!sdens) {err = 18144; return err;}
	} else return 18145;
	for (int i=0; i<nstrip; i++) {
		if (!bStream) {//if bStream is false, nstrip is > 1. 
			stoffset = oStripByteCounts + sizeofStripByteCounts * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18146; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18147; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
			icount = inum;
			stoffset = oStripOffsets + sizeofStripOffsets * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18148; return err;}
			if ((int)fp->Read(&inum, sizeofStripOffsets) != sizeofStripOffsets) {err = 18149; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
			if ((unsigned long)fp->Seek(inum, CFile::begin) != inum) {err = 18101; return err;}
		}
		if (icount <= 0) {err = 18150; return err;}
		if (iBitsPerSample == 8) {
			const int nrow = icount / width;//only for 8 bit format
			for (int k=0; k<nrow; k++) {
				if (i * iRowsPerStrip + k >= height) break;
				if (fp->Read(dens, sizeof(char) * width) != sizeof(char) * width) {err = 18151; return err;}
				int idx0 = width * (i * iRowsPerStrip + k);
				for (int j=0; j<width; j++) {(*buffer)[idx0 + j] = dens[j];}
			}
		} else if (iBitsPerSample == 16) {
			const int nrow = icount / width / 2;//for 16 bit format?
			for (int k=0; k<nrow; k++) {
				if (i * iRowsPerStrip + k >= height) break;
				if (fp->Read(sdens, sizeof(short) * width) != sizeof(short) * width) {err = 18152; return err;}
				int idx0 = width * (i * iRowsPerStrip + k);
				if (bLittleEndian) {
					for (int j=0; j<width; j++) {(*buffer)[idx0 + j] = sdens[j];}
				} else {
					for (int j=0; j<width; j++) {
						ConvertToLittleEndian(&(sdens[j]), sizeof(short), carg);
						(*buffer)[idx0 + j] = sdens[j];
					}
				}
			}
		} else {
			//place holder 4 bit format
		}
		//break;////////
	}
	if (dens) delete [] dens;
	if (sdens) delete [] sdens;
	//
	CString scr = "";
	int ncarg = nArtist < READTIF_CARG_SIZE ? nArtist : (READTIF_CARG_SIZE-1);
	if (oArtist >= 0) {
		if (fp->Seek(oArtist, CFile::begin) != oArtist) {err = 18103; return err;}
		if (fp->Read(carg, sizeof(char) * ncarg) != sizeof(char) * ncarg) {err = 18104; return err;}
		scr = carg;
	}
	//CString msg = scr + "\r\n"; CString line; //121024
	ncarg = nImageDescription < READTIF_CARG_SIZE ? nImageDescription : (READTIF_CARG_SIZE-1);
	if (scr.Left(6) == "SP-uCT") {
		if (oImageDescription >= 0) {
			if (fp->Seek(oImageDescription, CFile::begin) != oImageDescription) {err = 18107; return err;}
			if (fp->Read(carg, sizeof(char) * ncarg) != sizeof(char) * ncarg) {err = 18108; return err;}
			scr = carg;
		}
		//msg += scr + "\r\n";
		int iflt;
		if ((pixDiv != NULL) && (pixBase != NULL) && (fCenter != NULL) && (fPixelWidth != NULL) && (iFilter != NULL)) {
			int narg = sscanf_s(scr, "%d %f %f %f %f %d", &inum, fPixelWidth, pixDiv, pixBase, fCenter, &iflt);
			//line.Format("%d %d %d", inum, iflt, narg); 
			*iFilter = iflt - 1;
		}
		if (nSino) *nSino = inum;
	}
	/*121024
	if ((scr.Left(6) == "SP-uCT")&&(oImageDescription >= 0)) {
		if (fp->Seek(oImageDescription, CFile::begin) != oImageDescription) {err = 18107; return err;}
		if (fp->Read(carg, sizeof(char) * ncarg) != sizeof(char) * ncarg) {err = 18108; return err;}
		scr = carg;
	}
	msg += scr + "\r\n"; CString line; 
	int iflt;
	if ((pixDiv != NULL) && (pixBase != NULL) && (fCenter != NULL) && (fPixelWidth != NULL) && (iFilter != NULL)) {
		int narg = sscanf_s(scr, "%d %f %f %f %f %d", &inum, fPixelWidth, pixDiv, pixBase, fCenter, &iflt);
		line.Format("%d %d %d", inum, iflt, narg); 
		*iFilter = iflt - 1;
	}
	if (nSino) *nSino = inum;
	*/
	//AfxMessageBox(msg + line);
	return err;

	//FILE* stream = fopen("D:\\text001.log", "wt");
	//fwrite( carg, sizeof( char ), ncarg, stream );
	//fclose(stream);
	//AfxMessageBox(line);
	/*/test code
	CString line = "", scr;
	unsigned int bmax = 0;
	for (i=0; i<height; i+=30) {
		for (int j=0; j<width; j+=10) {
			unsigned int ichar = (*buffer)[width * i + j];
			if (ichar > bmax) bmax = ichar;
			if (ichar < 10) line += "-";
			else if (ichar < 20) line += "1"; else if (ichar < 30) line += "2";
			else if (ichar < 40) line += "3";	else if (ichar < 50) line += "4";
			else if (ichar < 60) line += "5";	else if (ichar < 70) line += "6";
			else if (ichar < 80) line += "7";	else if (ichar < 90) line += "8";
			else if (ichar < 100) line += "9"; else if (ichar < 110) line += "A";
			else if (ichar < 120) line += "B"; else if (ichar < 130) line += "C";
			else if (ichar < 140) line += "D"; else if (ichar < 150) line += "E";
			else if (ichar < 160) line += "F"; else if (ichar < 170) line += "G";
			else if (ichar < 180) line += "H"; else if (ichar < 190) line += "I";
			else if (ichar < 200) line += "J"; else if (ichar < 210) line += "K";
			else if (ichar < 220) line += "L"; else if (ichar < 230) line += "M";
			else if (ichar < 240) line += "N"; else if (ichar < 250) line += "P";
			else line += "Q";
		}
		line += "\r\n";
	}
	scr.Format("max %d", bmax); line += scr;
	AfxMessageBox(line);
	return err;
	/*
	//scr.Format("\r\n%d", bSameWidth); line += scr;
	CString line = "", scr;
	scr.Format("ImageWidth %d\r\nImageLength %d\r\nBitsPerSample %d\r\nCompression %d\r\n",
		iImageWidth, iImageLength, iBitsPerSample, iCompression);
	line += scr;
	scr.Format("Photometric %d\r\nRowsPerStrip %d\r\nResolutionUnit %d\r\n",
		iPhotometric, iRowsPerStrip, iResolutionUnit);
	line += scr;
	scr.Format("SamplesPerPixel %d\r\nPlanarConfiguration %d\r\n",
		iSamplesPerPixel, iPlanarConfiguration);
	line += scr;
	scr.Format("StripOffsets %d x %d\r\nStripByteCounts %d x %d\r\nXresolution %d\r\nYResolution %d\r\n",
		oStripOffsets, nStripOffsets, oStripByteCounts, nStripByteCounts, 
		oXResolution, oYResolution);
	line += scr;
	///*///
}

TErr ReadTifStrip(CFile* fp, short* sbuffer, int iLine, int iWidth, int iMultiplex, int* piFlag) {
	//
	int iprevH = 0, iprevW = 0;
	int* prevH = &iprevH;
	int* prevW = &iprevW;
	//float* pixDiv = NULL;
	//float* pixBase = NULL;
	//float* fCenter = NULL;
	//int* iFilter = NULL;
	//float* fPixelWidth = NULL;
	//int* nSino = NULL;
	//
	TErr err = 0;
	//byte order
	char carg[READTIF_CARG_SIZE];
	unsigned char uchar[4];
	bool bLittleEndian = true;
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18121; return err;}
	if (strncmp( carg, "II" , 2 ) == 0) bLittleEndian = true;
	else if (strncmp( carg, "MM" , 2 ) == 0) bLittleEndian = false;
	else {err = 18122; return err;}
	//BigEdian is now supported
	//if (!bLittleEndian) {err = 18123; return err;}
	//confirm identifier
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18124; return err;}
	if (bLittleEndian) {
		if ((carg[0] != 42)||(carg[1] != 0)) {err = 18125; return err;}
	} else {
		if ((carg[1] != 42)||(carg[0] != 0)) {err = 18125; return err;}
	}
	//offset
	unsigned long offset = 0;
	if (fp->Read(&offset, sizeof(long)) != sizeof(long)) {err = 18126; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&offset, sizeof(int), carg);
	if ((unsigned long)(fp->Seek(offset, CFile::begin)) != offset) {err = 18127; return err;}
	short iDirEnt = 0, iIFD, itype; unsigned long inum;
	if (fp->Read(&iDirEnt, sizeof(short)) != sizeof(short)) {err = 18128; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&iDirEnt, sizeof(short), carg);
	int iImageWidth = -1, iImageLength = -1, iBitsPerSample = 8, iCompression = 1, 
		iPhotometric = 1, iRowsPerStrip = -1, iResolutionUnit = 1, iSamplesPerPixel = 1,
		iPlanarConfiguration = 1;
	int oStripOffsets = -1, oStripByteCounts = -1, oXResolution = -1, oYResolution = -1;
	int nStripOffsets = 0, nStripByteCounts = 0;
	int oArtist = -1, oImageDescription = -1;
	int nArtist = 0, nImageDescription = 0;
	short sizeofStripOffsets = sizeof(long);
	short sizeofStripByteCounts = sizeof(long);
	for (int i=0; i<iDirEnt; i++) {
		if (fp->Read(&iIFD, sizeof(short)) != sizeof(short)) {err = 18129; return err;}
		if (fp->Read(&itype, sizeof(short)) != sizeof(short)) {err = 18130; return err;}
		if (fp->Read(&inum, sizeof(unsigned long)) != sizeof(unsigned long)) {err = 18131; return err;}
		if (!bLittleEndian) {
			ConvertToLittleEndian(&iIFD, sizeof(short), carg);
			ConvertToLittleEndian(&itype, sizeof(short), carg);
			ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		for (int j=0; j<4; j++) {
			if (fp->Read(&(uchar[j]), sizeof(char)) != sizeof(char)) {err = 18132; return err;}
		}
		int iarg0 = 0, iarg1 = 0; bool bIndirect = false;
		switch (itype) {
			case TIF_BYTE: {}
			case TIF_ASCII: {
				if (inum > 4) bIndirect = true;
				break;}
			case TIF_SHORT: {
				if (inum > 2) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256; iarg1 = uchar[2] + uchar[3] * 256;}
					else {iarg0 = uchar[0] * 256 + uchar[1]; iarg1 = uchar[2] * 256 + uchar[3];}
				}
				break;}
			case TIF_LONG: {
				if (inum > 1) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;}
					else {iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;}
				}
				break;}
			case TIF_RATIONAL: {
				bIndirect = true;
				break;}
			default: {
				err = 18133; return err;
				break;}
		}
		if (bIndirect) {
			if (bLittleEndian) iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;
			else iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;
		}
		switch (iIFD) {
			case TIF_IMAGEWIDTH: {iImageWidth = iarg0; break;}
			case TIF_IMAGELENGTH: {iImageLength = iarg0; break;}
			case TIF_BITSPERSAMPLE: {iBitsPerSample = iarg0; break;}
			case TIF_COMPRESSION: {iCompression = iarg0; break;}
			case TIF_PHOTOMETRIC: {iPhotometric = iarg0; break;}
			case TIF_ROWSPERSTRIP: {iRowsPerStrip = iarg0; break;}
			case TIF_RESOLUTIONUNIT: {iResolutionUnit = iarg0; break;}
			case TIF_STRIPOFFSETS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripOffsets = sizeof(short);}
				else {err = 18134; return err;}
				oStripOffsets = iarg0; 
				nStripOffsets = inum; 
				break;}
			case TIF_STRIPBYTECOUNTS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripByteCounts = sizeof(short);}
				else {err = 18135; return err;}
				oStripByteCounts = iarg0; 
				nStripByteCounts = inum; 
				break;}
			case TIF_XRESOLUTION: {oXResolution = iarg0; break;}
			case TIF_YRESOLUTION: {oYResolution = iarg0; break;}
			case TIF_SAMPLESPERPIXEL: {iSamplesPerPixel = iarg0; break;}
			case TIF_PLANARCONFIGURATION: {iPlanarConfiguration = iarg0; break;}
			case TIF_ARTIST: {oArtist = iarg0; nArtist = inum; break;}
			case TIF_IMAGEDESCRIPTION: {oImageDescription = iarg0; nImageDescription = inum; break;}
			case TIF_SOFTWARE: {}//NOP
			case TIF_DATETIME: {}//NOP
			default: break;
		}
	}
	//unsupported formats
	if (iCompression != 1) {err = 18111; return err;}
	if (iSamplesPerPixel != 1) {err = 18112; return err;}
	if (iPlanarConfiguration != 1) {err = 18113; return err;}
	if (iPhotometric > 1) {err = 18101; return err;}
	if ((iBitsPerSample != 8)&&(iBitsPerSample != 16)) {err = 18101; return err;}
	//if (iBitsPerSample != 8) {err = 18101; return err;}
	//irregular values
	if (iRowsPerStrip <= 0) {err = 18114; return err;}
	if (oStripOffsets <= 0) {err = 18115; return err;}
	//pixel width and height
	const int width = iImageWidth;
	const int height = iImageLength;
	int nstrip = height / iRowsPerStrip; if (height % iRowsPerStrip) nstrip++;
	if ((width <= 0)||(height <= 0)) {err = 18106; return err;}
	if (*prevH) {
		if ((*prevH != height)||(*prevW != width)) {err = 18106; return err;}
	} else {
		*prevH = height; *prevW = width;
	}
	//if (!buffer) {//if size inquiry only
	//	//*pMaxBuffer = width * height * 8 / iBitsPerSample;
	//	*pMaxBuffer = width * height;
	//	return err;
	//}
	//
	//if (*pMaxBuffer < width * height) {
	//	if (*buffer) delete [] (*buffer);
	//	*buffer = NULL;
	//}
	//if (!(*buffer)) {
	//	if ((*buffer = new int[width * height]) == NULL) {err = 18105; return err;}
	//	*pMaxBuffer = width * height;
	//	for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	//}
	//strip offsets and strip byte counts
	int iprev = -1; bool bStream = true;
	unsigned int ifirst = 0; unsigned int icount = 0;
	unsigned long stoffset = 0;
	for (int i=0; i<nstrip; i++) {
		if (nStripOffsets == 1) {//121013
			inum = oStripOffsets;
			//CString msg; msg.Format("121013-1 %d %d", nStripOffsets, inum); AfxMessageBox(msg);
		} else {
			stoffset = oStripOffsets + sizeofStripOffsets * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18136; return err;}
			if ((short)fp->Read(&inum, sizeofStripOffsets) != sizeofStripOffsets) {err = 18137; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		if (iprev < 0) {
			ifirst = inum;
		} else {
			if (iBitsPerSample == 8) {
				if ((int)inum - iprev != (int)icount) {bStream = false; break;}
			} else if (iBitsPerSample == 16) {
				if ((int)inum - iprev != (int)icount) {bStream = false; break;}
			}
		}
		iprev = inum;
		if (nStripByteCounts == 1) {//121013
			inum = oStripByteCounts;
			//CString msg; msg.Format("121013-2 %d %d", nStripByteCounts, inum); AfxMessageBox(msg);
		} else {
			stoffset = oStripByteCounts + sizeofStripByteCounts * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18138; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18139; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		icount = inum;
	}
	//Data
	if (bStream) {
		if (nStripByteCounts == 1) {//121013
			inum = oStripByteCounts;
			//CString msg; msg.Format("121013-3 %d %d", nStripByteCounts, inum); AfxMessageBox(msg);
		} else {
			if ((int)(fp->Seek(oStripByteCounts, CFile::begin)) != oStripByteCounts) {err = 18140; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18141; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		icount = inum;
		if ((unsigned long)(fp->Seek(ifirst, CFile::begin)) != ifirst) {err = 18142; return err;}
	}// else {AfxMessageBox("bStream false");}//////
	unsigned char* dens = NULL;
	unsigned short* sdens = NULL;
	if (piFlag) (*piFlag) &= ~(READTIF8bit | READTIF16bit);
	if (iBitsPerSample == 8) {
		dens = new unsigned char[width];
		if (!dens) {err = 18143; return err;}
		if (piFlag) (*piFlag) |= READTIF8bit;
	} else if (iBitsPerSample == 16) {
		sdens = new unsigned short[width];
		if (!sdens) {err = 18144; return err;}
		if (piFlag) (*piFlag) |= READTIF16bit;
	} else return 18145;
	for (int i=0; i<nstrip; i++) {
			//CString msg; msg.Format("121022-23 %d %d %d %d %d", i, k, iy, nrow, width); AfxMessageBox(msg);
		if (!bStream) {//if bStream is false, nstrip is > 1. 
			stoffset = oStripByteCounts + sizeofStripByteCounts * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18146; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18147; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
			icount = inum;
			stoffset = oStripOffsets + sizeofStripOffsets * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18148; return err;}
			if ((int)fp->Read(&inum, sizeofStripOffsets) != sizeofStripOffsets) {err = 18149; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
			if ((unsigned long)fp->Seek(inum, CFile::begin) != inum) {err = 18101; return err;}
		}
		if (icount <= 0) {err = 18150; return err;}
		if (iBitsPerSample == 8) {
			const int nrow = icount / width;//only for 8 bit format
			for (int k=0; k<nrow; k++) {
				const int iy = i * iRowsPerStrip + k;
				if (iy >= height) break;
				if (iy >= iLine + iMultiplex) break;
				if (fp->Read(dens, sizeof(char) * width) != sizeof(char) * width) {err = 18151; return err;}
				if (iy < iLine) continue;
				int idx0 = width * (iy - iLine);
				for (int j=0; j<iWidth; j++) {sbuffer[idx0 + j] = dens[j];}
			}
		} else if (iBitsPerSample == 16) {
			const int nrow = icount / width / 2;//for 16 bit format?
			for (int k=0; k<nrow; k++) {
				const int iy = i * iRowsPerStrip + k;
				if (iy >= height) break;
				if (iy >= iLine + iMultiplex) break;
				if (fp->Read(sdens, sizeof(short) * width) != sizeof(short) * width) {err = 18152; return err;}
				if (iy < iLine) continue;
				//int idx0 = width * (i * iRowsPerStrip + k);
				int idx0 = width * (iy - iLine);
				if (bLittleEndian) {
					for (int j=0; j<iWidth; j++) {sbuffer[idx0 + j] = (sdens[j]);}
					//for (int j=0; j<iWidth; j++) {sbuffer[idx0 + j] = (sdens[j] / 4);}///121023 unsigned ==> signed
				} else {
					for (int j=0; j<iWidth; j++) {
						ConvertToLittleEndian(&(sdens[j]), sizeof(short), carg);
						sbuffer[idx0 + j] = (sdens[j]);
						//sbuffer[idx0 + j] = (sdens[j] / 4);///121023 unsigned ==> signed
					}
				}
			}
		} else {
			//place holder 4 bit format
		}
		//break;////////
	}
	if (dens) delete [] dens;
	if (sdens) delete [] sdens;
	//
	return err;
}

/*121022
TErr ReadTifStrip(CFile* fp, short* sbuffer, int iLine, int iWidth, int iMultiplex, 
				  int* buffer) {
	TErr err = 0;
	//byte order
	char carg[READTIF_CARG_SIZE];
	unsigned char uchar[4];
	bool bLittleEndian = true;
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18121; return err;}
	if (strncmp( carg, "II" , 2 ) == 0) bLittleEndian = true;
	else if (strncmp( carg, "MM" , 2 ) == 0) bLittleEndian = false;
	else {err = 18122; return err;}
	//confirm identifier
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 18124; return err;}
	if (bLittleEndian) {
		if ((carg[0] != 42)||(carg[1] != 0)) {err = 18125; return err;}
	} else {
		if ((carg[1] != 42)||(carg[0] != 0)) {err = 18125; return err;}
	}
	//offset
	unsigned long offset = 0;
	if (fp->Read(&offset, sizeof(long)) != sizeof(long)) {err = 18126; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&offset, sizeof(int), carg);
	if ((unsigned long)(fp->Seek(offset, CFile::begin)) != offset) {err = 18127; return err;}
	short iDirEnt = 0, iIFD, itype; unsigned long inum;
	if (fp->Read(&iDirEnt, sizeof(short)) != sizeof(short)) {err = 18128; return err;}
	if (!bLittleEndian) ConvertToLittleEndian(&iDirEnt, sizeof(short), carg);
	int iImageWidth = -1, iImageLength = -1, iBitsPerSample = 8, iCompression = 1, 
		iPhotometric = 1, iRowsPerStrip = -1, iResolutionUnit = 1, iSamplesPerPixel = 1,
		iPlanarConfiguration = 1;
	int oStripOffsets = -1, oStripByteCounts = -1, oXResolution = -1, oYResolution = -1;
	int nStripOffsets = 0, nStripByteCounts = 0;
	int oArtist = -1, oImageDescription = -1;
	int nArtist = 0, nImageDescription = 0;
	short sizeofStripOffsets = sizeof(long);
	short sizeofStripByteCounts = sizeof(long);
	for (int i=0; i<iDirEnt; i++) {
		if (fp->Read(&iIFD, sizeof(short)) != sizeof(short)) {err = 18129; return err;}
		if (fp->Read(&itype, sizeof(short)) != sizeof(short)) {err = 18130; return err;}
		if (fp->Read(&inum, sizeof(unsigned long)) != sizeof(unsigned long)) {err = 18131; return err;}
		if (!bLittleEndian) {
			ConvertToLittleEndian(&iIFD, sizeof(short), carg);
			ConvertToLittleEndian(&itype, sizeof(short), carg);
			ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		for (int j=0; j<4; j++) {
			if (fp->Read(&(uchar[j]), sizeof(char)) != sizeof(char)) {err = 18132; return err;}
		}
		int iarg0 = 0, iarg1 = 0; bool bIndirect = false;
		switch (itype) {
			case TIF_BYTE: {}
			case TIF_ASCII: {
				if (inum > 4) bIndirect = true;
				break;}
			case TIF_SHORT: {
				if (inum > 2) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256; iarg1 = uchar[2] + uchar[3] * 256;}
					else {iarg0 = uchar[0] * 256 + uchar[1]; iarg1 = uchar[2] * 256 + uchar[3];}
				}
				break;}
			case TIF_LONG: {
				if (inum > 1) bIndirect = true;
				else {
					if (bLittleEndian) {iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;}
					else {iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;}
				}
				break;}
			case TIF_RATIONAL: {
				bIndirect = true;
				break;}
			default: {
				err = 18133; return err;
				break;}
		}
		if (bIndirect) {
			if (bLittleEndian) iarg0 = uchar[0] + uchar[1] * 256 + uchar[2] * 65536 + uchar[3] * 16777216;
			else iarg0 = uchar[3] + uchar[2] * 256 + uchar[1] * 65536 + uchar[0] * 16777216;
		}
		switch (iIFD) {
			case TIF_IMAGEWIDTH: {iImageWidth = iarg0; break;}
			case TIF_IMAGELENGTH: {iImageLength = iarg0; break;}
			case TIF_BITSPERSAMPLE: {iBitsPerSample = iarg0; break;}
			case TIF_COMPRESSION: {iCompression = iarg0; break;}
			case TIF_PHOTOMETRIC: {iPhotometric = iarg0; break;}
			case TIF_ROWSPERSTRIP: {iRowsPerStrip = iarg0; break;}
			case TIF_RESOLUTIONUNIT: {iResolutionUnit = iarg0; break;}
			case TIF_STRIPOFFSETS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripOffsets = sizeof(short);}
				else {err = 18134; return err;}
				oStripOffsets = iarg0; 
				nStripOffsets = inum; 
				break;}
			case TIF_STRIPBYTECOUNTS: {
				if (itype == TIF_LONG) {}
				else if (itype == TIF_SHORT) {sizeofStripByteCounts = sizeof(short);}
				else {err = 18135; return err;}
				oStripByteCounts = iarg0; 
				nStripByteCounts = inum; 
				break;}
			case TIF_XRESOLUTION: {oXResolution = iarg0; break;}
			case TIF_YRESOLUTION: {oYResolution = iarg0; break;}
			case TIF_SAMPLESPERPIXEL: {iSamplesPerPixel = iarg0; break;}
			case TIF_PLANARCONFIGURATION: {iPlanarConfiguration = iarg0; break;}
			case TIF_ARTIST: {oArtist = iarg0; nArtist = inum; break;}
			case TIF_IMAGEDESCRIPTION: {oImageDescription = iarg0; nImageDescription = inum; break;}
			case TIF_SOFTWARE: {}//NOP
			case TIF_DATETIME: {}//NOP
			default: break;
		}
	}
	//unsupported formats
	if (iCompression != 1) {err = 18111; return err;}
	if (iSamplesPerPixel != 1) {err = 18112; return err;}
	if (iPlanarConfiguration != 1) {err = 18113; return err;}
	if (iPhotometric > 1) {err = 18101; return err;}
	if ((iBitsPerSample != 8)&&(iBitsPerSample != 16)) {err = 18101; return err;}
	//if (iBitsPerSample != 8) {err = 18101; return err;}
	//irregular values
	if (iRowsPerStrip <= 0) {err = 18114; return err;}
	if (oStripOffsets <= 0) {err = 18115; return err;}
	if (oStripByteCounts <= 0) {err = 18115; return err;}//110112
	//pixel width and height
	const int width = iImageWidth;
	const int height = iImageLength;
	//110112 int nstrip = height / iRowsPerStrip; if (height % iRowsPerStrip) nstrip++;
	int nstrip1 = iLine / iRowsPerStrip; if (iLine % iRowsPerStrip) nstrip1++;
	int nstrip2 = (iLine + iMultiplex) / iRowsPerStrip; if ((iLine + iMultiplex) % iRowsPerStrip) nstrip2++;
	if (nstrip2 == nstrip1) nstrip2++;////////121013
	CString msg; msg.Format("121022 %d %d %d\r\n%d %d", iLine, iRowsPerStrip, iMultiplex, nstrip1, nstrip2); AfxMessageBox(msg);
	if ((width <= 0)||(height <= 0)) {err = 18106; return err;}
	if (iWidth != width) return 18106;
	//
	//Data
	unsigned int icount = 0;
	unsigned long stoffset = 0;
	unsigned char* dens = NULL;
	unsigned short* sdens = NULL;
	if (iBitsPerSample == 8) {
		dens = new unsigned char[width];
		if (!dens) {err = 18143; return err;}
	} else if (iBitsPerSample == 16) {
		sdens = new unsigned short[width];
		if (!sdens) {err = 18144; return err;}
	} else return 18145;
	//
	for (int i=nstrip1; i<nstrip2; i++) {
		if (nStripByteCounts == 1) {//121013
			inum = oStripByteCounts;
		} else {
			stoffset = oStripByteCounts + sizeofStripByteCounts * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18146; return err;}
			if ((short)fp->Read(&inum, sizeofStripByteCounts) != sizeofStripByteCounts) {err = 18147; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		icount = inum;
		if (nStripOffsets == 1) {//121013
			inum = oStripOffsets;
		} else {
			stoffset = oStripOffsets + sizeofStripOffsets * i;
			if ((unsigned long)(fp->Seek(stoffset, CFile::begin)) != stoffset) {err = 18148; return err;}
			if ((int)fp->Read(&inum, sizeofStripOffsets) != sizeofStripOffsets) {err = 18149; return err;}
			if (!bLittleEndian) ConvertToLittleEndian(&inum, sizeof(unsigned long), carg);
		}
		if ((unsigned long)fp->Seek(inum, CFile::begin) != inum) {err = 18101; return err;}
		//
		if (icount <= 0) {err = 18150; return err;}
		if (iBitsPerSample == 8) {
			const int nrow = icount / width;//only for 8 bit format
			for (int k=0; k<nrow; k++) {
				if (i * iRowsPerStrip + k >= height) break;
				if (fp->Read(dens, sizeof(char) * width) != sizeof(char) * width) {err = 18151; return err;}
				const int idx1 = i * iRowsPerStrip + k;
				const int idx0 = width * idx1;
				for (int j=0; j<iWidth; j++) {
					sbuffer[(idx1 - iLine) * iWidth + j] = sdens[j];
					if (buffer) buffer[idx0 + j] = sbuffer[(idx1 - iLine) * iWidth + j];
				}
			}
		} else if (iBitsPerSample == 16) {
			const int nrow = icount / width / 2;//for 16 bit format?
			for (int k=0; k<nrow; k++) {
				if (i * iRowsPerStrip + k >= height) break;
				if (fp->Read(sdens, sizeof(short) * width) != sizeof(short) * width) {err = 18152; return err;}
				const int idx1 = i * iRowsPerStrip + k;
				const int idx0 = width * idx1;
				if (bLittleEndian) {
					const int isize = sizeof(short) * iWidth;
					memcpy_s(&(sbuffer[(idx1 - iLine) * iWidth]), isize, sdens, isize);
					for (int j=0; j<iWidth; j++) {
						if (buffer) buffer[idx0 + j] = sbuffer[(idx1 - iLine) * iWidth + j];
					}
				} else {
					for (int j=0; j<iWidth; j++) {
						ConvertToLittleEndian(&(sdens[j]), sizeof(short), carg);
						sbuffer[(idx1 - iLine) * iWidth + j] = sdens[j];
						if (buffer) buffer[idx0 + j] = sbuffer[(idx1 - iLine) * iWidth + j];
					}
				}
			}
		} else {
			//place holder 4 bit format
		}
		//break;////////
	}
	if (dens) delete [] dens;
	if (sdens) delete [] sdens;
	return err;
}*/

TErr WriteTifMonochrome(CFile* fp, unsigned char* buffer, int height, int width,
												CString sImageDesc, CString sArtist) {
	TErr err = 0;
	////////
	//sizeof(char) = 1 bytes
	//sizeof(short) = 2 bytes
	//sizeof(int) = 4 bytes
	////////
	//FILE* file = fopen( filePath, "wb" );
	if (!fp) return 18100;
	char carg[81];
	//byte order: Little Endian
	strcpy_s(carg, "II" );
	fp->Write(carg, sizeof(char) * 2);
	//identifier
	carg[0] = 42; carg[1] = 0;
	fp->Write(carg, sizeof(char) * 2);
	//DirEnt offset
	const int offset = 8;
	unsigned long oDirEnt = offset + height * width;
	fp->Write(&oDirEnt, sizeof(long));
	//data
	for (int k=0; k<height; k++) {
		unsigned char* dens = &(buffer[width * k]);
		fp->Write(dens, sizeof(unsigned char) * width);
		//if (fwrite(dens, sizeof(char), width, file) != (unsigned int)width) {err = 18107; fclose(file); return err;}
	}
	//#DirEnt
	const short iDirEnt = 12;
	fp->Write(&iDirEnt, sizeof(short));
	//Dir Entries
	short iIFD, itype; unsigned long inum;
	const long iImageWidth = width;
	const long iImageLength = height;
	const long iBitsPerSample = 8;
	const long iCompression = 1;
	const long iPhotometric = 1;
	unsigned long nImageDescription = sImageDesc.GetLength();
	if (nImageDescription < 4) {sImageDesc += "    "; nImageDescription += 4;}
	if (nImageDescription > 80) {sImageDesc = sImageDesc.Left(80); nImageDescription = 80;}
	nImageDescription++;
	const long oImageDescription = oDirEnt + sizeof(short) + 
		iDirEnt * (sizeof(short) * 2 + sizeof(long) * 2) + sizeof(long);
	const unsigned long nStripOffsets = height;
	long oStripOffsets = oImageDescription + nImageDescription;
	if (nImageDescription & 1) oStripOffsets++;
	const long iSamplesPerPixel = 1;
	const long iRowsPerStrip = 1;
	const unsigned long nStripByteCounts = height;
	const long oStripByteCounts = oStripOffsets + nStripOffsets * sizeof(long);
	const long iPlanarConfiguration = 1;
	unsigned long nArtist = sArtist.GetLength();
	if (nArtist < 4) {sArtist += "    "; nArtist += 4;}
	if (nArtist > 80) {sArtist = sArtist.Left(80); nArtist = 80;}
	nArtist++;
	const long oArtist = oStripByteCounts + nStripByteCounts * sizeof(long);
	iIFD = TIF_IMAGEWIDTH; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iImageWidth, sizeof( long ));
	iIFD = TIF_IMAGELENGTH; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iImageLength, sizeof( long ));
	iIFD = TIF_BITSPERSAMPLE; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iBitsPerSample, sizeof( long ));
	iIFD = TIF_COMPRESSION; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iCompression, sizeof( long ));
	iIFD = TIF_PHOTOMETRIC; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iPhotometric, sizeof( long ));
	iIFD = TIF_IMAGEDESCRIPTION; itype = TIF_ASCII;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nImageDescription, sizeof( unsigned long ));
	fp->Write( &oImageDescription, sizeof( long ));
	iIFD = TIF_STRIPOFFSETS; itype = TIF_LONG;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nStripOffsets, sizeof( unsigned long ));
	fp->Write( &oStripOffsets, sizeof( long ));
	iIFD = TIF_SAMPLESPERPIXEL; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iSamplesPerPixel, sizeof( long ));
	iIFD = TIF_ROWSPERSTRIP; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iRowsPerStrip, sizeof( long ));
	iIFD = TIF_STRIPBYTECOUNTS; itype = TIF_LONG;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nStripByteCounts, sizeof( unsigned long ));
	fp->Write( &oStripByteCounts, sizeof( long ));
	iIFD = TIF_PLANARCONFIGURATION; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iPlanarConfiguration, sizeof( long ));
	iIFD = TIF_ARTIST; itype = TIF_ASCII;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nArtist, sizeof( unsigned long ));
	fp->Write( &oArtist, sizeof( long ));
	//end code
	oDirEnt = 0;
	fp->Write( &oDirEnt, sizeof( long ));
	//image description
	strcpy_s(carg, sImageDesc);
	fp->Write( carg, sizeof( char ) * nImageDescription);
	if (nImageDescription & 1) {
		carg[0] = 0;
		fp->Write( carg, sizeof( char ));
	}
	//strip offsets
	for (int i=0; i<height; i++) {
		inum = offset + i * width;
		fp->Write( &inum, sizeof( unsigned long ));
	}
	//strip byte counts
	for (int i=0; i<height; i++) {
		inum = width;
		fp->Write( &inum, sizeof( unsigned long ));
	}
	//artist
	strcpy_s(carg, sArtist);
	fp->Write( carg, sizeof( char ) * nArtist);
	if (nArtist & 1) {
		carg[0] = 0;
		fp->Write( carg, sizeof( char ));
	}
	return err;
}

TErr WriteTifMonochrome16(CFile* fp, int* buffer, int height, int width,
												CString sImageDesc, CString sArtist) {
	TErr err = 0;
	////////
	//sizeof(char) = 1 bytes
	//sizeof(short) = 2 bytes
	//sizeof(int) = 4 bytes
	////////
	if (!fp) return 18100;
	unsigned short* dens = new unsigned short[width];
	if (!dens) return 18102;
	char carg[READTIF_CARG_SIZE];
	//byte order: Little Endian
	strcpy_s(carg, "II" );
	fp->Write(carg, sizeof(char) * 2);
	//identifier
	carg[0] = 42; carg[1] = 0;
	fp->Write(carg, sizeof(char) * 2);
	//DirEnt offset
	const int offset = 8;
	unsigned long oDirEnt = offset + height * width * sizeof(unsigned short);
	fp->Write(&oDirEnt, sizeof(long));
	//data
	for (int k=0; k<height; k++) {
		//for (int j=0; j<width; j++) {
		//	short ipx = (short)(j * 10);
		//  fp->Write(&ipx, sizeof(short));
		//}
		for (int j=0; j<width; j++) {
			dens[j] = (buffer[width * k + j] > 0) ? (unsigned short)buffer[width * k + j] : 0;
		}
		//for (int j=0; j<width; j++) {dens[j] = (unsigned short)j;}
		//unsigned char* dens = &(buffer[width * k]);
		fp->Write(dens, sizeof(unsigned short) * width);
	}
	delete [] dens;
	//#DirEnt
	const short iDirEnt = 12;
	fp->Write(&iDirEnt, sizeof(short));
	//Dir Entries
	short iIFD, itype; unsigned long inum;
	const long iImageWidth = width;
	const long iImageLength = height;
	const long iBitsPerSample = 16;
	const long iCompression = 1;
	const long iPhotometric = 1;
	unsigned long nImageDescription = sImageDesc.GetLength();
	if (nImageDescription < 4) {sImageDesc += "    "; nImageDescription += 4;}
	if (nImageDescription > READTIF_CARG_SIZE-1) {
		sImageDesc = sImageDesc.Left(READTIF_CARG_SIZE-1); nImageDescription = READTIF_CARG_SIZE-1;
	}
	nImageDescription++;
	const long oImageDescription = oDirEnt + sizeof(short) + 
		iDirEnt * (sizeof(short) * 2 + sizeof(long) * 2) + sizeof(long);
	const unsigned long nStripOffsets = height;
	long oStripOffsets = oImageDescription + nImageDescription;
	if (nImageDescription & 1) oStripOffsets++;
	const long iSamplesPerPixel = 1;
	const long iRowsPerStrip = 1;
	const unsigned long nStripByteCounts = height;
	const long oStripByteCounts = oStripOffsets + nStripOffsets * sizeof(long);
	const long iPlanarConfiguration = 1;
	unsigned long nArtist = sArtist.GetLength();
	if (nArtist < 4) {sArtist += "    "; nArtist += 4;}
	if (nArtist > READTIF_CARG_SIZE-1) {
		sArtist = sArtist.Left(READTIF_CARG_SIZE-1); nArtist = READTIF_CARG_SIZE-1;
	}
	nArtist++;
	const long oArtist = oStripByteCounts + nStripByteCounts * sizeof(long);
	iIFD = TIF_IMAGEWIDTH; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iImageWidth, sizeof( long ));
	iIFD = TIF_IMAGELENGTH; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iImageLength, sizeof( long ));
	iIFD = TIF_BITSPERSAMPLE; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iBitsPerSample, sizeof( long ));
	iIFD = TIF_COMPRESSION; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iCompression, sizeof( long ));
	iIFD = TIF_PHOTOMETRIC; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iPhotometric, sizeof( long ));
	iIFD = TIF_IMAGEDESCRIPTION; itype = TIF_ASCII;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nImageDescription, sizeof( unsigned long ));
	fp->Write( &oImageDescription, sizeof( long ));
	iIFD = TIF_STRIPOFFSETS; itype = TIF_LONG;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nStripOffsets, sizeof( unsigned long ));
	fp->Write( &oStripOffsets, sizeof( long ));
	iIFD = TIF_SAMPLESPERPIXEL; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iSamplesPerPixel, sizeof( long ));
	iIFD = TIF_ROWSPERSTRIP; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iRowsPerStrip, sizeof( long ));
	iIFD = TIF_STRIPBYTECOUNTS; itype = TIF_LONG;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nStripByteCounts, sizeof( unsigned long ));
	fp->Write( &oStripByteCounts, sizeof( long ));
	iIFD = TIF_PLANARCONFIGURATION; itype = TIF_SHORT; inum = 1;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &inum, sizeof( unsigned long ));
	fp->Write( &iPlanarConfiguration, sizeof( long ));
	iIFD = TIF_ARTIST; itype = TIF_ASCII;
	fp->Write( &iIFD, sizeof( short ));
	fp->Write( &itype, sizeof( short ));
	fp->Write( &nArtist, sizeof( unsigned long ));
	fp->Write( &oArtist, sizeof( long ));
	//end code
	oDirEnt = 0;
	fp->Write( &oDirEnt, sizeof( long ));
	//image description
	strcpy_s(carg, sImageDesc);
	fp->Write( carg, sizeof( char ) * nImageDescription);
	if (nImageDescription & 1) {
		carg[0] = 0;
		fp->Write( carg, sizeof( char ));
	}
	//strip offsets
	for (int i=0; i<height; i++) {
		inum = offset + i * width * sizeof(unsigned short);
		fp->Write( &inum, sizeof( unsigned long ));
	}
	//strip byte counts
	for (int i=0; i<height; i++) {
		inum = width * sizeof(unsigned short);
		fp->Write( &inum, sizeof( unsigned long ));
	}
	//artist
	strcpy_s(carg, sArtist);
	fp->Write( carg, sizeof( char ) * nArtist);
	if (nArtist & 1) {
		carg[0] = 0;
		fp->Write( carg, sizeof( char ));
	}
	return err;
}

TErr ReadITEX(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, CString* pComment) {
	TErr err = 0;
	//identifier
	char carg[81];
	//AfxMessageBox("pre-IM");
	if (fp->Read(carg, sizeof(char) * 2) != sizeof(char) * 2) {err = 20101; return err;}
	if (strncmp( carg, "IM" , 2 ) != 0) {err = 20102; return err;}
	//params
	short iCommentLength, iImageWidth, iImageHeight, iXoffset, iYoffset, iFileType;
	if (fp->Read(&iCommentLength, sizeof(short)) != sizeof(short)) {err = 20101; return err;}
	if (fp->Read(&iImageWidth, sizeof(short)) != sizeof(short)) {err = 20101; return err;}
	if (fp->Read(&iImageHeight, sizeof(short)) != sizeof(short)) {err = 20101; return err;}
	if (fp->Read(&iXoffset, sizeof(short)) != sizeof(short)) {err = 20101; return err;}
	if (fp->Read(&iYoffset, sizeof(short)) != sizeof(short)) {err = 20101; return err;}
	if (fp->Read(&iFileType, sizeof(short)) != sizeof(short)) {err = 20101; return err;}

	if (*iHeight) {//160803
		if ((*iHeight != iImageHeight)||(*iWidth != iImageWidth)) return WARN_READIMAGE_SIZECHANGE;
	}

	char* comment;
	comment = new char[iCommentLength];
	if (comment) {
		long stoffset = 64;
		if (fp->Seek(stoffset, CFile::begin) != stoffset) {err = 20104; return err;}
		if (fp->Read(comment, sizeof(char) * iCommentLength) != sizeof(char) * iCommentLength) {
			err = 20101; return err;
		}
	}
	if (pComment) *pComment = comment;
	//CString line1 = comment; AfxMessageBox(line1);
	if (comment) delete [] comment;
	//Data
	const int nData = iImageWidth * iImageHeight;
	if (*pMaxBuffer < nData) {
		if (*buffer) delete [] (*buffer);
		*buffer = NULL;
	}
	if (!(*buffer)) {
		if ((*buffer = new int[nData]) == NULL) {err = 20103; return err;}
		*pMaxBuffer = nData;
		for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	}
	switch (iFileType) {
		case 0: {
			BYTE* data = new BYTE[nData];//typedef unsigned char BYTE
			if (data == NULL) return 20106;
			if (fp->Read(data, sizeof(BYTE) * nData) != sizeof(BYTE) * nData) {err = 20101; return err;}
			for (int i=0; i<nData; i++) {(*buffer)[i] = data[i];}
			delete [] data;
			break;}
		case 1: {return 20105;}
		case 2: {
			unsigned short* data = new unsigned short[nData];
			if (data == NULL) return 20106;
			if (fp->Read(data, sizeof(unsigned short) * nData) != sizeof(unsigned short) * nData) {err = 20101; return err;}
			for (int i=0; i<nData; i++) {(*buffer)[i] = data[i];}
			delete [] data;
			break;}
		case 3: {
			long* data = new long[nData];
			if (data == NULL) return 20106;
			if (fp->Read(data, sizeof(long) * nData) != sizeof(long) * nData) {err = 20101; return err;}
			for (int i=0; i<nData; i++) {(*buffer)[i] = data[i];}
			delete [] data;
			break;}
	}
	*iHeight = iImageHeight; *iWidth = iImageWidth;
	return 0;
}

TErr WriteITEX(CFile* fp, int* buffer, int iHeight, int iWidth, CString comment, int iXoffset, int iYoffset, int iFileType) {
	TErr err = 0;
	//identifier
	char carg[81];
	strcpy_s(carg, "IM");
	fp->Write(carg, sizeof(char) * 2);
	//params
	//const CString comment = "RecView output";
	short iCommentLength = comment.GetLength();
	char* cComment;
	cComment = new char[iCommentLength+1];
	if (cComment) {
		strcpy_s(cComment, iCommentLength+1, comment);
	} else {
		comment = "RecView output";
		iCommentLength = comment.GetLength();
		strcpy_s(carg, 81, comment);
		cComment = carg;
	}
	//int iImageWidth, iImageHeight, iXoffset, iYoffset, iFileType;
	fp->Write(&iCommentLength, sizeof(short));
	fp->Write(&iWidth, sizeof(short));
	fp->Write(&iHeight, sizeof(short));
	fp->Write(&iXoffset, sizeof(short));
	fp->Write(&iYoffset, sizeof(short));
	fp->Write(&iFileType, sizeof(short));
	//Comment
	const long stoffset = 64;
	if (fp->Seek(stoffset, CFile::begin) != stoffset) {err = 20104; return err;}
	fp->Write(cComment, sizeof(char) * iCommentLength);
	//Data
	const int nData = iWidth * iHeight;
	switch (iFileType) {
		case 0: {
			BYTE* data = new BYTE[nData];
			if (data == NULL) return 20106;
			for (int i=0; i<nData; i++) {data[i] = buffer[i];}
			fp->Write(data, sizeof(BYTE) * nData);
			delete [] data;
			break;}
		case 1: {return 20105;}
		case 2: {
			short* data = new short[nData];
			if (data == NULL) return 20106;
			for (int i=0; i<nData; i++) {data[i] = buffer[i];}
			fp->Write(data, sizeof(short) * nData);
			delete [] data;
			break;}
		case 3: {
			long* data = new long[nData];
			if (data == NULL) return 20106;
			for (int i=0; i<nData; i++) {data[i] = buffer[i];}
			fp->Write(data, sizeof(long) * nData);
			delete [] data;
			break;}
	}
	return 0;
}

TErr ReadITEXstrip(FILE* fp, short* buffer, int iLine, int iWidth, int iMultiplex) {
	TErr err = 0;
	//identifier
	char carg[81];
	if (fread(carg, sizeof(char), 2, fp) != 2) return 20101;
	if (strncmp( carg, "IM" , 2 ) != 0) return 20102;
	//params
	short iCommentLength, iImageWidth, idummy[3], iFileType;
	if (fread(&iCommentLength, sizeof(short), 1, fp) != 1) return 20101;
	if (fread(&iImageWidth, sizeof(short), 1, fp) != 1) return 20101;
	if (iImageWidth != iWidth) return 20110;
	if (fread(idummy, sizeof(short), 3, fp) != 3) return 20101;
	if (fread(&iFileType, sizeof(short), 1, fp) != 1) return 20101;
	//Seek
	if (iFileType != 2) return 20105;
	const long stoffset = 64 + iCommentLength + sizeof(short) * iWidth * iLine;
	if (fseek(fp, stoffset, SEEK_SET)) return 20104;
	//Read
	//if (fread(buffer, sizeof(short), iWidth * iMultiplex, fp) == (unsigned int)iWidth) return 0;
	//
	//if (fseek(fp, stoffset, SEEK_SET)) return 20104;
	short* sbuf = buffer;
	for (int i=0; i<iMultiplex; i++) {
		//if (fread(buffer, sizeof(short), iWidth, fp) != (unsigned int)iWidth) return 20101;
		sbuf = &(buffer[i * iWidth]);
		if (fread(sbuf, sizeof(short), iWidth, fp) != (unsigned int)iWidth) break;
	}
	return 0;
}

//160521
TErr ReadHDR5Frame(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, CHDR5* pHDR5, 
			  unsigned int uiFrame, int iDataEntry, CString* pComment) {
	TErr err = 0;
	//
	pHDR5->SetFile(fp);
	if (err = pHDR5->ReadSuperBlock(pComment)) return err;
	if (err = pHDR5->FindChildSymbol("exchange", -1, pComment)) return err;
	pHDR5->MoveToChildTree();
	//when iDataEntry >= 0, goto iDataEntry; if not, search for "data".
	if (err = pHDR5->FindChildSymbol("data", iDataEntry, pComment)) return err;
	if (pHDR5->m_sChildTitle.Left(4) != "data") return 16052221;
	if (err = pHDR5->GetDataObjHeader(pComment)) return err;
	const __int64 lImageHeight = pHDR5->m_plDataSize[1];
	const __int64 lImageWidth = pHDR5->m_plDataSize[2];
	//Data
	const int nData = (int)(lImageWidth * lImageHeight);
	if (*pMaxBuffer < nData) {
		if (*buffer) delete [] (*buffer);
		*buffer = NULL;
	}
	if (!(*buffer)) {
		try {*buffer = new int[nData];}
		catch(CException* e) {e->Delete(); err = 20103; return err;}
		*pMaxBuffer = nData;
		for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	}
	if (err = pHDR5->ReadFrame(uiFrame, *buffer, pComment)) return err;
	*iHeight = (int)lImageHeight; *iWidth = (int)lImageWidth;
	return 0;
}

TErr ReadHDR5Theta(CFile* fp, CHDR5* pHDR5, float* pfDeg, DWORD* pdwFrame, CString* pComment) {
	TErr err = 0;
	//
	pHDR5->SetFile(fp);
	if (err = pHDR5->ReadSuperBlock(pComment)) return err;
	if (err = pHDR5->FindChildSymbol("exchange", -1, pComment)) return err;
	pHDR5->MoveToChildTree();
	if (err = pHDR5->FindChildSymbol("theta", -1, pComment)) return err;
	if (pHDR5->m_sChildTitle != "theta") return 16052611;
	if (err = pHDR5->GetDataObjHeader(pComment)) return err;
	if (pHDR5->m_ucDataDim != 1) return 16052612;
	if (pdwFrame) {
		if (*pdwFrame <= 0) {*pdwFrame = (DWORD)(pHDR5->m_plDataSize[0]); return 0;}
	}
	return err;
}

//110913
TErr Read_hishead(CFile* fimg, HISHeader* his, CString* pComment) {
	char	buffer[HIS_Header_Size];

// reading header without comment and setup 
	if (fimg->Read(buffer, sizeof(char) * HIS_Header_Size) != sizeof(char) * HIS_Header_Size) return 20120;
	memcpy(&his->head,           buffer,       2);	// char x 2
	memcpy(&his->comment_length, buffer +  2,  2);	// short
	memcpy(&his->width,          buffer +  4,  2);	// short
	memcpy(&his->height,         buffer +  6,  2);	// short
	memcpy(&his->x_offset,       buffer +  8,  2);	// short
	memcpy(&his->y_offset,       buffer + 10,  2);	// short
	memcpy(&his->type,           buffer + 12,  2);	// short
	memcpy(&his->n_image1,       buffer + 14,  2);	// short
	memcpy(&his->n_image2,       buffer + 16,  2);	// short
	memcpy(&his->reserve1,       buffer + 18,  2);	// short
	memcpy(&his->reserve2,       buffer + 20,  2);	// short
	memcpy(&his->time_stamp,     buffer + 22,  8);	// double
	memcpy(&his->maker,          buffer + 30,  4);	// long
	memcpy(&his->reserved,       buffer + 34, 30);	// char x 30

	char* comment = new char[his->comment_length+1];
	if (comment == NULL) return 20121;
	if (fimg->Read(comment, sizeof(char) * his->comment_length) != sizeof(char) * his->comment_length) {
		delete [] comment;
		return 20122;
	}
	comment[his->comment_length] = '\0';
	if (pComment) *pComment = comment;
	delete [] comment;

	//CString msg = "", line;
	//line.Format("%s\r\n", his->head); msg += line;
	//line.Format("%d\r\n", his->comment_length); msg += line;		/*2-3*/
	//line.Format("%d\r\n", his->width); msg += line;				/*4-5*/
	//line.Format("%d\r\n", his->height); msg += line;				/*6-7*/
	//line.Format("%d\r\n", his->x_offset); msg += line;			/*8-9*/
	//line.Format("%d\r\n", his->y_offset); msg += line;			/*10-11*/
	//line.Format("%d\r\n", his->type); msg += line;				/*12-13*/
	//line.Format("%d\r\n", his->n_image1); msg += line;			/*14-15*/
	//line.Format("%ld\r\n", his->n_image2*65536); msg += line;		/*16-17*/
	//line.Format("%d\r\n", his->reserve1); msg += line;			/*18-19*/
	//line.Format("%d\r\n", his->reserve2); msg += line;			/*20-21*/
	//line.Format("%lf\r\n", his->time_stamp); msg += line;			/*22-29*/
	//line.Format("%ld\r\n", his->maker); msg += line;				/*30-33*/
	//line.Format("%s\r\n", his->reserved); msg += line;			/*34-64*/
	//if (pComment) line.Format("%s\r\n", *pComment); msg += line;
	//AfxMessageBox(msg);

	return 0;
}

TErr ReadHIS(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, HISHeader* his, CString* pComment) {
// read comment and image data from HIS file
	if (fp->Read(his, sizeof(char) * HIS_Header_Size) != sizeof(char) * HIS_Header_Size) return 20123;
	if (strncmp(his->head, "IM", 2)) return 20124;

	char* comment = NULL;
	try {comment = new char[his->comment_length+1];}
	catch(CException* e) {e->Delete(); return 20103;}
	if (comment) {
		if (fp->Read(comment, sizeof(char) * his->comment_length) != sizeof(char) * his->comment_length) { 
			delete [] comment; 
			return 20126;
		}
		comment[his->comment_length] = '\0';
		if (pComment) *pComment = comment;
		delete [] comment;
	}
	
	//Data
	const int iImageWidth = his->width;
	const int iImageHeight = his->height;
	const long nData = iImageWidth * iImageHeight;
	const long nDataAlloc = nData + 2;
	if (*pMaxBuffer < nDataAlloc) {
		if (*buffer) delete [] (*buffer);
		*buffer = NULL;
	}
	if (!(*buffer)) {
		try {*buffer = new int[nDataAlloc];}
		catch(CException* e) {
			e->Delete();
			AfxMessageBox("Run out of memory");
			return 20103;
		}
		*pMaxBuffer = nDataAlloc;
		memset(*buffer, 0, sizeof(int) * nDataAlloc);
		//for (int i=0; i<*pMaxBuffer; i++) {(*buffer)[i] = 0;}
	}

	switch (his->type) {
		case 2: {//16 bit
			unsigned short	*data = NULL;
			const long NP = nData;
			const long NPalloc = NP + 2;
			try {data = new unsigned short[NPalloc];}
			catch (CException* e) {e->Delete(); return 20127;}
			if (fp->Read(data, sizeof(unsigned short) * NP) != sizeof(unsigned short) * NP) {
				delete [] data;
				return 20128;
			}
			for(int jj=0;jj<NP;++jj){(*buffer)[jj] = data[jj];}
			delete [] data;
			break;}
		case 6: {//12 bit
			unsigned char	*cdata = NULL;
			const long NP= nData * 3 / 2;
			const long NPalloc = NP + 3;
			try {cdata = new unsigned char[NPalloc];}
			catch (CException* e) {e->Delete(); return 20127;}
			//int ibyte = fp->Read(cdata, sizeof(unsigned char) * NP);
			if (fp->Read(cdata, sizeof(unsigned char) * NP) != sizeof(unsigned char) * NP) {
				//CString msg; msg.Format("%d %d", ibyte, sizeof(unsigned char) * NP); AfxMessageBox(msg);
				delete [] cdata;
				return 20128;
			}
			int j=0;
			for(int k=0;k<NP;k+=3){
				(*buffer)[j++] = (cdata[k] << 4) + (cdata[k+1] >> 4);
				(*buffer)[j++] = ((cdata[k+1] & 0x0f) << 8) + cdata[k+2];
			}
			delete [] cdata;
			break;}
		default: {return 20129;}
	}

	//sprintf(img->head,"IM");
//	img->comment_length=his->comment_length;
	//img->comment_length=0;
	//img->width=his->width;
	//img->height=his->height;
	//img->x_offset=his->x_offset;
	//img->y_offset=his->y_offset;
//	img->type=his->type;
	//img->type=2;
//	sprintf(img->reserved,"");
//	printf("%s\n",his->comment);
//	sprintf(img->comment,"%s",his->comment);

	//CString msg = "", line;
	//line.Format("%s\r\n", his->head); msg += line;
	//line.Format("%d\r\n", his->comment_length); msg += line;		/*2-3*/
	//line.Format("%d\r\n", his->width); msg += line;				/*4-5*/
	//line.Format("%d\r\n", his->height); msg += line;				/*6-7*/
	//line.Format("%d\r\n", his->x_offset); msg += line;			/*8-9*/
	//line.Format("%d\r\n", his->y_offset); msg += line;			/*10-11*/
	//line.Format("%d\r\n", his->type); msg += line;				/*12-13*/
	//line.Format("%d\r\n", his->n_image1); msg += line;			/*14-15*/
	//line.Format("%ld\r\n", his->n_image2*65536); msg += line;		/*16-17*/
	//line.Format("%d\r\n", his->reserve1); msg += line;			/*18-19*/
	//line.Format("%d\r\n", his->reserve2); msg += line;			/*20-21*/
	//line.Format("%lf\r\n", his->time_stamp); msg += line;			/*22-29*/
	//line.Format("%ld\r\n", his->maker); msg += line;				/*30-33*/
	//line.Format("%s\r\n", his->reserved); msg += line;			/*34-64*/
	//if (pComment) line.Format("%s\r\n", *pComment); msg += line;
	//AfxMessageBox(msg);

	*iHeight = iImageHeight; *iWidth = iImageWidth;
	return 0;
}

TErr SkipHISframe(CFile* fp, int nframe) {
	if (nframe == 0) return 0;
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	const BOOL bFast = pApp->dlgProperty.m_EnFastSeek;
	HIS_Header his;
	//CString msg = "", line;
	//int n = 0;
	const int nLoop = bFast ? 1 : nframe;
	//140110 for (int i=0; i<nframe; i++) {
	for (int i=0; i<nLoop; i++) {
		if (fp->Read(&his, sizeof(char) * HIS_Header_Size) != sizeof(char) * HIS_Header_Size) {
			//CString msg; msg.Format("%d %d", i, nframe); AfxMessageBox(msg);
			return 20123;
		}
		fp->Seek(sizeof(char) * his.comment_length, CFile::current);
		//n += sizeof(char) * HIS_Header_Size + sizeof(char) * his.comment_length;
		switch (his.type) {
			case 2: {//16 bit
				const LONGLONG nData = his.width * his.height;
				//120715 const long nData = his.width * his.height;
				//n+= sizeof(unsigned short) * nData;
				fp->Seek(sizeof(unsigned short) * nData, CFile::current);
				break;}
			case 6: {//12 bit
				const LONGLONG nData = his.width * his.height * 3 / 2;
				//120715 const long nData = his.width * his.height * 3 / 2;
				//n+= sizeof(unsigned char) * nData;
				fp->Seek(sizeof(unsigned char) * nData, CFile::current);
				break;}
			default: {return 20129;}
		}
		//n++;
		//line.Format("%d ", n-5529664); msg += line;
	}
	//CString msg; msg.Format("%d", n); AfxMessageBox(msg);

	//140110
	if (!bFast) return 0;
	if (nframe == 1) return 0;
	return SkipHISframeFast(fp, nframe-1);//This works because only the first file contains a lengthy comment and others null.
}

TErr SkipHISframeFast(CFile* fp, int nframe) {
	//This routine can be used only if the header size including comment length is constant throughout the skipping block.
	if (nframe == 0) return 0;
	//fp->Seek(62728508416, CFile::current);
	//return 0;

	LONGLONG llnframe = nframe;
	HIS_Header his;
	if (fp->Read(&his, sizeof(char) * HIS_Header_Size) != sizeof(char) * HIS_Header_Size) {
		//CString msg; msg.Format("%d %d", i, nframe); AfxMessageBox(msg);
		return 20123;
	}
	switch (his.type) {
		case 2: {//16 bit
			const LONGLONG nData = (llnframe-1) * sizeof(char) * HIS_Header_Size
								+ llnframe * (sizeof(char) * his.comment_length + sizeof(unsigned short) * (his.width * his.height));
			//CString msg; msg.Format("%lld %d", nData, nframe); AfxMessageBox(msg);
			fp->Seek(nData, CFile::current);
			break;}
		case 6: {//12 bit
			const LONGLONG nData =  (llnframe-1) * sizeof(char) * HIS_Header_Size
								+ llnframe * (sizeof(char) * his.comment_length + sizeof(unsigned char) * (his.width * his.height * 3 / 2));
			fp->Seek(nData, CFile::current);
			break;}
		default: {return 20129;}
	}
	return 0;
}
//110917
TErr ReadHISstrip(CFile* fp, unsigned char** uctmp, int* pMaxTmp, short* sbuffer, 
				  int iLine, int iWidth, int iMultiplex, HISHeader* his) {
	TErr err = 0;
	//identifier
	if (fp->Read(his, sizeof(char) * HIS_Header_Size) != sizeof(char) * HIS_Header_Size) return 20123;
	if (strncmp(his->head, "IM", 2)) return 20124;
	//skip comment
	fp->Seek(sizeof(char) * his->comment_length, CFile::current);
	//params
	const int iImageWidth = his->width;
	if (iImageWidth != iWidth) return 20110;
	//alloc ltmp
	if (his->type == 6) {//12 bit
		const long nDataAlloc = iImageWidth * iMultiplex * 3 / 2 + 3;
		if (*pMaxTmp < nDataAlloc) {
			if (*uctmp) delete [] (*uctmp);
			*uctmp = NULL;
		}
		if (!(*uctmp)) {
			try {*uctmp = new unsigned char[nDataAlloc];}
			catch(CException* e) {e->Delete();return 20103;}
			*pMaxTmp = nDataAlloc;
			memset(*uctmp, 0, sizeof(unsigned char) * nDataAlloc);
		}
	}
	//CString msg = "", line;
	//line.Format("%d %d %d %d\r\n", *uctmp, *pMaxTmp, iLine, iWidth); msg += line;
	//AfxMessageBox(msg);

	switch (his->type) {
		case 2: {//16 bit
			//Seek
			fp->Seek(sizeof(unsigned short) * iWidth * iLine, CFile::current);
			//Read
			const int NP = iWidth * iMultiplex;
			if (fp->Read(sbuffer, sizeof(short) * NP) != sizeof(short) * NP) return 20128;
			//120715 seek remaining data
			const LONGLONG nData = iWidth * his->height - iWidth * iLine - NP;
			fp->Seek(sizeof(unsigned short) * nData, CFile::current);
			//120715
			break;}
		case 6: {//12 bit
			//Seek
			if (iWidth * iLine) fp->Seek(sizeof(unsigned char) * iWidth * iLine * 3 / 2, CFile::current);
			//Read
			const int NP = iWidth * iMultiplex * 3 / 2;
			if (fp->Read(*uctmp, sizeof(unsigned char) * NP) != sizeof(unsigned char) * NP) return 20128;
			//120715 seek remaining data
			const LONGLONG nData = iWidth * his->height * 3 / 2 - iWidth * iLine * 3 / 2 - NP;
			fp->Seek(sizeof(unsigned char) * nData, CFile::current);
			//120715
			int j = 0;
			for(int k=0;k<NP;k+=3){
				sbuffer[j++] = ((*uctmp)[k] << 4) + ((*uctmp)[k+1] >> 4);
				sbuffer[j++] = (((*uctmp)[k+1] & 0x0f) << 8) + (*uctmp)[k+2];
			}
			break;}
		default: {return 20129;}
	}
	return 0;
}

int StringCompare( const void *arg1, const void *arg2 ) {
   return _tcsicmp( * ( char** ) arg1, * ( char** ) arg2 );
}

//*160910
#ifdef _WIN64
unsigned __stdcall DeconvBackProjThread(void* pArg) {
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) {
		//_endthreadex( 0 );
		return 0;
	}
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	//AfxMessageBox("131014 #ifdef _WIN64  DeconvBackProjThread");
	if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) {
		//-----INTEL body-----//
		const int ixdim = ri->ixdim;
		const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
		const int iIntpDim = (int) pow((double)2, iZooming);
		const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
		const int ndim = (int) pow((double)2, ndimp);
		//const int iIntpDim = ri->ipintp;
		const double center = ri->center;
		const int ixdimp = ixdim * iIntpDim;
		//
		CCmplx* p = NULL;
		int* igp = NULL;
		const int imargin = ixdimp;
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
			//error.Log(28802);//120720
			return 0;
		}
		CFft fft;
		fft.Init1(ndimp, -1);
		memset(igp, 0, sizeof(int) * igpdim);
		const int ixdimh = ixdimp / 2;
		const int ihoffset = ndim / 2 - 1 - ixdimh;
		//const __int64 icenter = (__int64)((ixdimh + center - (int)(center)) * DBPT_PNT);
		//if (pApp->dlgProperty.bEnableSIMD) param[7] |= 0x0001;
		const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
		int iCurrStep = 0;
		CGazoDoc* pd = (CGazoDoc*)(ri->pDoc);
		CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
		const BOOL bReport = pApp->dlgProperty.m_EnReport;
		for (int i=(ri->iStartSino); i<(ri->iLenSinogr-1); i+=(ri->iStepSino)) {
			if (bReport) {
				if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
			}
			if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
			//100315 const int sidx = ixdim * (i * ri->iMultiplex + ri->iOffset);
			//if (sidx >= ri->maxLenSinogr) break;
			//short* iStrip = &(ri->iSinogr[sidx]);
			const int sidx = i * ri->iMultiplex + ri->iOffset;
			if (sidx >= ri->maxSinogrLen) break;
			short* iStrip = ri->iSinogr[sidx];
			(*(ri->nSinogr))++;
			//140611
			if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
				if (i & 1) {
					if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue;
				} else {
					if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue;
				}
			}
			//
				memset(p, 0, sizeof(CCmplx) * ndim);//111206
				const int idx0 = (0 - (int)center) * iIntpDim + (ndim / 2 - 1);
				const int idx1 = (ixdim-1 - (int)center) * iIntpDim + (ndim / 2 - 1) + 1;
				for (int m=0; m<idx0; m++) {p[m].re = iStrip[0];}
				for (int m=idx1; m<ndim; m++) {p[m].re = iStrip[ixdim-1];}
				//111206
			for (int k=0; k<ixdim; k++) {
				int idx = (k - (int)center) * iIntpDim + (ndim / 2 - 1);
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
			//090211 for (int j=0; j<ixdimp; j++) {igp[j + imargin] = (int)(p[j + ihoffset].re * BACKPROJ_SCALE);}
			for (int j=0; j<ixdimp; j++) {
				const TCmpElmnt p0 = p[j + ihoffset].re * BACKPROJ_SCALE;
				const TCmpElmnt p1p0 = (j == ixdimp -1)? 
					0.0f : (p[j + ihoffset + 1].re - p[j + ihoffset].re) / DBPT_GINTP * BACKPROJ_SCALE;
				const int gidx = (j + imargin) * DBPT_GINTP;
				for (int k=0; k<DBPT_GINTP; k++) {igp[gidx + k] = (int)(p0 + p1p0 * k);}
			}
			//080318 float th = ri->fdeg[i] * (float)DEG_TO_RAD;
			float th = (ri->fdeg[i] + ri->fTiltAngle) * (float)DEG_TO_RAD;
			const float fcos = (float)(cos(th) * DBPT_GINTP);
			const float fsin = (float)(-sin(th) * DBPT_GINTP);
			const float fcenter = ((ixdimh + (float)center - (int)(center)) * DBPT_GINTP);
			const float foffset = fcenter - ixdimh * (fcos + fsin);
			__int64 iparam6 = ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP;
			int* ipgp = (int*)(iparam6);
			int* ifp = ri->iReconst;
			const int ixdimpg = ixdimp << DBPT_LOG2GINTP;
			for (int iy=0; iy<ixdimp; iy++) {
				int ifpidx = iy * ixdimp - 1;
				double dyoff = iy * fsin + foffset;
				for (int ix=0; ix<ixdimp; ix++) {
					int ix0 = (int)(ix * fcos + dyoff);
					ifpidx++;
					if (ix0 < 0) continue;
					if (ix0 >= ixdimpg) continue;
					ifp[ifpidx] += ipgp[ix0];
				}
			}
		}

		delete [] igp;
		delete [] p;
		//-----end of INTEL body-----//
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		if (pApp->dlgProperty.bUseCUDAFFT) {
			//kernel 4/8 CUDA-FFT
			CudaReconstHostFFT(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
			//kernel 4/8 CUDA-FFT with long integer
			//CudaReconstHostLong(ri, ri->iStartSino);
		} else {
			//kernel 4/8 with my FFT
			CudaReconstHost(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
		}
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
		CLReconstHost(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
	} else {
		CString line; line.Format("Unknown processor type: %d", pApp->dlgProperty.m_ProcessorType);
		AfxMessageBox(line);
	}
	//_endthreadex( 0 );
	if (ri->iStatus == RECONST_INFO_ERROR) return 0;
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

#endif // ifdef _WIN64

#ifndef _WIN64
///*///
unsigned __stdcall DeconvBackProjThread(void* pArg) {
	//AfxMessageBox("131014 #ifndef _WIN64  DeconvBackProjThread");
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) {
		//_endthreadex( 0 );
		return 0;
	}
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) {
		//-----INTEL body-----//
		const int ixdim = ri->ixdim;
		const int iZooming = (ri->iInterpolation > CDLGRECONST_OPT_ZOOMING_NONE) ? (ri->iInterpolation - CDLGRECONST_OPT_ZOOMING_NONE) : 0;
		const int iIntpDim = (int) pow((double)2, iZooming);
		const int ndimp = (int)((log((double)ixdim) / LOG2)) + 1 + iZooming;
		const int ndim = (int) pow((double)2, ndimp);
		const double center = ri->center;
		const int ixdimp = ixdim * iIntpDim;
		//
		CCmplx* p = NULL;
		int* igp = NULL;
		const int imargin = ixdimp;
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
			//error.Log(28802);//120720
			return 0;
		}
		CFft fft;
		fft.Init1(ndimp, -1);
		memset(igp, 0, sizeof(int) * igpdim);
		const int ixdimh = ixdimp / 2;
		const int ihoffset = ndim / 2 - 1 - ixdimh;
		//const __int64 icenter = (__int64)((ixdimh + center - (int)(center)) * DBPT_PNT);
		//if (pApp->dlgProperty.bEnableSIMD) param[7] |= 0x0001;
		const int iProgStep = ri->iLenSinogr / PROGRESS_BAR_UNIT;
		int iCurrStep = 0;
		CGazoDoc* pd = (CGazoDoc*)(ri->pDoc);
		CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
		const BOOL bReport = pApp->dlgProperty.m_EnReport;
		for (int i=(ri->iStartSino); i<(ri->iLenSinogr-1); i+=(ri->iStepSino)) {
			if (bReport) {
				if (DBProjDlgCtrl(ri, iProgStep, i, &iCurrStep)) break;
			}
			if (!(ri->bInc[i] & CGAZODOC_BINC_SAMPLE)) continue;
			//100315 const int sidx = ixdim * (i * ri->iMultiplex + ri->iOffset);
			//if (sidx >= ri->maxLenSinogr) break;
			//short* iStrip = &(ri->iSinogr[sidx]);
			const int sidx = i * ri->iMultiplex + ri->iOffset;
			if (sidx >= ri->maxSinogrLen) break;
			short* iStrip = ri->iSinogr[sidx];
			(*(ri->nSinogr))++;
			//140611
			if (ri->dReconFlags & (RQFLAGS_USEONLYEVENFRAMES | RQFLAGS_USEONLYODDFRAMES)) {
				if (i & 1) {
					if (ri->dReconFlags & RQFLAGS_USEONLYEVENFRAMES) continue;
				} else {
					if (ri->dReconFlags & RQFLAGS_USEONLYODDFRAMES) continue;
				}
			}
			memset(p, 0, sizeof(CCmplx) * ndim);//111206
			const int idx0 = (0 - (int)center) * iIntpDim + (ndim / 2 - 1);
			const int idx1 = (ixdim-1 - (int)center) * iIntpDim + (ndim / 2 - 1) + 1;
			for (int m=0; m<idx0; m++) {p[m].re = iStrip[0];}
			for (int m=idx1; m<ndim; m++) {p[m].re = iStrip[ixdim-1];}
			for (int k=0; k<ixdim; k++) {
				int idx = (k - (int)center) * iIntpDim + (ndim / 2 - 1);
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
			for (int j=0; j<ixdimp; j++) {
				const TCmpElmnt p0 = p[j + ihoffset].re * BACKPROJ_SCALE;
				const TCmpElmnt p1p0 = (j == ixdimp -1)? 
					0.0f : (p[j + ihoffset + 1].re - p[j + ihoffset].re) / DBPT_GINTP * BACKPROJ_SCALE;
				const int gidx = (j + imargin) * DBPT_GINTP;
				for (int k=0; k<DBPT_GINTP; k++) {igp[gidx + k] = (int)(p0 + p1p0 * k);}
			}
			float th = (ri->fdeg[i] + ri->fTiltAngle) * (float)DEG_TO_RAD;
			const float fcos = (float)(cos(th) * DBPT_GINTP);
			const float fsin = (float)(-sin(th) * DBPT_GINTP);
			const float fcenter = (ixdimh + (float)center - (int)(center)) * DBPT_GINTP;
			const float foffset = fcenter - ixdimh * (fcos + fsin);
			int iparam6 = ((DWORD_PTR) igp) + imargin * sizeof(int) * DBPT_GINTP;
			int* ipgp = (int*)(iparam6);
			int* ifp = ri->iReconst;
			const int ixdimpg = ixdimp << DBPT_LOG2GINTP;
			for (int iy=0; iy<ixdimp; iy++) {
				const int ifpidx = iy * ixdimp;
				const double dyoff = iy * fsin + foffset;
				for (int ix=0; ix<ixdimp; ix++) {
					int ix0 = (int)(ix * fcos + dyoff);
					if (ix0 < 0) continue;
					if (ix0 >= ixdimpg) continue;
					ifp[ifpidx + ix] += ipgp[ix0];
				}
			}
		}
		delete [] igp;
		delete [] p;
		//-----end of INTEL body-----//
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
		if (pApp->dlgProperty.bUseCUDAFFT) {
			//kernel 4/8 CUDA-FFT
			CudaReconstHostFFT(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
			//kernel 4/8 CUDA-FFT with long integer
			//CudaReconstHostLong(ri, ri->iStartSino);
		} else {
			//kernel 4/8 with my FFT
			CudaReconstHost(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
		}
	} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_ATISTREAM) {
		CLReconstHost(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
	} else {
		CString line; line.Format("Unknown processor type: %d", pApp->dlgProperty.m_ProcessorType);
		AfxMessageBox(line);
	}
	//_endthreadex( 0 );
	if (ri->iStatus == RECONST_INFO_ERROR) return 0;
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

#endif // ifndef _WIN64

bool DBProjDlgCtrl(RECONST_INFO* ri, int iProgStep, int iSino, int* pCurrStep) {
	CGazoDoc* pd = (CGazoDoc*)(ri->pDoc);
	CMainFrame* pf = (CMainFrame*) AfxGetMainWnd();
	if (pd->dlgReconst.iStatus == CDLGRECONST_STOP) return true;
	if (!ri->bMaster) return false;
	if (pd->dlgReconst.m_hWnd) {
		if (iProgStep > 0) {//121013
			if ((*pCurrStep) < iSino / iProgStep) {
				(*pCurrStep)++;
				if ((*pCurrStep) < PROGRESS_BAR_UNIT) pd->dlgReconst.m_Progress.StepIt();
			}
		}
	}
	if ( ((iSino - ri->iStartSino) / ri->iStepSino) % 10 == 0) {
		::ProcessMessage();
		if (pf) {
			CString line;
			line.Format("Projection %d of ", *(ri->nSinogr));
			pf->m_wndStatusBar.SetPaneText(0, line + ri->dataName);
		}
	}
	return false;
}

unsigned __stdcall GenerateSinogramThread(void* pArg) {
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) {
		//_endthreadex( 0 );
		return 0;
	}
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	//Each processor gives correct sinograms, 
	//but CUDA routine affects the resulant tomograms by memory allocation of d_Strip.
	//Therefore, intel processors should be used for sinogram generation.
	//CUDA(QuadroFX3700, 112 sp): 3.3 sec / 100 sinograms of 2000 width
	//Intel(Xeon 2.33 GHz, 4 core): 5.1 sec / 100 sinograms of 2000 width
	//
	//090902 if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_INTEL) {
	if (true) {
		//-----INTEL body-----//
		const int isino = ri->maxSinogrLen;
		const int ixlen = ri->ixdim;
		const int iMultiplex = ri->iMultiplex;
		const int iBinning = (ri->iInterpolation == 0) ? 4 : ((ri->iInterpolation == 1) ? 2 : 1);
		short** iSinogr = ri->iSinogr;
		char* bInc = ri->bInc;
		float* fexp = ri->fexp;
		float* fdeg = ri->fdeg;
		const int iSINOGRAM_PIXEL_MIN = (ri->iFlag & READTIF8bit) ? SINOGRAM_PIXEL_MIN8bit : SINOGRAM_PIXEL_MIN16bit;
		//short* iDark = iSinogr[iMultiplex * (isino - 1)];
		//short* iIncident0 = NULL;
		//short* iIncident1 = NULL;
		float t0;
		//121127 short* iIncidentx = new short[ixlen];
		int* iIncidentx = new int[ixlen];
		if (iIncidentx == NULL) {ri->iStatus = RECONST_INFO_ERROR; return 0;}
		int i0 = -1, i1 = -1;
		//121019
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
			if ((i0 < 0)||(i1 < 0)) {ri->iStatus = RECONST_INFO_ERROR; return 0;}
			//100315 const int i0 = (int)(iIncident0 - iSinogr) / ixlen / iMultiplex;
			//const int i1 = (int)(iIncident1 - iSinogr) / ixlen / iMultiplex;
			if (i % ri->iStepSino != ri->iStartSino) continue;
			t0 = fexp[i0];
			const float t1 = fexp[i1];
			const float ti = fexp[i];
			if (t0 == t1) t0 = 0;
			else t0 = (ti - t0) / (t1 - t0);
			//100330 drift correction
			const double cth = cos(fdeg[i] * DEG_TO_RAD); const double sth = sin(fdeg[i] * DEG_TO_RAD);
			int idrift = 0;
			if ((ri->drEnd > ri->drStart)&&(ri->dReconFlags & RQFLAGS_DRIFTPARAMS)) {
				if ((ri->drStart < i)&&(i < ri->drEnd)&&(ri->drOmit)) {
					for (int k=0; k<iMultiplex; k++) {
						short* iStrip = iSinogr[iMultiplex * i + k];
						for (int j=0; j<ixlen; j++) {iStrip[j] = 0;}
					}
					continue;
				}
				if (i >= ri->drEnd) idrift = (int)(ri->drX * cth - ri->drY * sth);
				else if (i > ri->drStart) idrift = (int)((ri->drX * cth - ri->drY * sth) * (i - ri->drStart) / (ri->drEnd - ri->drStart));
			}
			//120501 drift collection
			if ((ri->dReconFlags & RQFLAGS_DRIFTLIST)&&(ri->piDrift)) idrift -= (ri->piDrift)[i];
			//
			for (int k=0; k<iMultiplex; k++) {
				//121127 short* iDark = iSinogr[iMultiplex * (isino - 1) + k];
				//121127 short* iIncident0 = iSinogr[iMultiplex * i0 + k];
				//121127 short* iIncident1 = iSinogr[iMultiplex * i1 + k];
				unsigned short* iDark = (unsigned short*)(iSinogr[iMultiplex * (isino - 1) + k]);
				unsigned short* iIncident0 = (unsigned short*)(iSinogr[iMultiplex * i0 + k]);
				unsigned short* iIncident1 = (unsigned short*)(iSinogr[iMultiplex * i1 + k]);
				for (int j=0; j<ixlen; j++) {
					iIncidentx[j] = iIncident0[j] + (short)((iIncident1[j] - iIncident0[j] + 0.5) * t0) - iDark[j];
				}
				short* iStrip = iSinogr[iMultiplex * i + k];
				unsigned short* iuStrip = (unsigned short*)(iSinogr[iMultiplex * i + k]);//121127
				if (ri->dReconFlags & RQFLAGS_ZERNIKE) {//110920
					for (int j=0; j<ixlen; j++) {
						if (iIncidentx[j] <= 0) {iStrip[j] = 0; continue;}
						int iSample = iuStrip[j] - iDark[j];
						if (iSample < iSINOGRAM_PIXEL_MIN) {
							iStrip[j] = 0; 
							continue;
						}
						iStrip[j] = (short)(iIncidentx[j] * ZERNIKE_SCALE / iSample);//ZERNIKE_SCALE=1000 assumes min transmittance of 0.03.
					}
				} else {
					for (int j=0; j<ixlen; j++) {
						if (iIncidentx[j] <= 0) {iStrip[j] = 0; continue;}
						int iSample = iuStrip[j] - iDark[j];
						if (iSample < iSINOGRAM_PIXEL_MIN) {
							//CString msg; msg.Format("121030 %d %d %d %d", j, iSample, iStrip[j], iDark[j]); AfxMessageBox(msg); 
							iStrip[j] = 0; 
							continue;
						}
						iStrip[j] = (short)(log((double)iIncidentx[j] / (double)iSample) * LOG_SCALE + 0.5);
					}
				}
				if (idrift > 0) {
					for (int j=ixlen-1; j>=0; j--) {
						if (j + idrift / iBinning >= ixlen) continue;
						else iStrip[j + idrift / iBinning] = iStrip[j];
					}
					for (int j=idrift / iBinning -1; j>=0; j--) {iStrip[j] = 0;}
				} else if (idrift < 0) {
					for (int j=0; j<ixlen; j++) {
						if (j + idrift / iBinning < 0) continue;
						else iStrip[j + idrift / iBinning] = iStrip[j];
					}
					for (int j=ixlen+idrift / iBinning; j<ixlen; j++) {iStrip[j] = 0;}
				}
			}//k
		}
		delete [] iIncidentx;
	}
	//Commented out because of the reason above.
	//090902===>
	//} else if (pApp->dlgProperty.m_ProcessorType == CDLGPROPERTY_PROCTYPE_CUDA) {
	//	//-----CUDA body-----//
	//	CudaSinogramHost(ri, ri->iStartSino, (pApp->dlgProperty.m_EnReport)? TRUE : FALSE);
	//}
	//===>090902
	if (ri->iStatus == RECONST_INFO_ERROR) return 0;
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

unsigned __stdcall RefracCorrThread(void* pArg) {
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) {
		//_endthreadex( 0 );
		return 0;
	}
	const CGazoApp* pApp = (CGazoApp*) AfxGetApp();
	CGazoDoc* pd = (CGazoDoc*)(ri->pDoc);
	CFile fimg;
	CString fn = "";
	TErr err = 0;
	//
	const int ideg = (int)(log10((double)(ri->iLenSinogr))) + 1;
	char* bInc = ri->bInc;
	float* fexp = ri->fexp;
	REFRAC_QUEUE* refq = (REFRAC_QUEUE*)((void*)(ri->fFilter));
	CString* fname = (CString*)((void*)(ri->nSinogr));
	int* iDark = ri->iReconst;
	////
	const int ndimxp = (int)((log((double)(refq->iXdim)) / LOG2)) + 1;
	const int ndimx = (int) pow((double)2, ndimxp);
	const int ndimyp = (int)((log((double)(refq->iYdim)) / LOG2)) + 1;
	const int ndimy = (int) pow((double)2, ndimyp);
	CCmplx* cPixel;
	if ((cPixel = new CCmplx[ndimx * ndimy]) == NULL) {
		ri->iStatus = RECONST_INFO_ERROR; 
		return 0;
	}
	//
	int* iSample = NULL; int maxSample = 0; int imgx, imgy;
	int* iIncident0 = NULL; int maxIncident0 = 0; int ixIncident0, iyIncident0;
	int* iIncident1 = NULL; int maxIncident1 = 0; int ixIncident1, iyIncident1;
	int iInc0pos = -1, iInc1pos = -1;
	for (int is=(ri->iStartSino); is<(ri->iLenSinogr)-1; is+=(ri->iStepSino)) {
		if (ri->bMaster) {
			::ProcessMessage();
			if (pd->dlgRefraction.m_hWnd)	{
				fn = "Processing "+ refq->outFilePrefix + fname[is].Right(ideg);
				pd->dlgRefraction.GetDlgItem(IDC_REFR_STATUS)->SetWindowText(fn);
			}
			if (pd->dlgRefraction.iStatus == CDLGREFRAC_STOP) {
				if (pd->dlgRefraction.m_hWnd)	{
					pd->dlgRefraction.GetDlgItem(IDC_REFR_STATUS)->SetWindowText("Abort");
				}
				ri->iStatus = RECONST_INFO_ERROR; 
				break;
			}
		} else {
			if (pd->dlgRefraction.iStatus == CDLGREFRAC_STOP) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;
			} else if (pd->dlgRefraction.iStatus & CDLGREFRAC_PAUSE) {
				while (pd->dlgRefraction.iStatus & CDLGREFRAC_BUSY) {
					::ProcessMessage();
					if (!(pd->dlgRefraction.iStatus & CDLGREFRAC_PAUSE)) {
						//pd->dlgRefraction.GetDlgItem(IDOK)->SetWindowText("Pause");
						break;
					}
				}
			}
		}
		CString comment = "Null";
		//read sample
		fn = refq->dataPath + refq->itexFilePrefix + fname[is].Right(ideg) + ".img";
		if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
			ri->iStatus = RECONST_INFO_ERROR; 
			break;
		} else {
			err = ReadITEX(&fimg, &iSample, &maxSample, &imgy, &imgx, &comment);
			fimg.Close();
		}
		if (err||(imgx > refq->iXdim)||(imgy > refq->iYdim)) {
			ri->iStatus = RECONST_INFO_ERROR; 
			break;
		}
		//when incident image
		if ((bInc[is] & CGAZODOC_BINC_SAMPLE) == 0) {
			for (int i=0; i<imgx*imgy; i++) {iSample[i] = 16384;}
			fn = refq->dataPath + refq->outFilePrefix + fname[is].Right(ideg) + ".img";
			if (fimg.Open(fn, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite)) {
				err = WriteITEX(&fimg, iSample, imgy, imgx, comment, 0, 0, 2);
				fimg.Close();
				if (err) {
					ri->iStatus = RECONST_INFO_ERROR; 
					break;//is loop
				}
			}
			continue;
		}
		//when sample image
		//read incidents
		if ((is <= iInc0pos)||(is >= iInc1pos)) {
			//find incident
			iInc0pos = -1; iInc1pos = -1;
			for (int i=is; i<(ri->iLenSinogr)-1; i++) {
				if ((bInc[i] & CGAZODOC_BINC_SAMPLE) == 0) {iInc1pos = i; break;}
			}
			for (int i=is; i>=0; i--) {
				if ((bInc[i] & CGAZODOC_BINC_SAMPLE) == 0) {iInc0pos = i; break;}
			}
			bool bIncErr = false;
			if (iInc0pos < 0) {
				if (iInc1pos >= 0) iInc0pos = iInc1pos;
				else bIncErr = true;
			}
			if (iInc1pos < 0) {
				if (iInc0pos >= 0) iInc1pos = iInc0pos;
				else bIncErr = true;
			}
			if (bIncErr) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;//is loop
			}
			err = 0;
			fn = refq->dataPath + refq->itexFilePrefix + fname[iInc0pos].Right(ideg) + ".img";
			if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;
			} else {
				err += ReadITEX(&fimg, &iIncident0, &maxIncident0, &iyIncident0, &ixIncident0);
				fimg.Close();
			}
			fn = refq->dataPath + refq->itexFilePrefix + fname[iInc1pos].Right(ideg) + ".img";
			//line += fn + "\r\n";
			if (!fimg.Open(fn, CFile::modeRead | CFile::shareDenyWrite)) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;
			} else {
				err += ReadITEX(&fimg, &iIncident1, &maxIncident1, &iyIncident1, &ixIncident1);
				fimg.Close();
			}
			if (err) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;//is loop
			}
		}
		//
		const double fexp0 = fexp[iInc0pos];
		const double fexp1 = fexp[iInc1pos];
		double fd = 0;
		if (fexp0 != fexp1) fd = (fexp[is] - fexp0) / (fexp1 - fexp0);
		//
		for (int i=0; i<ndimx*ndimy; i++) {cPixel[i].Reset();}
		for (int i=0; i<imgx; i++) {
			for (int j=0; j<imgy; j++) {
				const int idx = i+j*imgx;
				double dIncident = iIncident0[idx] + fd * (iIncident1[idx] - iIncident0[idx]);
				double dSample = (iSample[idx] - iDark[idx]) * 16384;
				if ((dSample < 0)||(dIncident < 2)) cPixel[i+j*ndimx].re = 0;
				else cPixel[i+j*ndimx].re = (float)(dSample / dIncident);
			}
		}
		CFft fft2;
		fft2.Init2(ndimxp, -1, ndimyp, -1);
		//
		//RefracCorr(refq, &fft2, cPixel);
		fft2.FFT2Rev(cPixel);
		const double mu = refq->dLAC;
		const double deltaz0 = refq->ndz0;
		const double psize = refq->dPixelWidth * 0.0001;//cm
		const double px2 = psize * psize * ndimx * ndimx;//cm2
		const double py2 = psize * psize * ndimy * ndimy;//cm2
		for (int i=0; i<ndimx; i++) {
			int ni = i;
			if (ni > ndimx/2) ni = ndimx - i;
			const double pni = ni * ni / px2;
			for (int j=0; j<ndimy; j++) {
				int nj = j;
				if (nj > ndimy/2) nj = ndimy - j;
				cPixel[i + j*ndimx] *= (float)(mu / (deltaz0 * (pni + nj * nj / py2) + mu));
			}
		}
		fft2.FFT2(cPixel);
		//
		for (int i=0; i<imgx; i++) {
			for (int j=0; j<imgy; j++) {
				iSample[i+j*imgx] = (int)cPixel[i+j*ndimx].re;
			}
		}
		fn = refq->dataPath + refq->outFilePrefix + fname[is].Right(ideg) + ".img";
		if (fimg.Open(fn, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite)) {
			err = WriteITEX(&fimg, iSample, imgy, imgx, comment, 0, 0, 2);
			fimg.Close();
			//if (err) AfxMessageBox("ERROR in averaging. File output:\r\n " + fn);
			if (err) {
				ri->iStatus = RECONST_INFO_ERROR; 
				break;//is loop
			}
		}
	}//is
	//
	if (iSample) delete [] iSample;
	if (iIncident0) delete [] iIncident0;
	if (iIncident1) delete [] iIncident1;
	if (cPixel) delete [] cPixel;
	////
	if (ri->iStatus == RECONST_INFO_ERROR) return 0;
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

unsigned __stdcall LsqfitThread(void* pArg) {
	RECONST_INFO* ri = (RECONST_INFO*)pArg;
	if (!ri) return 0;
	//
	const int ix = ri->ixdim;
	const int iy = ri->iInterpolation;
	const int ixref = ri->iLenSinogr;
	const int iyref = ri->iMultiplex;
	const int ixqry = ri->iOffset;
	const int iyqry = ri->maxSinogrLen;
	short** ppRefPixel = ri->ppRef;
	short** ppQryPixel = ri->ppQry;
	ri->i64result = 0;//__int64
	ri->i64sum = 0;//__int64
	const int nRefFiles = ri->max_d_ifp;
	const int nQryFiles = ri->max_d_igp;
	const int iz = ri->drEnd;
	//
	for (int jrz=0; jrz<nRefFiles; jrz++) {
		const int jqz = jrz + iz;
		if ((jqz < 0)||(jqz >= nQryFiles)) continue;
		short* pRef = ppRefPixel[jrz];
		short* pQry = ppQryPixel[jqz];
		for (int jry=0; jry<iyref; jry++) {
			const int jqy = jry + iy;
			if ((jqy < 0)||(jqy >= iyqry)) continue;
			int idx0r = jry * ixref;
			int idx0q = jqy * ixqry;
			for (int jrx=0; jrx<ixref; jrx++) {
				const int jqx = jrx + ix;
				if ((jqx < 0)||(jqx >= ixqry)) continue;
				if (pRef[idx0r + jrx] == SHRT_MIN) continue;
				if (pQry[idx0q + jqx] == SHRT_MIN) continue;
				__int64 idiff = (int)(pRef[idx0r + jrx]) - (int)(pQry[idx0q + jqx]);
				(ri->i64result) += idiff * idiff;
				(ri->i64sum)++;
			}
		}
	}
	//
	ri->iStatus = RECONST_INFO_IDLE;
	return 0;
}

TErr InvMatrix(TReal* a, int n, TReal eps) {
	int i, j; TReal aw, am;
	if (eps < 0) return 10082;
	if (n == 0) return 10082;
	else if (n == 1) {a[0] = 1.0 / a[0]; return 0;}
	else if (n == 2) {
		am = a[0] * a[3] - a[1] * a[2];
		if (fabs(am) < eps) return 10081;
		aw = a[0];
		a[0] = a[3] / am; a[1] = -a[1] / am; a[2] = -a[2] / am; a[3] = aw / am;
		return 0;
	} else if (n < 0) return 10083;
	//TReal* wk; TReal* wk2; int* ip;
	TReal* wk = new TReal[n];//) == NULL) return 10080;
	TReal* wk2 = new TReal[n];//) == NULL) return 10080;
	int* ip = new int[n];//) == NULL) return 10080;
	if (!wk || !wk2 || !ip) {
		if (wk) delete [] wk; if (wk2) delete [] wk2; if (ip) delete [] ip;
		return 10080;
	}
	for (i=0; i<n; i++) {wk[i] = 0.;}
	for (j=0; j<n; j++) {
		for (i=0; i<n; i++) {if (fabs(a[i*n+j]) > wk[i]) wk[i] = fabs(a[i*n+j]);}
	}
	for (i=0; i<n; i++) {if (wk[i]) wk[i] = 1. / wk[i];}
	for (int k=0; k<n; k++) {
		int kp = k;
		//am = max of abs(a[i][k]), kp = index of max a[i][k]
		am = fabs(a[k*n+k]) * wk[k];
		/*selecting partial pivot (comment out 050429)
		if (k != (n-1)) {
			for (i=k+1; i<n; i++) {
				aw = fabs(a[i*n+k]) * wk[i]; 
				if (aw > am) {kp = i; am = aw;}
			}
		}/**/
		if (am < eps) {
			if (wk) delete [] wk; if (wk2) delete [] wk2; if (ip) delete [] ip;
			//CString line; line.Format("%f", am); AfxMessageBox(line);//151014
			return 10081;
		}
		ip[k] = kp;
		// replace diagonal
		if (kp != k) {
			aw = a[kp*n+k]; a[kp*n+k] = a[k*n+k]; a[k*n+k] = aw;
			aw = wk[kp]; wk[kp] = wk[k]; wk[k] = aw;
		}
		a[k*n+k] = 1. / a[k*n+k];
		if (k != (n-1)) {
			//a[i][k] = a[i][k] / a[k][k]
			TReal pivot = a[k*n+k];
			for (i=k+1; i<n; i++) {a[i*n+k] *= pivot;}
			for (j=k+1; j<n; j++) {
				if (kp != k) {aw = a[kp*n+j]; a[kp*n+j] = a[k*n+j]; a[k*n+j] = aw;}
				aw = -a[k*n+j];
				for (i=k+1; i<n; i++) {a[i*n+j] += aw * a[i*n+k];}
			}
		}
	}// k
	for (int k=n-1; k>=0; k--) {
		for (j=k+1; j<n; j++) {wk[j] = 0.;}
		for (i=k+1; i<n; i++) {
			for (j=k+1; j<n; j++) {wk[j] -= a[i*n+k] * a[j*n+i];}
			wk2[i] = a[k*n+i];
		}
		for (j=k+1; j<n; j++) {a[j*n+k] = wk[j];}
		TReal s = 0.0;
		for (i=k+1; i<n; i++) {s += wk[i] * wk2[i];}
		TReal dg = a[k*n+k];
		a[k*n+k] = (1.0 - s) * dg;
		for (j=k+1; j<n; j++) {
			s = 0.0;
			for (i=k+1; i<n; i++) {s += wk2[i] * a[i*n+j];}
			a[k*n+j] = -s * dg;
		}
	}
	//CString msg = "";
	for (int k=n-2; k>=0; k--) {
		if (ip[k] != k) {
			//CString line; line.Format("ip[%d] = %d\n", k, ip[k]); msg += line;
			for (j=0; j<n; j++) {wk[j] = a[j*n+ip[k]];}
			for (j=0; j<n; j++) {a[j*n+ip[k]] = a[j*n+k]; a[j*n+k] = wk[j];}
			//if (k) {
				//for (j=k; j<n; j++) {wk[j] = a[ip[k]*n+j];}
				//for (j=k; j<n; j++) {a[ip[k]*n+j] = a[k*n+j]; a[k*n+j] = wk[j];}
				//for (j=k; j<n; j++) {wk[j] = a[j*n+ip[k]];}
				//for (j=k; j<n; j++) {a[j*n+ip[k]] = a[j*n+k]; a[j*n+k] = wk[j];}
				//for (j=0; j<n; j++) {wk[j] = a[j*n+ip[k]];}
				//for (j=0; j<n; j++) {a[j*n+ip[k]] = a[j*n+k]; a[j*n+k] = wk[j];}
			//} else {
				//for (j=0; j<n; j++) {wk[j] = a[j*n+ip[k]];}
				//for (j=0; j<n; j++) {a[j*n+ip[k]] = a[j*n+k]; a[j*n+k] = wk[j];}
			//}
		}
	}
	//AfxMessageBox(msg);
	if (wk) delete [] wk; if (wk2) delete [] wk2; if (ip) delete [] ip;
	return 0;
}

TErr ProjTransformGetCoeff(TReal* prPoint, TReal* prCoeff) {
	if ((prPoint == NULL)||(prCoeff == NULL)) return 10094;
	//input: point-by-point matching
	//(prPoint[0], prPoint[1]) ==> (prPoint[8], prPoint[9])
	//(prPoint[2], prPoint[3]) ==> (prPoint[10], prPoint[11])
	//(prPoint[4], prPoint[5]) ==> (prPoint[12], prPoint[13])
	//(prPoint[6], prPoint[7]) ==> (prPoint[14], prPoint[15])
	//
	//output: transform equation
	//u = (prCoeff[0]*x + prCoeff[1]*y + prCoeff[2]) / (prCoeff[6]*x + prCoeff[7]*y + 1)
	//v = (prCoeff[3]*x + prCoeff[4]*y + prCoeff[5]) / (prCoeff[6]*x + prCoeff[7]*y + 1)

	TErr err = 0;
	const TReal x1 = prPoint[0];
	const TReal y1 = prPoint[1];
	const TReal x2 = prPoint[2];
	const TReal y2 = prPoint[3];
	const TReal x3 = prPoint[4];
	const TReal y3 = prPoint[5];
	const TReal x4 = prPoint[6];
	const TReal y4 = prPoint[7];
	const TReal u1 = prPoint[8];
	const TReal v1 = prPoint[9];
	const TReal u2 = prPoint[10];
	const TReal v2 = prPoint[11];
	const TReal u3 = prPoint[12];
	const TReal v3 = prPoint[13];
	const TReal u4 = prPoint[14];
	const TReal v4 = prPoint[15];

	TReal p0 = (y2-y1)*(x3-x1) - (y3-y1)*(x2-x1);
	TReal q0 = (y2-y1)*(x4-x1) - (y4-y1)*(x2-x1);
	TReal p1 = (x3*u3 - x1*u1)*(x2-x1) - (x2*u2 - x1*u1)*(x3-x1);
	TReal q1 = (x4*u4 - x1*u1)*(x2-x1) - (x2*u2 - x1*u1)*(x4-x1);
	TReal p2 = (y3*u3 - y1*u1)*(x2-x1) - (y2*u2 - y1*u1)*(x3-x1);
	TReal q2 = (y4*u4 - y1*u1)*(x2-x1) - (y2*u2 - y1*u1)*(x4-x1);
	TReal p3 = (u2-u1)*(x3-x1) - (u3-u1)*(x2-x1);
	TReal q3 = (u2-u1)*(x4-x1) - (u4-u1)*(x2-x1);
	TReal r0 = p0;
	TReal s0 = q0;
	TReal r1 = (x3*v3 - x1*v1)*(x2-x1) - (x2*v2 - x1*v1)*(x3-x1);
	TReal s1 = (x4*v4 - x1*v1)*(x2-x1) - (x2*v2 - x1*v1)*(x4-x1);
	TReal r2 = (y3*v3 - y1*v1)*(x2-x1) - (y2*v2 - y1*v1)*(x3-x1);
	TReal s2 = (y4*v4 - y1*v1)*(x2-x1) - (y2*v2 - y1*v1)*(x4-x1);
	TReal r3 = (v2-v1)*(x3-x1) - (v3-v1)*(x2-x1);
	TReal s3 = (v2-v1)*(x4-x1) - (v4-v1)*(x2-x1);
	TReal m[4];
	m[0] = p1*q0 - q1*p0; m[1] = p2*q0 - q2*p0; m[2] = r1*s0 - s1*r0; m[3] = r2*s0 - s2*r0;
	if (err = InvMatrix(m, 2, 1E-6)) return err;
	TReal g = m[0] * (p3*q0 - q3*p0) + m[1] * (r3*s0 - s3*r0);
	TReal h = m[2] * (p3*q0 - q3*p0) + m[3] * (r3*s0 - s3*r0);
	TReal b = 0, e = 0;
	if (fabs(p0) > 1E-10) {
		b = (p3 - g*p1 - h*p2) / p0;
		e = (r3 - g*r1 - h*r2) / r0;
	} else if (fabs(q0) > 1E-10) {
		b = (q3 - g*q1 - h*q2) / q0;
		e = (s3 - g*s1 - h*s2) / s0;
	} else {
		return 10091;
	}
	TReal a = 0, d = 0;
	if (fabs(x2-x1) > 1E-10) {
		a = (u2-u1 - b*(y2-y1) + g*(x2*u2-x1*u1) + h*(y2*u2-y1*u1)) / (x2-x1);
		d = (v2-v1 - e*(y2-y1) + g*(x2*v2-x1*v1) + h*(y2*v2-y1*v1)) / (x2-x1);
	} else if (fabs(x3-x1) > 1E-10) {
		a = (u3-u1 - b*(y3-y1) + g*(x3*u3-x1*u1) + h*(y3*u3-y1*u1)) / (x3-x1);
		d = (v3-v1 - e*(y3-y1) + g*(x3*v3-x1*v1) + h*(y3*v3-y1*v1)) / (x3-x1);
	} else if (fabs(x4-x1) > 1E-10) {
		a = (u4-u1 - b*(y4-y1) + g*(x4*u4-x1*u1) + h*(y4*u4-y1*u1)) / (x4-x1);
		d = (v4-v1 - e*(y4-y1) + g*(x4*v4-x1*v1) + h*(y4*v4-y1*v1)) / (x4-x1);
	} else {
		return 10092;
	}
	TReal c = u1 - a*x1 - b*y1 + g*x1*u1 + h*y1*u1;
	TReal f = v1 - d*x1 - e*y1 + g*x1*v1 + h*y1*v1;

	prCoeff[0] = a;
	prCoeff[1] = b;
	prCoeff[2] = c;
	prCoeff[3] = d;
	prCoeff[4] = e;
	prCoeff[5] = f;
	prCoeff[6] = g;
	prCoeff[7] = h;
	return err;
}

TErr GetFileList(CString sFilter, CString* psFileList) {
	WIN32_FIND_DATA win32fd;
	HANDLE hFindFile = FindFirstFile(sFilter, &win32fd);
	if (hFindFile == INVALID_HANDLE_VALUE) return 16073101;
	do {
		if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			*psFileList += win32fd.cFileName;
			*psFileList += "\r\n";
		}
	} while (FindNextFile(hFindFile, &win32fd));
	FindClose(hFindFile);
	return 0;
}

#endif // _GENERAL_CPP_
