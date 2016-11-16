// float3.h: interface for the float3 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FLOAT3_H
#define FLOAT3_H

#include <math.h>

#define FLOAT3_SMALL_NUM 0.0000001f

class float2
{
public:
/*	inline void* operator new(size_t size){return mempool.Alloc(size);};
	inline void* operator new(size_t n, void *p){return p;}; //cp visual
	inline void operator delete(void* p,size_t size){mempool.Free(p,size);};
*/
	inline float2() /*: x(0), y(0)*/ {};

	inline float2(const float x,const float y) : x(x),y(y) {}
	inline float2(const float xy) : x(xy),y(xy) {}

	inline ~float2(){};

	inline float2 operator+ (const float2 &f) const{
		return float2(x+f.x,y+f.y);
	}
	inline float2 operator+ (const float f) const{
		return float2(x+f,y+f);
	}
	inline void operator+= (const float2 &f){
		x+=f.x;
		y+=f.y;
	}

	inline float2 operator- (const float2 &f) const{
		return float2(x-f.x,y-f.y);
	}
	inline float2 operator- () const{
		return float2(-x,-y);
	}
	inline float2 operator- (const float f) const{
		return float2(x-f,y-f);
	}
	inline void operator-= (const float2 &f){
		x-=f.x;
		y-=f.y;
	}

	inline float2 operator* (const float2 &f) const{
		return float2(x*f.x,y*f.y);
	}
	inline float2 operator* (const float f) const{
		return float2(x*f,y*f);
	}
	inline void operator*= (const float2 &f){
		x*=f.x;
		y*=f.y;
	}
	inline void operator*= (const float f){
		x*=f;
		y*=f;
	}

	inline float2 operator/ (const float2 &f) const{
		return float2(x/f.x,y/f.y);
	}
	inline float2 operator/ (const float f) const{
        float t = 1.0f/f;
		return float2(x*t,y*t);
	}
	inline void operator/= (const float2 &f){
		x/=f.x;
		y/=f.y;
	}
	inline void operator/= (const float f){
        float t = 1.0f/f;
		x*=t;
		y*=t;
	}

	inline bool operator== (const float2 &f) const {
		return ((x==f.x) && (y==f.y));
	}
	inline bool operator!= (const float2 &f) const {
		return ((x!=f.x) || (y!=f.y));
	}
	//inline float& operator[] (const int t) {
	//	return xy[t];
	//}
	//inline const float& operator[] (const int t) const {
	//	return xy[t];
	//}

	inline float distance(const float2 &f) const{
		return sqrtf((x-f.x)*(x-f.x)+(y-f.y)*(y-f.y));
	}
	inline float Length() const{
		return sqrtf(x*x+y*y);
	}
	inline float LengthSquared() const{
		return x*x+y*y;
	}
    inline void Clear()
    {
        x = 0.0f;
        y = 0.0f;
    }
    inline void Set(float fX,float fY)
    {
        x = fX;
        y = fY;
    }
    inline void SetAbs()
    {
        x = fabsf(x);
        y = fabsf(y);
    }
	/*union {
		struct{
			float x;
			float y;
		};
		float xy[2];
	};*/
	float x;
	float y;
};

class float3
{
public:
/*	inline void* operator new(size_t size){return mempool.Alloc(size);};
	inline void* operator new(size_t n, void *p){return p;}; //cp visual
	inline void operator delete(void* p,size_t size){mempool.Free(p,size);};
*/
	inline float3() /*: x(0), y(0), z(0)*/ {};

	inline float3(const float x,const float y,const float z) : x(x),y(y),z(z) {}
	inline float3(const float xyz) : x(xyz),y(xyz),z(xyz) {}

	inline ~float3(){};

	inline float3 operator+ (const float3 &f) const{
		return float3(x+f.x,y+f.y,z+f.z);
	}
	inline float3 operator+ (const float f) const{
		return float3(x+f,y+f,z+f);
	}
	inline void operator+= (const float3 &f){
		x+=f.x;
		y+=f.y;
		z+=f.z;
	}

	inline float3 operator- (const float3 &f) const{
		return float3(x-f.x,y-f.y,z-f.z);
	}
	inline float3 operator- () const{
		return float3(-x,-y,-z);
	}
	inline float3 operator- (const float f) const{
		return float3(x-f,y-f,z-f);
	}
	inline void operator-= (const float3 &f){
		x-=f.x;
		y-=f.y;
		z-=f.z;
	}

	inline float3 operator* (const float3 &f) const{
		return float3(x*f.x,y*f.y,z*f.z);
	}
	inline float3 operator* (const float f) const{
		return float3(x*f,y*f,z*f);
	}
	inline void operator*= (const float3 &f){
		x*=f.x;
		y*=f.y;
		z*=f.z;
	}
	inline void operator*= (const float f){
		x*=f;
		y*=f;
		z*=f;
	}

	inline float3 operator/ (const float3 &f) const{
		return float3(x/f.x,y/f.y,z/f.z);
	}
	inline float3 operator/ (const float f) const{
        float t = 1.0f/f;
		return float3(x*t,y*t,z*t);
	}
	inline void operator/= (const float3 &f){
		x/=f.x;
		y/=f.y;
		z/=f.z;
	}
	inline void operator/= (const float f){
        float t = 1.0f/f;
		x*=t;
		y*=t;
		z*=t;
	}

	inline bool operator== (const float3 &f) const {
		return ((x==f.x) && (y==f.y) && (z==f.z));
	}
	inline bool operator!= (const float3 &f) const {
		return ((x!=f.x) || (y!=f.y) || (z!=f.z));
	}
	//inline float& operator[] (const int t) {
	//	return xyz[t];
	//}
	//inline const float& operator[] (const int t) const {
	//	return xyz[t];
	//}

    inline float3 lerp(const float3 &f, float t) const{
        float t2 = 1-t;
        float3 v;
        v.x = x*t2 + f.x*t;
        v.y = y*t2 + f.y*t;
        v.z = z*t2 + f.z*t;
        return v;
    }

	inline float distance(const float3 &f) const{
		return sqrtf((x-f.x)*(x-f.x)+(y-f.y)*(y-f.y)+(z-f.z)*(z-f.z));
	}
	inline float Length() const{
		return sqrtf(x*x+y*y+z*z);
	}
	inline float Length2D() const{
		return sqrtf(x*x+y*y);
	}
	inline float LengthSquared() const{
		return x*x+y*y+z*z;
	}
    inline float3 Abs() const{
        float3 ret(fabsf(x),fabsf(y),fabsf(z));
        return ret;
    }
    inline void Clear()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }
    inline void Set(float fX,float fY,float fZ)
    {
        x = fX;
        y = fY;
        z = fZ;
    }
    inline void SetAbs()
    {
        x = fabsf(x);
        y = fabsf(y);
        z = fabsf(z);
    }
	/*union {
		struct{
			float x;
			float y;
			float z;
		};
		float xyz[3];
	};*/
	float x;
	float y;
	float z;
};

class float4
{
public:
/*	inline void* operator new(size_t size){return mempool.Alloc(size);};
	inline void* operator new(size_t n, void *p){return p;}; //cp visual
	inline void operator delete(void* p,size_t size){mempool.Free(p,size);};
*/
	inline float4() /* : x(0), y(0), z(0), w(0)*/ {};

	inline float4(const float x,const float y,const float z,const float w) : x(x),y(y),z(z),w(w) {}
	inline float4(const float3 xyz,const float w) : x(xyz.x),y(xyz.y),z(xyz.z),w(w) {}
	inline float4(const float xyzw) : x(xyzw),y(xyzw),z(xyzw),w(xyzw) {}

	inline ~float4(){};

    inline float3 GetXYZ() const { return float3(x,y,z); }

	inline float4 operator+ (const float4 &f) const{
		return float4(x+f.x,y+f.y,z+f.z,w+f.w);
	}
	inline float4 operator+ (const float f) const{
		return float4(x+f,y+f,z+f,w+f);
	}
	inline void operator+= (const float4 &f){
		x+=f.x;
		y+=f.y;
		z+=f.z;
        w+=f.w;
	}

	inline float4 operator- (const float4 &f) const{
		return float4(x-f.x,y-f.y,z-f.z,z-f.w);
	}
	inline float4 operator- () const{
		return float4(-x,-y,-z,-w);
	}
	inline float4 operator- (const float f) const{
		return float4(x-f,y-f,z-f,w-f);
	}
	inline void operator-= (const float4 &f){
		x-=f.x;
		y-=f.y;
		z-=f.z;
		w-=f.w;
	}

	inline float4 operator* (const float4 &f) const{
		return float4(x*f.x,y*f.y,z*f.z,w*f.w);
	}
	inline float4 operator* (const float f) const{
		return float4(x*f,y*f,z*f,w*f);
	}
	inline void operator*= (const float4 &f){
		x*=f.x;
		y*=f.y;
		z*=f.z;
		w*=f.w;
	}
	inline void operator*= (const float f){
		x*=f;
		y*=f;
		z*=f;
        w*=f;
	}

	inline float4 operator/ (const float4 &f) const{
		return float4(x/f.x,y/f.y,z/f.z,w/f.w);
	}
	inline float4 operator/ (const float f) const{
        float t = 1.0f/f;
		return float4(x*t,y*t,z*t,w*t);
	}
	inline void operator/= (const float4 &f){
		x/=f.x;
		y/=f.y;
		z/=f.z;
		w/=f.w;
	}
	inline void operator/= (const float f){
        float t = 1.0f/f;
		x*=t;
		y*=t;
		z*=t;
        w*=t;
	}

	inline bool operator== (const float4 &f) const {
		return ((x==f.x) && (y==f.y) && (z==f.z) && (w==f.w));
	}
	inline bool operator!= (const float4 &f) const {
		return ((x!=f.x) || (y!=f.y) || (z!=f.z) || (w!=f.w));
	}
	//inline float& operator[] (const int t) {
	//	return xyzw[t];
	//}
	//inline const float& operator[] (const int t) const {
	//	return xyzw[t];
	//}

	inline float distance(const float4 &f) const{
		return sqrtf((x-f.x)*(x-f.x)+(y-f.y)*(y-f.y)+(z-f.z)*(z-f.z)+(w-f.w)*(w-f.w));
	}
	inline float Length() const{
		return sqrtf(x*x+y*y+z*z+w*w);
	}
	inline float LengthSquared() const{
		return x*x+y*y+z*z+w*w;
	}
    inline float4 Abs() const{
        float4 ret(fabsf(x),fabsf(y),fabsf(z),fabsf(w));
        return ret;
    }
    inline void Clear()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }
    inline void Set(float fX,float fY,float fZ,float fW)
    {
        x = fX;
        y = fY;
        z = fZ;
        w = fW;
    }
    inline void SetAbs()
    {
        x = fabsf(x);
        y = fabsf(y);
        z = fabsf(z);
        w = fabsf(w);
    }
	/*union {
		struct{
			float x;
			float y;
			float z;
            float w;
		};
		float xyzw[4];
	};*/
	float x;
	float y;
	float z;
    float w;
};

inline float dot(const float3& a, const float3& b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float length_squared(const float3& v) {
  return dot(v,v);
}

inline float length(const float3& v) {
  return sqrtf(length_squared(v));
}

inline float3 normalize(const float3& v) {
  float len = dot(v,v);
  if (len <= FLOAT3_SMALL_NUM)
    return v;
  float t = 1.0f/sqrtf(len);
  return float3(v.x*t, v.y*t, v.z*t);
}

inline float3 cross(const float3& a, const float3& b) {
	float3 ut;
	ut.x = a.y*b.z - a.z*b.y;
	ut.y = a.z*b.x - a.x*b.z;
	ut.z = a.x*b.y - a.y*b.x;
	return ut;
}



inline float lerp(const float& a, const float& b, float t) {
    return b*t + a*(1.0f-t);
}
inline float2 lerp(const float2& a, const float2& b, float t) {
    return b*t + a*(1.0f-t);
}
inline float3 lerp(const float3& a, const float3& b, float t) {
    return b*t + a*(1.0f-t);
}
inline float4 lerp(const float4& a, const float4& b, float t) {
    return b*t + a*(1.0f-t);
}

inline float3 lerp(const float3& a, const float3& b, float3 t) {
    return b*t + a*(float3(1.0f,1.0f,1.0f)-t);
}

typedef struct  
{
    float3 x_axis;
    float3 y_axis;
    float3 z_axis;
} float3x3;

#endif // !defined(AFX_FLOAT3_H__026451C1_6B7D_11D4_AD55_0080ADA84DE3__INCLUDED_)
