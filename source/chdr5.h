/*	chdr5.h		v0.01	5/15/2016
	Recon from APS data in HDR5 format
	Only work with limited type of HDR5 file
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/

#if !defined( _CHDR5_H_ )
#define _CHDR5_H_

//#include "stdtypdf.h"
#include "general.h"

#define CHDR5_BUFSIZE 262144
#define CHDR5_MAXDIM 4
#define CHDR5_DATAELEMTYPE_NONE 0
#define CHDR5_DATAELEMTYPE_FLOAT 1
#define CHDR5_DATAELEMTYPE_DOUBLE 2
#define CHDR5_DATAELEMTYPE_SHORT 3

class CHDR5 {
public:
	CHDR5();
	~CHDR5();
	void Init();
	void SetFile(CFile* pfile);
	int LoadBuf(LONGLONG lpos = 0);
	unsigned char* GetPointer(LONGLONG lpos, unsigned int ibyte);
	int ReadSuperBlock(CString* psMsg = NULL);
	int FindChildSymbol(CString sSymbol, int iEntry = -1, CString* psMsg = NULL);
	CString GetStringFromLocalHeap(__int64 lLinkNameOffset, CString* psMsg = NULL);
	void MoveToChildTree();
	int GetDataObjHeader(CString* psMsg = NULL);
	int ReadFrame(unsigned int uiFrame, int* piData, CString* psMsg = NULL);
	int ReadTheta(float* pfData, CString* psMsg = NULL);
	TErr ReadStrip(short* psBuf, unsigned int uiFrame, int iLine, int iMultiplex = 1);
	void Dump(CString* psMsg);
	void Debug();

	unsigned char m_ucDataDim;
	__int64 m_plDataSize[CHDR5_MAXDIM];
	__int64 m_plDataMaxSize[CHDR5_MAXDIM];
	__int64 m_puiChunkDimSize[CHDR5_MAXDIM];
	int m_iChildEntry;
	CString m_sChildTitle;
	unsigned __int64 m_uiSecondsAfterUnixEpoch;
private:
	__int64 GetOffsetParam(unsigned char* pcPnt, __int64 lpos);
	__int64 GetLengthParam(unsigned char* pcPnt, __int64 lpos);
	int GetBTreeNodeParams(__int64 lBTreeAddr, char* pcNodeType, char* pcNodeLevel, int* piEntriesUsed,
							__int64* plLeftSiblingAddr, __int64 *plRightSiblingAddr, CString* psMsg = NULL);
	int GetSymbolTable(__int64 lChildPointer, CString* psSymbol, int* piEntry, 
						  __int64* plSymTblObjHeaderAddr,
						  __int64* plSymTblBTreeAddr, __int64* plSymTblNameHeapAddr,
						  CString* psMsg = NULL);
	unsigned char m_pcBuffer[CHDR5_BUFSIZE];
	int m_iBufferLength;
	//unsigned int uinBuffer;
	LONGLONG m_lBufStart;
	CFile* m_pFile;
	int m_iSzOffset, m_iSzLength, m_iLeafNodeK, m_iIntNodeK;
	int m_iIndexedStorageIntNodeK;
	__int64 m_lBaseAddr, m_lGlobalFreeSpaceIdxAddr, m_lEOFAddr, m_lDriverInfoBlockAddr;
	__int64 m_lBTreeAddr, m_lNameHeapAddr, m_lObjHeaderAddr;
	__int64 m_lChildBTreeAddr, m_lChildNameHeapAddr, m_lChildObjHeaderAddr;
	__int64 m_lRawDataAddress, m_lRawDataSize;
	unsigned int m_uiDataElementSize;
	unsigned int m_uiDataElementType;
	unsigned int m_uiChunkDatasetElemSize;
	__int64 m_lChunkDataAddress;
};

#endif // _CHDR5_H_
