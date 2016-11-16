#include "stdafx.h"

#include "matrix.h"

mat4x4 Transpose(mat4x4 m)
{
    mat4x4 x;
    x.row[0] = float4(m.row[0].x, m.row[1].x, m.row[2].x, m.row[3].x);
    x.row[1] = float4(m.row[0].y, m.row[1].y, m.row[2].y, m.row[3].y);
    x.row[2] = float4(m.row[0].z, m.row[1].z, m.row[2].z, m.row[3].z);
    x.row[3] = float4(m.row[0].w, m.row[1].w, m.row[2].w, m.row[3].w);
    return x;
}


mat4x4 Invert(mat4x4 m)
{
    m = Transpose(m);

    float d = 
	m.row[0].x * m.row[1].y * m.row[2].z * m.row[3].w +
	m.row[0].x * m.row[2].y * m.row[3].z * m.row[1].w +
	m.row[0].x * m.row[3].y * m.row[1].z * m.row[2].w +
	m.row[1].x * m.row[0].y * m.row[3].z * m.row[2].w +
	m.row[1].x * m.row[2].y * m.row[0].z * m.row[3].w +
	m.row[1].x * m.row[3].y * m.row[2].z * m.row[0].w +
	m.row[2].x * m.row[0].y * m.row[1].z * m.row[3].w +
	m.row[2].x * m.row[1].y * m.row[3].z * m.row[0].w +
	m.row[2].x * m.row[3].y * m.row[0].z * m.row[1].w +
	m.row[3].x * m.row[0].y * m.row[2].z * m.row[1].w +
	m.row[3].x * m.row[1].y * m.row[0].z * m.row[2].w +
	m.row[3].x * m.row[2].y * m.row[1].z * m.row[0].w +
	-m.row[0].x * m.row[1].y * m.row[3].z * m.row[2].w +
	-m.row[0].x * m.row[2].y * m.row[1].z * m.row[3].w +
	-m.row[0].x * m.row[3].y * m.row[2].z * m.row[1].w +
	-m.row[1].x * m.row[0].y * m.row[2].z * m.row[3].w + 
	-m.row[1].x * m.row[2].y * m.row[3].z * m.row[0].w +
	-m.row[1].x * m.row[3].y * m.row[0].z * m.row[2].w +
	-m.row[2].x * m.row[0].y * m.row[3].z * m.row[1].w +
	-m.row[2].x * m.row[1].y * m.row[0].z * m.row[3].w +
	-m.row[2].x * m.row[3].y * m.row[1].z * m.row[0].w +
	-m.row[3].x * m.row[0].y * m.row[1].z * m.row[2].w +
	-m.row[3].x * m.row[1].y * m.row[2].z * m.row[0].w +
	-m.row[3].x * m.row[2].y * m.row[0].z * m.row[1].w;

      float inv_d = 1.0f/d;

      float a00 =
	m.row[1].y * m.row[2].z * m.row[3].w +
	m.row[2].y * m.row[3].z * m.row[1].w + 
	m.row[3].y * m.row[1].z * m.row[2].w +
	-m.row[1].y * m.row[3].z * m.row[2].w +
	-m.row[2].y * m.row[1].z * m.row[3].w + 
	-m.row[3].y * m.row[2].z * m.row[1].w; 

      float a01 =
	m.row[1].x * m.row[3].z * m.row[2].w + 
	m.row[2].x * m.row[1].z * m.row[3].w +
	m.row[3].x * m.row[2].z * m.row[1].w +
	-m.row[1].x * m.row[2].z * m.row[3].w +
	-m.row[2].x * m.row[3].z * m.row[1].w +
	-m.row[3].x * m.row[1].z * m.row[2].w;

      float a02 =
	m.row[1].x * m.row[2].y * m.row[3].w +
	m.row[2].x * m.row[3].y * m.row[1].w + 
	m.row[3].x * m.row[1].y * m.row[2].w +
	-m.row[1].x * m.row[3].y * m.row[2].w +
	-m.row[2].x * m.row[1].y * m.row[3].w + 
	-m.row[3].x * m.row[2].y * m.row[1].w;

      float a03 =
	m.row[1].x * m.row[3].y * m.row[2].z + 
	m.row[2].x * m.row[1].y * m.row[3].z +
	m.row[3].x * m.row[2].y * m.row[1].z +
	-m.row[1].x * m.row[2].y * m.row[3].z +
	-m.row[2].x * m.row[3].y * m.row[1].z +
	-m.row[3].x * m.row[1].y * m.row[2].z; 

      float a10 = 
	m.row[0].y * m.row[3].z * m.row[2].w + 
	m.row[2].y * m.row[0].z * m.row[3].w +
	m.row[3].y * m.row[2].z * m.row[0].w +
	-m.row[0].y * m.row[2].z * m.row[3].w +
	-m.row[2].y * m.row[3].z * m.row[0].w +
	-m.row[3].y * m.row[0].z * m.row[2].w;

      float a11 =
	m.row[0].x * m.row[2].z * m.row[3].w +
	m.row[2].x * m.row[3].z * m.row[0].w + 
	m.row[3].x * m.row[0].z * m.row[2].w +
	-m.row[0].x * m.row[3].z * m.row[2].w +
	-m.row[2].x * m.row[0].z * m.row[3].w + 
	-m.row[3].x * m.row[2].z * m.row[0].w;

      float a12 =
	m.row[0].x * m.row[3].y * m.row[2].w + 
	m.row[2].x * m.row[0].y * m.row[3].w +
	m.row[3].x * m.row[2].y * m.row[0].w +
	-m.row[0].x * m.row[2].y * m.row[3].w +
	-m.row[2].x * m.row[3].y * m.row[0].w +
	-m.row[3].x * m.row[0].y * m.row[2].w;

      float a13 =
	m.row[0].x * m.row[2].y * m.row[3].z +
	m.row[2].x * m.row[3].y * m.row[0].z + 
	m.row[3].x * m.row[0].y * m.row[2].z +
	-m.row[0].x * m.row[3].y * m.row[2].z +
	-m.row[2].x * m.row[0].y * m.row[3].z + 
	-m.row[3].x * m.row[2].y * m.row[0].z;

      float a20 =
	m.row[0].y * m.row[1].z * m.row[3].w +
	m.row[1].y * m.row[3].z * m.row[0].w + 
	m.row[3].y * m.row[0].z * m.row[1].w +
	-m.row[0].y * m.row[3].z * m.row[1].w +
	-m.row[1].y * m.row[0].z * m.row[3].w + 
	-m.row[3].y * m.row[1].z * m.row[0].w;

      float a21 = 
	-m.row[0].x * m.row[1].z * m.row[3].w +
	-m.row[1].x * m.row[3].z * m.row[0].w +
	-m.row[3].x * m.row[0].z * m.row[1].w + 
	m.row[0].x * m.row[3].z * m.row[1].w + 
	m.row[1].x * m.row[0].z * m.row[3].w +
	m.row[3].x * m.row[1].z * m.row[0].w;

      float a22 = 
	m.row[0].x * m.row[1].y * m.row[3].w +
	m.row[1].x * m.row[3].y * m.row[0].w + 
	m.row[3].x * m.row[0].y * m.row[1].w +
	-m.row[0].x * m.row[3].y * m.row[1].w +
	-m.row[1].x * m.row[0].y * m.row[3].w + 
	-m.row[3].x * m.row[1].y * m.row[0].w;

      float a23 = 
	m.row[0].x * m.row[3].y * m.row[1].z + 
	m.row[1].x * m.row[0].y * m.row[3].z +
	m.row[3].x * m.row[1].y * m.row[0].z +
	-m.row[1].x * m.row[3].y * m.row[0].z +
	-m.row[3].x * m.row[0].y * m.row[1].z + 
	-m.row[0].x * m.row[1].y * m.row[3].z;

      float a30 = 
	m.row[0].y * m.row[2].z * m.row[1].w + 
	m.row[1].y * m.row[0].z * m.row[2].w +
	m.row[2].y * m.row[1].z * m.row[0].w +
	-m.row[0].y * m.row[1].z * m.row[2].w +
	-m.row[1].y * m.row[2].z * m.row[0].w +
	-m.row[2].y * m.row[0].z * m.row[1].w; 

      float a31 =
	m.row[0].x * m.row[1].z * m.row[2].w +
	m.row[1].x * m.row[2].z * m.row[0].w + 
	m.row[2].x * m.row[0].z * m.row[1].w +
	-m.row[0].x * m.row[2].z * m.row[1].w +
	-m.row[1].x * m.row[0].z * m.row[2].w + 
	-m.row[2].x * m.row[1].z * m.row[0].w;

      float a32 =
	m.row[0].x * m.row[2].y * m.row[1].w + 
	m.row[1].x * m.row[0].y * m.row[2].w +
	m.row[2].x * m.row[1].y * m.row[0].w +
	-m.row[0].x * m.row[1].y * m.row[2].w +
	-m.row[1].x * m.row[2].y * m.row[0].w +
	-m.row[2].x * m.row[0].y * m.row[1].w; 

      float a33 =
	m.row[0].x * m.row[1].y * m.row[2].z +
	m.row[1].x * m.row[2].y * m.row[0].z + 
	m.row[2].x * m.row[0].y * m.row[1].z +
	-m.row[0].x * m.row[2].y * m.row[1].z +
	-m.row[1].x * m.row[0].y * m.row[2].z + 
	-m.row[2].x * m.row[1].y * m.row[0].z;

    mat4x4 ret;
    ret.row[0] = float4(a00, a01, a02, a03)*inv_d;
    ret.row[1] = float4(a10, a11, a12, a13)*inv_d;
    ret.row[2] = float4(a20, a21, a22, a23)*inv_d;
    ret.row[3] = float4(a30, a31, a32, a33)*inv_d;

    return ret;
}


