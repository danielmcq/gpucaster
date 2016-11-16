//--------------------------------------------------------------------------------------
// ambo.hlsl
//--------------------------------------------------------------------------------------
// This set of shaders casts ambient occlusion rays, and writes out the % that were able
// to escape the scene without hitting the surface.  It writes out a value between
// [0.001 and 1.0], usually in 0.3 ... 0.5; the value 0 is reserved to mean "ambo has not
// yet been computed."

#include "constants.hlsl"

Texture3D tex_noise3D_full_a : register( t0 );
Texture3D tex_noise3D_full_b : register( t1 );
Texture3D tex_noise3D_full_c : register( t2 );
Texture3D tex_noise3D_full_d : register( t3 );
Texture2D tex_alt_map : register( t4 );
Texture1D tex_noise1D : register( t5 );
Texture2D tex_main_depth : register( t6 );  // surface depth from prev frame
Texture2D tex_main_norm  : register( t7 );
Texture2D tex_sincos  : register( t8 );  // 1D lookup tex; .x = sin, .y = cos.
Texture2D tex_noise_unorm_2d : register( t9 );
Texture1D tex_ray_dirs : register( t10 );
Texture3D tex_noise3D_packed_a : register( t11 );
Texture3D tex_noise3D_packed_b : register( t12 );
Texture3D tex_noise3D_packed_c : register( t13 );
Texture3D tex_noise3D_packed_d : register( t14 );
Texture2D tex_refl_depth   : register( t15 );  // depth along the start of the reflection ray, til intersection
Texture2D tex_refl_norm    : register( t16 );  // norm of reflection ray where it hits the surface

SamplerState sam_linear_wrap   : register( s0 );
SamplerState sam_linear_clamp  : register( s1 );
SamplerState sam_nearest_wrap  : register( s2 );
SamplerState sam_nearest_clamp : register( s3 );

#include "eval.hlsl"

//--------------------------------------------------------------------------------------
// Settings
//--------------------------------------------------------------------------------------

// Settings now hooked up to dialog (see ApplyDefaultSettings() in demo.h):
// Note: keep 'kAmboSamplesPerRay' >= 32,
//       and  'kAmboSamplePlacementAlongRayPow' >= 2.5f,
//       to avoid large glowing edges on large flag surfaces.
//static const int   kAmboRaysPerPass   = 64;
//static const int   kAmboSamplesPerRay = 32;  // Stay above 32 - otherwise, you get glowy edges (as rays go through the edge).
//static const float kAmboSamplePlacementAlongRayPow = 3;  // as this goes up, we place more of the samples nearby.  Raising this value places more emphasis on reacting to nearby occluders, and less on faraway occluders; it also shrinks 'glowiness' on big, long, flat edges.
//static const float kAmboFalloffPower = 2.5f;//1;//2.0f;   //KIV.  Bound to the perceived length of the rays (wavelen of ambo).  0.5 = very short rays; 3.0 = very long rays (makes scene look "big").
//static const float kAmboPixelsAlongEyeRayToPullBack = 0.1f;  // Because z-refine might not be 100% perfect.
//static const float kAmboPixelsAlongNormalToJumpFwd  = 0.2f;  // Get out away from the surface ever so slightly.
//static const float kAmboPixelsAlongAmboRayToJumpFwd = 0.0f;


//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------

struct PsOut
{
    float4 ambo_data : SV_Target0;    // .xyz = normal, .w = frac_escaped
};


//--------------------------------------------------------------------------------------
// CastAmboRay
//--------------------------------------------------------------------------------------

float CastAmboRay(float3 start_pt, float3 ambo_ray_dir, float ws_pixel_size, float frand)
{
    float escaped = 1;
    
    LOOP_HINT for (int i=0; i<kAmboSamplesPerRay; i++)  
    {
        // Note: using i-1 and clamping it to 0 makes sure that all rays get tested at the very start,
        // as well as at staggered points thereafter.  This prevents ugly low-freq glowing creases.
        float  t = max(0, (i - 1 + frand) * (1.0f/(kAmboSamplesPerRay)));    // [0..1]
        float  dist_along_ray = pow(t, kAmboSamplePlacementAlongRayPow) * max_dist + (kAmboPixelsAlongAmboRayToJumpFwd * ws_pixel_size);//0.04f; //TODO: use ws_pixel_size here, or better yet, move start_pt by normal * ws_pixel_size * 0.2f
        float3 ws = start_pt + (ambo_ray_dir * dist_along_ray);
        float  density = Eval(ws).density;
        
        // The GPU has issues if you try to do early-exit of any kind; 
        //   it's best to fully process all rays, but the value we return is 
        //   based solely on the distance to the *nearest* blockage.
        // *UPDATE*: using D3D10 (rather than 11) interface seems way less buggy 
        //   for shader compilation; trying this again...
        if (density > 0 && escaped >= 1) {
            escaped = pow( t * (1.0f/max_dist), kAmboFalloffPower );
            return escaped;    // <- CAREFUL!
        }
    }
    return escaped;
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
    float3 norm_at_ws;
    {
      // Fetch previously-found surface depth.
      if (g_intersection_set == 0) 
      {
        // Use primary ray (from eye):
        ray_start = ws_eye_pos;
        ray_dir = main_ray_dir;
        prev_depth_along_ray = main_depth;
        norm_at_ws = normalize( tex_main_norm.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).xyz );
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
        norm_at_ws = normalize( tex_refl_norm.SampleLevel( sam_nearest_clamp, input.uv.xy, 0 ).xyz );
      }
    }



    PsOut ret;
    ret.ambo_data = 0;
    
    if (main_depth > 0 && prev_depth_along_ray > 0)
    {
        float3 ws = ray_start + ray_dir * prev_depth_along_ray;
        float  ws_pixel_size = g_ws_pixel_size_at_depth_1 * length(ws - ws_eye_pos);
    
        ws +=   norm_at_ws * kAmboPixelsAlongNormalToJumpFwd * ws_pixel_size
              - ray_dir * (kAmboPixelsAlongEyeRayToPullBack*ws_pixel_size);
        
        float4 ambo_sum = 0;
        int rays_cast = 0;
        LOOP_HINT for (int i=0; i<kAmboRaysPerPass; i++)  //TODO: use real distribution of rays here.
        {
            // Choose a random value [0..1] that gives each ray a random starting point.
            // The value is frac(a + b), where a varies from pass to pass (actually tile to tile),
            // and b varies from pixel to pixel (repeating @ each tile, but that's fine).
            // Together, this is enough randomness to get an even distribution at every frame, pixle, and ray.
            const float rand_ray = time_unorm[i % (TIME_UNORM_COUNT)].y;
            const float2 uv_for_noise = input.uv.xy * g_image_size.xy * (1.0/(float)(NOISE_2D_SIZE));  // exactly 1:1
            const float rand_pixel = tex_noise_unorm_2d.SampleLevel( sam_nearest_wrap, uv_for_noise, 0 ).x;
            const float frand = frac(rand_ray + rand_pixel);
            
            // Fetch a ray direction.
            int j = (i + kAmboRaysPerPass*g_cur_ambo_pass);
            float3 ambo_ray_dir = normalize( tex_ray_dirs.SampleLevel( sam_nearest_wrap, 
                                                                       j * (1.0f/(float)(AMBO_RAY_DIRS)),
                                                                       0
                                                                     ).xyz );
            
            // Cast the ray.                                                         
            float pure_ambo = CastAmboRay(ws, ambo_ray_dir, ws_pixel_size, frand);
            
            // Accumulate the results.
            ambo_sum.xyzw += float4(ambo_ray_dir, 1) * pure_ambo;
            rays_cast++;
        }
        
        float4 ambo_avg = ambo_sum / (float)rays_cast;
        ambo_avg.w = max(0.001f, ambo_avg.w);
        
        ret.ambo_data = ambo_avg;
    }
    
    return ret;
}