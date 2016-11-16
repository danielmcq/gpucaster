#ifndef _QUAT_H_
#define _QUAT_H_

#include <math.h>

inline float4 MakeQuaternion(const float3& rot_axis_normalized, float radians)
{
    assert( fabsf(dot(rot_axis_normalized, rot_axis_normalized) - 1.0f) < 0.0001f ); 
    
    // generate a quaternion for a rotation about the origin,
    // around 'axis', by 'radians':
    float4 q;
    q.x = cosf(radians * 0.5f);
    q.y = sinf(radians * 0.5f) * rot_axis_normalized.x;
    q.z = sinf(radians * 0.5f) * rot_axis_normalized.y;
    q.w = sinf(radians * 0.5f) * rot_axis_normalized.z;
  
    return q;
}

inline float3 RotatePointByQuaternion(const float3& p1, const float4& q)
{
    float3 p2;
  
    const float a = q.x;
    const float b = q.y;
    const float c = q.z;
    const float d = q.w;
    const float t2 =   a*b;
    const float t3 =   a*c;
    const float t4 =   a*d;
    const float t5 =  -b*b;
    const float t6 =   b*c;
    const float t7 =   b*d;
    const float t8 =  -c*c;
    const float t9 =   c*d;
    const float t10 = -d*d;
    p2.x = 2*( (t8 + t10)*p1.x + (t6 -  t4)*p1.y + (t3 + t7)*p1.z ) + p1.x;
    p2.y = 2*( (t4 +  t6)*p1.x + (t5 + t10)*p1.y + (t9 - t2)*p1.z ) + p1.y;
    p2.z = 2*( (t7 -  t3)*p1.x + (t2 +  t9)*p1.y + (t5 + t8)*p1.z ) + p1.z;
  
    return p2;
}

#endif  // _QUAT_H_