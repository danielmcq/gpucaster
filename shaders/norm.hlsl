//--------------------------------------------------------------------------------------
// norm.hlsl
//--------------------------------------------------------------------------------------
// This set of shaders generates surface normals - for either the initial intersection
// (from the camera), or for the reflection ray hitting the surface - and writes them 
// out to a texture.

#include "constants.hlsl"

Texture3D tex_noise3D_full_a : register( t0 );
Texture3D tex_noise3D_full_b : register( t1 );
Texture3D tex_noise3D_full_c : register( t2 );
Texture3D tex_noise3D_full_d : register( t3 );
Texture2D tex_alt_map : register( t4 );
Texture1D tex_noise1D : register( t5 );
Texture2D tex_main_depth   : register( t6 );  // surface depth from prev frame
Texture2D tex_sincos  : register( t7 );  // 1D lookup tex; .x = sin, .y = cos.
Texture3D tex_noise3D_packed_a : register( t8 );
Texture3D tex_noise3D_packed_b : register( t9 );
Texture3D tex_noise3D_packed_c : register( t10 );
Texture3D tex_noise3D_packed_d : register( t11 );
Texture2D tex_refl_depth   : register( t12 );  // depth along the start of the reflection ray, til intersection
Texture2D tex_main_norm    : register( t13 );  // normal from main ray (iff generating refl normals)
Texture1D tex_normal_ray_dirs : register( t14 );

SamplerState sam_linear_wrap   : register( s0 );
SamplerState sam_linear_clamp  : register( s1 );
SamplerState sam_nearest_wrap  : register( s2 );
SamplerState sam_nearest_clamp : register( s3 );

#include "eval.hlsl"

// Settings now hooked up to dialog (see ApplyDefaultSettings() in demo.h):
static const int kNormRefinementSteps = 12;             //KIV

//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------

struct PsOut
{
    float4 surface_normal : SV_Target0;
};

//--------------------------------------------------------------------------------------
// Misc.
//--------------------------------------------------------------------------------------

//#include "poisson_homogenous_512.hlsl"

//--------------------------------------------------------------------------------------
// GetSurfaceNormalLQ()
//--------------------------------------------------------------------------------------
//float4 GetSurfaceNormalLQ(float3 ws, float ws_pixel_size)
//{
//    float delta = ws_pixel_size * 0.5f * kNormSampleRadiusMult;
//    float3 N;
//    N.x = Eval( ws + float3(delta, 0, 0) ).density - Eval( ws - float3(delta, 0, 0) ).density;
//    N.y = Eval( ws + float3(0, delta, 0) ).density - Eval( ws - float3(0, delta, 0) ).density;
//    N.z = Eval( ws + float3(0, 0, delta) ).density - Eval( ws - float3(0, 0, delta) ).density;
//    return float4(normalize(N), 0.5f);
//}


//--------------------------------------------------------------------------------------
// GradHelper() - helper function for GetSurfaceNormalHQ_old()
//--------------------------------------------------------------------------------------
// returns a vector in the plane made by the two vectors, N and T,
// that is parallel to the surface, at the point ws.
float3 GradHelper(const float3 N, const float3 T, const float3 ws, const float ws_pixel_size)
{
    // note: DOUBLE_SPEED_HQ_NORMALS only works for curved surfaces if rad is super-tiny, and if z-precision is very high.
    //... maybe if we can flip signs of v1 and v2 to be more consistent, DOUBLE_SPEED_HQ_NORMALS will work?
    const float rad = ws_pixel_size*0.5f * kNormSampleRadiusMult;

    //#define DOUBLE_SPEED_HQ_NORMALS  //KIV

    float3 intersection_pt[2];
    #ifdef DOUBLE_SPEED_HQ_NORMALS
        int side = 0;
    #else
        LOOP_HINT for (int side=0; side<2; side++)     
    #endif
    {
        float lo = 0;
        float hi = 3.1415926535898f;
        const float T_sign = (side==0) ? 1.0f : -1.0f;
        LOOP_HINT for (int it=0; it<kNormRefinementSteps; it++)
        {
            float mid = (lo+hi)*0.5f;
            float2 sin_cos = MySinCos(mid);
            float3 pt = ws + N*(rad*-sin_cos.y) + T*(sin_cos.x*rad*T_sign);
            float density = Eval(pt).density;
            if (density > 0)                
                lo = mid;
            else
                hi = mid;
        }
        float mid = (lo+hi)*0.5f;
        float2 sin_cos = MySinCos(mid);
        intersection_pt[side] = ws + N*(rad*-sin_cos.y) + T*(sin_cos.x*rad*T_sign);
    }

    // Do normalize here, in case the vectors are getting too tiny to store in float.
    #ifdef DOUBLE_SPEED_HQ_NORMALS
        float3 v = (ws - intersection_pt[0]); 
    #else
        float3 v = (intersection_pt[1] - intersection_pt[0]);  
    #endif

    return normalize(v);
}





//--------------------------------------------------------------------------------------
// GetSurfaceNormalMQ()
//--------------------------------------------------------------------------------------
// Takes 64 samples out at 1/2 a pixel's radius; averages the direction
//   of the samples that are 'escaped' (in air, not rock).
// Also returns the fraction escaped in .w, for debugging purposes.
//float4 GetSurfaceNormalMQ(float3 ws, float ws_pixel_size)
//{
//    const float rad = ws_pixel_size*0.5f * kNormSampleRadiusMult;
//    
//    int escaped_count = 0;
//    float3 average_escape_ray = float3(0,0,0);
//    LOOP_HINT for (int i=0; i<64; i++)
//    {
//        float3 dir = normalize(g_poisson_64[i]);
//        float3 ws2 = ws + (dir * rad);
//        float density = Eval(ws2).density;
//        if (density <= 0) {
//            escaped_count++;
//            average_escape_ray += dir;
//        }
//    }
//    return float4(normalize(average_escape_ray), escaped_count/64.0f);
//}



//--------------------------------------------------------------------------------------
// GetSurfaceNormalHQ()
//--------------------------------------------------------------------------------------
// Takes 512 samples out at 1/2 a pixel's radius; averages the direction
//   of the samples that are 'escaped' (in air, not rock).
// Also returns the fraction escaped in .w, for debugging purposes.
//TODO: see if there is a faster way to do this.  (Low-pri)
//TODO: once dialog is in, let user choose # of rays - 256, 512, or 1024.
float4 GetSurfaceNormalHQ(float3 ws, float ws_pixel_size)
{
    float rad = ws_pixel_size*0.5f * kNormSampleRadiusMult;
    
    int escaped_count = 0;
    float3 average_escape_ray = float3(0,0,0);
    LOOP_HINT for (int i=0; i<NORMAL_RAY_DIRS; i++)
    {
        float3 dir = normalize( tex_normal_ray_dirs.SampleLevel( sam_nearest_wrap, i * (1.0f/(float)(NORMAL_RAY_DIRS)), 0).xyz );
        float3 ws2 = ws + (dir * rad);
        float density = Eval(ws2).density;
        if (density <= 0) {
            escaped_count++;
            average_escape_ray += dir;
        }
    }
    float3 N         = normalize(average_escape_ray);
    float  ambo_frac = escaped_count/(float)(NORMAL_RAY_DIRS);
            
    return float4(N, ambo_frac);
}

//--------------------------------------------------------------------------------------
// GetSurfaceNormalHQ_old()
//--------------------------------------------------------------------------------------
// First, samples the normal using medium-quality algorithm
// then refines using interval halving along two perpendicular semicircular arcs.
// This method seems to work fine for very smooth functions, but for quickly-changing
// geometries (like those using quaternions), it falls short, and at ss>1, you
// get big errors. 
//float4 GetSurfaceNormalHQ_old(float3 ws, float ws_pixel_size)
//{
//    // DO NOT CALL LQ HERE; it's not a good enough initial predictor,
//    // and when it's off by > 90 degrees, the HQ normal comes out borked,
//    // and you get sparkles.
//    float4 mq_data = GetSurfaceNormalHQ(ws, ws_pixel_size);
//    const float3 orig_grad = mq_data.xyz;
//    
//    float3 grad = orig_grad;
//    {
//        float3 v1 =                      cross(grad, float3(0,1,0));
//        if (abs(grad.x) < 0.57735f) v1 = cross(grad, float3(1,0,0));
//        if (abs(grad.z) < 0.57735f) v1 = cross(grad, float3(0,0,1));
//        v1 = normalize(v1);
//        float3 v2 = normalize(cross(v1, grad));
//        
//        // 3. cross the two vectors, and you have the refined normal.
//        // 1. refine within the plane formed by N and v1;
//        //    get a vector in that plane that is parallel to the surface.
//        float3 v3 = GradHelper(grad, normalize(v1), ws, ws_pixel_size);
//
//        // 2. refine within the plane formed by N and v2;
//        //    get a vector in that plane that is parallel to the surface.
//        float3 v4 = GradHelper(grad, normalize(v2), ws, ws_pixel_size);
//  
//        grad = -normalize(cross(v3, v4));
//    }
//  
//    return float4(grad, mq_data.w);
//}

float4 GetSurfaceNormalHQ_refined(float3 ws, float ws_pixel_size)
{
    float4 mq_data = GetSurfaceNormalHQ(ws, ws_pixel_size);
    const float3 orig_grad = mq_data.xyz;
    
    float3 grad = orig_grad;

    // Refine.  This is pretty much essential for reflection, otherwise reflections are ultra-fuzzy.
    const float3 s = float3( grad.x > 0 ? 1 : -1,
                             grad.y > 0 ? 1 : -1,
                             grad.z > 0 ? 1 : -1 );
    
    //OPTIMIZE: since we're doing a hard select below, we really only need to call
    // GradHelper twice above, instead of 3 times, and vary the inputs.

    const float3 M = abs(grad);
    const float fmax = max(M.x, max(M.y, M.z));
    const float fmin = min(M.x, min(M.y, M.z));
    
    float3 du[2];
    float3 dv[2];
    float sign[2];
    
    if (M.z >= fmax) {
      // Valid except in XY plane.
      du[0] = float3(0,s.y,0);
      dv[0] = float3(0,0,1);
      du[1] = float3(0,0,s.z);
      dv[1] = float3(1,0,0);
      sign[0] = s.y;
      sign[1] = s.z;
    } else if (M.x >= fmax) {
      // Valid except in YZ plane.
      du[0] = float3(0,0,s.z);
      dv[0] = float3(1,0,0);
      du[1] = float3(s.x,0,0);
      dv[1] = float3(0,1,0);
      sign[0] = s.z;
      sign[1] = s.x;
    } else {
      // Valid except in XZ plane.
      du[0] = float3(s.x,0,0);
      dv[0] = float3(0,1,0);
      du[1] = float3(0,s.y,0);
      dv[1] = float3(0,0,1);
      sign[0] = s.x;
      sign[1] = s.y;
    }
    
    // Only run GradHelper() twice, because it's expensive:
    float3 z[2];
    LOOP_HINT for (int i=0; i<2; i++) 
    {
        z[i] = GradHelper(du[i], dv[i], ws, ws_pixel_size) * sign[i];
    }

    if (abs(length(z[0]) - 1) < 0.01 && 
        abs(length(z[1]) - 1) < 0.01 &&
        abs(dot(z[0],z[1])) < 0.99
        )
    {
      float3 new_grad = normalize(cross(z[0], z[1])) * sign[1]; //[sic]
      if (abs(length(new_grad) - 1) < 0.01)
      {
        grad = new_grad;
      }
    }
    
    return float4(grad, mq_data.w);
}


//--------------------------------------------------------------------------------------
// Pixel shader 
//--------------------------------------------------------------------------------------
PsOut PS(FSQuadVsOut input)
{   
    const float3 main_ray_dir = ComputeViewRayDirFromUV(input.uv);
    const float  main_depth   = tex_main_depth.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).x;

    // Get the 3 core pieces of info about the ray.
    float3 ray_start;
    float3 ray_dir;
    float  prev_depth_along_ray;
    {
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
        
        prev_depth_along_ray = tex_refl_depth.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).x;
      }
    }
    
    PsOut ret;
    ret.surface_normal = 0;
    
    if (main_depth > 0 && prev_depth_along_ray > 0) 
    {
      float3 ws = ray_start + ray_dir * prev_depth_along_ray;
      float  ws_pixel_size = g_ws_pixel_size_at_depth_1 * length(ws - ws_eye_pos);
      ws -= ray_dir * (kNormPixelsAlongEyeRayToPullBack * ws_pixel_size);
      
      float4 data = GetSurfaceNormalHQ_refined(ws, ws_pixel_size);
    
      ret.surface_normal.xyz = data.xyz;
      ret.surface_normal.w   = data.w;    // debugging info
    }
            
    return ret;
}




