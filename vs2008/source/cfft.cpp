/*	cfft.cpp		v0.01	6/20/2000
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"
/// added for precompiled header definition 

#if !defined( _CFFT_CPP_ )
#define _CFFT_CPP_
#include <math.h>
#include "cfft.h"
#include "ccmplx.h"
//#include "resource.h"
//#include "DlgRefine.h"
//#include "DlgMap.h"
//#include "DlgAnalysis.h"

CFft::CFft() {
	w0r = NULL; w0i = NULL; w1r = NULL; w1i = NULL; w2r = NULL; w2i = NULL;
	x0r = NULL; x0i = NULL; x1r = NULL; x1i = NULL;
	n0 = 0; n1 = 0; n2 = 0; np0 = 0; np1 = 0; np2 = 0;
	nmin0 = 0; nmin1 = 0; nmin2 = 0; 
	nmax = 0;
	init = false;
	//dialog = NULL; mapdialog = NULL; analdialog = NULL;
}
CFft::~CFft() {
	if (w0r) delete [] w0r; if (w0i) delete [] w0i;
	if (w1r) delete [] w1r; if (w1i) delete [] w1i;
	if (w2r) delete [] w2r; if (w2i) delete [] w2i;
	if (x0r) delete [] x0r; if (x0i) delete [] x0i;
	if (x1r) delete [] x1r; if (x1i) delete [] x1i;
}
//void CFft::SetDialogCtrl(CDlgRefine* dlg) {dialog = dlg;}
//void CFft::SetMapDialogCtrl(CDlgMap* dlg) {mapdialog = dlg;}
//void CFft::SetAnalDialogCtrl(CDlgAnalysis* dlg) {analdialog = dlg;}

TErr CFft::InitParam(int narg0, int narg1, int narg2,
								int min0, int min1, int min2) {
	if ((narg0 < 3)||(narg1 < 3)||(narg2 < 3)) return 15003;
	nmin0 = min0; 
	nmin1 = min1;
	nmin2 = min2;
	if (init) {
		if ((np0 ==  narg0)&&(np1 ==  narg1)&&(np2 ==  narg2)) return 0;//return 100;//050224
	}
	np0 = narg0;
	np1 = narg1;
	np2 = narg2;
	n0 = (int) pow((double)2, narg0); 
	n1 = (int) pow((double)2, narg1); 
	n2 = (int) pow((double)2, narg2);
	nmax = n0;
	nmax = nmax > n1 ? nmax : n1;
	nmax = nmax > n2 ? nmax : n2;
	if ( w0r ) {
		delete [] w0r; delete [] w0i; delete [] w1r; delete [] w1i; delete [] w2r; delete [] w2i;
		delete [] x0r; delete [] x0i; delete [] x1r; delete [] x1i;
	}
	if ((w0r = new float[n0 / 2]) == NULL) return 15002;
	if ((w1r = new float[n1 / 2]) == NULL) return 15002;
	if ((w2r = new float[n2 / 2]) == NULL) return 15002;
	if ((x0r = new float[nmax]) == NULL) return 15002;
	if ((x1r = new float[nmax]) == NULL) return 15002;
	if ((w0i = new float[n0 / 2]) == NULL) return 15002;
	if ((w1i = new float[n1 / 2]) == NULL) return 15002;
	if ((w2i = new float[n2 / 2]) == NULL) return 15002;
	if ((x0i = new float[nmax]) == NULL) return 15002;
	if ((x1i = new float[nmax]) == NULL) return 15002;
	return 0;
}
TErr CFft::Init(int narg0, int narg1, int narg2,
								int min0, int min1, int min2) {
	int err = this->InitParam(narg0, narg1, narg2, min0, min1, min2);
	if (err == 100) return 0;
	if (err != 0) return err;
	float a;
	for (int i=0; i<(n0 / 2); i++) {
		a = (float)__PI2 * ((float)i / (float)n0);
		w0r[i] = (float)cos(a); w0i[i] = (float)sin(a);
	}
	for (int i=0; i<(n1 / 2); i++) {
		a = (float)__PI2 * ((float)i / (float)n1);
		w1r[i] = (float)cos(a); w1i[i] = (float)sin(a);
	}
	for (int i=0; i<(n2 / 2); i++) {
		a = (float)__PI2 * ((float)i / (float)n2);
		w2r[i] = (float)cos(a); w2i[i] = (float)sin(a);
	}
	init = true; return 0;
}

TErr CFft::InitRev(int narg0, int narg1, int narg2,
								int min0, int min1, int min2, int sgnrot) {
	int err = this->InitParam(narg0, narg1, narg2, min0, min1, min2);
	float p0 = (float)__PI2 * sgnrot; float p1 = p0; float p2 = p0;
	if (nmin0 < 0) {nmin0 = - (n0 / 2) + 1; p0 *= -1;}
	if (nmin1 < 0) {nmin1 = - (n1 / 2) + 1; p1 *= -1;}
	if (nmin2 < 0) {nmin2 = - (n2 / 2) + 1; p2 *= -1;}
	if (err == 100) return 0;
	if (err != 0) return err;
	float a;
	for (int i=0; i<(n0 / 2); i++) {
		a = p0 * ((float)i / (float)n0);
		w0r[i] = (float)cos(a); w0i[i] = (float)sin(a);
//		w0[i].Exp(p0 * ((double)i / (double)n0));
	}
	for (int i=0; i<(n1 / 2); i++) {
		a = p1 * ((float)i / (float)n1);
		w1r[i] = (float)cos(a); w1i[i] = (float)sin(a);
//		w1[i].Exp(p1 * ((double)i / (double)n1));
	}
	for (int i=0; i<(n2 / 2); i++) {
		a = p2 * ((float)i / (float)n2);
		w2r[i] = (float)cos(a); w2i[i] = (float)sin(a);
//		w2[i].Exp(p2 * ((double)i / (double)n2));
	}
	init = true; return 0;
}

void CFft::FFT3(CCmplx* f) {//f(hkl) = f[l*kmax*hmax+k*hmax+h]???
	if (!(init)) return;
	int idx0; 
	const int n1n0 = n1 * n0;
	for (int i=0; i<n1; i++) {
		//if (dialog || analdialog) ProcessMessage();
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (analdialog) {if (analdialog->analStatus == STATUS_STOP) break;}
		idx0 = i * n0;
		for (int j=0; j<n2; j++) {
			this->FFT(w0r, w0i, n0, np0, nmin0, f, idx0, 1);
			idx0 += n1n0;
		}
	}
	for (int i=0; i<n0; i++) {
		//if (dialog || analdialog) ProcessMessage(); 
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (analdialog) {if (analdialog->analStatus == STATUS_STOP) break;}
		idx0 = i;
		for (int j=0; j<n2; j++) {
			this->FFT(w1r, w1i, n1, np1, nmin1, f, idx0, n0);
			idx0 += n1n0;
		}
	}
	for (int i=0; i<n0; i++) {
		//if (dialog || analdialog) ProcessMessage(); 
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (analdialog) {if (analdialog->analStatus == STATUS_STOP) break;}
		idx0 = i;
		for (int j=0; j<n1; j++) {
			this->FFT(w2r, w2i, n2, np2, nmin2, f, idx0, n1n0);
			idx0 += n0;
		}
	}
}

void CFft::FFT3Rev(CCmplx* f) {
	//f(hkl) = f[(l-nmin2)*kmax*hmax+(k-nmin1)*hmax+(h-nmin0)]???
	if (!(init)) return;
	int idx0; 
	const int n1n0 = n1 * n0;
	for (int i=0; i<n1; i++) {
		//if (dialog || mapdialog) ProcessMessage();
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (mapdialog) {if (mapdialog->mapStatus == STATUS_STOP) break;}
		idx0 = i * n0;
		for (int j=0; j<n2; j++) {
			this->FFTrev(w0r, w0i, n0, np0, nmin0, f, idx0, 1);
			idx0 += n1n0;
		}
	}
	for (int i=0; i<n0; i++) {
		//if (dialog || mapdialog) ProcessMessage(); 
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (mapdialog) {if (mapdialog->mapStatus == STATUS_STOP) break;}
		idx0 = i;
		for (int j=0; j<n2; j++) {
			this->FFTrev(w1r, w1i, n1, np1, nmin1, f, idx0, n0);
			idx0 += n1n0;
		}
	}
	for (int i=0; i<n0; i++) {
		//if (dialog || mapdialog) ProcessMessage(); 
		//if (dialog) {if (dialog->refnStatus == STATUS_STOP) break;}
		//if (mapdialog) {if (mapdialog->mapStatus == STATUS_STOP) break;}
		idx0 = i;
		for (int j=0; j<n1; j++) {
			this->FFTrev(w2r, w2i, n2, np2, nmin2, f, idx0, n1n0);
			idx0 += n0;
		}
	}//cout <<"fft"<<endl;return;
}

void CFft::FFTrev(float* wr, float* wi, int n, int np, int nmin,
								CCmplx* f, int idxbs, int idxinc) {
	int al = 1; int bt = n/2; float scrr, scri; int jal, jal2, abinc, idx;
	float* swpr; float* swpi;
//	CCmplx* swp;
	//l=0 //x1[j,k]=x[j*alpha*2+k]
	if (nmin < 0) {
		abinc = idxinc * bt; 
		idx = idxbs + abinc - idxinc; //idx=idxbs+idxinc*(bt-1)
		for (int j=0; j<bt; j++) {
																	//x0[j] = f[idxbs+(bt-1-j)*idxinc]
//				scr = f[idx + abinc];			//scr = x0[j+bt];
//				x1[j*2] = f[idx] + scr;		//x1[j*2] = x0[j] + scr;
//				x1[j*2+1] = f[idx] - scr;	//x1[j*2+1] = x0[j] - scr;
			scrr = (float)f[idx + abinc].re; scri = (float)f[idx + abinc].im;
			x1r[j*2] = (float)f[idx].re + scrr; x1i[j*2] = (float)f[idx].im + scri;
			x1r[j*2+1] = (float)f[idx].re - scrr; x1i[j*2+1] = (float)f[idx].im - scri;
			idx -= idxinc;
		}
	} else {
		idx = idxbs; abinc = idxinc * bt;
		for (int j=0; j<bt; j++) {
																	//x0[j] = f[idxbs + j * idxinc]
//				scr = f[idx + abinc];			//scr = x0[j+bt];
//				x1[j*2] = f[idx] + scr;		//x1[j*2] = x0[j] + scr;
//				x1[j*2+1] = f[idx] - scr;	//x1[j*2+1] = x0[j] - scr;
			scrr = (float)f[idx + abinc].re; scri = (float)f[idx + abinc].im;
			x1r[j*2] = (float)f[idx].re + scrr; x1i[j*2] = (float)f[idx].im + scri;
			x1r[j*2+1] = (float)f[idx].re - scrr; x1i[j*2+1] = (float)f[idx].im - scri;
			idx += idxinc;
		}
	}
	al *= 2; bt /= 2;
	abinc = n/2;
	int jal3, kbt;
	for (int l=1; l<(np-1); l++) {//x[j,k]=x[j*alpha + k]
//		swp = x1; x1 = x0; x0 = swp;
		swpr = x1r; x1r = x0r; x0r = swpr; swpi = x1i; x1i = x0i; x0i = swpi;
		for (int j=0; j<bt; j++) {
			jal = j * al; jal2 = jal * 2;
			for (int k=0; k<abinc; k+=bt) {
//			for (int k=0; k<al; k++) {
//				scr = x0[jal+abinc] * w[k * bt];
//				x1[jal2+al] = x0[jal] - scr;
//				x1[jal2] = x0[jal] + scr;
				jal3 = jal + abinc;
				scrr = x0r[jal3] * wr[k] - x0i[jal3] * wi[k];
				scri = x0r[jal3] * wi[k] + x0i[jal3] * wr[k];
				x1r[jal2+al] = x0r[jal] - scrr; x1i[jal2+al] = x0i[jal] - scri;
				x1r[jal2] = x0r[jal] + scrr; x1i[jal2] = x0i[jal] + scri;
				jal++; jal2++;
			}
		}
		al *= 2; bt /= 2;
	}
	//l=np-1
	idx = idxbs; abinc = al * idxinc;
//	int k = 0;
//		scr = x1[n/2];
//		f[idx] = x1[0] + scr;
//		f[idx+abinc] = x1[0] - scr;
		scrr = x1r[n/2]; scri = x1i[n/2];
		f[idx].re = x1r[0] + scrr; f[idx].im = x1i[0] + scri;
		f[idx+abinc].re = x1r[0] - scrr; f[idx+abinc].im = x1i[0] - scri;
	idx = idxbs + abinc - idxinc;
	for (int k=1; k<al; k++) {
//		scr = x1[n/2+k] * w[k * bt];
//		f[idx+abinc] = x1[k] + scr;
//		f[idx] = x1[k] - scr;
		jal3 = n/2+k; kbt = k * bt;
		scrr = x1r[jal3] * wr[kbt] - x1i[jal3] * wi[kbt];
		scri = x1r[jal3] * wi[kbt] + x1i[jal3] * wr[kbt];
		f[idx+abinc].re = x1r[k] + scrr; f[idx+abinc].im = x1i[k] + scri;
		f[idx].re = x1r[k] - scrr; f[idx].im = x1i[k] - scri;
		idx -= idxinc;
	}/**/
/*
	idx = idxbs; abinc = al * idxinc;
	for (int k=0; k<al; k++) {
		scr = x1[n/2+k] * w[k * bt];
		f[idx] = x1[k] + scr;
		f[idx+abinc] = x1[k] - scr;
		idx += idxinc;
	}/**/
}

void CFft::FFT(float* wr, float* wi, int n, int np, int nmin,
								CCmplx* f, int idxbs, int idxinc) {//k=-n/2 ~ n/2-1 or 0 ~ n-1
	int al = 1; int bt = n/2; CCmplx scr; int jal, jal2;// CCmplx* swp;
	float scrr, scri; float* swpr; float* swpi;
	//l=0 //x1[j,k]=x[j*alpha*2+k]
	int idx = idxbs; int abinc = idxinc * bt;
		for (int j=0; j<n; j+=2) {
																	//x0[j] = f[idxbs + j * idxinc]
//				scr = f[idx + abinc];			//scr = x0[j+bt];
//				x1[j] = f[idx] + scr;		//x1[j*2] = x0[j] + scr;
//				x1[j+1] = f[idx] - scr;	//x1[j*2+1] = x0[j] - scr;
			scrr = (float)f[idx + abinc].re; scri = (float)f[idx + abinc].im;
			x1r[j] = (float)f[idx].re + scrr; x1i[j] = (float)f[idx].im + scri;
			x1r[j+1] = (float)f[idx].re - scrr; x1i[j+1] = (float)f[idx].im - scri;
			idx += idxinc;
		}
		al *= 2; bt /= 2;
	abinc = n/2;
	int jal3, kbt;
//	for (int l=1; l<(np-1); l++) {//x[j,k]=x[j*alpha + k]
//		swp = x1; x1 = x0; x0 = swp;
//		for (int j=0; j<bt; j++) {
//			jal = j * al; jal2 = jal * 2;
//			for (int k=0; k<al; k++) {
//				scr = x0[jal+abinc] * w[k * bt];
//				x1[jal2+al] = x0[jal] - scr;
//				x1[jal2] = x0[jal] + scr;
//				jal++; jal2++;
//			}
//		}
//		al *= 2; bt /= 2;
//	}
	float wrk, wik;
	for (int l=1; l<(np/2); l++) {//x[j,k]=x[j*alpha + k]
		swpr = x1r; x1r = x0r; x0r = swpr; swpi = x1i; x1i = x0i; x0i = swpi;
		for (int k=0; k<al; k++) {
			kbt = k * bt; wrk = wr[kbt]; wik = wi[kbt];
			for (int j=k; j<abinc; j+=al) {
				jal2 = j * 2 - k;
				jal3 = j + abinc;
				scrr = x0r[jal3] * wrk - x0i[jal3] * wik;
				scri = x0r[jal3] * wik + x0i[jal3] * wrk;
				x1r[jal2+al] = x0r[j] - scrr; x1i[jal2+al] = x0i[j] - scri;
				x1r[jal2] = x0r[j] + scrr; x1i[jal2] = x0i[j] + scri;
			}
		}
		al *= 2; bt /= 2;
	}
	for (int l=(np/2); l<(np-1); l++) {//x[j,k]=x[j*alpha + k]
		swpr = x1r; x1r = x0r; x0r = swpr; swpi = x1i; x1i = x0i; x0i = swpi;
		for (int j=0; j<abinc; j+=al) {
			jal = j; jal2 = j * 2;
			for (int k=0; k<abinc; k+=bt) {
				jal3 = jal + abinc;
				scrr = x0r[jal3] * wr[k] - x0i[jal3] * wi[k];
				scri = x0r[jal3] * wi[k] + x0i[jal3] * wr[k];
				x1r[jal2+al] = x0r[jal] - scrr; x1i[jal2+al] = x0i[jal] - scri;
				x1r[jal2] = x0r[jal] + scrr; x1i[jal2] = x0i[jal] + scri;
				jal++; jal2++;
			}
		}
		al *= 2; bt /= 2;
	}
	//l=np-1
	idx = idxbs; abinc = al * idxinc;
	if (nmin >= 0) {
		for (int k=0; k<al; k++) {
//			scr = x1[n/2+k] * w[k * bt];
//			f[idx] = x1[k] + scr;
//			f[idx+abinc] = x1[k] - scr;
			jal3 = n/2+k; kbt = k * bt;
			scrr = x1r[jal3] * wr[kbt] - x1i[jal3] * wi[kbt];
			scri = x1r[jal3] * wi[kbt] + x1i[jal3] * wr[kbt];
			f[idx].re = x1r[k] + scrr; f[idx].im = x1i[k] + scri;
			f[idx+abinc].re = x1r[k] - scrr; f[idx+abinc].im = x1i[k] - scri;
			idx += idxinc;
		}
	} else {
		for (int k=0; k<al; k++) {
//			scr = x1[n/2+k] * w[k * bt];
//			f[idx] = x1[k] - scr;
//			f[idx+abinc] = x1[k] + scr;
			jal3 = n/2+k; kbt = k * bt;
			scrr = x1r[jal3] * wr[kbt] - x1i[jal3] * wi[kbt];
			scri = x1r[jal3] * wi[kbt] + x1i[jal3] * wr[kbt];
			f[idx].re = x1r[k] - scrr; f[idx].im = x1i[k] - scri;
			f[idx+abinc].re = x1r[k] + scrr; f[idx+abinc].im = x1i[k] + scri;
			idx += idxinc;
		}
	}
}

//TErr CFft::ConvInt() {
//}

TErr CFft::Init1(int narg0, int min0) {
	if (narg0 < 3) return 15003;
	if (init) {
		if (np0 == narg0) {
			if ((min0 < 0)&&(nmin0 < 0)) return 0;
			if ((min0 >= 0)&&(nmin0 >= 0)) return 0;
		}
	}
	nmin0 = min0; 
	np0 = narg0;
	n0 = (int) pow((double)2, narg0); 
	nmax = n0;
	if ( w0r ) {
		delete [] w0r; delete [] w0i; 
		delete [] x0r; delete [] x0i; delete [] x1r; delete [] x1i;
	}
	//120720
	try {
		w0r = new float[n0 / 2];
		w0i = new float[n0 / 2];
		x0r = new float[nmax];
		x0i = new float[nmax];
		x1r = new float[nmax];
		x1i = new float[nmax];
	}
	catch(CException* e) {
		e->Delete();
		return 15002;
	}
	float p0 = (float)__PI2;
	if (nmin0 < 0) {nmin0 = - (n0 / 2) + 1; p0 *= -1;}
	float a;
	for (int i=0; i<(n0 / 2); i++) {
		a = p0 * ((float)i / (float)n0);
		w0r[i] = (float)cos(a); w0i[i] = (float)sin(a);
	}
	init = true; return 0;
}
void CFft::FFT1(CCmplx* f) {//f(h) = f[h]???
	if (!(init)) return;
	//int idx0; 
	//int i=0;
	//	idx0 = i * n0;
	//	this->FFTrev(w0r, w0i, n0, np0, nmin0, f, idx0, 1);
	this->FFT(w0r, w0i, n0, np0, nmin0, f, 0, 1);
	const float a = 1.f / n0;
	if (nmin0 < 0) {
		CCmplx sw;
		for (int i=0; i<n0/2; i++) {
			const int idx = n0 - 1 - i;
			sw = f[i]; f[i] = f[idx] * a; f[idx] = sw * a;
		}
	} else {
		for (int i=0; i<n0; i++) {f[i] = f[i] * a;}
	}
}

void CFft::FFT1Rev(CCmplx* f) {//f(h) = f[h]???
	if (!(init)) return;
	this->FFTrev(w0r, w0i, n0, np0, nmin0, f, 0, 1);
}

TErr CFft::Init2(int narg0, int min0, int narg1, int min1) {
	if ((narg0 < 3)||(narg1 < 3)) return 15003;
	if (init) {
		if ((np0 == narg0)&&(np1 == narg1)) {
			int nskip = 0;
			if ((min0 < 0)&&(nmin0 < 0)) nskip++;
			else if ((min0 >= 0)&&(nmin0 >= 0)) nskip++;
			if ((min1 < 0)&&(nmin1 < 0)) nskip++;
			else if ((min1 >= 0)&&(nmin1 >= 0)) nskip++;
			if (nskip == 2) return 0;
		}
	}
	nmin0 = min0; 
	nmin1 = min1; 
	np0 = narg0;
	np1 = narg1;
	n0 = (int) pow((double)2, narg0); 
	n1 = (int) pow((double)2, narg1); 
	nmax = n0;
	nmax = nmax > n1 ? nmax : n1;
	if ( w0r ) {
		delete [] w0r; delete [] w0i; delete [] w1r; delete [] w1i;
		delete [] x0r; delete [] x0i; delete [] x1r; delete [] x1i;
	}
	if ((w0r = new float[n0 / 2]) == NULL) return 15002;
	if ((w1r = new float[n1 / 2]) == NULL) return 15002;
	if ((w0i = new float[n0 / 2]) == NULL) return 15002;
	if ((w1i = new float[n1 / 2]) == NULL) return 15002;
	if ((x0r = new float[nmax]) == NULL) return 15002;
	if ((x0i = new float[nmax]) == NULL) return 15002;
	if ((x1r = new float[nmax]) == NULL) return 15002;
	if ((x1i = new float[nmax]) == NULL) return 15002;
	float a, p0;
	if (nmin0 < 0) {nmin0 = - (n0 / 2) + 1; p0 = -(float)__PI2;}	else p0 = (float)__PI2;
	for (int i=0; i<(n0 / 2); i++) {
		a = p0 * ((float)i / (float)n0);
		w0r[i] = (float)cos(a); w0i[i] = (float)sin(a);
	}
	if (nmin1 < 0) {nmin1 = - (n1 / 2) + 1; p0 = -(float)__PI2;}	else p0 = (float)__PI2;
	for (int i=0; i<(n1 / 2); i++) {
		a = p0 * ((float)i / (float)n1);
		w1r[i] = (float)cos(a); w1i[i] = (float)sin(a);
	}
	init = true; return 0;
}

void CFft::FFT2(CCmplx* f) {//f(hk0) = f[k*hmax+h]???
	if (!(init)) return;
	const int n1n0 = n1 * n0;
	for (int i=0; i<n1; i++) {
		const int idx0 = i * n0;
		this->FFT(w0r, w0i, n0, np0, nmin0, f, idx0, 1);
		const float a = 1.f / n0;
		if (nmin0 < 0) {
			CCmplx sw;
			for (int j=0; j<n0/2; j++) {
				const int idx = n0 - 1 - j;
				sw = f[j+idx0]; f[j+idx0] = f[idx+idx0] * a; f[idx+idx0] = sw * a;
			}
		} else {
			for (int j=0; j<n0; j++) {f[j+idx0] = f[j+idx0] * a;}
		}
	}
	for (int i=0; i<n0; i++) {
		const int idx0 = i;
		this->FFT(w1r, w1i, n1, np1, nmin1, f, idx0, n0);
		const float a = 1.f / n1;
		if (nmin1 < 0) {
			CCmplx sw;
			for (int j=0; j<n1/2; j++) {
				const int idx = n1 - 1 - j;
				sw = f[j*n0+idx0]; f[j*n0+idx0] = f[idx*n0+idx0] * a; f[idx*n0+idx0] = sw * a;
			}
		} else {
			for (int j=0; j<n1; j++) {f[j*n0+idx0] = f[j*n0+idx0] * a;}
		}
	}
}

void CFft::FFT2Rev(CCmplx* f) {
	if (!(init)) return;
	int idx0; 
	const int n1n0 = n1 * n0;
	for (int i=0; i<n1; i++) {
		idx0 = i * n0;
		this->FFTrev(w0r, w0i, n0, np0, nmin0, f, idx0, 1);
	}
	for (int i=0; i<n0; i++) {
		idx0 = i;
		this->FFTrev(w1r, w1i, n1, np1, nmin1, f, idx0, n0);
	}
}


#include <sys\timeb.h>

void CFft::Debug() {
	const int ndimp = 7;
	const int ndim = (int) pow((double)2, ndimp);
	CCmplx* F; CCmplx* G; CCmplx* H;
	F = new CCmplx[ndim]; G = new CCmplx[ndim]; H = new CCmplx[ndim];
	int i;
	for (i=0; i<ndim; i++) {G[i] = F[i].Exp(__PI2*(i)/2.0);}
	//for (i=0; i<ndim; i++) {
	//	double gcoeff = -0.2;
	//	F[i].re = exp(gcoeff * (i-(ndim/2-1)) * (i-(ndim/2-1)));
	//	F[i].im = 0;
	//	G[i] = F[i];
	//}
	//for (i=0; i<ndim; i++) {G[i] = G[i] * 1.1; F[i] = F[i] * 1.1;}
	//for (i=100; i<ndim; i++) {G[i] = F[i] = CCmplx(0.0, 0.0);}
	this->Init1(ndimp, -1);
	//this->Init1(ndimp, 0);
	struct _timeb tstruct;
	_ftime_s( &tstruct );
	TReal tm0 = tstruct.time + tstruct.millitm * 0.001;
	//
	this->FFT1Rev(F);
	///
	for (i=0; i<ndim; i++) {H[i] = F[i];}
	//for (i=0; i<ndim/2; i++) {H[i] *= 1. * i / ndim;}
	//for (i=ndim/2; i<ndim; i++) {H[i].Reset();}
	//this->Init1(ndimp, 0);
	this->FFT1(H);
	_ftime_s( &tstruct );
	///
	CString line = "G => F => H\r\n", scr;
	for (i=0; i<ndim; i+=(ndim / 32)) {
	//for (i=0; i<16; i++) {
	//for (i=56; i<72; i++) {
		scr.Format("%d (%f %f) (%f %f) (%f %f)\r\n",
			i, G[i].re, G[i].im, F[i].re, F[i].im, H[i].re, H[i].im);
		line += scr;
	}
	double sumH = 0, sumG = 0;
	for (i=0; i<ndim; i++) {sumH += H[i].re; sumG += G[i].re;}
	//scr.Format("time %f", tstruct.time + tstruct.millitm * 0.001 - tm0);
	scr.Format("average %f", sumH / sumG);
	line += scr;
	AfxMessageBox(line);
	if (F) delete [] F;
	if (G) delete [] G;
	if (H) delete [] H;
}

void CFft::Debug2() {
	const int ndimp = 6;
	const int ndim = (int) pow((double)2, ndimp);
	const int ndimp2 = 5;
	const int ndim2 = (int) pow((double)2, ndimp2);
	CCmplx* F; CCmplx* G; CCmplx* H;
	F = new CCmplx[ndim*ndim2]; G = new CCmplx[ndim*ndim2]; H = new CCmplx[ndim*ndim2];
	int i;
	for (int j=0; j<ndim2; j++) {
		CCmplx v;
		v.Exp(__PI2*(j-15)/4.0);
		for (i=0; i<ndim; i++) {
			CCmplx w;
			w.Exp(__PI2*(i-63)/16.0);
			G[i+j*ndim] = F[i+j*ndim] = w * v;
		}
	}
	//for (i=0; i<ndim; i++) {G[i] = G[i] * 1.1; F[i] = F[i] * 1.1;}
	//for (i=100; i<ndim; i++) {G[i] = F[i] = CCmplx(0.0, 0.0);}
	this->Init2(ndimp, -1, ndimp2, -1);
	//this->Init1(ndimp, -1);
	struct _timeb tstruct;
	_ftime_s( &tstruct );
	TReal tm0 = tstruct.time + tstruct.millitm * 0.001;
	//
	this->FFT2Rev(F);
	///
	for (int j=0; j<ndim2; j++) {
		for (i=0; i<ndim; i++) {H[i+j*ndim] = F[i+j*ndim];}
	}
	//for (i=0; i<ndim/2; i++) {H[i] *= 1. * i / ndim;}
	//for (i=ndim/2; i<ndim; i++) {H[i].Reset();}
	//this->Init1(ndimp, 0);
	this->FFT2(H);
	_ftime_s( &tstruct );
	///
	CString line = "G => F => H\r\n", scr;
	for (i=0; i<ndim; i+=(ndim / 16)) {
		//scr.Format("%d (%f %f) (%f %f) (%f %f)\r\n",
		//	i, G[i].re, G[i].im, F[i].re, F[i].im, H[i].re, H[i].im);
		scr.Format("%d(", i); line += scr;
		for (int j=0; j<ndim2; j+=(ndim2 / 8)) {scr.Format("%4.2f ", G[i+ndim*j].re); line += scr;}
		line += ")(";
		for (int j=0; j<ndim2; j+=(ndim2 / 8)) {scr.Format("%4.2f ", F[i+ndim*j].re); line += scr;}
		line += ")(";
		for (int j=0; j<ndim2; j+=(ndim2 / 8)) {scr.Format("%4.2f ", H[i+ndim*j].re); line += scr;}
		line += ")\r\n";
	}
	double sumH = 0, sumG = 0;
	for (i=0; i<ndim*ndim2; i++) {sumH += H[i].re; sumG += G[i].re;}
	scr.Format("time %f; ", tstruct.time + tstruct.millitm * 0.001 - tm0);
	line += scr;
	scr.Format("average %f", sumH / sumG);
	line += scr;
	AfxMessageBox(line);
	if (F) delete [] F;
	if (G) delete [] G;
	if (H) delete [] H;
}

#endif // _CFFT_CPP_
