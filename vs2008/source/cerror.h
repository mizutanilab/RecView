/*	cerror.h		v0.01	1/30/2000
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#if !defined( _CERROR_H_ )
#define _CERROR_H_
#include <afx.h> // CString
//080217 #include "cstdiofileread.h"
#include "general.h"

#define FATAL_ERROR_LEVEL 10000
#define WARNING_LEVEL 100
#define TOO_MUCH_ERROR 19999
#define MAX_ERROR 1000
#define WARN_NOT_CONNECTED 9001
//160803
#define WARN_READIMAGE_SIZECHANGE 9002

class CError {
public:
	CError();
	~CError();
	void Log(TErr err, CString msg = "");
	void LogNoDup(TErr err, CString msg = "");
	//080217 void Log(TErr err, const CStdioFileRead& errfile, CString msg = "");
	void Log(CString msg);
	bool IsFatal();
	bool IsAsserted();
	//int IsFatal(int err, CString msg);
	//int IsFatal(int err, int line);
	//int IsFatal(int err);
	void ResetPointer();
	bool GetMessage(int* code, CString* log);
	CString Report();
	void Clear();
	void ClearFlag();
	void SetLogLevel(int arg);
private:
	bool asserted;
	bool fatal;
	bool rotation;
	int numOfErr, getpoint;
	int errCode[MAX_ERROR];
	CString message[MAX_ERROR];
	int logLevel;
};


#endif // _CERROR_H_

