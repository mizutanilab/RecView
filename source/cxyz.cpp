/*	cxyz.cpp		v0.01	9/20/1998
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
	Goal: CXyz class body.
*/

#include "stdafx.h"
/// added for precompiled header definition 

#if !defined( _CXYZ_CPP_ )
#define _CXYZ_CPP_
#include <math.h>
//#include "stdtypdf.h"
#include "cxyz.h"

CXyz::CXyz(float coor[]) {
	x = coor[0];
	y = coor[1];
	z = coor[2];
}
CXyz::CXyz(TReal argx, TReal argy, TReal argz) {
	x = argx; y = argy;	z = argz;
}
CXyz::CXyz() {
	x = COORD_DISABLED;
	y = COORD_DISABLED;
	z = COORD_DISABLED;
}
CXyz::~CXyz() {
	// no operation.
}
void CXyz::Set(TReal arg[]) {
	x = arg[0]; y = arg[1]; z = arg[2]; return; }
void CXyz::Reset() {
	x = 0.0;
	y = 0.0;
	z = 0.0;
}
TReal CXyz::Length() { // nagasa
	return sqrt( x * x + y * y + z * z );
}
TReal CXyz::Length2() { // nagasa
	return ( x * x + y * y + z * z );
}
void CXyz::UnitLength() {
	TReal len;
	if ((len = sqrt( x * x + y * y + z * z )) == 0.) return;
	x /= len; y /= len; z /= len; return;
}

//150222
TReal CXyz::Curvature(int nCrd) {
	double dCurv = 0.; int nCurv = 0;
	for (int j=0; j<nCrd-2; j++) {
		CXyz ct1 = this[j+1] - this[j]; ct1.UnitLength();
		CXyz ct2 = this[j+2] - this[j+1]; ct2.UnitLength();
		double ds = (this[j+2] - this[j]).Length2() * 0.25;
		if (fabs(ds) < 0.1) continue;
		dCurv += sqrt((ct2 - ct1).Length2() / ds);
		nCurv++;
	}
	//
	if (nCurv > 2) return nCurv/dCurv;
	return -1;
}

CXyz CXyz::operator +(CXyz a) {
	return CXyz(x+a.x, y+a.y, z+a.z);
}
CXyz CXyz::operator +=(CXyz a) {
	x += a.x;	y += a.y;	z += a.z;
	return CXyz(x, y, z);
}
CXyz CXyz::operator -(CXyz a) {
	return CXyz(x-a.x, y-a.y, z-a.z);
}
CXyz CXyz::X(TReal t) const{
	return CXyz(t * x, t * y, t * z);
}
TReal CXyz::X(CXyz t) {
	return (t.x * x + t.y * y + t.z * z);
}
CXyz CXyz::operator *(CXyz a) { // gai-seki
	return CXyz( y * a.z - z * a.y,
				 z * a.x - x * a.z,
				 x * a.y - y * a.x );
}
bool CXyz::operator ==(CXyz a) {
	return ((x == a.x)&&(y == a.y)&&(z == a.z));
}
bool CXyz::operator !=(CXyz a) {
	return ((x != a.x)||(y != a.y)||(z != a.z));
}
/*inline TReal CXyz::operator *(CXyz a) { // nai-seki
	return x * a.x + y * a.y + z * a.z;
}
inline TReal CXyz::operator |(CXyz a) { // distance
	TReal dx = x - a.x;
	TReal dy = y - a.y;
	TReal dz = z - a.z;
	return sqrt( dx * dx + dy * dy + dz * dz );
}
inline TReal CXyz::operator ^(CXyz a) { // angle
	return acos( (x * a.x + y * a.y + z * a.z) /
				 sqrt( x * x + y * y + z * z ) /
				 sqrt( a.x * a.x + a.y * a.y + a.z * a.z )
			   );
}*/
					
#endif // _CXYZ_CPP_
