/*	cxyz.h		v0.01	10/18/2015
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
	Subdevided from structure.h (v0.01 since 9/5/1998)
*/

#if !defined( _CXYZ_H_ )
#define _CXYZ_H_

#define COORD_DISABLED 9999.999
#include "general.h"

class CXyz {
public:
	CXyz( float coor[] ); // coor[3] --> x,y,z
	CXyz(TReal argx, TReal argy, TReal argz);
	CXyz();
	~CXyz();
	void Set(TReal arg[]); // arg[3] --> x,y,z
	void Reset();
	TReal Length(); // nagasa
	TReal Length2(); // nagasa^2
	void UnitLength();
	TReal Curvature(int nCrd);
	CXyz operator +(CXyz a);
	CXyz operator -(CXyz a);
	CXyz operator +=(CXyz a);
	CXyz X(TReal t) const;
	TReal X(CXyz t); // nai-seki
	CXyz operator *(CXyz a); // gai-seki
	bool operator ==(CXyz a);
	bool operator !=(CXyz a);
//private:
	TReal x, y, z;
};

#endif // _CXYZ_H_
