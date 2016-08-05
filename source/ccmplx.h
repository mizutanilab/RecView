/*	cerror.h		v1.10	2/23/2008
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "general.h"

#if !defined( _CCMPLX_H_ )
#define _CCMPLX_H_

class CCmplx {
public:
	CCmplx();
	~CCmplx();
	CCmplx(TReal arg1, TReal arg2);
	CCmplx operator +(CCmplx a);
	CCmplx operator -(CCmplx a);
	CCmplx operator *(TReal a);
	CCmplx operator /(TReal a);
	CCmplx operator *(CCmplx a);
	CCmplx operator =(CCmplx a);
	CCmplx operator +=(CCmplx a);
	CCmplx operator -=(CCmplx a);
	CCmplx operator *=(CCmplx a);
	CCmplx operator *=(TReal a);
	CCmplx operator *=(float a);
	CCmplx Exp(TReal a);
	CCmplx Rev();
	CCmplx Conj();
	TCmpElmnt Real();
	TCmpElmnt Imag();
	TCmpElmnt Modulus();
	TCmpElmnt Modulus2();
	void Reset();
	void AddToReal(TCmpElmnt a);
	TCmpElmnt re, im;
};

#endif // _CCMPLX_H_
