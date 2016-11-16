#ifndef _NOISE_HLSL_
#define _NOISE_HLSL_

#include "constants.hlsl"

// These have been moved to constants.hlsl:
//static const int   kNoiseCubeSize  = 16;   //FIXME - don't hardcode this, pass it in.
//static const float kNoiseVoxelSize = 1.0f/kNoiseCubeSize;

// smooth1(): sufficient for smooth geometry.
float  smooth1(float  t) { return (3 - 2*t)*t*t; }
float2 smooth1(float2 t) { return (3 - 2*t)*t*t; }
float3 smooth1(float3 t) { return (3 - 2*t)*t*t; }
float4 smooth1(float4 t) { return (3 - 2*t)*t*t; }

// smooth2(): sufficient for smooth geometry AND normals (i.e. lighting).
float  smooth2(float  t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float2 smooth2(float2 t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float3 smooth2(float3 t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float4 smooth2(float4 t) { return t * t * t * (t * (t * 6 - 15) + 10); }

static const float kNoiseAmpBias = 0.93f;//1.11f;  //FIXME/KIV - as this goes >1, higher freqs are biased toward higher amplitudes.  SplatCaster: was 1.13f.

// Goes from -1 to 1 and back, each time t goes from [0..1].
float PseudoSine(float t)
{
  float t2 = frac(t) * 2 - 1;
  float t3 = abs(t2);
  float t4 = smooth1(t3);
  return t4 * 2 - 1;
}

//--------------------------------------------------------------------------------------
// High-quality Sine & Cosine
//--------------------------------------------------------------------------------------
// Looks up sin and cos of an angle from a hi-precision lookup texture.
// Returns sin(angle) in .x, and cos(angle) in .y.
float2 MySinCos(float angle_radians)
{
    float t = angle_radians * 0.1591549430918;   // 1.0f/(pi*2)
    return tex_sincos.SampleLevel( sam_linear_wrap, t, 0 ).xy;
}

//--------------------------------------------------------------------------------------
// Quaternions
//--------------------------------------------------------------------------------------
float4 MakeQuaternion(float3 rot_axis_normalized, float radians)
{
    // Generate a quaternion for a rotation about the origin, around 'axis', by 'radians':
    float4 q;
    float2 sincos;// = MySinCos(radians * 0.5f);          //BROKEN on new NVIDIA GPU.   //FIXME!
    sincos.x = sin(radians * 0.5f);
    sincos.y = cos(radians * 0.5f);
    q.x = sincos.y;
    q.y = sincos.x * rot_axis_normalized.x;
    q.z = sincos.x * rot_axis_normalized.y;
    q.w = sincos.x * rot_axis_normalized.z;
  
    return q;
}

float3 RotatePointByQuaternion(float3 p1, float4 q)
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

float3 RotatePointAboutAxis(float3 rot_axis_normalized, float3 pivot, float3 ws_point, const float radians)
{
    const float4 q       = MakeQuaternion(rot_axis_normalized, radians);
    const float3 pt      = ws_point - pivot;
    const float3 rotated = RotatePointByQuaternion(pt, q);
    const float3 final   = rotated + pivot;
    return final;
}

//--------------------------------------------------------------------------------------
// Noise
//--------------------------------------------------------------------------------------

float UHQNoise_helper(Texture3D tex, float3 uvw000, float3 t, float4 scales, float e_x, float f_x)
{
    const float2 delta = float2(0, kNoiseVoxelSize);
    //TODO: why doesn't using 'nearest' work?  Wouldn't it be faster, too?
    const float3 c000 = tex.SampleLevel( sam_linear_wrap, uvw000            , 0 ).xyz;  // Gradient at (i,j,k); each item in [-1..1] range.
    const float3 c001 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.xxy, 0 ).xyz;
    const float3 c010 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.xyx, 0 ).xyz;
    const float3 c011 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.xyy, 0 ).xyz;
    const float3 c100 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.yxx, 0 ).xyz;
    const float3 c101 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.yxy, 0 ).xyz;
    const float3 c110 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.yyx, 0 ).xyz;
    const float3 c111 = tex.SampleLevel( sam_linear_wrap, uvw000 + delta.yyy, 0 ).xyz;  // Gradient at (i+1,j+1,k+1)
    const float4 proj1 = float4( dot(c000, t                ),
                                 dot(c001, t - float3(0,0,1)),
                                 dot(c010, t - float3(0,1,0)),
                                 dot(c011, t - float3(0,1,1)) );
    const float4 proj2 = float4( dot(c100, t - float3(1,0,0)),
                                 dot(c101, t - float3(1,0,1)),
                                 dot(c110, t - float3(1,1,0)),
                                 dot(c111, t - 1           ) );
    const float4 proj3 = (proj1 * f_x + proj2 * e_x) * scales;
    return dot(proj3, 1);
}

float4 UHQNoise4(float3 uvw)    // Returns values in [-1..1].
{
    const float3 uvw_magnified = uvw;
    const float3 t = frac(uvw_magnified);       // [0..1]
    const float3 uvw_real = uvw * kNoiseVoxelSize;    // Incoming 'uvw' is 1 unit per noise value... here we remap [0..1] to whole cube.
    const float3 uvw000 = uvw_real - t * kNoiseVoxelSize;
    const float3 e = smooth2(t);
    const float3 f = 1 - e;
    const float4 scales = float4(f.y, f.y, e.y, e.y) * float4(f.z, e.z, f.z, e.z);

    float4 ret;
    ret.x = UHQNoise_helper(tex_noise3D_full_a, uvw000, t, scales, e.x, f.x);
    ret.y = UHQNoise_helper(tex_noise3D_full_b, uvw000, t, scales, e.x, f.x);  //FIXME: use alt. noise cube here.
    ret.z = UHQNoise_helper(tex_noise3D_full_c, uvw000, t, scales, e.x, f.x);  //FIXME: use alt. noise cube here.
    ret.w = UHQNoise_helper(tex_noise3D_full_d, uvw000, t, scales, e.x, f.x);  //FIXME: use alt. noise cube here.
    return ret;
}




float4 HQNoise4_helper(float3 uvw000, float3 t, float4 scales, float e_x, float f_x)
{
    const float2 delta = float2(0, kNoiseVoxelSize);
    
    // Note that using sam_nearest_wrap is the same as 
    // { using sam_linear_wrap, and adding kNoiseVoxelSize*0.5 to uvw }.
    //uvw000 = (floor(uvw000 * kNoiseCubeSize) + 0.5) * kNoiseVoxelSize;
    
    const float4 c000 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000            , 0 );  // Value at (i,j,k); each item in [-1..1] range.
    const float4 c001 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.xxy, 0 );
    const float4 c010 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyx, 0 );
    const float4 c011 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyy, 0 );
    const float4 c100 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxx, 0 );
    const float4 c101 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxy, 0 );
    const float4 c110 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyx, 0 );
    const float4 c111 = tex_noise3D_full_a.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyy, 0 );  // Value at (i+1,j+1,k+1)
    const float3 g000 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000            , 0 ).xyz;  // Value at (i,j,k); each item in [-1..1] range.
    const float3 g001 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.xxy, 0 ).xyz;
    const float3 g010 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyx, 0 ).xyz;
    const float3 g011 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyy, 0 ).xyz;
    const float3 g100 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxx, 0 ).xyz;
    const float3 g101 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxy, 0 ).xyz;
    const float3 g110 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyx, 0 ).xyz;
    const float3 g111 = tex_noise3D_full_b.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyy, 0 ).xyz;  // Value at (i+1,j+1,k+1)
    const float3 e = smooth2(t);
    const float3 f = 1 - e;
    
    float4 ret = (c000 + dot(g000, t                )) * f.x * f.y * f.z 
               + (c001 + dot(g001, t - float3(0,0,1))) * f.x * f.y * e.z 
               + (c010 + dot(g010, t - float3(0,1,0))) * f.x * e.y * f.z 
               + (c011 + dot(g011, t - float3(0,1,1))) * f.x * e.y * e.z 
               + (c100 + dot(g100, t - float3(1,0,0))) * e.x * f.y * f.z 
               + (c101 + dot(g101, t - float3(1,0,1))) * e.x * f.y * e.z 
               + (c110 + dot(g110, t - float3(1,1,0))) * e.x * e.y * f.z 
               + (c111 + dot(g111, t - 1           ) ) * e.x * e.y * e.z;
    
    return ret;
}

// Input: uvw in range [0..1]x[0..1]x[0..1] (index into the 3D texture).
// Output: values in [-1..1] range.
float4 HQNoise4(float3 uvw)    // Returns values in [-1..1].
{
    const float3 uvw_magnified = uvw;
    const float3 t = frac(uvw_magnified);       // [0..1]
    const float3 uvw_real = uvw * kNoiseVoxelSize;    // Incoming 'uvw' is 1 unit per noise value... here we remap [0..1] to whole cube.
    const float3 uvw000 = uvw_real - t * kNoiseVoxelSize;
    const float3 e = smooth2(t);
    const float3 f = 1 - e;
    const float4 scales = float4(f.y, f.y, e.y, e.y) * float4(f.z, e.z, f.z, e.z);
    return HQNoise4_helper(uvw000, t, scales, e.x, f.x).xyzw;
}

float4 RegNoise4_helper(Texture3D tex, float3 uvw000, float3 t, float4 scales, float e_x, float f_x)
{
    const float2 delta = float2(0, kNoiseVoxelSize);
    
    // Note that using sam_nearest_wrap is the same as 
    // { using sam_linear_wrap, and adding kNoiseVoxelSize*0.5 to uvw }.
    //uvw000 = (floor(uvw000 * kNoiseCubeSize) + 0.5) * kNoiseVoxelSize;
    
    const float4 c000 = tex.SampleLevel( sam_nearest_wrap, uvw000            , 0 );  // Value at (i,j,k); each item in [-1..1] range.
    const float4 c001 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.xxy, 0 );
    const float4 c010 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyx, 0 );
    const float4 c011 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.xyy, 0 );
    const float4 c100 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxx, 0 );
    const float4 c101 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.yxy, 0 );
    const float4 c110 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyx, 0 );
    const float4 c111 = tex.SampleLevel( sam_nearest_wrap, uvw000 + delta.yyy, 0 );  // Value at (i+1,j+1,k+1)
    const float3 e = smooth2(t);
    const float3 f = 1 - e;
    
    float4 ret = c000 * f.x * f.y * f.z 
               + c001 * f.x * f.y * e.z 
               + c010 * f.x * e.y * f.z 
               + c011 * f.x * e.y * e.z 
               + c100 * e.x * f.y * f.z 
               + c101 * e.x * f.y * e.z 
               + c110 * e.x * e.y * f.z 
               + c111 * e.x * e.y * e.z;
    //float4 ret = ((c000 * f.z + c001 * e.z) * f.y + (c010 * f.z + c011 * e.z) * e.y) * f.x + 
    //             ((c100 * f.z + c101 * e.z) * f.y + (c110 * f.z + c111 * e.z) * e.y) * e.x;
    
    return ret;
}

// Input: uvw in range [0..1]x[0..1]x[0..1] (index into the 3D texture).
// Output: values in [-1..1] range.
float4 RegNoise4(float3 uvw)    // Returns values in [-1..1].
{
    const float3 uvw_magnified = uvw;
    const float3 t = frac(uvw_magnified);       // [0..1]
    const float3 uvw_real = uvw * kNoiseVoxelSize;    // Incoming 'uvw' is 1 unit per noise value... here we remap [0..1] to whole cube.
    const float3 uvw000 = uvw_real - t * kNoiseVoxelSize;
    const float3 e = smooth2(t);
    const float3 f = 1 - e;
    const float4 scales = float4(f.y, f.y, e.y, e.y) * float4(f.z, e.z, f.z, e.z);
    return RegNoise4_helper(tex_noise3D_full_a, uvw000, t, scales, e.x, f.x).xyzw;
}

// Input: uvw in range [0..1]x[0..1]x[0..1] (index into the 3D texture).
// Output: values in [-1..1] range.
float4 SnapNoise4(float3 uvw)
{
    return tex_noise3D_full_a.SampleLevel(sam_nearest_wrap, uvw * kNoiseVoxelSize, 0);
}
float4 SnapNoise4(float3 uvw, Texture3D tex)
{
    return tex.SampleLevel(sam_nearest_wrap, uvw * kNoiseVoxelSize, 0);
}









#define NOISE_SNAP 0    // Ultra-fast (single lookup).
#define NOISE_REG  1    // USE THIS.  Much faster noise.  Use this in most cases.  Basis shape slightly blocky, but good enough for most purposes.  8 lookups (float4 value @ each corner).
#define NOISE_HQ   2    // Quasi-Perlin noise - applies a value AND gradient @ each corner, but the 4 values all increase/decrease TOGETHER, rather than independently.  16 lookups instead of 8 (float4 value and gradient at each corner).  **NOTE - this one has higher amplitudes.**
#define NOISE_UHQ  3    // DO NOT USE.  Ultra-high-quality (full perlin) noise.  VERY slow, and has compilation issues (takes forever to compile; sometimes hangs).

#define NYQUIST_FADE
#define ROTATE_OCTAVES
#define MAX_OCTAVES 24 //16

float4 OctaveNoise(const int NOISE_TYPE,
                   const float3 ws,  // <----- THIS MUST BE THE REAL WS COORD, OTHERWISE NYQUIST LIMIT WON'T BE HONORED <-----
                   const float wavelen,       // How much distance, in world-space units, 1 texel of noise should cover.
                   const float ws_pixel_size,
                   const float _min_feature_size_pixels,
                   const bool bSmoothBasedOnLowerFreqs, 
                   const bool bCreased,
                   const int noiseIndex,    // allows each subseq. OctaveNoise call (within a single eval()) to produce different noise.
                   const int nMaxOctaves,   // = MAX_OCTAVES
                   const bool bDimply = false
                   )
{
  // TODO: make nOctaves a float, and allow partial octaves.  (Fade the last one.)

  float3 uvw_orig = ws * (1.0/wavelen);
  const float min_feature_size_pixels = _min_feature_size_pixels * 0.67f;  //KIV

  // For damping various octaves:
  float4 fRunningMult = float4(1,1,1,1);
  float4 fMult[4] = { float4(1,1,1,1), float4(1,1,1,1), float4(1,1,1,1), float4(1,1,1,1) };
  
  float4 noise_sum = 0;
  float3 uvw = uvw_orig;
  float _amp  = 1.0f;
  float _freq = 1.0f;
  
  const int N = min(nMaxOctaves, MAX_OCTAVES);
  
  LOOP_HINT for (int i=0; i<N; i++)
  {
    int randIndex = i * (noiseIndex+1) + noiseIndex*7 + 3;
    
    float amp  = _amp;// * (1 + 0.15f*seed_snorm[randIndex % SEED_SNORM_COUNT].z);  //KIV: amplitude bias!
    float freq = _freq * (1 + 0.15f*seed_snorm[randIndex % SEED_SNORM_COUNT].w);

    // Nyquist limit:
    float nyquist_fade = 1;
    #ifdef NYQUIST_FADE
        // Soft break (preferred)
        nyquist_fade = saturate( wavelen - min_feature_size_pixels * (freq * ws_pixel_size) + 0.5f);
        if (nyquist_fade <= 0)
            break;
    #else
        // Hard break
        if (wavelen < min_feature_size_pixels * freq * ws_pixel_size)
            break;
    #endif
    
    // Randomly rotate (and translate) the uvw coordinates for this octave.
    const float4x4 m = seed_rot_mat[randIndex % SEED_ROT_MAT_COUNT];
    float3 uvw;
    #ifdef ROTATE_OCTAVES
      uvw.x = dot( m[0].xyzw, float4(uvw_orig, kNoiseCubeSize) );
      uvw.y = dot( m[1].xyzw, float4(uvw_orig, kNoiseCubeSize) );
      uvw.z = dot( m[2].xyzw, float4(uvw_orig, kNoiseCubeSize) );
      //uvw.x = dot( float4(m[0].x, m[1].x, m[2].x, m[3].x), float4(uvw_orig, kNoiseCubeSize) );
      //uvw.y = dot( float4(m[0].y, m[1].y, m[2].y, m[3].y), float4(uvw_orig, kNoiseCubeSize) );
      //uvw.z = dot( float4(m[0].z, m[1].z, m[2].z, m[3].z), float4(uvw_orig, kNoiseCubeSize) );
    #else
      uvw = uvw_orig.xyz;
    #endif 
    //CHECK ROW VS. COL: amp *= saturate( m[0].w*999 );   (if you've got it right, then you should still see some noise.)
    
    // Scale them, crunch noise, scale the result, and add it to our running sum.
    float4 noise;
    if (NOISE_TYPE == NOISE_SNAP)
        noise = SnapNoise4(uvw * freq);
    else if (NOISE_TYPE == NOISE_REG)
        noise = RegNoise4(uvw * freq);
    else if (NOISE_TYPE == NOISE_HQ)
        noise = HQNoise4(uvw * freq);
    else 
        noise = UHQNoise4(uvw * freq);
        
    if (bSmoothBasedOnLowerFreqs)
    {
        #if 0
          // Nice!  @ 1.1, Naturalness is AMAZING, except it's a bit too smooth.
          // But unfortunately, cranking it up to 1.3 leaves very flat freq. distribs.
          noise *= fRunningMult;
          float4 fTerm = noise*0.5 + 0.5;  // Leave alone.
          fRunningMult *= fTerm*1.3;  // Tweak; should be > 1, < 1.5.
        #endif
        #if 0
          // Also nice.
          noise *= fRunningMult;
          float4 fTerm = noise*0.5 + 0.5;  // Leave alone.
          fRunningMult = lerp(fRunningMult*0.8, fTerm, _amp);
        #endif
        #if 0
          // This method [apply fTerm from 3 its ago] really starts to get a more
          // even distrib. of freqs!  But it's also too noisy.
          noise *= fMult[i % 3];
          float4 fTerm = noise*0.5 + 0.5;  // Leave alone.
          fMult[i % 3] = fTerm;
        #endif
        #if 0
          // This is starting to get really good, esp. when you set AmpBias to about 0.93.
          // But maybe the '3' is off?
          noise *= fMult[i % 3];
          float4 fTerm = noise*0.5 + 0.5;  // Leave alone.
          fMult[i % 3] = fTerm;
        #endif
        #if 1
          // VERY cool!!    3/3 and 4/4 seem to look best... and use 0.93 for AmpBias.
          noise *= fMult[i % 3];
          float4 fTerm = noise*0.5 + 0.5;  // Leave alone.
          fMult[i % 3] = fTerm;
        #endif
    }
    
    if (bCreased)
        noise = (0.4f - abs(noise))*2;

    if (bDimply) {
      noise = noise*0.5f + 0.5f;
      noise *= noise;
      noise -= 0.25f;
      noise *= -1.333f;
    }
    
    noise_sum += noise * (amp * nyquist_fade);
    
    _amp  *= 0.5f * kNoiseAmpBias;
    _freq *= 2.0f;
  }
  return noise_sum;
}





#endif  // _EVAL_HLSL_