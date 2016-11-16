#ifndef _VECMAT_
#define _VECMAT_ 1

#include <math.h>
#include "float3.h"

class mat4x4
{
public:
    float4 row[4];
    
    // functions:
    void MultBy(mat4x4 m)
    {
        float4 t[4];
        
        float4* a = &row[0];
        float4* b = &m.row[0];

        t[0].x = a[0].x*b[0].x + a[0].y*b[1].x + a[0].z*b[2].x + a[0].w*b[3].x;
        t[0].y = a[0].x*b[0].y + a[0].y*b[1].y + a[0].z*b[2].y + a[0].w*b[3].y;
        t[0].z = a[0].x*b[0].z + a[0].y*b[1].z + a[0].z*b[2].z + a[0].w*b[3].z;
        t[0].w = a[0].x*b[0].w + a[0].y*b[1].w + a[0].z*b[2].w + a[0].w*b[3].w;

        t[1].x = a[1].x*b[0].x + a[1].y*b[1].x + a[1].z*b[2].x + a[1].w*b[3].x;
        t[1].y = a[1].x*b[0].y + a[1].y*b[1].y + a[1].z*b[2].y + a[1].w*b[3].y;
        t[1].z = a[1].x*b[0].z + a[1].y*b[1].z + a[1].z*b[2].z + a[1].w*b[3].z;
        t[1].w = a[1].x*b[0].w + a[1].y*b[1].w + a[1].z*b[2].w + a[1].w*b[3].w;

        t[2].x = a[2].x*b[0].x + a[2].y*b[1].x + a[2].z*b[2].x + a[2].w*b[3].x;
        t[2].y = a[2].x*b[0].y + a[2].y*b[1].y + a[2].z*b[2].y + a[2].w*b[3].y;
        t[2].z = a[2].x*b[0].z + a[2].y*b[1].z + a[2].z*b[2].z + a[2].w*b[3].z;
        t[2].w = a[2].x*b[0].w + a[2].y*b[1].w + a[2].z*b[2].w + a[2].w*b[3].w;

        t[3].x = a[3].x*b[0].x + a[3].y*b[1].x + a[3].z*b[2].x + a[3].w*b[3].x;
        t[3].y = a[3].x*b[0].y + a[3].y*b[1].y + a[3].z*b[2].y + a[3].w*b[3].y;
        t[3].z = a[3].x*b[0].z + a[3].y*b[1].z + a[3].z*b[2].z + a[3].w*b[3].z;
        t[3].w = a[3].x*b[0].w + a[3].y*b[1].w + a[3].z*b[2].w + a[3].w*b[3].w;

        row[0] = t[0];
        row[1] = t[1];
        row[2] = t[2];
        row[3] = t[3];
    }
    float4 TransformPt(float4 pt)
    {
        //float4 ret =   row[0] * pt.x
        //             + row[1] * pt.y
        //             + row[2] * pt.z
        //             + row[3] * pt.w
        //;
        float4 ret;
        ret.x = row[0].dot(pt);
        ret.y = row[1].dot(pt);
        ret.z = row[2].dot(pt);
        ret.w = row[3].dot(pt);
        return ret;
    }   
    float3 TransformVec(float3 vec)
    {
        float3 ret;
        ret.x = row[0].x*vec.x + row[0].y*vec.y + row[0].z*vec.z;
        ret.y = row[1].x*vec.x + row[1].y*vec.y + row[1].z*vec.z;
        ret.z = row[2].x*vec.x + row[2].y*vec.y + row[2].z*vec.z;
        return ret;
    }   
};

mat4x4 Invert(mat4x4 m);
mat4x4 Transpose(mat4x4 m);










#endif
