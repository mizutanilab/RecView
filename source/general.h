/*	general.h		v1.00	7/26/2016
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*   Moved from Stdafx.h
*/

#if !defined( _GENERAL_H_ )
#define _GENERAL_H_


//manually
#include <afx.h> // CString, LPCTSTR
#include "reconstinfo.h"
#include "constants.h"

typedef int TErr;// not 0 on error.
typedef double TReal;
typedef float TCmpElmnt;

class CGazoDoc;
class CHDF5;//160521

//HIS format
#define HIS_Header_Size 64
struct HIS_Header
{
	char			head[2];			/*0-1*/
	short			comment_length;		/*2-3*/
	short			width;				/*4-5*/
	short			height;				/*6-7*/
	short			x_offset;			/*8-9*/
	short			y_offset;			/*10-11*/
	short			type;				/*12-13*/
	unsigned short			n_image1;			/*14-17*/
	unsigned short			n_image2;			/*14-17*/
	short			reserve1;			/*18-19*/
	short			reserve2;			/*20-21*/
	double			time_stamp;			/*22-29*/
	long			maker;				/*30-33*/
	char			reserved[30];		/*34-63*/
	char			*comment;
};
typedef struct HIS_Header HISHeader;

#define READTIF16bit 1
#define READTIF8bit 2

//general functions
void ProcessMessage();
void SleepSecond(TReal sec = 1.0);
DWORD GetProcessorCoreCount();
void ConvertToLittleEndian(void* param, rsize_t bytes, char* carg);
COLORREF HSBtoRGB(double H, double S, double B);
TErr ReadBmp(CString filePath, int** buffer, int* pMaxBuffer, int* prevH, int* prevW,
	char* paletteBlue = NULL, char* paletteGreen = NULL, char* paletteRed = NULL, int maxPalette = 0);
TErr WriteBmpTrueColor(CString filePath, COLORREF* buffer, int height, int width);
TErr ReadTif(CFile* fp, int** buffer, int* pMaxBuffer, int* prevH, int* prevW,
						 float* pixDiv = NULL, float* pixBase = NULL, float* fCenter = NULL, int* iFilter = NULL, float* fPixelWidth = NULL,
						 int* nSino = NULL);
TErr ReadTifStrip(CFile* fp, short* sbuffer, int iLine, int iWidth, int iMultiplex, int* piFlag);
TErr WriteTifMonochrome(CFile* fp, unsigned char* buffer, int height, int width,
												CString sImageDesc = "tif image", CString sArtist = "SP-uCT, gazoView");
TErr WriteTifMonochrome16(CFile* fp, int* buffer, int height, int width,
												CString sImageDesc = "tif image", CString sArtist = "SP-uCT, gazoView");
TErr ReadITEX(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, CString* pComment = NULL);
TErr ReadITEXstrip(FILE* fp, short* buffer, int iLine, int iWidth, int iMultiplex = 1);
TErr WriteITEX(CFile* fp, int* buffer, int iHeight, int iWidth, CString comment, int iXoffset = 0, int iYoffset = 0, int iFileType = 2);
TErr Read_hishead(CFile* fimg, HISHeader* his, CString* pComment = NULL);
TErr ReadHIS(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, HISHeader* his, CString* pComment = NULL);
TErr SkipHISframe(CFile* fp, int nframe);
TErr SkipHISframeFast(CFile* fp, int nframe);
TErr ReadHISstrip(CFile* fp, unsigned char** uctmp, int* pMaxTmp, short* sbuffer, 
				  int iLine, int iWidth, int iMultiplex, HISHeader* his);
TErr ReadHDF5Frame(CFile* fp, int** buffer, int* pMaxBuffer, int* iHeight, int* iWidth, CHDF5* pHDF5, 
			  unsigned int uiFrame = 0, int iDataEntry = -1, CString* pComment = NULL);
TErr ReadHDF5Theta(CFile* fp, CHDF5* pHDF5, float* pfDeg, DWORD* pdwFrame, CString* pComment = NULL);
TErr GetFileList(CString sFilter, CString* psFileList);

//math
TErr InvMatrix(TReal* a, int n, TReal eps);
TErr ProjTransformGetCoeff(TReal* prPoint, TReal* prCoeff);

//thread functions
unsigned __stdcall DeconvBackProjThread(void* pArg);

unsigned __stdcall GenerateSinogramThread(void* pArg);

unsigned __stdcall RefracCorrThread(void* pArg);

unsigned __stdcall LsqfitThread(void* pArg);

#ifdef _WIN64
//void Projection(unsigned _int64 pParam);
//unsigned _int64 ProjectionAsm(unsigned _int64);
#endif //_WIN64

//misc functions
int StringCompare( const void *arg1, const void *arg2 );



#endif // _GENERAL_H_
