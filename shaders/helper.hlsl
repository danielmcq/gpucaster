#ifndef _HELPER_HLSL_
#define _HELPER_HLSL_

#define cosf cos
#define sinf sin
#define tanf tan
#define powf pow

static const float golden_ratio = 1.6180339887498948482045868343656;

// The center points of the 12 pentagon-shaped faces of a dodecahedron:
// (These are also the *vertices* of an icosahedron.)
static const float3 dodec[12] = {
  normalize(float3(0,  1,  golden_ratio)), 
  normalize(float3(0, -1,  golden_ratio)), 
  normalize(float3(0,  1, -golden_ratio)), 
  normalize(float3(0, -1, -golden_ratio)), 
  normalize(float3( 1,  golden_ratio, 0)), 
  normalize(float3(-1,  golden_ratio, 0)), 
  normalize(float3( 1, -golden_ratio, 0)), 
  normalize(float3(-1, -golden_ratio, 0)), 
  normalize(float3( golden_ratio, 0,  1)), 
  normalize(float3( golden_ratio, 0, -1)), 
  normalize(float3(-golden_ratio, 0,  1)), 
  normalize(float3(-golden_ratio, 0, -1)), 
};

// The center points of the 20 triangle-shaped faces of an icosahedron:
static const float3 icosa[20] = {
  // should these be normalized?
  normalize(float3( 1,  1,  1)), 
  normalize(float3( 1,  1, -1)), 
  normalize(float3( 1, -1,  1)), 
  normalize(float3( 1, -1, -1)), 
  normalize(float3(-1,  1,  1)), 
  normalize(float3(-1,  1, -1)), 
  normalize(float3(-1, -1,  1)), 
  normalize(float3(-1, -1, -1)), 
  normalize(float3(0,  golden_ratio,  1/golden_ratio)), 
  normalize(float3(0, -golden_ratio,  1/golden_ratio)), 
  normalize(float3(0,  golden_ratio, -1/golden_ratio)), 
  normalize(float3(0, -golden_ratio, -1/golden_ratio)), 
  normalize(float3( golden_ratio,  1/golden_ratio, 0)), 
  normalize(float3(-golden_ratio,  1/golden_ratio, 0)), 
  normalize(float3( golden_ratio, -1/golden_ratio, 0)), 
  normalize(float3(-golden_ratio, -1/golden_ratio, 0)), 
  normalize(float3( 1/golden_ratio, 0,  golden_ratio)), 
  normalize(float3( 1/golden_ratio, 0, -golden_ratio)), 
  normalize(float3(-1/golden_ratio, 0,  golden_ratio)), 
  normalize(float3(-1/golden_ratio, 0, -golden_ratio)), 
};

// Input: t in [0..1]
// Output: an L2 smooth interpolation from [0..1].  (Lighting will look good.)
float SmoothL1(float t) {
  t = saturate(t);
  return t * t * (3 - 2 * t);
}

// Input: t in [0..1]
// Output: an L2 smooth interpolation from [0..1].  (Lighting will look good.)
float SmoothL2(float t) {
  t = saturate(t);
  return t * t * t * (t * (t * 6 - 15) + 10);
}

// These perform rotation ONLY - they skip the translation (if there is any in the matrix).
float3 RotatePointByMatrix(float3 a, float4x4 m) {
  return float3(dot(m[0].xyz, a), dot(m[1].xyz, a), dot(m[2].xyz, a));
};
float3 RotatePointByMatrix(float3 a, float3x4 m) {
  return float3(dot(m[0].xyz, a), dot(m[1].xyz, a), dot(m[2].xyz, a));
};
float3 RotatePointByMatrix(float3 a, float4x3 m) {
  return float3(dot(m[0].xyz, a), dot(m[1].xyz, a), dot(m[2].xyz, a));
};
float3 RotatePointByMatrix(float3 a, float3x3 m) {
  return float3(dot(m[0].xyz, a), dot(m[1].xyz, a), dot(m[2].xyz, a));
};

// Reflects a point about a plan with the given normal that intersects the origin.
float3 ReflectAboutPlane(float3 p, float3 N) {
  float t = dot(p, N);
  if (t < 0) {
    return p - 2 * t * N;
  } else {
    return p;
  }
}


#endif  // _HELPER_HLSL_