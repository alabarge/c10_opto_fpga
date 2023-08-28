#pragma once

#include <stdlib.h>

#define US_IMAGE unsigned char

namespace LATTE{

	template <typename T> class PPoint{
	public:
		PPoint(){};
		~PPoint(){};
		PPoint(T setX, T setY, T setZ) {x=setX, y=setY, z=setZ;}
      PPoint (const PPoint<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

		PPoint<T> operator=(const PPoint<T> &rhs) { x= rhs.x, y=rhs.y, z=rhs.z; return *this;}	
		PPoint<T> operator=(const T &rhs) { x=y=z=rhs; return *this;}	
		void operator +=(const PPoint<T> &rhs) { x+= rhs.x, y+=rhs.y, z+=rhs.z; }
		void operator -=(const PPoint<T> &rhs) { x-= rhs.x, y-=rhs.y, z-=rhs.z; } 
		//void operator *=(const PPoint<T> &rhs) { x*= rhs.x, y*=rhs.y, z*=rhs.z; }
		void operator /=(const PPoint<T> &rhs) { x/= rhs.x, y/=rhs.y, z/=rhs.z; }
		void operator /=(const T &rhs) { x/= rhs, y/=rhs, z/=rhs; }


		T Norm2() {return (x*x+y*y+z*z);}
		T Norm()  {return sqrt(x*x+y*y+z*z);}
		void Set(T setX, T setY, T setZ) {x=setX, y=setY, z=setZ;}

		T x, y, z;
	};

	template <typename T> inline PPoint<T> operator+(PPoint<T>lhs, const PPoint<T>& rhs) {lhs += rhs; return lhs;};
	template <typename T> inline PPoint<T> operator-(PPoint<T>lhs, const PPoint<T>& rhs) {lhs -= rhs; return lhs;};
	template <typename T> inline PPoint<T> operator*(PPoint<T>lhs, const T& v) {lhs.x *= v,lhs.y*=v,lhs.z *=v; return lhs;};
	template <typename T> inline PPoint<T> operator/(PPoint<T>lhs, const T& v) {lhs.x /= v,lhs.y/=v,lhs.z /=v; return lhs;};
	template <typename T> inline PPoint<T> operator*(const T& v, const PPoint<T>& rhs) {PPoint<T> res= rhs; res.x *= v, res.y*=v, res.z *=v; return res;};
	template <typename T> inline T operator*(const PPoint<T>& lhs, const PPoint<T>& rhs) {return lhs.x*rhs.x+lhs.y*rhs.y+lhs.z*rhs.z;};
	template <typename T> inline T operator/(PPoint<T> lhs, const PPoint<T>& rhs) {return lhs/=rhs;};
	template <typename T> inline PPoint<T> Min(const PPoint<T>& lhs, const PPoint<T>& rhs) { PPoint<T> res; res.x = __min(lhs.x,rhs.x),res.y = __min(lhs.y,rhs.y),res.z = __min(lhs.z,rhs.z); return res;} 
	template <typename T> inline PPoint<T> Max(const PPoint<T>& lhs, const PPoint<T>& rhs) { PPoint<T> res; res.x = __max(lhs.x,rhs.x),res.y = __max(lhs.y,rhs.y),res.z = __max(lhs.z,rhs.z); return res;} 


	typedef  PPoint<int> VPoint;
	typedef  PPoint<double> FPoint;

	template <typename T> struct BBox{
		T pmin, pmax;
	};

	typedef BBox<VPoint> VBOX;
	typedef BBox<FPoint> FBOX;
}