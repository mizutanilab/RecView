/*	cerror.cpp		v0.01	1/30/2000
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"
/// added for precompiled header definition 

#if !defined( _CERROR_CPP_ )
#define _CERROR_CPP_
#include <iostream>
#include "cerror.h"

CError::CError() {
	logLevel = 0;
	Clear();
}
CError::~CError() {}
void CError::Clear() {
	numOfErr = 0; 
	fatal = false; 
	ClearFlag();
	getpoint = 0;
	for (int i=0; i<MAX_ERROR; i++) {message[i] = ""; errCode[i] = -1;}
}
void CError::ClearFlag() {
	fatal = false; 
	asserted = false;
}
void CError::SetLogLevel(int arg) {logLevel = arg;}
bool CError::IsFatal() {return fatal;}
bool CError::IsAsserted() {return asserted;}
void CError::LogNoDup(TErr err, CString msg) {
	for (int i=0; i<numOfErr; i++) {if (errCode[i] == err) return;}
	Log(err, msg);
}
void CError::Log(TErr err, CString msg) {
	if (err <= logLevel) return;
	if (err > 0) asserted = true;
	if (err >= FATAL_ERROR_LEVEL) fatal = true;
	if (numOfErr >= MAX_ERROR) {numOfErr = 0; rotation = true;}
	errCode[numOfErr] = err;
	message[numOfErr++] = msg;
	return;
}
void CError::Log(CString msg) {this->Log(0, msg);}
/*080217
void CError::Log(TErr err, const CStdioFileRead& errfile, CString msg) {
	if (err <= logLevel) return;
	if (err > 0) asserted = true;
	if (err >= FATAL_ERROR_LEVEL) fatal = true;
	if (numOfErr >= MAX_ERROR) {numOfErr = 0; rotation = true;}
	errCode[numOfErr] = err;
	char s[50];
	sprintf(s, "%d", errfile.Lines());
	message[numOfErr++] = msg +
												" at line " + s +
												" of file " + errfile.GetFileName();
	return;
}*/

void CError::ResetPointer() {getpoint = 0;}
bool CError::GetMessage(int* code, CString* log) {
	if (getpoint >= MAX_ERROR) {*code = -1; return false;}
	int ip;
	if (rotation) {
		ip = (getpoint + numOfErr) % MAX_ERROR;
		*code = errCode[ip];
		*log = message[ip];
		getpoint++;
		return true;
	} else {
		ip = getpoint;
		*code = errCode[ip];
		*log = message[ip];
		getpoint++;
		if (*code < 0) return false; else return true;
	}
}

CString CError::Report() {
	CString rtn; CString line;
	if (fatal) rtn = "!!FATAL ERROR!!\r\n"; else rtn = ""; 
	if (rotation) {
		for (int i=numOfErr; i<MAX_ERROR; i++) {
			if (errCode[i] >= FATAL_ERROR_LEVEL) rtn += "Error ";
			else if (errCode[i] >= WARNING_LEVEL) rtn += "Warning ";
			if (errCode[i] >= WARNING_LEVEL) {
				line.Format("%d\r\n", errCode[i]); rtn += line;
			}
			if (!message[i].IsEmpty()) rtn += message[i] + "\r\n";
		}
	}
	for (int i=0; i<numOfErr; i++) {
		if (errCode[i] >= FATAL_ERROR_LEVEL) rtn += "Error ";
		else if (errCode[i] >= WARNING_LEVEL) rtn += "Warning ";
		if (errCode[i] >= WARNING_LEVEL) {
			line.Format("%d\r\n", errCode[i]); rtn += line;
		}
		if (!message[i].IsEmpty()) rtn += message[i] + "\r\n";
	}
	return rtn;
}

#endif // _CERROR_CPP_
