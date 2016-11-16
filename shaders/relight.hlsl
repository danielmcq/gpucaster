//--------------------------------------------------------------------------------------
// relight.hlsl
//--------------------------------------------------------------------------------------
// This set of shaders performs surface-intersection (geometry-building) 
// and generates surface normals.

Texture2D tex_main_depth : register( t0 );
Texture2D tex_main_norm  : register( t1 );
Texture2D tex_main_ambo  : register( t2 );
  // For testing:
  Texture3D tex_noise3D_full_a : register( t3 );
  Texture3D tex_noise3D_full_b : register( t4 );
  Texture3D tex_noise3D_full_c : register( t5 );
  Texture3D tex_noise3D_full_d : register( t6 );
  Texture2D tex_alt_map : register( t7 );
  Texture1D tex_noise1D : register( t8 );
  Texture2D tex_sincos  : register( t9 );  // 1D lookup tex; .x = sin, .y = cos.
  Texture3D tex_noise3D_packed_a : register( t10 );
  Texture3D tex_noise3D_packed_b : register( t11 );
  Texture3D tex_noise3D_packed_c : register( t12 );
  Texture3D tex_noise3D_packed_d : register( t13 );
Texture2D tex_refl_depth : register( t14 );
Texture2D tex_refl_norm  : register( t15 );
Texture2D tex_refl_ambo  : register( t16 );
//TextureCube tex_sky : register( t17 );

SamplerState sam_linear_wrap   : register( s0 );
SamplerState sam_linear_clamp  : register( s1 );
SamplerState sam_nearest_wrap  : register( s2 );
SamplerState sam_nearest_clamp : register( s3 );

#include "constants.hlsl"
#include "eval.hlsl"
#include "noise.hlsl"

float3 DoFog(float3 pre_fog_surface_color, float depth) 
{
  float fog_depth = ((depth<=0) ? max_dist : depth) * g_fog_params.x;
  float fog_str = saturate(pow(fog_depth, g_fog_params.y));
  return lerp(pre_fog_surface_color, g_fog_color.xyz, fog_str);
}

float3 DoLighting(float3 ws, 
                  float3 incoming_refl_light,
                  EvalResult evalResult,
                  float depth, 
                  float3 N,
                  float3 ambo_dir,   // direction (might be 0)
                  float  ambo_frac,  // fraction escaped (~0.5 = bright)
                  float spec_scale,  // based on normal curvature (N.w)
                  int _p    // 0 = reflection ray landing spot, 1 = primary ray landing spot.
                  )   
{
    // TODO: load the constant buffer up with scene lighting info,
    // and do real lighting here.
    
    // TODO: try point lights w/falloff now?
    
    float3 lit_color = 0;
    
    if (depth > 0)// && depth < max_dist) 
    {
        const int num_lights = min(5, MAX_LIGHTS);    // DO NOT EXCEED 'MAX_LIGHTS'.
        
        float ambo_diffuse = saturate( (ambo_frac - kLightAmboDiffuseUnLitThresh.x) * (1.0f/(kLightAmboDiffuseFullyLitThresh.x - kLightAmboDiffuseUnLitThresh.x)) );
        ambo_diffuse = ambo_diffuse*ambo_diffuse*(3 - 2*ambo_diffuse);
        ambo_diffuse = pow(ambo_diffuse, kLightAmboDiffuseExponent.x) * 0.8f;
       
        float ambo_spec = saturate( (ambo_frac - kLightAmboSpecLo.x) * (1.0f/(kLightAmboSpecHi.x - kLightAmboSpecLo.x)) );
        ambo_spec = ambo_spec*ambo_spec*(3 - 2*ambo_spec);
            
        // Diffuse light:
        for (int i=0; i<num_lights; i++)
        {
            const float3 L = -g_light_dir[i].xyz;
            float orig_str = dot(N, L);
            if (length(ambo_dir) > 0.01 && kLightUseDirectionalAmbo > 0)
            {
                float cone_str = dot(ambo_dir, L) + (ambo_frac*2 - 1);
                orig_str *= saturate(cone_str * kLightAmboDirScale + kLightAmboDirBias);   //KIV - was 4, 0.1
            }
            float wrap = g_light_wrap[i].x;
            const float wrap_str = lerp(orig_str, orig_str*0.5f + 0.5f, wrap);   // light wrap                  //TWEAK
            lit_color += g_light_col[i].xyz * max(0, wrap_str) * ambo_diffuse;
        }
        
        // Apply surface color.
        //float3 t = (length(ws - float3(0,5,5))*11) * (1.0 + 0.6*g_light_unorm_all.xyz) * 2;
        float3 surface_color = lerp(1, evalResult.surface_color, kSurfaceColorAmount); //1 + float3(cos(t.x), cos(t.y), cos(t.z))*0.15;   
        lit_color *= surface_color;
        
        // Add spec light.
        for (i=0; i<num_lights; i++)
        {
            if (g_light_spec_enabled[i].x > 0)
            {
                const float light_lum = (g_light_col[i].x + g_light_col[i].y + g_light_col[i].z) * 0.33333 * g_light_spec_scale[i].x;
                const float3 V = normalize(ws_eye_pos - ws);
                float d = dot(N, V);
                if (d > 0)        //FIXME
                {
                    float3 R = normalize( N*d*2 - V );
                    float d2 = -dot(R, g_light_dir[i].xyz);
                    if (d2 > 0)
                    {
                        float spec_pow = pow(2, kLightSpecPow);
                        float this_spec_str = pow( d2, spec_pow );
                        lit_color += kLightSpecStr * this_spec_str * light_lum * ambo_spec * spec_scale;
                    }
                }
            }
        }           
        
        // Add reflected light (pre-fog).
        lit_color += incoming_refl_light;
        
        // Apply water.
        //if (ws.y < 1.55)
        //  lit_color = lerp(lit_color, float3(0.05, 0.12, 0.3), 0.76f);
    }
            
    // Apply fog.
    lit_color = DoFog(lit_color, depth);
    
    return lit_color;
}

//--------------------------------------------------------------------------------------
// 'Relight' Pixel shader 
//--------------------------------------------------------------------------------------
float4 PS(FSQuadVsOut input) : SV_Target
{   
    float3 final_color = 0;

    //const float max_refl_dist = 0.4;  //TWEAK

    // uv (0,0) corresponds to upper-left corner of *screen*.
    float2 uv = input.uv;

    // Mouse zoom:
    if (g_mouse_info.z > 1)
    {
        float2 uv_mouse = g_mouse_info.xy /** g_supersampling*/ * g_image_size.zw;
        float dist = length(uv_mouse - uv);
        if (dist < 0.5f)
            uv = (uv - uv_mouse) * (1.0f/(float)g_mouse_info.z) + uv_mouse;
    }

    const float3 main_ray_dir = ComputeViewRayDirFromUV(uv);

    const float  orig_main_depth = tex_main_depth.SampleLevel( sam_nearest_clamp, uv, 0 ).x;
    const float4 orig_main_norm  = tex_main_norm.SampleLevel( sam_nearest_clamp, uv, 0 );
    const float4 orig_main_ambo  = tex_main_ambo.SampleLevel( sam_nearest_clamp, uv, 0 );
    const float  orig_refl_depth = tex_refl_depth.SampleLevel( sam_nearest_clamp, uv, 0 ).x;
    const float4 orig_refl_norm  = tex_refl_norm.SampleLevel( sam_nearest_clamp, uv, 0 );
    const float4 orig_refl_ambo  = tex_refl_ambo.SampleLevel( sam_nearest_clamp, uv, 0 );

    bool apply_final_brightness_and_sat = true;

    // p==0: 2nd ray intersection pt (...where the reflection ray landed).
    // p==1: primary ray intersection pt (...where the primary ray landed).
    for (int p=0; p<2; p++)    
    {
        float3 lit_color = 0;
        
        float orig_depth;
        float4 orig_norm;
        float4 orig_ambo;
        int ambo_pass;
        if (p == 0) {
          // Reflected ray.
          orig_depth = orig_refl_depth;
          orig_norm  = orig_refl_norm ;
          orig_ambo  = orig_refl_ambo ;
          ambo_pass  = g_refl_ambo_pass;
        } else {
          // Main ray.
          orig_depth = orig_main_depth;
          orig_norm  = orig_main_norm ;
          orig_ambo  = orig_main_ambo ;
          ambo_pass  = g_main_ambo_pass;
        }
        
        const float3 surface_normal = normalize(orig_norm.xyz);
        // 'spec_and_refl_scale': this keeps corners from reflecting light
        //   (both specular & ray reflections) so that they don't cause
        //   single-pixel bright spots on corners/edges/creases.
        //   Based on orig_norm.w, which stores the % of the normal samples
        //   (on a sphere) that were in vs. out.
        float  spec_and_refl_scale = 1;//1 - saturate( (abs(orig_norm.w - 0.5) - 0.03 )* 999 ); //TWEAK
        const int    ambo_passes = ambo_pass;//(int)(length(orig_ambo.xyz) + 0.95f);  // 1.0, 1.95, 2.9, 3.88, 4.87...
        const float3 ambo_dir  = normalize(orig_ambo.xyz);
        const float  ambo_frac = orig_ambo.w * (1.0f/(float)(ambo_passes));
        
        // Fetch previously-found surface depth.
        float3 ray_start;
        float3 ray_dir;
        float  prev_depth_along_ray;
        if (p == 0) 
        {
          // Use reflected ray (from first surface intersection, bounced around normal):
          ray_start = ws_eye_pos + main_ray_dir * orig_main_depth;
          ray_dir   = ComputeBouncedRayDir(main_ray_dir, orig_main_norm.xyz);

          // Nudge reflected ray starting point back by 1/2 a pixel, along the original ray:
          float dist_from_eye = length(ray_start - ws_eye_pos);
          float ws_pixel_size_main = g_ws_pixel_size_at_depth_1 * dist_from_eye;
          ray_start -= main_ray_dir * ws_pixel_size_main * kReflectionPullbackAlongIncomingRay;    //TWEAK
          
          prev_depth_along_ray = orig_refl_depth;
        } 
        else 
        {
          // Use primary ray (from eye):
          ray_start = ws_eye_pos;
          ray_dir = main_ray_dir;
          prev_depth_along_ray = orig_main_depth;
        }
        float3 ws = ray_start + ray_dir * prev_depth_along_ray;
                    
        // Apply ambo - IF it's there.
        float temp_ambo = (orig_ambo.w > 0) ? ambo_frac : 0.5;
        
        // Evaluate various properties at this ws location.
        const float ws_pixel_size = length(ws_eye_pos - ws) * g_ws_pixel_size_at_depth_1;
        EvalResult evalResult = Eval(ws - surface_normal*ws_pixel_size*0.02);
        

//xxx//FIXME...xxx;
//final_color = tex_sky.SampleLevel( sam_linear_clamp, orig_refl_norm, 0 ).xyz;
        /*
        if (p == 0 && orig_depth <= 0.001) 
        {
          final_color = tex_sky.SampleLevel( sam_linear_clamp, orig_refl_norm, 0 ).xyz;
          orig_depth = 0.001;   // So that fog doesn't clobber it, in DoLighting().
        }
        */
        
       
        if (p==1) 
        {
            // Tone down prev. reflection by reflectivity @ this ws point.
            // (Have to do this pre-fog).
            final_color *= evalResult.reflectivity * kReflectivity * spec_and_refl_scale;
        }

        final_color = DoLighting(ws, final_color, evalResult, orig_depth, surface_normal, ambo_dir, temp_ambo, spec_and_refl_scale, p);
        
        // p==0, g_intersection_set==1 : reflected ray color.
        // p==1, g_intersection_set==0 : main ray color.
        if (g_intersection_set == 1-p)
        {
            // viz modes:                     
            // 0 = regular (final lit color)  
            // 1 = pink depth                 
            // 2 = striped depth
            // 3 = normals                    
            // 4 = striped normals            
            // 5 = normal.w (frac escaped; should never be white or black)
            // 6 = ambo.w (raw)               
            // 7 = ambo_dir
            // 8 = (ambo_dir - surface_normal)
            // (Later: surface color w/ or w/o ambo (but unlit).)
            //                           0  1  2  3  4  5  6  7  8  9
            const int remap_no_N[16] = { 1, 1, 2, 1, 1, 1, 6, 7, 1, 9, 10, 11, 12, 13, 14, 15 };

            int viz_mode = g_viz_mode;
            if (orig_depth > 0 && abs(length(orig_norm.xyz)-1) > 0.1)
              viz_mode = remap_no_N[viz_mode];
        
            if (g_viz_mode > 0 || viz_mode != g_viz_mode)
            {
                // VIZ:
                if (viz_mode == 0)    // normal
                  final_color = final_color;
                else if (viz_mode == 1)  // pink depth
                {
                  float t = orig_depth * (1.0f/max_dist);
                  final_color.x = pow(t, 0.25);
                  final_color.y = t;
                  final_color.z = sqrt(t);
                }
                else if (viz_mode == 2)  // striped depth
                  //final_color.xyz = abs(frac(orig_depth * float3(8, 0.5, 2)) * 2 - 1);
                  final_color.xyz = frac(orig_depth * float3(8, 0.5, 2) * 50);
                else if (viz_mode == 3)  // normals
                  final_color.xyz = 0.5 + 0.5*orig_norm.xyz;
                else if (viz_mode == 4)  // striped normals
                  final_color.xyz = frac(orig_norm.xyz * 20);
                else if (viz_mode == 5)  // surface normal % escaped
                  final_color = orig_norm.w;
                else if (viz_mode == 6)  // reflectivity
                  final_color = evalResult.reflectivity;
                //else if (viz_mode == 6)  // ambo frac
                //  final_color = ambo_frac*1.333f;
                //else if (viz_mode == 7)  // ambo_dir
                //  final_color = ambo_dir*0.5 + 0.5;
                //else if (viz_mode == 8)  // ambo_dir vs. surface_normal
                //  final_color = 0.5 + (ambo_dir - surface_normal)*1;

                apply_final_brightness_and_sat = false;
            }
        }
    }
    
    if (apply_final_brightness_and_sat) 
    {
        // apply final brightness & saturation:
        float lum = (final_color.x + final_color.y + final_color.z) * 0.33333;
        final_color = lerp(lum.xxx, final_color, kLightSat.xxx);
        final_color *= kLightScale.xxx;
    }
    
    // Gamma correction:
    final_color = pow(final_color, kFinalGamma.xxx);
      
    return float4(saturate(final_color), 1);
}




