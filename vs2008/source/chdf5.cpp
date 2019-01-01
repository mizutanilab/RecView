/*	chdf5.cpp		v0.01	5/15/2016
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"
/// added for precompiled header definition 

#if !defined( _CHDF5_CPP_ )
#define _CHDF5_CPP_

#include "chdf5.h"

CHDF5::CHDF5() {
	Init();
}

void CHDF5::Init() {
	m_pFile = NULL;
	m_lBufStart = -1;
	m_iBufferLength = 0;
	m_iSzOffset = 8; m_iSzLength = 8; m_iLeafNodeK = 4; m_iIntNodeK = 16;
	m_iIndexedStorageIntNodeK = 100;
	m_lBaseAddr = 0; m_lGlobalFreeSpaceIdxAddr = -1; m_lEOFAddr = 0; m_lDriverInfoBlockAddr = -1;
	m_lBTreeAddr = 0; m_lNameHeapAddr = 0; m_lObjHeaderAddr = 0;
	m_lChildBTreeAddr = 0; m_lChildNameHeapAddr = 0; m_lChildObjHeaderAddr = 0; m_iChildEntry = 0;
	m_ucDataDim = 0;
	for (int i=0; i<CHDF5_MAXDIM; i++) {
		m_plDataSize[i] = 0; m_plDataMaxSize[i] = 0; m_puiChunkDimSize[i] = 0;
	}
	m_lRawDataAddress = 0; m_lRawDataSize = 0;
	m_uiDataElementSize = 0;
	m_uiDataElementType = CHDF5_DATAELEMTYPE_NONE;
	m_uiChunkDatasetElemSize = 0;
	m_lChunkDataAddress = 0;
	m_sChildTitle.Empty();
	m_uiSecondsAfterUnixEpoch = 0;
}

CHDF5::~CHDF5() {
	//if (pcBuffer) delete [] pcBuffer;
}

void CHDF5::SetFile(CFile* pfile) {
	Init();
	m_pFile = pfile;
}

int CHDF5::LoadBuf(LONGLONG lpos) {
	if (m_pFile == NULL) return 16051500;
	lpos = m_pFile->Seek(lpos, CFile::begin);
	m_iBufferLength = m_pFile->Read(m_pcBuffer, sizeof(char) * CHDF5_BUFSIZE);
//	if (m_pFile->Read(m_pcBuffer, sizeof(char) * CHDF5_BUFSIZE) != sizeof(char) * CHDF5_BUFSIZE)
//		{AfxMessageBox("16051501"); return 16051501;}
	m_lBufStart = lpos;
	return 0;
}

unsigned char* CHDF5::GetPointer(LONGLONG lpos, unsigned int ibyte) {
	if ((m_lBufStart < 0)||(lpos + ibyte > m_lBufStart + CHDF5_BUFSIZE)||(lpos < m_lBufStart)) {
		if (LoadBuf(lpos)) return NULL;
	}
	if (lpos - m_lBufStart >= m_iBufferLength) return NULL;
	return &(m_pcBuffer[lpos - m_lBufStart]);
}

__int64 CHDF5::GetOffsetParam(unsigned char* pcPnt, __int64 lpos) {
	switch (m_iSzOffset) {
		case 1: {return *((unsigned char*)(&(pcPnt[lpos])));}
		case 2: {return *((unsigned short*)(&(pcPnt[lpos])));}
		case 4: {return *((unsigned int*)(&(pcPnt[lpos])));}
		case 8: {return *((__int64*)(&(pcPnt[lpos])));}
		default: {return 0;}
	}
	return 0;
}

__int64 CHDF5::GetLengthParam(unsigned char* pcPnt, __int64 lpos) {
	switch (m_iSzLength) {
		case 1: {return *((unsigned char*)(&(pcPnt[lpos])));}
		case 2: {return *((unsigned short*)(&(pcPnt[lpos])));}
		case 4: {return *((unsigned int*)(&(pcPnt[lpos])));}
		case 8: {return *((__int64*)(&(pcPnt[lpos])));}
		default: {return 0;}
	}
	return 0;
}

void CHDF5::Dump(CString* psMsg) {
#define HDF5_buffersize 10000
#define HDF5_dataperline 20
//	unsigned char buffer[HDF5_buffersize];
//	if (fhdf5.Read(buffer, sizeof(char) * HDF5_buffersize) != sizeof(char) * HDF5_buffersize) return;
//	unsigned char* pcDump = GetPointer(0, HDF5_buffersize);
	unsigned char* pcDump = GetPointer(236780, HDF5_buffersize);
	CString msg = "", line;
	for (int i=0; i<HDF5_buffersize; i++) {
		if (i % HDF5_dataperline == 0) {line.Format("%4d: ", i); msg += line;}
		line.Format("%3d ", pcDump[i]); msg += line;
		if (i % HDF5_dataperline == HDF5_dataperline-1) {
			for (int j=i-(HDF5_dataperline-1); j<=i; j++) {
				if ((pcDump[j] >= 0x20)&&(pcDump[j] < 0x80)) line.Format("%c",pcDump[j]);
				else line = "_";
				msg += line;
			}
			msg += "\r\n";
		}
	}
	if (psMsg) *psMsg = msg;

//	fname = "output.txt";
//	CFileDialog fileDlg2(TRUE, defaultExt, fname, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
//	if (fileDlg2.DoModal() == IDCANCEL) return;
//	fname = fileDlg2.GetPathName();
//	CStdioFile fout;
//	if (!fout.Open(fname, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) return;
//	fout.WriteString(msg);
//	fout.Close();
}

int CHDF5::ReadSuperBlock(CString* psMsg) {
	if (psMsg) {*psMsg += "[ReadSuperBlock]\r\n";}
	//
	unsigned char* pcPnt = GetPointer(0, 96);
	CString line;
	if (pcPnt == NULL) return 16051502;
	if ((pcPnt[0] == 137)&&(pcPnt[1] == 72)&&(pcPnt[2] == 68)&&(pcPnt[3] == 70)&&(pcPnt[4] == 13)&&
		(pcPnt[5] == 10)&&(pcPnt[6] == 26)&&(pcPnt[7] == 10)&&
		(pcPnt[9] == 0)&&(pcPnt[10] == 0)&&(pcPnt[11] == 0)&&(pcPnt[12] == 0)&&(pcPnt[15] == 0)) {
			if (psMsg) {line.Format("Version= %d\r\n", pcPnt[8]); *psMsg += line;}
			if (pcPnt[8] >= 2) return 16051504;//not version 0/1;
			m_iSzOffset = pcPnt[13];
			m_iSzLength = pcPnt[14];
			m_iLeafNodeK = *((unsigned short*)(&(pcPnt[16])));
			m_iIntNodeK = *((unsigned short*)(&(pcPnt[18])));
			__int64 lpos = 24;
			if (pcPnt[8] == 1) {//version 1
				m_iIndexedStorageIntNodeK = *((unsigned short*)(&(pcPnt[lpos])));
				lpos += 4;
			}
			m_lBaseAddr = GetOffsetParam(pcPnt, lpos);
			lpos += m_iSzOffset;
			m_lGlobalFreeSpaceIdxAddr = GetOffsetParam(pcPnt, lpos);
			lpos += m_iSzOffset;
			m_lEOFAddr = GetOffsetParam(pcPnt, lpos);
			lpos += m_iSzOffset;
			m_lDriverInfoBlockAddr = GetOffsetParam(pcPnt, lpos);
			lpos += m_iSzOffset;
			//symbol table entry
//			__int64 lpos = 56;
			__int64 lLinkNameOffset = GetOffsetParam(pcPnt, lpos);
			lpos += m_iSzOffset;
			m_lObjHeaderAddr = GetOffsetParam(pcPnt, lpos) + m_lBaseAddr;
			lpos += m_iSzOffset;
			int iCacheType = *((unsigned int*)(&(pcPnt[lpos])));
			lpos += (4 + 4);
			if (psMsg) {
				line.Format("OffsetParamSize= %d\r\n", m_iSzOffset); *psMsg += line;
				line.Format("LengthParamSize= %d\r\n", m_iSzLength); *psMsg += line;
				line.Format("LeafNodeK= %d\r\n", m_iLeafNodeK); *psMsg += line;
				line.Format("IntNodeK= %d\r\n", m_iIntNodeK); *psMsg += line;
				line.Format("BaseAddr= %lld\r\n", m_lBaseAddr); *psMsg += line;
				line.Format("GlobalFreeSpaceIdxAddr= %lld\r\n", m_lGlobalFreeSpaceIdxAddr); *psMsg += line;
				line.Format("EOFAddr= %lld\r\n", m_lEOFAddr); *psMsg += line;
				line.Format("DriverInfoBlockAddr= %lld\r\n", m_lDriverInfoBlockAddr); *psMsg += line;
				line.Format("LinkNameOffset= %lld\r\n", lLinkNameOffset); *psMsg += line;
				line.Format("ObjHeaderAddr= %lld\r\n", m_lObjHeaderAddr); *psMsg += line;
				line.Format("CacheType= %d\r\n", iCacheType); *psMsg += line;
			}
			if (iCacheType == 1) {
				m_lBTreeAddr = GetOffsetParam(pcPnt, lpos) + m_lBaseAddr;
				lpos += m_iSzOffset;
				m_lNameHeapAddr =  GetOffsetParam(pcPnt, lpos) + m_lBaseAddr;
				if (psMsg) {
					line.Format("BTreeAddr= %lld\r\n", m_lBTreeAddr); *psMsg += line;
					line.Format("NameHeapAddr= %lld\r\n", m_lNameHeapAddr); *psMsg += line;
				}
			} else {
				return 16051505;
			}
			return 0;
	}
	return 16051503;
}

int CHDF5::FindChildSymbol(CString sSymbol, int iEntry, CString* psMsg) {
	if (psMsg) {*psMsg += "[FindChildSymbol]\r\n";}
	char cNodeType = 0, cNodeLevel = 0;
	int iEntriesUsed = 0;
	__int64 lLeftSiblingAddr = 0, lRightSiblingAddr = 0;
	int ierr = GetBTreeNodeParams(m_lBTreeAddr, &cNodeType, &cNodeLevel, &iEntriesUsed,
									&lLeftSiblingAddr, &lRightSiblingAddr, psMsg);
	if (ierr) return ierr;
	if ((cNodeType != 0)||(cNodeLevel != 0)) return 16051511;

	CString line;
	__int64 lpos = 8 + m_iSzOffset * 2;
	int imaxbyte = 24 + (2*m_iLeafNodeK+1) * 8;
	for (int i=0; i<2*m_iLeafNodeK; i++) {
		if (i >= iEntriesUsed) {line += "No symbol matched\r\n"; return 16052211;}
		unsigned char* pcPnt = GetPointer(m_lBTreeAddr, imaxbyte);
		if (pcPnt == NULL) return 16052212;
		__int64 lByteOffsetOfLocalHeap = GetLengthParam(pcPnt, lpos);
		lpos += m_iSzLength;
		__int64 lChildPointer = GetOffsetParam(pcPnt, lpos);
		lpos += m_iSzOffset;
		if (psMsg) {
			line.Format("ByteOffsetOfLocalHeap= %d\r\n", lByteOffsetOfLocalHeap); *psMsg += line;
			line.Format("ChildPointer= %d\r\n", lChildPointer); *psMsg += line;
		}
		if (lpos > imaxbyte) {line += "Out of tree childs\r\n"; break;}
//		int iEntry = -1;
		__int64 lSymTblObjHeaderAddr = 0;
		__int64 lSymTblBTreeAddr = 0;
		__int64 lSymTblNameHeapAddr = 0;
		int ierr = GetSymbolTable(lChildPointer, &sSymbol, &iEntry, 
						  &lSymTblObjHeaderAddr, &lSymTblBTreeAddr, &lSymTblNameHeapAddr,
						  psMsg);
		if (psMsg) {line.Format("EntryNumber= %d\r\n", iEntry); *psMsg += line;}
		if (ierr) continue;
		if (iEntry >= 0) {
			m_lChildObjHeaderAddr = lSymTblObjHeaderAddr;// + lByteOffsetOfLocalHeap;
			m_lChildBTreeAddr = lSymTblBTreeAddr;
			m_lChildNameHeapAddr = lSymTblNameHeapAddr;// + lByteOffsetOfLocalHeap;
			m_iChildEntry = iEntry;
			m_sChildTitle = sSymbol;
			break;
		}
	}
	return 0;
}

void CHDF5::MoveToChildTree() {
	m_lBTreeAddr = m_lChildBTreeAddr;
	m_lNameHeapAddr = m_lChildNameHeapAddr; 
	m_lObjHeaderAddr = m_lChildObjHeaderAddr;
	m_lChildBTreeAddr = 0; m_lChildNameHeapAddr = 0; m_lChildObjHeaderAddr = 0;
	m_iChildEntry = 0; m_sChildTitle.Empty();
}

int CHDF5::GetDataObjHeader(CString* psMsg) {
	if (psMsg) {*psMsg += "[GetDataObjHeader]\r\n";}
	unsigned int uimaxbyte = 256;
	unsigned char* pcPnt = GetPointer(m_lChildObjHeaderAddr, uimaxbyte);
	if (pcPnt == NULL) return 16052212;
	__int64 lpos = 0;
	char cVersion = pcPnt[lpos++];
	CString line;
	if (psMsg) {line.Format("Version= %d\r\n", cVersion); *psMsg += line;}
	if (cVersion != 1) return 16052101;
	if (pcPnt[lpos++] != 0) return 16052102;//reserved byte
	unsigned int uiTotalHeaderMsg = *((unsigned short*)(&(pcPnt[lpos])));
	lpos += 2;
	unsigned int uiObjRefCount = *((unsigned int*)(&(pcPnt[lpos])));
	lpos += 4;
	unsigned int uiObjHeaderSize = *((unsigned int*)(&(pcPnt[lpos])));
	lpos += 4;
	if (psMsg) {
		line.Format("TotalHeaderMsg= %d\r\n", uiTotalHeaderMsg); *psMsg += line;
		line.Format("ObjRefCount= %d\r\n", uiObjRefCount); *psMsg += line;
		line.Format("ObjHeaderSize= %d\r\n", uiObjHeaderSize); *psMsg += line;
	}
	if (uiObjHeaderSize > uimaxbyte) {
		pcPnt = GetPointer(m_lChildObjHeaderAddr, uiObjHeaderSize);
		if (pcPnt == NULL) return 16052212;
	}
	int ierr = 0;
	for (unsigned int i=0; i<uiTotalHeaderMsg; i++) {
		unsigned int uiHeaderMsgType = *((unsigned short*)(&(pcPnt[lpos])));
		lpos += 2;
		unsigned int uiHeaderMsgDataSize = *((unsigned short*)(&(pcPnt[lpos])));
		lpos += 2;
		if (psMsg) {
			line.Format("HeaderMsgType= %d\r\n", uiHeaderMsgType); *psMsg += line;
			line.Format("HeaderMsgDataSize= %d\r\n", uiHeaderMsgDataSize); *psMsg += line;
		}
		switch (uiHeaderMsgType) {
			case 0: {
				break;}
			case 1: {
				unsigned char ucHeaderMsgFlag = pcPnt[lpos];
				if (psMsg) {line.Format(" HeaderMsgFlag= %d\r\n", ucHeaderMsgFlag); *psMsg += line;}
				lpos += 4;
				__int64 lposdata = lpos;
				unsigned char ucDataSpaceVer = pcPnt[lpos++];
				m_ucDataDim = pcPnt[lpos++];
				unsigned char ucDataSpaceFlag = pcPnt[lpos++];
				if (psMsg) {
					line.Format(" DataSpaceVer= %d\r\n", ucDataSpaceVer); *psMsg += line;
					line.Format(" DataSpaceDim= %d\r\n", m_ucDataDim); *psMsg += line;
					line.Format(" DataSpaceFlag= %d\r\n", ucDataSpaceFlag); *psMsg += line;
				}
				if (ucDataSpaceVer != 1) {ierr = 16052103; break;}
				for (int j=0; j<CHDF5_MAXDIM; j++) {
					m_plDataSize[j] = 0; m_plDataMaxSize[j] = 0;
				}
				lpos += 5;
				for (unsigned int j=0; j<m_ucDataDim; j++) {
					m_plDataSize[j] = GetLengthParam(pcPnt, lpos);
					lpos += m_iSzLength;
					if (psMsg) {line.Format(" Dim%dSize= %lld\r\n", j, m_plDataSize[j]); *psMsg += line;}
				}
				if (ucDataSpaceFlag & 0x01) {
					for (unsigned int j=0; j<m_ucDataDim; j++) {
						m_plDataMaxSize[j] = GetLengthParam(pcPnt, lpos);
						lpos += m_iSzLength;
						if (psMsg) {line.Format(" Dim%dMax= %lld\r\n", j, m_plDataMaxSize[j]); *psMsg += line;}
					}
				}
				if (ucDataSpaceFlag & 0x02) {
					*psMsg += "Permutation detected\r\n";
					ierr = 16052104; break;
				}
				//
				lpos = lposdata + uiHeaderMsgDataSize;
				break;}
			case 3: {
				unsigned char ucHeaderMsgFlag = pcPnt[lpos];
				if (psMsg) {line.Format(" HeaderMsgFlag= %d\r\n", ucHeaderMsgFlag); *psMsg += line;}
				lpos += 4;
				__int64 lposdata = lpos;
				unsigned char ucClassVersion = pcPnt[lpos++];
				unsigned char ucClassBitField0 = pcPnt[lpos++];
				unsigned char ucClassBitField8 = pcPnt[lpos++];
				unsigned char ucClassBitField16 = pcPnt[lpos++];
				m_uiDataElementSize = *((unsigned int*)(&(pcPnt[lpos])));
				lpos += 4;
				if (ucClassVersion == 17) {
					unsigned int uiBitOffset = *((unsigned short*)(&(pcPnt[lpos])));
					lpos += 2;
					unsigned int uiBitPrecision = *((unsigned short*)(&(pcPnt[lpos])));
					lpos += 2;
					unsigned char ucExpLocation = pcPnt[lpos++];
					unsigned char ucExpSize = pcPnt[lpos++];
					unsigned char ucMantissaLocation = pcPnt[lpos++];
					unsigned char ucMantissaSize = pcPnt[lpos++];
					unsigned int uiExpBias = *((unsigned int*)(&(pcPnt[lpos])));
					lpos += 4;
					if (psMsg) {
						line.Format(" ClassVersion= %d (floating)\r\n", ucClassVersion); *psMsg += line;
						line.Format(" ClassBitField0= %d\r\n", ucClassBitField0); *psMsg += line;
						line.Format(" ClassBitField8= %d\r\n", ucClassBitField8); *psMsg += line;
						line.Format(" ClassBitField16= %d\r\n", ucClassBitField16); *psMsg += line;
						line.Format(" DataElementSize= %d\r\n", m_uiDataElementSize); *psMsg += line;
						line.Format(" BitOffset= %d\r\n", uiBitOffset); *psMsg += line;
						line.Format(" BitPrecision= %d\r\n", uiBitPrecision); *psMsg += line;
						line.Format(" ExpLocation= %d\r\n", ucExpLocation); *psMsg += line;
						line.Format(" ExpSize= %d\r\n", ucExpSize); *psMsg += line;
						line.Format(" MantissaLocation= %d\r\n", ucMantissaLocation); *psMsg += line;
						line.Format(" MantissaSize= %d\r\n", ucMantissaSize); *psMsg += line;
						line.Format(" ExpBias= %d\r\n", uiExpBias); *psMsg += line;
					}
					if (m_uiDataElementSize == 4) {
						if ((ucClassBitField0 == 32)&&(ucClassBitField8 == 31)&&
							(ucClassBitField16 == 0)&&(uiBitOffset == 0)&&(uiBitPrecision == 32)&&
							(ucExpLocation == 23)&&(ucExpSize == 8)&&
							(ucMantissaLocation == 0)&&(ucMantissaSize == 23)&&(uiExpBias == 127)) {
								m_uiDataElementType = CHDF5_DATAELEMTYPE_FLOAT;
						} else {
							ierr = 16052122; break;
						}
					} else if (m_uiDataElementSize == 8) {
						if ((ucClassBitField0 == 32)&&(ucClassBitField8 == 63)&&
							(ucClassBitField16 == 0)&&(uiBitOffset == 0)&&(uiBitPrecision == 64)&&
							(ucExpLocation == 52)&&(ucExpSize == 11)&&
							(ucMantissaLocation == 0)&&(ucMantissaSize == 52)&&(uiExpBias == 1023)) {
								m_uiDataElementType = CHDF5_DATAELEMTYPE_DOUBLE;
						} else {
							ierr = 16052123; break;
						}
					} else {
						ierr = 16061601; break;
					}
				} else if (ucClassVersion == 16) {
					unsigned int uiBitOffset = *((unsigned short*)(&(pcPnt[lpos])));
					lpos += 2;
					unsigned int uiBitPrecision = *((unsigned short*)(&(pcPnt[lpos])));
					lpos += 2;
					if (psMsg) {
						line.Format(" ClassVersion= %d (fixed)\r\n", ucClassVersion); *psMsg += line;
						line.Format(" ClassBitField0= %d\r\n", ucClassBitField0); *psMsg += line;
						line.Format(" ClassBitField8= %d\r\n", ucClassBitField8); *psMsg += line;
						line.Format(" ClassBitField16= %d\r\n", ucClassBitField16); *psMsg += line;
						line.Format(" DataElementSize= %d\r\n", m_uiDataElementSize); *psMsg += line;
						line.Format(" BitOffset= %d\r\n", uiBitOffset); *psMsg += line;
						line.Format(" BitPrecision= %d\r\n", uiBitPrecision); *psMsg += line;
					}
					if (m_uiDataElementSize == 2) {
						if ((ucClassBitField0 == 0)&&(ucClassBitField8 == 0)&&
							(ucClassBitField16 == 0)&&(uiBitOffset == 0)&&(uiBitPrecision == 16)) {
								m_uiDataElementType = CHDF5_DATAELEMTYPE_SHORT;
						} else {
							ierr = 16061602; break;
						}
					} else {
						ierr = 16061601; break;
					}
				} else {//if (ucClassVersion)
					ierr = 16052121; break;
				}//if (ucClassVersion)
				//
				lpos = lposdata + uiHeaderMsgDataSize;
				break;}
			case 8: {
				unsigned char ucHeaderMsgFlag = pcPnt[lpos];
				if (psMsg) {line.Format(" HeaderMsgFlag= %d\r\n", ucHeaderMsgFlag); *psMsg += line;}
				lpos += 4;
				__int64 lposdata = lpos;
				unsigned char ucVersion = pcPnt[lpos++];
				if (psMsg) {line.Format(" Version= %d\r\n", ucVersion); *psMsg += line;}
				if (ucVersion != 3) {ierr = 16052110; break;}
				unsigned char ucLayoutClass = pcPnt[lpos++];
				if (psMsg) {line.Format(" LayoutClass= %d\r\n", ucLayoutClass); *psMsg += line;}
				if (ucLayoutClass == 1) {//contiguous
					m_lRawDataAddress = GetOffsetParam(pcPnt, lpos);
					lpos += m_iSzOffset;
					m_lRawDataSize = GetLengthParam(pcPnt, lpos);
					lpos += m_iSzLength;
					if (psMsg) {
						line.Format(" RawDataAddress= %lld\r\n", m_lRawDataAddress); *psMsg += line;
						line.Format(" RawDataSize= %lld\r\n", m_lRawDataSize); *psMsg += line;
					}
				} else if (ucLayoutClass == 2) {//chunked
					unsigned char ucDimension = pcPnt[lpos++];
					m_lChunkDataAddress = GetOffsetParam(pcPnt, lpos);
					m_lRawDataAddress = 0;
					lpos += m_iSzOffset;
					if (psMsg) {
						line.Format(" ChunkDimensionality= %d\r\n", ucDimension); *psMsg += line;
						line.Format(" ChunkDataAddres= %lld\r\n", m_lChunkDataAddress); *psMsg += line;
					}
					if (ucDimension > CHDF5_MAXDIM) {ierr = 16061621; break;}
					for (int j=0; j<(int)ucDimension; j++) {
						m_puiChunkDimSize[j] = *((unsigned int*)(&(pcPnt[lpos])));
						lpos += 4;
						if (psMsg) {
							line.Format(" ChunkDimSize%d= %lld\r\n", j, m_puiChunkDimSize[j]); *psMsg += line;
						}
					}
					m_uiChunkDatasetElemSize = *((unsigned int*)(&(pcPnt[lpos])));
					lpos += 4;
					if (psMsg) {
						line.Format(" ChunkDatasetElemSize= %lld\r\n", m_uiChunkDatasetElemSize); *psMsg += line;
					}
				} else {//if (ucLayoutClass)
					ierr = 16052111; break;
				}
				lpos = lposdata + uiHeaderMsgDataSize;
				break;}
			case 16: {
				unsigned char ucHeaderMsgFlag = pcPnt[lpos];
				if (psMsg) {line.Format(" HeaderMsgFlag= %d\r\n", ucHeaderMsgFlag); *psMsg += line;}
				lpos += 4;
				__int64 lContinuationMsgOffset = GetOffsetParam(pcPnt, lpos);
				lpos += m_iSzOffset;
				__int64 lContinuationMsgLength = GetLengthParam(pcPnt, lpos);
				lpos += m_iSzLength;
				if (psMsg) {
					line.Format(" ContinuationMsgOffset= %lld\r\n", lContinuationMsgOffset); *psMsg += line;
					line.Format(" ContinuationMsgLength= %lld\r\n", lContinuationMsgLength); *psMsg += line;
				}
				pcPnt = GetPointer(lContinuationMsgOffset, (unsigned int)lContinuationMsgLength);
				if (pcPnt == NULL) {ierr = 16052212; break;}
				lpos = 0;
				break;}
			case 18: {
				unsigned char ucHeaderMsgFlag = pcPnt[lpos];
				if (psMsg) {line.Format(" HeaderMsgFlag= %d\r\n", ucHeaderMsgFlag); *psMsg += line;}
				lpos += 4;
				unsigned char ucVersion = pcPnt[lpos];
				if (psMsg) {line.Format(" Version= %d\r\n", ucVersion); *psMsg += line;}
				if (ucVersion != 1) {ierr = 16062811; break;}
				lpos += 4;
				m_uiSecondsAfterUnixEpoch = *((unsigned int*)(&(pcPnt[lpos])));
				if (psMsg) {line.Format(" SecondsAfterUnixEpoch= %lld\r\n", m_uiSecondsAfterUnixEpoch); *psMsg += line;}
				lpos += 4;
				break;}
			default: {
				lpos += (4 + uiHeaderMsgDataSize);
				break;}
		}
		if (ierr) break;
	}
	return ierr;
}

int CHDF5::ReadFrame(unsigned int uiFrame, int* piData, CString* psMsg) {
	if (psMsg) {*psMsg += "[ReadFrame]\r\n";}
	CString line;
	if (psMsg) {
		line.Format("Dimension= %d\r\n", m_ucDataDim); *psMsg += line;
		line.Format("DataSize= %lld x %lld x %lld\r\n", 
			m_plDataSize[2], m_plDataSize[1], m_plDataSize[0]); *psMsg += line;
		line.Format("Frame= %d\r\n", uiFrame); *psMsg += line;
		if (m_uiDataElementType == CHDF5_DATAELEMTYPE_FLOAT) {*psMsg += "DataFormat= float\r\n";}
		else if (m_uiDataElementType == CHDF5_DATAELEMTYPE_DOUBLE) {*psMsg += "DataFormat= double\r\n";}
		else if (m_uiDataElementType == CHDF5_DATAELEMTYPE_SHORT) {*psMsg += "DataFormat= short\r\n";}
		else {*psMsg += "DataFormat= unknown\r\n"; return 16052204;}
	}
	if (m_ucDataDim != 3) return 16052201;
	if (piData == NULL) return 16052202;
	if (m_plDataSize[2] > CHDF5_BUFSIZE) return 16052203;
	if (uiFrame >= (unsigned)m_plDataSize[0]) return 16052205;
	int idx = 0;
	if (m_lRawDataAddress) {
		const __int64 laddr = m_lRawDataAddress + uiFrame * m_plDataSize[2] * m_plDataSize[1] * m_uiDataElementSize;
		for (int i=0; i<m_plDataSize[1]; i++) {
			unsigned char* pcPnt = GetPointer(laddr + m_plDataSize[2] * i * m_uiDataElementSize, 
												(unsigned int)m_plDataSize[2] * m_uiDataElementSize);
			if (pcPnt == NULL) return 16052206;
			__int64 lpos = 0;
			switch (m_uiDataElementType) {
				case CHDF5_DATAELEMTYPE_FLOAT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						piData[idx++] = (int)(*((float*)(&(pcPnt[lpos]))));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_DOUBLE: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						piData[idx++] = (int)*((double*)(&(pcPnt[lpos])));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_SHORT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						piData[idx++] = (int)*((short*)(&(pcPnt[lpos])));
						lpos += m_uiDataElementSize;
					}
					break;}
				default: {
					return 16061611;}
			}
		}
	} else if (m_lChunkDataAddress) {
		if (psMsg) {*psMsg += "[ReadChunkData]\r\n";}
		//m_lBTreeAddr = m_lChunkDataAddress;
		char cNodeType = 0, cNodeLevel = 0;
		int iEntriesUsed = 0;
		__int64 lLeftSiblingAddr = 0, lRightSiblingAddr = 0;
		int ierr = GetBTreeNodeParams(m_lChunkDataAddress, &cNodeType, &cNodeLevel, &iEntriesUsed,
										&lLeftSiblingAddr, &lRightSiblingAddr, psMsg);
		if (ierr) return ierr;
		if ((cNodeType != 1)||(cNodeLevel != 0)) return 16061632;
		if (uiFrame >= (unsigned int)iEntriesUsed) return 16061633;
		int imaxbyte = 8 + m_iSzOffset * 2 + iEntriesUsed * (8 + (m_ucDataDim+1) * 8 + 8);
		unsigned char* pcPnt = GetPointer(m_lChunkDataAddress, imaxbyte);
		if (pcPnt == NULL) return 16061634;
		__int64 lpos = 8 + m_iSzOffset * 2;
		__int64 lChildAddrPointer = 0;
		__int64 plidx[CHDF5_MAXDIM];
		for (int i=0; i<iEntriesUsed; i++) {
			unsigned int uiSizeOfChunk = *((unsigned int*)(&(pcPnt[lpos])));
			lpos += 4;
			unsigned int uiFilterMask = *((unsigned int*)(&(pcPnt[lpos])));
			lpos += 4;
			for (int j=0; j<m_ucDataDim+1; j++) {
				plidx[j] =  *((__int64*)(&(pcPnt[lpos])));
				lpos += 8;
			}
			lChildAddrPointer = *((__int64*)(&(pcPnt[lpos])));
			lpos += 8;
			if (plidx[0] == uiFrame) {
				if (psMsg) {
					line.Format("SizeOfChunk= %d\r\n", uiSizeOfChunk); *psMsg += line;
					line.Format("FilterMask= %d\r\n", uiFilterMask); *psMsg += line;
					for (int j=0; j<m_ucDataDim+1; j++) {
						line.Format("Dim%d= %lld\r\n", j, plidx[j]); *psMsg += line;
					}
					line.Format("ChildAddrPointer= %lld\r\n", lChildAddrPointer); *psMsg += line;
				}
				break;
			}
		}
		if (lChildAddrPointer) {
			for (int i=0; i<m_plDataSize[1]; i++) {
				unsigned char* pcPnt = GetPointer(lChildAddrPointer + m_plDataSize[2] * i * m_uiDataElementSize, 
													(unsigned int)m_plDataSize[2] * m_uiDataElementSize);
				if (pcPnt == NULL) return 16052206;
				__int64 lpos = 0;
				switch (m_uiDataElementType) {
					case CHDF5_DATAELEMTYPE_FLOAT: {
						for (int j=0; j<m_plDataSize[2]; j++) {
							piData[idx++] = (int)(*((float*)(&(pcPnt[lpos]))));
							lpos += m_uiDataElementSize;
						}
						break;}
					case CHDF5_DATAELEMTYPE_DOUBLE: {
						for (int j=0; j<m_plDataSize[2]; j++) {
							piData[idx++] = (int)*((double*)(&(pcPnt[lpos])));
							lpos += m_uiDataElementSize;
						}
						break;}
					case CHDF5_DATAELEMTYPE_SHORT: {
						for (int j=0; j<m_plDataSize[2]; j++) {
							piData[idx++] = (int)*((short*)(&(pcPnt[lpos])));
							lpos += m_uiDataElementSize;
						}
						break;}
					default: {
						return 16061611;}
				}
			}
		}//if (lChildAddrPointer)
	} else return 16061631;
	return 0;
}

TErr CHDF5::ReadStrip(short* psBuf, unsigned int uiFrame, int iLine, int iMultiplex) {
	//
	TErr ierr = 0;
	if (m_ucDataDim != 3) return 16052711;
	if (psBuf == NULL) return 16052712;
	if (m_plDataSize[2] > CHDF5_BUFSIZE) return 16052713;
	if (m_plDataSize[1] < iLine + iMultiplex) {
		//CString line; line.Format("DataSize1=%lld < %d + %d", m_plDataSize[1], iLine, iMultiplex); AfxMessageBox(line);
		return 16052713;
	}
	if (uiFrame >= (unsigned)m_plDataSize[0]) return 16052715;
	int idx = 0;
	if (m_lRawDataAddress) {
		const __int64 laddr = m_lRawDataAddress + uiFrame * m_plDataSize[2] * m_plDataSize[1] * m_uiDataElementSize;
		for (int i=iLine; i<iLine+iMultiplex; i++) {
			unsigned char* pcPnt = GetPointer(laddr + m_plDataSize[2] * i * m_uiDataElementSize, 
												(unsigned int)m_plDataSize[2] * m_uiDataElementSize);
			if (pcPnt == NULL) return 16052216;
			__int64 lpos = 0;
			switch (m_uiDataElementType) {
				case CHDF5_DATAELEMTYPE_FLOAT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = (short)(*((float*)(&(pcPnt[lpos]))));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_DOUBLE: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = (short)(*((double*)(&(pcPnt[lpos]))));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_SHORT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = *((short*)(&(pcPnt[lpos])));
						lpos += m_uiDataElementSize;
					}
					break;}
				default: {
					ierr = 16061641;
					break;}
			}
		}
	} else if (m_lChunkDataAddress) {
		//m_lBTreeAddr = m_lChunkDataAddress;
		char cNodeType = 0, cNodeLevel = 0;
		int iEntriesUsed = 0;
		__int64 lLeftSiblingAddr = 0, lRightSiblingAddr = 0;
		ierr = GetBTreeNodeParams(m_lChunkDataAddress, &cNodeType, &cNodeLevel, &iEntriesUsed,
										&lLeftSiblingAddr, &lRightSiblingAddr, NULL);
		if (ierr) return ierr;
		if ((cNodeType != 1)||(cNodeLevel != 0)) return 16061702;
		if (uiFrame >= (unsigned int)iEntriesUsed) return 16061703;
		int imaxbyte = 8 + m_iSzOffset * 2 + iEntriesUsed * (8 + (m_ucDataDim+1) * 8 + 8);
		unsigned char* pcPnt = GetPointer(m_lChunkDataAddress, imaxbyte);
		if (pcPnt == NULL) return 16061705;
		__int64 lpos = 8 + m_iSzOffset * 2;
		__int64 lChildAddrPointer = 0;
		__int64 plidx[CHDF5_MAXDIM];
		unsigned int uiSizeOfChunk; unsigned int uiFilterMask;
		for (int i=0; i<iEntriesUsed; i++) {
			uiSizeOfChunk = *((unsigned int*)(&(pcPnt[lpos])));
			lpos += 4;
			uiFilterMask = *((unsigned int*)(&(pcPnt[lpos])));
			lpos += 4;
			for (int j=0; j<m_ucDataDim+1; j++) {
				plidx[j] =  *((__int64*)(&(pcPnt[lpos])));
				lpos += 8;
			}
			lChildAddrPointer = *((__int64*)(&(pcPnt[lpos])));
			lpos += 8;
			if (plidx[0] == uiFrame) break;
		}
		if (lChildAddrPointer == 0) return 16061701;
		for (int i=iLine; i<iLine+iMultiplex; i++) {
			unsigned char* pcPnt = GetPointer(lChildAddrPointer + m_plDataSize[2] * i * m_uiDataElementSize, 
												(unsigned int)m_plDataSize[2] * m_uiDataElementSize);
			if (pcPnt == NULL) return 16061704;
			lpos = 0;
			switch (m_uiDataElementType) {
				case CHDF5_DATAELEMTYPE_FLOAT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = (short)(*((float*)(&(pcPnt[lpos]))));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_DOUBLE: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = (short)(*((double*)(&(pcPnt[lpos]))));
						lpos += m_uiDataElementSize;
					}
					break;}
				case CHDF5_DATAELEMTYPE_SHORT: {
					for (int j=0; j<m_plDataSize[2]; j++) {
						psBuf[idx++] = *((short*)(&(pcPnt[lpos])));
						lpos += m_uiDataElementSize;
					}
					break;}
				default: {
					ierr = 16061641;
					break;}
			}//switch (m_uiDataElementType)
			if (ierr) break;
		}//for (int i=iLine; i<iLine+iMultiplex; i++)
	} else return 16061642;
	return ierr;
}


int CHDF5::ReadTheta(float* pfData, CString* psMsg) {
	if (psMsg) {*psMsg += "[ReadTheta]\r\n";}
	CString line;
	if (psMsg) {
		line.Format("Dimension= %d\r\n", m_ucDataDim); *psMsg += line;
		line.Format("DataSize= %lld x %lld x %lld\r\n", 
			m_plDataSize[2], m_plDataSize[1], m_plDataSize[0]); *psMsg += line;
		if (m_uiDataElementSize == 4) {*psMsg += "DataFormat= float\r\n";}
		else if (m_uiDataElementSize == 8) {*psMsg += "DataFormat= double\r\n";}
		else {*psMsg += "DataFormat= unknown\r\n"; return 16052624;}
	}
	if (m_ucDataDim != 1) return 16052621;
	if (pfData == NULL) return 16052622;
	if (m_plDataSize[0] > CHDF5_BUFSIZE) return 16052623;
	int idx = 0;
	unsigned char* pcPnt = GetPointer(m_lRawDataAddress, (unsigned int)m_plDataSize[0] * m_uiDataElementSize);
	if (pcPnt == NULL) return 16052206;
	__int64 lpos = 0;
	switch (m_uiDataElementSize) {
		case 4: {
			for (int j=0; j<m_plDataSize[0]; j++) {
				pfData[idx++] = *((float*)(&(pcPnt[lpos])));
				lpos += m_uiDataElementSize;
			}
			break;}
		case 8: {
			for (int j=0; j<m_plDataSize[0]; j++) {
				pfData[idx++] = (float)*((double*)(&(pcPnt[lpos])));
				lpos += m_uiDataElementSize;
			}
			break;}
	}
	return 0;
}

int CHDF5::GetBTreeNodeParams(__int64 lBTreeAddr, char* pcNodeType, char* pcNodeLevel, int* piEntriesUsed,
							__int64* plLeftSiblingAddr, __int64 *plRightSiblingAddr, CString* psMsg) {
	if (psMsg) {*psMsg += "[GetBTreeNodeData]\r\n";}
	CString line;
	__int64 lpos = 0;
	int imaxbyte = 24 + (2*m_iLeafNodeK+1) * 8;
	unsigned char* pcPnt = GetPointer(lBTreeAddr, imaxbyte);
	if (pcPnt == NULL) return 16052206;
	//TREE
	line.Format("%c%c%c%c", pcPnt[lpos], pcPnt[lpos+1], pcPnt[lpos+2], pcPnt[lpos+3]);
	if (psMsg) *psMsg += line + "\r\n";
	if (line != "TREE") return 16051510;
	lpos += 4;
	*pcNodeType = pcPnt[lpos];
	lpos++;
	*pcNodeLevel = pcPnt[lpos];
	lpos++;
	if (psMsg) {
		line.Format("NodeType= %d\r\n", *pcNodeType); *psMsg += line;
		line.Format("NodeLevel= %d\r\n", *pcNodeLevel); *psMsg += line;
	}
	//160616 if ((*pcNodeType != 0)||(*pcNodeLevel != 0)) return 16051511;

	*piEntriesUsed = *((unsigned short*)(&(pcPnt[lpos])));
	lpos += 2;
	*plLeftSiblingAddr = GetOffsetParam(pcPnt, lpos);
	lpos += m_iSzOffset;
	*plRightSiblingAddr = GetOffsetParam(pcPnt, lpos);
	lpos += m_iSzOffset;
	if (psMsg) {
		line.Format("EntriesUsed= %d\r\n", *piEntriesUsed); *psMsg += line;
		line.Format("LeftSiblingAddr= %d\r\n", *plLeftSiblingAddr); *psMsg += line;
		line.Format("RightSiblingAddr= %d\r\n", *plRightSiblingAddr); *psMsg += line;
	}
	return 0;
}

int CHDF5::GetSymbolTable(__int64 lChildPointer, CString* psSymbol, int* piEntry, 
						  __int64* plSymTblObjHeaderAddr,
						  __int64* plSymTblBTreeAddr, __int64* plSymTblNameHeapAddr,
						  CString* psMsg) {
	//when *piEntry<0 and psSymbol is empty ===> only returns number of entries in *piEntry
	//when *piEntry<0 and psSymbol is filled ==> returns name-matched entry
	//when *piEntry>=0                       ==> returns number-matched entry
	if (psMsg) *psMsg += "[GetSymbolTable]\r\n";
	CString line;
	//SNOD
	__int64 lchild = 0;
	unsigned char* pcChild = GetPointer(lChildPointer, 100);
	if (pcChild == NULL) return 16052206;
	line.Format("%c%c%c%c", pcChild[lchild], pcChild[lchild+1], pcChild[lchild+2], pcChild[lchild+3]);
	if (psMsg) *psMsg += line + "\r\n";
	if (line != "SNOD") return 16051521;
	lchild += 4;
	char cSymbolTableNodeVer = pcChild[lchild];
	if (psMsg) {
		line.Format("SymbolTableNodeVer= %d\r\n", cSymbolTableNodeVer); *psMsg += line;
	}
	if (cSymbolTableNodeVer != 1) return 16051522;
	lchild += (1 + 1);
	int iSymTblNumEntries = *((unsigned short*)(&(pcChild[lchild])));
	if (psMsg) {
		line.Format("SymTblNumEntries= %d\r\n", iSymTblNumEntries); *psMsg += line;
	}
	if (*piEntry < 0) {
		if (psSymbol->IsEmpty()) {*piEntry = iSymTblNumEntries; return 0;}
	}
	pcChild = GetPointer(lChildPointer, 8 + iSymTblNumEntries * (m_iSzOffset*2 + 24));
	if (pcChild == NULL) return 16052206;
	lchild += 2;
	bool bFound = false;
	for (int j=0; j<iSymTblNumEntries; j++) {
		if (psMsg) {line.Format("Entry %d\r\n", j); *psMsg += line;}
		__int64 lSymTblLinkNameOffset = GetOffsetParam(pcChild, lchild);
		lchild += m_iSzOffset;
		__int64 lSymTblObjHeaderAddr = GetOffsetParam(pcChild, lchild);
		lchild += m_iSzOffset;
		unsigned int uiSymTblCacheType = *((unsigned int*)(&(pcChild[lchild])));
		lchild += (4 + 4);
		__int64 lSymTblBTreeAddr = GetOffsetParam(pcChild, lchild);
		lchild += m_iSzOffset;
		__int64 lSymTblNameHeapAddr = GetOffsetParam(pcChild, lchild);
		lchild += m_iSzOffset;
		if (psMsg) {
			line.Format("SymTblLinkNameOffset= %lld\r\n", lSymTblLinkNameOffset); *psMsg += line;
			line.Format("SymTblObjHeaderAddr= %lld\r\n", lSymTblObjHeaderAddr); *psMsg += line;
			line.Format("SymTblCacheType= %d\r\n", uiSymTblCacheType); *psMsg += line;
			line.Format("SymTblBTreeAddr= %lld\r\n", lSymTblBTreeAddr); *psMsg += line;
			line.Format("SymTblNameHeapAddr= %lld\r\n", lSymTblNameHeapAddr); *psMsg += line;
		}
		CString sLinkName = GetStringFromLocalHeap(lSymTblLinkNameOffset, psMsg);
		if (*piEntry < 0) {//symbol to be checked
			if (sLinkName == *psSymbol) {
				*plSymTblObjHeaderAddr = lSymTblObjHeaderAddr;
				if (uiSymTblCacheType == 1) {
					*plSymTblBTreeAddr = lSymTblBTreeAddr;
					*plSymTblNameHeapAddr = lSymTblNameHeapAddr;
				}
				*piEntry = j;
				if (psMsg) {*psMsg += "--Symbol matched--\r\n";}
				bFound = true;
				break;
			}
		} else {//return *piEntry
			if (j == *piEntry) {
				*plSymTblObjHeaderAddr = lSymTblObjHeaderAddr;
				if (uiSymTblCacheType == 1) {
					*plSymTblBTreeAddr = lSymTblBTreeAddr;
					*plSymTblNameHeapAddr = lSymTblNameHeapAddr;
				}
				*psSymbol = sLinkName;
				if (psMsg) {*psMsg += "--Entry number matched--\r\n";}
				bFound = true;
				break;
			}
		}
	}//(j<iSymTblNumEntries)
	if (bFound) return 0;
	return 15062212;
}

CString CHDF5::GetStringFromLocalHeap(__int64 lLinkNameOffset, CString* psMsg) {
	if (psMsg) {*psMsg += "[GetStringFromLocalHeap]\r\n";}
	CString line;
	__int64 lpos = 0;
	int imaxbyte = 100;
	unsigned char* pcPnt = GetPointer(m_lNameHeapAddr, imaxbyte);
	if (pcPnt == NULL) return "";
	//HEAP
	line.Format("%c%c%c%c", pcPnt[lpos], pcPnt[lpos+1], pcPnt[lpos+2], pcPnt[lpos+3]);
	if (psMsg) *psMsg += line + "\r\n";
	if (line != "HEAP") return "";
	lpos += 4;
	char cLocalHeapVer = pcPnt[lpos];
	if (psMsg) {
		line.Format("LocalHeapVer= %d\r\n", cLocalHeapVer); *psMsg += line;
	}
	if (cLocalHeapVer != 0) return "";
	lpos += 4;
	__int64 lLocalHeapDataSegSize = GetLengthParam(pcPnt, lpos);
	lpos += m_iSzLength;
	__int64 lLocalHeapOffsetToFreeList = GetLengthParam(pcPnt, lpos);
	lpos += m_iSzLength;
	__int64 lLocalHeapDataSegAddr = GetOffsetParam(pcPnt, lpos);
	lpos += m_iSzOffset;
	if (psMsg) {
		line.Format("LocalHeapDataSegSize= %lld\r\n", lLocalHeapDataSegSize); *psMsg += line;
		line.Format("LocalHeapOffsetToFreeList= %lld\r\n", lLocalHeapOffsetToFreeList); *psMsg += line;
		line.Format("LocalHeapDataSegAddr= %lld\r\n", lLocalHeapDataSegAddr); *psMsg += line;
	}
	unsigned char* pcHeap = GetPointer(lLocalHeapDataSegAddr, (unsigned int)lLocalHeapDataSegSize);
	if (pcHeap) line.Format("%s", &(pcHeap[lLinkNameOffset]));
	if (psMsg) *psMsg += "String= " + line + "\r\n";
	return line;
}

void CHDF5::Debug() {
	/*
	CString msg = "", line;
	CString fname = "tooth.h5";
	static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
	static char BASED_CODE defaultExt[] = "h5";
	CFileDialog fileDlg(TRUE, defaultExt, fname, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg.DoModal() == IDCANCEL) {
		AfxMessageBox("No file speciifed");
		return;
	}
	fname = fileDlg.GetPathName();
	CFile fhdf5;
	if (!fhdf5.Open(fname, CFile::modeRead | CFile::shareDenyWrite)) {
			fhdf5.Close(); AfxMessageBox("file open error"); return;
	}
	CHDF5 hdf5;
	hdf5.SetFile(&fhdf5);
	int ierr = hdf5.ReadSuperBlock(&msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.FindChildSymbol("exchange", &msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	hdf5.MoveToChildTree();
	ierr = hdf5.FindChildSymbol("data", &msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.GetDataObjHeader(&msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.GetDataFloat(&msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.FindChildSymbol("data_dark", &msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.GetDataObjHeader(&msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	ierr = hdf5.GetDataFloat(&msg);
	if (ierr) {line.Format("Err %d", ierr); AfxMessageBox(line);}
	fhdf5.Close();

	fname = "msg.txt";
	CFileDialog fileDlg2(TRUE, defaultExt, fname, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
	if (fileDlg2.DoModal() == IDCANCEL) return;
	fname = fileDlg2.GetPathName();
	CStdioFile fout;
	if (!fout.Open(fname, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) return;
	fout.WriteString(msg);
	fout.Close();
	*/

//hdf5 dump
//#define HDF5_buffersize 10000
//#define HDF5_dataperline 20
//	unsigned char buffer[HDF5_buffersize];
//	if (fhdf5.Read(buffer, sizeof(char) * HDF5_buffersize) != sizeof(char) * HDF5_buffersize) return;
//	for (int i=0; i<HDF5_buffersize; i++) {
//		if (i % HDF5_dataperline == 0) {line.Format("%4d: ", i); msg += line;}
//		line.Format("%3d ", buffer[i]); msg += line;
//		if (i % HDF5_dataperline == HDF5_dataperline-1) {
//			for (int j=i-(HDF5_dataperline-1); j<=i; j++) {
//				if ((buffer[j] >= 0x20)&&(buffer[j] < 0x80)) line.Format("%c", buffer[j]);
//				else line = "_";
//				msg += line;
//			}
//			msg += "\r\n";
//		}
//	}
//
//	fname = "output.txt";
//	CFileDialog fileDlg2(TRUE, defaultExt, fname, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, NULL);
//	if (fileDlg2.DoModal() == IDCANCEL) return;
//	fname = fileDlg2.GetPathName();
//	CStdioFile fout;
//	if (!fout.Open(fname, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) return;
//	fout.WriteString(msg);
//	fout.Close();

}

#endif // _CHDF5_CPP_
