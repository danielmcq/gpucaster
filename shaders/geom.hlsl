//--------------------------------------------------------------------------------------
// geom.hlsl
//--------------------------------------------------------------------------------------
// This set of shaders performs surface-intersection (geometry-building) - either for the
// primary ray (from the camera), or for the reflection ray.

#include "constants.hlsl"

Texture3D tex_noise3D_full_a : register( t0 );
Texture3D tex_noise3D_full_b : register( t1 );
Texture3D tex_noise3D_full_c : register( t2 );
Texture3D tex_noise3D_full_d : register( t3 );
Texture2D tex_alt_map : register( t4 );
Texture1D tex_noise1D : register( t5 );
Texture2D tex_main_depth : register( t6 );  // surface depth from prev frame
Texture2D tex_sincos  : register( t7 );  // 1D lookup tex; .x = sin, .y = cos.
Texture2D tex_noise_unorm_2d : register( t8 );
Texture3D tex_noise3D_packed_a : register( t9 );
Texture3D tex_noise3D_packed_b : register( t10 );
Texture3D tex_noise3D_packed_c : register( t11 );
Texture3D tex_noise3D_packed_d : register( t12 );
Texture2D tex_cur_depth   : register( t13 );  // depth along either main ray OR reflection ray from prev frame, depending on which we're generating.
Texture2D tex_main_norm   : register( t14 );  // normal generated @ the intersection of the primary ray.

SamplerState sam_linear_wrap   : register( s0 );
SamplerState sam_linear_clamp  : register( s1 );
SamplerState sam_nearest_wrap  : register( s2 );
SamplerState sam_nearest_clamp : register( s3 );

#include "eval.hlsl"

// Settings now hooked up to dialog (see ApplyDefaultSettings() in demo.h):
//static const int   kGeomSamplesPerFrame = 16;
//static const float kGeomPercentNeighborDepthRand = 0.02;


// Vary the types of samples we take over time.
// This results in the geometry being pretty much solved, to within a single pixel, after 16 frames! 
// Try to keep the sum of the 3 constant, for each index.
static const int kFrameShift = 2;
static const int kWorkMult = 8;//1;
static const int _kBigDistSamplesPerFrame  [8] = { 16, 12,  8,  6,   4,  2, 2, 2  };
static const int _kNeighborSamplesPerFrame [8] = {  0,  4,  8, 10,  10, 10, 7, 4  };
static const int _kSmallDistSamplesPerFrame[8] = {  0,  0,  0,  0,   2,  4, 7, 10 };


//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------

struct PsOut
{
    float  surface_depth  : SV_Target0;
};

float TryDepth(float3 start_pos, 
               float3 ray_dir, 
               float  t, 
               float  best_t)
{
    float3 ws = start_pos + (ray_dir * t);
    float  density = Eval(ws).density;
    if (density > 0 && t < best_t)
        return t;
    return best_t;
}

//--------------------------------------------------------------------------------------
// Pixel shader 
//--------------------------------------------------------------------------------------
PsOut PS(FSQuadVsOut input)
{   
    // Get the 3 core pieces of info about the ray.
    float3 ray_start;
    float3 ray_dir;
    float  prev_depth_along_ray;
    {
      const float3 main_ray_dir = ComputeViewRayDirFromUV(input.uv);
      const float  main_depth   = tex_main_depth.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).x;
      
      // Fetch previously-found surface depth.
      if (g_intersection_set == 0) 
      {
        // Use primary ray (from eye):
        ray_start = ws_eye_pos;
        ray_dir = main_ray_dir;
        prev_depth_along_ray = main_depth;
      } 
      else 
      {
        // Use reflected ray (from first surface intersection, bounced around normal):
        float3 main_norm = normalize( tex_main_norm.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).xyz );
        ray_start = ws_eye_pos + main_ray_dir * main_depth;
        ray_dir   = ComputeBouncedRayDir(main_ray_dir, main_norm);

        // Nudge reflected ray starting point back by 1/2 a pixel, along the original ray:
        float dist_from_eye = length(ray_start - ws_eye_pos);
        float ws_pixel_size_main = g_ws_pixel_size_at_depth_1 * dist_from_eye;
        ray_start -= main_ray_dir * ws_pixel_size_main * kReflectionPullbackAlongIncomingRay;    //TWEAK
        
        prev_depth_along_ray = tex_cur_depth.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).x;
      }
    }

    // Test more points along the ray, but don't bother testing anything beyond 
    // the nearest known surface depth.
    float best_t = (prev_depth_along_ray <= 0) ? max_dist : prev_depth_along_ray;
    
    int samples = 0;

    const int fr = min(g_geom_frames_since_geom_clear.x >> kFrameShift, 7);
    const int kNeighborSamplesPerFrame  = _kNeighborSamplesPerFrame[fr]  * kGeomSamplesPerFrame * kWorkMult / 16;
    const int kBigDistSamplesPerFrame   = _kBigDistSamplesPerFrame[fr]   * kGeomSamplesPerFrame * kWorkMult / 16;
    const int kSmallDistSamplesPerFrame = _kSmallDistSamplesPerFrame[fr] * kGeomSamplesPerFrame * kWorkMult / 16;

    const float2 uv_for_noise = input.uv.xy * g_image_size.xy * (1.0/(float)(NOISE_2D_SIZE));  // exactly 1:1
    const float4 rand_pixel   = tex_noise_unorm_2d.SampleLevel( sam_nearest_wrap, uv_for_noise, 0 );

    // Check near a neighbors' depth values, if they're nonzero.  
    // (For those that are zero, we'll just try a random depth.)
    const float2 delta[4] = { float2(1,0), float2(-1,0), float2(0,1), float2(0,-1) };
    LOOP_HINT for (int i=0; i<kNeighborSamplesPerFrame; i++)
    {
        const float rand_sample = time_unorm[samples % (TIME_UNORM_COUNT)].z;
        const float frand = frac(rand_sample + rand_pixel.z);
    
        int k = (i + g_cur_frame) & 3;
        float neighbor_t = tex_cur_depth.SampleLevel( sam_nearest_clamp, input.uv.xy + g_image_size.zw * delta[k], 0 ).x;
        float t;
        if (neighbor_t <= 0)
            t = frand * best_t;
        else
            //t = neighbor_t * (0.99 + 0.02*frand);           //KIV: perturb amount
            t = neighbor_t * ((1-kGeomPercentNeighborDepthRand*0.5) + kGeomPercentNeighborDepthRand*frand);
            
        best_t = TryDepth(ray_start, ray_dir, t, best_t);
        samples++;
    }

    // Try some more random depths, as well as some small refinements.
    //FIXME: this breaks horribly (for complex scenes, like quats) 
    // if we break this up into 2 separate loops.  Why?
    // (It causes the depth to be too near, where density is actually < 0, 
    //  and then the normals get borked because in GetGradientMQ() all 64 samples are < 0.
    LOOP_HINT for (int i=0; i<kBigDistSamplesPerFrame + kSmallDistSamplesPerFrame; i++)
    {
        const float rand_sample = time_unorm[samples % (TIME_UNORM_COUNT)].z;
        const float frand = frac(rand_sample + rand_pixel.z);

        float t;
        if (i < kBigDistSamplesPerFrame) 
        {
            // Try some big darts.
            t = frand;
            if (best_t < max_dist) 
                t = 1 - t*t;    //KIV: exponent
            t *= best_t;
        } 
        else 
        {
            // Try some little darts.
            if (best_t < max_dist) {
                float dist_from_eye = length(ray_start + ray_dir * best_t - ws_eye_pos);
                const float ws_pixel_size = g_ws_pixel_size_at_depth_1 * dist_from_eye;
                t = best_t - ws_pixel_size * frand;
            } else {
                t = frand * best_t;
            }
        }        
                
        best_t = TryDepth(ray_start, ray_dir, t, best_t);
        samples++;
    }

    // Did we find the surface?
    PsOut ret;
    if (best_t > 0 && best_t < max_dist)
        ret.surface_depth = best_t;
    else
        ret.surface_depth = 0;

    return ret;
}




