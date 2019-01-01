/*	cfft.h		v0.01	11/22/2006
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
	Subdevided from structure.h (v0.01 since 9/5/1998)
*/

#if !defined( _CFFT_H_ )
#define _CFFT_H_

//#include "stdtypdf.h"
#include "ccmplx.h"

class CFft {
public:
	CFft();
	~CFft();
	TErr Init(int narg0, int narg1, int narg2, int min0, int min1, int min2);
	TErr InitRev(int narg0, int narg1, int narg2, int min0, int min1, int min2, int sgnrot = 1);
	void FFT3(CCmplx* f);
	void FFT3Rev(CCmplx* f);
	TErr Init1(int narg0, int min0);
	void FFT1(CCmplx* f);
	void FFT1Rev(CCmplx* f);
	TErr Init2(int narg0, int min0, int narg1, int min1);
	void FFT2(CCmplx* f);
	void FFT2Rev(CCmplx* f);
	void Debug();
	void Debug2();
private:
	TErr InitParam(int narg0, int narg1, int narg2, int min0, int min1, int min2);
	void FFT(float* wr, float* wi, int n, int np, int nmin, CCmplx* f, int idxbs, int idxinc);
	void FFTrev(float* wr, float* wi, int n, int np, int nmin, CCmplx* f, int idxbs, int idxinc);
	float* w0r; float* w0i; float* w1r; float* w1i; float* w2r; float* w2i;
	float* x0r; float* x0i; float* x1r; float* x1i;
	int n0, n1, n2, np0, np1, np2;
	int nmax, nmin0, nmin1, nmin2;
	bool init;
	//CDlgRefine* dialog; CDlgMap* mapdialog; CDlgAnalysis* analdialog;
};

#endif // _CFFT_H_
