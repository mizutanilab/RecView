/*	ccmplx.cpp		v0.01	5/10/2000
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"
/// added for precompiled header definition 

#if !defined( _CCMPLX_CPP_ )
#define _CCMPLX_CPP_
#include <math.h>
#include "ccmplx.h"
//#include "structure.h"

CCmplx::CCmplx() {re = 0.0; im = 0.0;}
CCmplx::CCmplx(TReal arg1, TReal arg2) {re = (TCmpElmnt)arg1; im = (TCmpElmnt)arg2;}
CCmplx::~CCmplx() {/*place holder*/}
CCmplx CCmplx::operator +(CCmplx a) {
	return CCmplx(re + a.re, im + a.im);
}
CCmplx CCmplx::operator -(CCmplx a) {
	return CCmplx(re - a.re, im - a.im);
}
CCmplx CCmplx::operator *(CCmplx a) {
	return CCmplx(re * a.re - im * a.im, re * a.im + im * a.re);
}
CCmplx CCmplx::operator *(TReal a) {
	return CCmplx(re * a, im * a);
}
CCmplx CCmplx::operator /(TReal a) {
	return CCmplx(re / a, im / a);
}
CCmplx CCmplx::operator =(CCmplx a) {
	re = a.re; im = a.im; return CCmplx(re, im);
}
CCmplx CCmplx::operator +=(CCmplx a) {
	re += a.re; im += a.im; return CCmplx(re, im);
}
CCmplx CCmplx::operator -=(CCmplx a) {
	re -= a.re; im -= a.im; return CCmplx(re, im);
}
CCmplx CCmplx::operator *=(CCmplx a) {
	TCmpElmnt r = re * a.re - im * a.im;
	im = re * a.im + im * a.re;
	re = r; return CCmplx(re, im);
}
CCmplx CCmplx::operator *=(TReal a) {
	re *= (TCmpElmnt)a; im *= (TCmpElmnt)a; return *this;
}
CCmplx CCmplx::operator *=(float a) {
	re *= (TCmpElmnt)a; im *= (TCmpElmnt)a; return *this;
}
CCmplx CCmplx::Exp(TReal a) {re = (TCmpElmnt)cos(a); im = (TCmpElmnt)sin(a); return CCmplx(re, im);}
CCmplx CCmplx::Rev() {return CCmplx(-im, re);}
CCmplx CCmplx::Conj() {return CCmplx(re, -im);}
TCmpElmnt CCmplx::Real() {return re;}
TCmpElmnt CCmplx::Imag() {return im;}
TCmpElmnt CCmplx::Modulus() {return (TCmpElmnt)sqrt(re * re + im * im);}
TCmpElmnt CCmplx::Modulus2() {return (re * re + im * im);}
void CCmplx::AddToReal(TCmpElmnt a) {re += a;}
void CCmplx::Reset() {re = 0.0; im = 0.0;}
					
#endif // _CCMPLX_CPP_
