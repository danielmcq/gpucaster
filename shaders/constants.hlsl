#ifndef _CONSTANTS_HLSL_
#define _CONSTANTS_HLSL_

#include "master_include.hlsl"

// TODO: move these into the CB.
static const float kReflectionPullbackAlongIncomingRay = 1.05;//0.95f; //TWEAK

//--------------------------------------------------------------------------------------
// Our primary constant buffer
//--------------------------------------------------------------------------------------
cbuffer cb0 : register( b0 )
{
    // **** The layout of this structure must match that in demo.h    
    // **** (...with the exception that everything is padded to float4 there)

    // Note: do NOT try to use float3 *arrays*.  Won't align correctly.

    float4x4   seed_rot_mat[SEED_ROT_MAT_COUNT];
    float4     seed_dir    [SEED_DIR_COUNT];      // Random unit-length vectors (use .xyz only).
    float4     seed_unorm  [SEED_UNORM_COUNT];    // [0..1]
    float4     seed_snorm  [SEED_SNORM_COUNT];    // [-1..1]
    uint4      seed_uint   [SEED_UINT_COUNT];     // [0..2^32-1]
    //float4     seed_quat   [SEED_QUAT_COUNT];   // Quaternions (rotations)
    
    float4     time_unorm  [TIME_UNORM_COUNT];    // [0..1]
    float4     time_snorm  [TIME_SNORM_COUNT];    // [-1..1]
    uint4      time_uint   [TIME_UINT_COUNT];     // [0..2^32-1]
    
    // lighting:
    float3 g_light_dir[MAX_LIGHTS];   // [-1..1], unit length vectors
    float4 g_light_col[MAX_LIGHTS];   // [0..1] in xyz.  .w unused.
    float4 g_light_wrap[MAX_LIGHTS];  // 4 random unorm*unorm's [biased toward 0]
    float4 g_light_unorm[MAX_LIGHTS];
    float4 g_light_spec_scale[MAX_LIGHTS];   // only .x used.  Scale is product of both lum(light_col.xyz) and this.
    uint4  g_light_spec_enabled[MAX_LIGHTS]; // only .x used
    float4 g_light_unorm_all;
    float4 g_fog_color;       // .w unused
    float4 g_fog_params;      // .x = fog thickness; .y = fog exponent
    //float4 g_light_ambo;    // 4 random unorms
    //float4 g_light_ambo2;   // 4 random unorms
    //float4 g_light_spec;    // 4 random unorms

    // camera/screen:    
    float3 ws_eye_pos;
    float3 ws_look_dir;
    float3 ws_screen_corner_UL;
    float3 ws_screen_across;
    float3 ws_screen_down;
    float3 ws_camera_x_axis;
    float3 ws_camera_y_axis;
    float3 ws_camera_z_axis;
    float4 g_image_size;  // .xy = w, h; .zw = 1/w, 1/h  
    uint4  g_mouse_info;  // .xy = client coords; .z = zoom factor (1, 2, 4...); .w = unused.
    
    // Misc. vectors:
    float4 g_dialog_coeffs[MAX_VECTOR_COEFFS];
        
    //TODO: put 'g_' in front of all these names.
    
    // Scalars:
    float  g_ws_pixel_size_at_depth_1;
    float  max_dist;
    uint   g_cur_frame;
    uint   g_main_frame;
    uint   g_refl_frame;
    uint   g_scene_seed;
    uint   g_cur_ambo_pass;
    uint   g_main_ambo_pass;
    uint   g_refl_ambo_pass;
    uint   g_supersampling;   // 1, 2, 3...
    uint   g_intersection_set;   // 0 = we're writing to main_; 1 = we're writing to refl_.
    uint   g_geom_frames_since_geom_clear;
    uint   g_viz_mode;
    float  test;
    
    // Scalar settings from dialog(s):
    int   kGeomSamplesPerFrame;
    float kGeomPercentNeighborDepthRand;
    float kNormSampleRadiusMult;
    float kNormPixelsAlongEyeRayToPullBack;
    int   kAmboRaysPerPass;
    int   kAmboSamplesPerRay;
    float kAmboSamplePlacementAlongRayPow;
    float kAmboFalloffPower;
    float kAmboPixelsAlongEyeRayToPullBack;
    float kAmboPixelsAlongNormalToJumpFwd;
    float kAmboPixelsAlongAmboRayToJumpFwd;
    int   kNoiseCubeSize;
    float kNoiseVoxelSize;    
    int   kLightUseDirectionalAmbo;
    float kLightAmboDiffuseUnLitThresh;
    float kLightAmboDiffuseFullyLitThresh;
    float kLightAmboDiffuseExponent;
    float kLightAmboSpecLo;
    float kLightAmboSpecHi;
    float kLightAmboDirScale;
    float kLightAmboDirBias;
    float kLightSpecStr;
    float kLightSpecPow;
    float kLightScale;
    float kLightSat;
    int   kTilesAcross;
    int   kTilesDown;
    int   kTilesTotal;
    float kInvTilesAcross;
    float kInvTilesDown;
    float kReflectivity;
    float kSurfaceColorAmount;
    float kFinalGamma;
};

struct FSQuadVsOut
{
    float4 pos : SV_Position;  // Screen-space position [-1..1] of pixel
    float2 uv  : TEXCOORD;     // Interesting... PS can't read 'pos', so we also put UV's here.
};

float3 ComputeViewRayDirFromUV(float2 uv)
{
    // Reconstruct a point somewhere on the virtual screen we're casting rays through.
    float3 some_ws_point_on_ray = ws_screen_corner_UL.xyz + 
                                  ws_screen_across.xyz * uv.xxx +
                                  ws_screen_down.xyz   * uv.yyy;
                                  
    #ifdef APPLY_CAMERA_ON_GPU
      // From that, build a unit-length view ray direction vector.
      float3 ray_dir = normalize(some_ws_point_on_ray);
      
      // Since, in this case, view frustum is defined in view-space,
      // apply the equivalent of the inverse view matrix here:
      ray_dir = normalize( ray_dir.x * ws_camera_x_axis.xyz +
                           ray_dir.y * ws_camera_y_axis.xyz +
                           ray_dir.z * ws_camera_z_axis.xyz );
    #else
      // From that, build a unit-length view ray direction vector.
      float3 ray_dir = normalize(some_ws_point_on_ray - ws_eye_pos.xyz);
    #endif
   
    return ray_dir;
}

float3 ComputeBouncedRayDir(float3 main_ray_dir, float3 main_norm) {
  float3 I = normalize(main_ray_dir);
  float3 N = normalize(main_norm);
  float3 R = normalize(I - N * (2 * dot(I, N)));
  return R;
}

#endif  // _CONSTANTS_HLSL_