#ifndef _MASTER_INCLUDE_HLSL_
#define _MASTER_INCLUDE_HLSL_

//--------------------------------------------------------------------------------------
// This file is #included by both the CODE and the shaders.
//--------------------------------------------------------------------------------------

// Uncomment this line if you want to use software rasterization (SLOW):
//#define USE_REFRAST

#ifdef USE_REFRAST
  #define FAST_LO_QUALITY_MODE          // optional
#endif 

#define LOOP_HINT [fastopt] [loop]       // Helps speed up shader compile times (from infinite to finite).  Try: [loop], [fastopt]   //KIV
//#define LOOP_HINT [fastopt]       // Helps speed up shader compile times (from infinite to finite).  Try: [loop], [fastopt]   //KIV
//#define LOOP_HINT [loop]          // Helps speed up shader compile times (from infinite to finite).  Try: [loop], [fastopt]   //KIV

// Use this.  When not using it, sometimes there is a weird camera skew bug that kicks in
// when you rotate the camera toward +Y or -Y, and the image is horizontally stretched.
#define APPLY_CAMERA_ON_GPU

// Sets of random values, generated from a random number seed:
#define SEED_ROT_MAT_COUNT  ( 40 ) // Random 3x3 rotation matrices
#define SEED_DIR_COUNT      ( 64 ) // Random unit-length vectors
#define SEED_UNORM_COUNT    ( 64 ) // [0..1]
#define SEED_SNORM_COUNT    ( 64 ) // [-1..1]
#define SEED_UINT_COUNT     ( 64 ) // [0..2^32-1]
#define SEED_QUAT_COUNT     ( 64 ) // Quaternions (rotations)
#define TIME_UNORM_COUNT    ( 64 ) // Keep this >= 64, so ambo rays all get a unique start distance.
#define TIME_SNORM_COUNT    ( 16 )
#define TIME_UINT_COUNT     ( 16 )

// Misc:
#define NOISE_2D_SIZE       ( 128 )
#define AMBO_RAY_DIRS       ( 8192 )  // Must match, or be less than, # of float3's in ambo_ray_dirs.txt.
#define NORMAL_RAY_DIRS     ( 512 )   // Must *match* what's in normal_dirs_xxx.txt
#define AMBO_RAY_DIR_FILENAME   "shaders\\ambo_ray_dirs.txt"        // Progressive distrib.
#define NORMAL_RAY_DIR_FILENAME "shaders\\normal_dirs_512.txt"  // Homogenous distrib.

// Lighting:
#define MAX_LIGHTS          ( 8 )   // DO NOT CHANGE.  Import/Export will fail.

// Derived:
//#define TILES_TOTAL  ( (TILES_X)*(TILES_Y) )
//#define INV_TILES_X  (1.0f/(float)(TILES_X))
//#define INV_TILES_Y  (1.0f/(float)(TILES_Y))

#ifdef FAST_LO_QUALITY_MODE
  #define AMBO_RAYS_PER_PASS    ( 16 )
  #define AMBO_SAMPLES_PER_RAY  ( 16 )  // Stay above 32 - otherwise, you get glowy edges (as rays go through the edge).
#else
  #define AMBO_RAYS_PER_PASS    ( 64/4 )
  #define AMBO_SAMPLES_PER_RAY  ( 32*2 )  // Stay above 32 - otherwise, you get glowy edges (as rays go through the edge).
#endif

// The values in these are driven by a dialog
// (to help prevent frequent shader recompiles).
#define MAX_VECTOR_COEFFS 10   // DO NOT CHANGE - will break file format.
#define g_c0            g_dialog_coeffs[0]
#define g_c1            g_dialog_coeffs[1]
#define g_c2            g_dialog_coeffs[2]
#define g_c3            g_dialog_coeffs[3]
#define g_c4            g_dialog_coeffs[4]
#define g_c5            g_dialog_coeffs[5]
#define g_c6            g_dialog_coeffs[6]
#define g_c7            g_dialog_coeffs[7]
#define g_c8            g_dialog_coeffs[8]
#define g_c9            g_dialog_coeffs[9]



#endif  // _MASTER_INCLUDE_HLSL_