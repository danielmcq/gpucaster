#ifndef _CODE_GPUCASTER_DEMO_H_
#define _CODE_GPUCASTER_DEMO_H_

#include "stdafx.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <time.h>

#include "float3.h"
//#include "matrix.h"
#include "quaternion.h"
#include "../shaders/master_include.hlsl"
#include "config.h"
#include "resource.h"
#include "get_string.h"
#include "MersenneTwister.h"  // From http://www-personal.umich.edu/~wagnerr/MersenneTwister.html

#include <commctrl.h>  // for TBM_SETRANGE, etc.
#include <hash_map>
#include <process.h>

#define SAFE_RELEASE(x) { if (x) (x)->Release(); x = NULL; }
#define SAFE_DELETE(x) { delete x; x = NULL; }
//#define FRAND ((rand()%1317)/1316.0f)
//#define NRAND ((rand()) ^ (rand() << 20) ^ (rand() << 10))      // rand() returns 15 bits.
#define DRAND (g_rand.rand())    // double
#define FRAND ((float)g_rand.rand())    // float
#define NRAND (g_rand.randInt())    // 32 bits
inline float saturate(float t) { return min(1.0f, max(0.0f, t)); }  // Do not use #define! - breaks with rand() params.
#define IsALight(x) (x >= 0 && x < MAX_LIGHTS)

#define LOOKAT_DISTANCE 1

/*
TODO:
    -the weird color bug (w/fog) on +/- keys only happens WHEN IN REFL MODE; main mode is fine.  
        And it only happensz in "final color" viz mode.
    -ADD HDR GLOW
    
    -speed up normal shader compilation time (bottleneck) - try flags?
        -don't order GPU til you can figure this out!  70 seconds - yeeeesh...
    -try: D3DCOMPILE_WARNINGS_ARE_ERRORS
    -(for large images, geom pass: get more neighbor-sharing)

    -return struct from eval():
        d-density    1
        d-color      3
        -spec mult  1
        -refl mult  1

    -scene:
        -add 1d color ramp ~ height
        -try to get more swirlies on the pre-warp, when you zoom in.  (like burls)

    -noise:
        -kNoiseAmpBias 1 -> 0.9, much better; but also try freq-smoothing.
**          -> freq-smoothing looks decent; keep trying.
            -> last one looking good at 4/4; try 5/5, 4/5, 6/6.  (7/7 is way too much.)
        -todo: hook up eval1/2/3 params in dialog.  *Should they save?*

    -Refl wrap-up:
        -reflections (and spec) sucky @ sharp corners... 
            -> KIV: SET kNormSampleRadiusMult TO 0.5!!!!! (in demo.h)
            -> relight.hlsl: tweak coeffs for 'spec_and_refl_scale'
            -> but still not good enough... hmm... (have to rely on downsample @ end?)
        -try to remove 'max_refl_dist' from relight.hlsl / or add to panel.

        -REFLECTIONS @ A DISTANCE ARE NOISY, due to imperfect normals.  KEEP CURRENT SCENE
            until this is fixed.  Only shows @ ss:5.
        -kiv: kReflectionPullbackAlongIncomingRay: can it be 0.5?

        -geom: try mandelblox, blobs... and add sharp noise!
        -why do 'dodecagonal cauliflower' normals show up black now?
        -play with kReflectionPullbackAlongIncomingRay, in constants.h.
        -red tide: AHA: it's fog.  Turn fog off and it goes away.
        -(make neighbor depth hinting a bit more liberal, for refl case)

        -delay on 'x' key til end of pass
        -hook refl % up to KB / dialog
        -restore ambo viz modes; autopilot
        -chrome sky for escaped rays
    -ADD ABILITY TO LOAD A SEED, CAMERA, LIGHTS, ETC.


    -next:
        -for release:
          -(kiv: changed ambo to float4_32)
          -try dx10, ps4, and full optimizations
          -allow window resize / aspect ratio change?
          -clean up eval.hlsl
          -write up usage instructions
          -

        -ambo is now 1/2-speed... :(
        -noise:
            1. try making 2 noise cubes:  RandGradNoise, and RealGradNoise.  Then try RealGradNoise w/c==1.
            2. make N noise cubes.  
##      2. fix noise intra-octave dampening. [d?]
#       3. fix ambo_dir - looks very noisy @ ss=7.  Maybe need to scale vectors down for 'fuzzy' (far) blockages?
#       4. fix normals - on plain sphere, they look like crap.
    -back on Radeon:
        -geometry ideas:
            -honeycomb: put some space between them.
            -honeycomb: make each one at a random angle, as well as random height.  (basalt)
            -try packed cylinders, cut @ random angles &| heights.
            -
            -
        -tune ambo_dir until it looks as good as non-directional.
        -play w/SSS (kAmbo...).
    -low-pri: add 4 float4's to CB for surface color; add rand button to Lighting dialog.
#   -TEST FULL IMPORT from a screenshot.  
      -import: bools for: lights, camera, seed; and a bool for lights in ES or WS.
        1. b1: update camera or not
        2. b2: update lights or not
        3. b3: import lights in WS or in ES
    -fog
    -light#s:
        d-currently, keys 1-8 map to lights 0-7 (but they show in dlg as 1-8),
            and 0 maps to camera.
        d-todo: make '9' rotate all lights together.
        -wrap looks good... if it generally looks good w/high-quality scenes,
            increase default amounts.
        -also, might look best if both spec params are biased low (exponent & str).
    -describe each viz mode
    -lighting dialog:
        d-add import/export
        ok-keys don't pass through  :(
            -> der - b/c controls eat them.
        ok-'edit color' will block... hmm...
        ok-draw little spheres :)
        d-kiv: InitCommonControls()?
        d-kiv: IsDialogMessage() for modeless dialog box.
##  -faster screenshots: 1) keep CPU-side buffer allocated, and 2) block while we copy it to CPU,
              then kick off a 2nd thread to save it to disk (and resume main thread).

    -Next:
        -
        -someday: try to get TGA RLE compression working again.  Code should be working; does Irfanview just not read it?
        -(copy back to 'webpage\code': )
            -float3.h  (with .Normalize(), .dot(), .abs(), .cross() all gone, thank god)
            -back up GenPtsOnSphere -- +viz, +nice ordering

        -dialogs:
            3. kBlahBlah settings (geom, norm, ambo, noise) - 1 value per line.
                -key to bring it up, + Apply, OK, and Cancel buttons.
                    + another key to toggle low vs. high quality (will blindly write a few of these values).
                -kiv: kAmboSamplesPerRay 32 -> 64
                -add # of tiles?  (run experiment first)
                -LATER: put defaults in a text file?
            4. LATER: geom: torus, sphere, etc. - later
            5. LATER: control dialog (g/n/a/ESC modes; clear; reload shaders; save shot)
                -> SKIP FOR NOW.  KB is just fine.

        -our multi-freq noise looks like shit.  KIV: kNoiseAmpBias; method.
##      -at ss:6, when screen blanks, it doesn't always come back!
            -either prevent the blanking (and sleep!), or properly restore the objects (yes - for post-sleep!).
#       -go to 4:3 default aspect ratio (...in between 1:1 and 16:9)
#       -big renders (ss:6) have major normal malfunctions!
            -NormalMQ() is *fine*.
            -GetSurfaceNormalHQ_temp() is in there for now; 512-spl version of MQ.
#           -use 256 or 512?
            -> put it in a texture?  (nah... 512 is pretty small, can stay in shader.)
            -
        -what's up with little black streaks?  (quat scenes)
            -normal.w (% escaped in MQ part) looks fine.
            -aha, but normal.xyz is the culprit!
            -norm.hlsl: dropping kSampleRadiusMult from 1 to 0.5 helps, but, can't go much lower.
            -
        -small issues:
            -fix A-esc-Z (goes super dark)
            -fix A-esc-A (comes back mid-tile)
            -fix A-esc-L-A.
            -does making tiny tiles let it be responsive, when ss==3+?
        -add fog!
        -further tune usage of ambo_dir
        -ambo: 
#           -do raytracing?  (fuzzy reflections & shadows; 2,3 bounces; sss; etc.)
#           -forget ambo; do real lighting - we can!  (store str of 4 lights in ambo_tex.xyzw)
                -if you randomize light *dirs*, just recompute ambo.
                -but you can just go to lo-res, get some good dirs, switch to hi res, render 4-ch light map, and then randomize colors.
                /-option 2: use ambo as brightness, and lerp COLOR from among light sources.  But how does ambo_dir play in?
            -add fog
            -[wrap the diffuse light!  things like really dumb w/o it.  or better yet: bake diffuse light into a cube map.]
            -check ray distributions
            -kiv: tune kAmboFalloffPower
.       -reload shaders is slow -> ask which one!
        -fix ultra-high-pri DX11 thread when rendering w/supersampling
        -nyquist limit:
            -for folds: add
.           -for noise: soft version works fine for geom/normals, but BREAKS AMBO.
        -finish ambo:
            d-don't count it 100% if ray hits something @ great distance.  Look to CPU code for reference.
            -fix offsets (use normals / fix 1.5-pixel setback)
            -generate new & better ambo ray distributions.  Don't trust the ones in there!
            -To fix big jump distances:  Feed normals into ambo.hlsl.  Don't generate ambo unless a normal is there.  
                -Then adjust kPixelsAlongNormalToJumpFwd, kPixelsAlongAmboRayToJumpFwd. [ambo.hlsl]
                -See what CPU version does...
            -make it DIRECTIONAL
            -ultimately, add noise to each ray.
                -ADD pixel_noise.xyzw, and RANDOMIZE each ray within its tiny patch of "sky"!  (No, wait - that's probably not statistically sound...)
        -lighting:
            -add spec! / real lighting
            -add lighting control dialog & keys
            -use ambo_dir
        -noise:
            -try 1D perlin noise (slower, but will repeat way less)
            -
        -
        -[try, instead of ambo, just baking 8 light dirs, 8 bits each?]
    -content:
        -improve frequency-smoothing in noise
        -quaternion-folded plane
        -mandelbulb
----------------------------
   -rotate camera using *quaternions*
   -rotate lights
   -screenshots:
      -@ end of current pass
      -dump the CB, too
      -zip up shaders, code, and exe!!!!!
   -improve noise
      -multiple noise volumes
      -multiple cubes using different sets of rotmats
      d-is mat4x4 transposed?
#     -damp octave by prev. octave(s).
#     -nyquist limit!!!
      -
      -after camera is in, go to & stop @ nyquist limit!
      -add 'nearest' mode; add rot90-only mode.
      -play w/octaves; make libraries for doing many octaves, ROTATION, etc. & stop at ws_pixel_size_at_this_depth.
      -mips? -> DON'T NEED if we manually decide when to stop octaves!
   -(fix exec. dir, so 'code' dir is clean.  (stick under bin/; copy in *.hlsl; debug version has 'd' on end.))
   -chop rot mats to 4x3
   -HQ normals [once we get eval() functions with discontinuities].
   -clean up typedefs, mat4x4, etc
   d-add simple ambo (1-channel, fp16, 1-pass) for plane/sphere scene.
      d-additive fp16 blend works!!!  (ambo will have to be progressive; D3D will crap out even on a 40x40 if it takes > 1 second.  This way, we can do it without double-buffering!)
      -finish progressive:
          d-relight shader: make it aware of which ambo pass we're on, and divide accordingly.
>         -ambo rays: create a set of about 4,096 (fits!) so we can go & go & go.  That'll be enough, combined with a bit of jitter each pass through, to make infinite variety.
              -also try:
                  1. first 64: spread out evenly, and lock them.
                  2. double the size; spread new ones out evenly.
                  3. and so on.
              -make sure that they are sorted randomly.
              -view them in excel, w/transparency, to make sure they're not overlapping.
          -why are we still seeing so much banding?  After 128 rays (2 passes) it should look *Very* good!  Is it because we're used to randomness?
          -why are we seeing artifacts on sphere?  Is ray starting too close, in rare cases?
      -then make it 4-channel.
      -also, be able to vary not just the gen_mode_, but also, the g_viz_mode (depth1, depth2, normals, ambo, lit)

#  -add real camera, so that ws_pixel_size_at_this_depth works, then use it to stop noise octaves @ nyquist limit.
      -add to constants: real camera info, real light info, real [large] set of rich seed-driven random values,
       and 32 rotation matrices.
   -separate normals out (?) from depth gen (again)... but be careful... 
      that fp16 drift (from storage) might be weird,
      AND watch out - for some reason, the shader was buggy before, 
      and had tears along the Noise3D lattice lines, if you took >3 evals.
   -or try real shadows... or fuzzy shadows, with lots of rays. 

   -figure out, ultimately, how we'll combine fast previewing with good images. [OK]
      1. geom: 
          -64x64 squares; do one, then display.
          -for each pixel in the square, do, say, 32 samples, then generate QUICK normals (+6), and draw.
          -once all squares are done, repeat them, refining the samples & re-generating the normals.
          -do this until geom is 100% perfect.  (Normals will be cheapo, still, though.)
          -(note that we can vary the # of samples per pixel (up from 32), to get more geom work & less normal work.)
      2. HQ normals & ambo
          -do it on 64x64 squares, then display.
          -quality low/med/high (# of normal iterations + # of ambo rays (16/64/256/1024)

   -relight.hlsl: do real/detailed lighting (scene light data, fog, ambo, etc.)
   -add real camera, and use mouse to move it.
   -normals:
      -base radius on ws_pixel_size_at_depth 
      -drop in HQ formula
   -add two float buffers, and make the rendering progressive.
   -add real camera, and mouse to drive it.
   -add ReloadShaders() capability.
   -add window resizing.
   -add 'real' normal computations from before (not just cheap gradient).
   -kiv: SetLowThreadPriority
   -try to get cheaper tricubic noise.  Grr... notes on where I was w/the NV method:
            -nv paper: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter20.html
                  [gave up b/c there was way too much noise/error, when you zoomed in - plus, mipmaps are challenging.]
                  1. should 1D texture do [0..1) or [0..1]?
                  2. should we use WRAP on it?  Probably not... probably want to frac the input, and then clamp.
                  3. are r/g/b/a mixed up, in the 1D texture?
                  4. need mip levels for both the 1D and 3D textures!
            -other slides: http://developer.amd.com/assets/Tatarchuk-Noise(GDC07-D3D_Day).pdf
            -try replacing smoothstep2() with a texture lookup.  Make it 3-channel to do 3 at once.
                  But be careful: might not be faster, and might introduce artifacts!


   -add other mouse modes:
      1. camera rotation / pan  (...with re-splatting)
      2. show a zoomed-in version @ mouse
      3. light rotation

   -consider adding 64-bit configurations.
   x-create mipmaps for noise textures, using D3DX11FilterTexture.
   -use a 'real' random number generator, to seed the 3D noise textures.
   -use DXUT to set friendly names for all shaders, renderstates, etc.
   -r = reload shaders - slooow... 
      -wait until end of pass to do it (?)
      -reload individual PS's?
   -float3: fix .Normalize(), .Abs(), etc.
   ---------------< finishing touches only, beyond this point >-----------------
   -mem:
        4 depth0
        4 depth1
        8 float4_16 normal        -> save 4 bytes?
        8 float4_16 ambo          -> save 6 bytes?
        4 rgba final color buffer
        --
        28 bytes/pix = 32 MPix = 5600*5600 (with 0.9 GB) <current>
        24 bytes/pix = 38 MPix = 6100*6100 (with 0.9 GB) <very likely HQ>
        22 bytes/pix = 40 MPix = 6300*6300 (with 0.9 GB)
        18 bytes/pix = 50 MPix = 7000*7000 (with 0.9 GB)
        ...or if really fast, just automate the tiling.

        -ambo_dir: ditch this to save 6 bytes
        -normals:  1. store data in a UINT32: 15 bits/x, 15 bits/y, 1/bit sign of Z, and then reconstruct Z.  :)  <BEST>
                   2. or try DXGI_FORMAT_R10G10B10A2_UNORM (ideal)
                   3. or try DXGI_FORMAT_R11G11B10_FLOAT (meg)
*/

// Choose one:
//#define MY_D3D_DRIVER_TYPE  D3D_DRIVER_TYPE_HARDWARE
//#define MY_D3D_DRIVER_TYPE  D3D_DRIVER_TYPE_WARP      // KIV
//#define MY_D3D_DRIVER_TYPE  D3D_DRIVER_TYPE_REFERENCE       // actually slightly *faster* on my X61!

// Choose one:
//#define MY_FEATURE_LEVEL  D3D_FEATURE_LEVEL_11_0    // KIV
//#define MY_FEATURE_LEVEL  D3D_FEATURE_LEVEL_10_1
//#define MY_FEATURE_LEVEL  D3D_FEATURE_LEVEL_10_0

#ifdef USE_REFRAST
  // X61 laptop w/crappy Intel GPU
  #define MY_D3D_DRIVER_TYPE  D3D_DRIVER_TYPE_REFERENCE       // actually slightly *faster* on my X61!
  #define MY_FEATURE_LEVEL    D3D_FEATURE_LEVEL_10_0
  #define VS_VERSION "vs_4_0"
  #define PS_VERSION "ps_4_0"
  const DXGI_FORMAT  kDepthTexStorageFmt  = DXGI_FORMAT_R16_FLOAT; //FIXME + '//PACKDEPTH' x2
#else
  // Radeon HD 5770:  ($140 as of 12/2010)  (10,400 in 3DMark Vantage) (800 shader units)
  #define MY_D3D_DRIVER_TYPE  D3D_DRIVER_TYPE_HARDWARE  //D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE
  //#define MY_FEATURE_LEVEL    D3D_FEATURE_LEVEL_11_0    //D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_11_0
  //#define VS_VERSION "vs_5_0"
  //#define PS_VERSION "ps_5_0"
  #define MY_FEATURE_LEVEL    D3D_FEATURE_LEVEL_10_0    //D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_11_0
  //#define MY_FEATURE_LEVEL    D3D_FEATURE_LEVEL_10_1    //D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_11_0
  #define VS_VERSION "vs_4_0"
  #define PS_VERSION "ps_4_0"
  
  // DO NOT CHANGE; this buffer must be 32-bit so we can reuse it for 
  // the final (RGBA/8888) render, for screenshots.
  const DXGI_FORMAT  kDepthTexStorageFmt  = DXGI_FORMAT_R32_FLOAT; //DON'T CHANGE!
#endif

// Radeon 6970: (you can buy a 6950 and sw-upgrade it!)
// 23,000 3dmark vantage -- and 2 GB!!

const DXGI_FORMAT  kNormalTexStorageFmt = DXGI_FORMAT_R16G16B16A16_SNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT;  SNORM preferred because these are unit length... why waste bits storing magnitude (exponent)?
const DXGI_FORMAT  kAmboTexStorageFmt   = DXGI_FORMAT_R16G16B16A16_FLOAT; //safe?  //DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R16G16B16A16_FLOAT;
const DXGI_FORMAT  kFinalColorTexFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
const DXGI_FORMAT  k3DNoiseTexFmt = DXGI_FORMAT_R8G8B8A8_SNORM; //Highly recommend DXGI_FORMAT_R8G8B8A8_SNORM because trilinear interp is done at full fp32; but if you really need it, DXGI_FORMAT_R16G16B16A16_SNORM works great too.  Avoid actual fp16, as it's not as good of a format for this purpose, AND we don't know how to pack those values.
const int          k3DNoiseSize   = 16;
const int          kNumNoiseCubes = 8;
const DXGI_FORMAT  k1DNoiseTexFmt = DXGI_FORMAT_R16G16B16A16_UNORM;
const int          k1DNoiseLength = 1024;  // Small enough to stay in cache, but big enough to have a rich variety of #s.
const DXGI_FORMAT  kSinCosLookupTexFmt = DXGI_FORMAT_R16G16_SNORM;
const int          kSinCosLookupLength = 2048;
const DXGI_FORMAT  kNoise2DTexFmt = DXGI_FORMAT_R16G16B16A16_UNORM;
const DXGI_FORMAT  kAmboRayDirTexFmt = DXGI_FORMAT_R32G32B32A32_FLOAT;
const int          kAmboRayDirCount  = AMBO_RAY_DIRS;
const DXGI_FORMAT  kNormalRayDirTexFmt = DXGI_FORMAT_R32G32B32A32_FLOAT;
const int          kNormalRayDirCount  = NORMAL_RAY_DIRS;
const int          kMaxAmboPasses = AMBO_RAY_DIRS / AMBO_RAYS_PER_PASS;  //128;  // Good to limit this, otherwise fp16 accumulation starts to suffer from decimation.
// flags & compilation times:
//   -normal:                          20.7 / 30.6 / 77.0
//   -skip_opt + no ieee_strictness -> 18.8 / 29.1 / 70.6
//   -


/*
FIXME1; // see here!: http://groups.google.com/group/angleproject/browse_thread/thread/d337de31d1b3cf24
     // but... we are calling tex.SampleLevel(..., 0) everywhere already.  Hm.
Try using D3DCOMPILE_XXX shader flags instead -- http://msdn.microsoft.com/en-us/library/windows/desktop/gg615083(v=vs.85).aspx
  D3DCOMPILE_PREFER_FLOW_CONTROL
  D3DCOMPILE_SKIP_OPTIMIZATION
  D3DCOMPILE_WARNINGS_ARE_ERRORS
  D3DCOMPILE_OPTIMIZATION_LEVEL0..3
  D3DCOMPILE_NO_PRESHADER
  D3DCOMPILE_IEEE_STRICTNESS
  D3DCOMPILE_ENABLE_STRICTNESS

FIXME2 - fix & use sky cubemap.
*/

#if 0
  const unsigned int kShaderOptimizationFlag =   // choose one:
                                    D3DCOMPILE_NO_PRESHADER |
                                    D3DCOMPILE_SKIP_OPTIMIZATION |  // Turned this on b/c quat-torus ambo borked w/o it - even if you don't use sin!
                                    //D3DCOMPILE_OPTIMIZATION_LEVEL0 |  // Ice planet ambo breaks.  Lowest optimization level, but fast to compile.
                                    //D3DCOMPILE_OPTIMIZATION_LEVEL1 |  // Ice planet ambo breaks.
                                    //D3DCOMPILE_OPTIMIZATION_LEVEL2 |  // Ice planet ambo breaks.
                                    //D3DCOMPILE_OPTIMIZATION_LEVEL3 |  // Highest optimization level, but most buggy.
                                    0;
  const unsigned int kShaderFlags = kShaderOptimizationFlag | 
                                    D3DCOMPILE_WARNINGS_ARE_ERRORS |
                                    D3DCOMPILE_IEEE_STRICTNESS | 
                                    D3DCOMPILE_PREFER_FLOW_CONTROL;  // TODO: try turning strictness off, down the road, on a good scene.
#else
  const unsigned int kShaderOptimizationFlag =   // choose one:
                                    //0;   //FIXME
                                    D3D10_SHADER_SKIP_OPTIMIZATION;   // Turned this on b/c quat-torus ambo borked w/o it - even if you don't use sin!
                                    //D3D10_SHADER_OPTIMIZATION_LEVEL0;   // Ice planet ambo breaks.  Lowest optimization level, but fast to compile.
                                    //D3D10_SHADER_OPTIMIZATION_LEVEL1;   // Ice planet ambo breaks.
                                    //D3D10_SHADER_OPTIMIZATION_LEVEL2;   // Ice planet ambo breaks.
                                    //D3D10_SHADER_OPTIMIZATION_LEVEL3;   // Highest optimization level, but most buggy.
  const unsigned int kShaderFlags = kShaderOptimizationFlag | 
                                    //D3D10_SHADER_WARNINGS_ARE_ERRORS | 
                                    D3D10_SHADER_IEEE_STRICTNESS | 
                                    D3D10_SHADER_PREFER_FLOW_CONTROL;  // TODO: try turning strictness off, down the road, on a good scene.
#endif
const int          kInitialSuperSample = 1;
const float        kFOVDegrees = 50.0f;
const float        kMaxDist = 30.0f;
const DXGI_SWAP_CHAIN_FLAG kSwapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    //KIV.  Allows resizing backbuf later.

namespace gpucaster {

extern MTRand g_rand;  // Mersenne Twister random # generator.

// Typedefs
typedef unsigned __int32 uint;
//typedef float mat4x4[4][4];
class mat4x4 {
 public:
  //FIXME: are these transposed??
  float4 col[4];

  float3 TransformPnt(float3 pt)
  {
    float3 ret;
    ret.x = col[0].x*pt.x + col[1].x*pt.y + col[2].x*pt.z + col[3].x;
    ret.y = col[0].y*pt.x + col[1].y*pt.y + col[2].y*pt.z + col[3].y;
    ret.z = col[0].z*pt.x + col[1].z*pt.y + col[2].z*pt.z + col[3].z;
    return ret;
  }
};

struct MinMax {
  float min;
  float max;
};

class CameraInfo
{
public:
    //      +y ^     +z (into image)
    //         |   /
    //         | /
    // LHCS:   +----->  +x
    float3 pos;
    float3 ws_x_axis;    // right
    float3 ws_y_axis;    // up
    float3 ws_z_axis;    // camera looks down this axis.
    float  fov;   // radians, of the longer axis of the window.
    float  aspect;  // width/height

    // Rotations about the local axes.
    void RotateOnX(float radians) {
      float4 q = MakeQuaternion(ws_x_axis, radians);
      ws_y_axis = RotatePointByQuaternion(ws_y_axis, q);
      ws_z_axis = RotatePointByQuaternion(ws_z_axis, q);
      ws_y_axis = normalize(cross(ws_z_axis, ws_x_axis));
      ws_z_axis = normalize(cross(ws_x_axis, ws_y_axis));
    }
    void RotateOnY(float radians) {
      float4 q = MakeQuaternion(ws_y_axis, radians);
      ws_z_axis = RotatePointByQuaternion(ws_z_axis, q);
      ws_x_axis = RotatePointByQuaternion(ws_x_axis, q);
      ws_z_axis = normalize(cross(ws_x_axis, ws_y_axis));
      ws_x_axis = normalize(cross(ws_y_axis, ws_z_axis));
    }
    void RotateOnZ(float radians) {
      float4 q = MakeQuaternion(ws_z_axis, radians);
      ws_x_axis = RotatePointByQuaternion(ws_x_axis, q);
      ws_y_axis = RotatePointByQuaternion(ws_y_axis, q);
      ws_x_axis = normalize(cross(ws_y_axis, ws_z_axis));
      ws_y_axis = normalize(cross(ws_z_axis, ws_x_axis));
    }

    // Rotations about the world-space axes.
    void RotateOnWSX(float radians) {
      float4 q = MakeQuaternion(float3(1,0,0), radians);
      ws_y_axis = RotatePointByQuaternion(ws_y_axis, q);
      ws_z_axis = RotatePointByQuaternion(ws_z_axis, q);
      ws_y_axis = normalize(cross(ws_z_axis, ws_x_axis));
      ws_z_axis = normalize(cross(ws_x_axis, ws_y_axis));
    }
    void RotateOnWSY(float radians) {
      float4 q = MakeQuaternion(float3(0,1,0), radians);
      ws_z_axis = RotatePointByQuaternion(ws_z_axis, q);
      ws_x_axis = RotatePointByQuaternion(ws_x_axis, q);
      ws_z_axis = normalize(cross(ws_x_axis, ws_y_axis));
      ws_x_axis = normalize(cross(ws_y_axis, ws_z_axis));
    }
    void RotateOnWSZ(float radians) {
      float4 q = MakeQuaternion(float3(0,0,1), radians);
      ws_x_axis = RotatePointByQuaternion(ws_x_axis, q);
      ws_y_axis = RotatePointByQuaternion(ws_y_axis, q);
      ws_x_axis = normalize(cross(ws_y_axis, ws_z_axis));
      ws_y_axis = normalize(cross(ws_z_axis, ws_x_axis));
    }

};

// Helper functions.
HRESULT LoadShader(ID3D11Device* pDevice, LPCTSTR shader_filename, LPCSTR function_name, LPCSTR shader_profile, UINT shader_flags, void** pShader);
void RandomRot(mat4x4& m);

#define NUM_VIZ_MODES ( 7 )
static const char g_vizmode_names[NUM_VIZ_MODES][64] = {
  "final color",
  "depth",
  "frac(depth*N)",
  "normals",
  "frac(normal*N)",
  "ambo % rays escaped",
  "reflectivity"
};


enum GenMode {
  GENERATE_GEOM = 0,
  GENERATE_NORMALS = 1,
  GENERATE_AMBO = 2,
  GENERATE_NONE = 3
};
static const char g_mode_names[][64] = {
  "geom",
  "normals",
  "ambo",
  "idle",
  "????",
  "????",
  "????",
  "????",
};

// Clear flags:
#define CLEAR_GEOM 1
#define CLEAR_NORM 2
#define CLEAR_AMBO 4

class uint4 {
 public:
  uint4() : x(0), y(0), z(0), w(0) { }
  uint4(unsigned __int32 x, unsigned __int32 y, unsigned __int32 z, unsigned __int32 w) : x(x), y(y), z(z), w(w) { }
  unsigned __int32 x, y, z, w;
};

class shader_constants
{
public:
  // ****** THIS STRUCT MUST MATCH THE LAYOUT IN 'constants.hlsl' ******
  // ****** THIS STRUCT MUST MATCH THE LAYOUT IN 'constants.hlsl' ******
  // ****** THIS STRUCT MUST MATCH THE LAYOUT IN 'constants.hlsl' ******
  shader_constants() { }

  // Note: on the CPU side, to match GPU memory layout, 
  // all members here must be float4 or float,
  // and all 'float' values must go at end.

  // Note: do NOT try to use float3 *arrays*.  Won't align correctly.

  // FLOAT3 AND BIGGER:
  mat4x4  seed_rot_mat[SEED_ROT_MAT_COUNT];  // Random 3x3 rotation matrices
  float4  seed_dir    [SEED_DIR_COUNT];      // Random unit-length vectors
  float4  seed_unorm  [SEED_UNORM_COUNT];    // [0..1]
  float4  seed_snorm  [SEED_SNORM_COUNT];    // [-1..1]
  uint4   seed_uint   [SEED_UINT_COUNT];     // [0..2^32-1]
  //float4  seed_quat   [SEED_QUAT_COUNT];   // Quaternions (rotations)
  float4  time_unorm  [TIME_UNORM_COUNT];    // [0..1]
  float4  time_snorm  [TIME_SNORM_COUNT];    // [-1..1]
  uint4   time_uint   [TIME_UINT_COUNT];     // [0..2^32-1]

  // Lights:
  float4 g_light_dir[MAX_LIGHTS];
  float4 g_light_col[MAX_LIGHTS];   //[0..1] in xyz; lum in .w
  float4 g_light_wrap[MAX_LIGHTS];  // 4 random unorm*unorm's [biased toward 0]
  float4 g_light_unorm[MAX_LIGHTS];
  float4 g_light_spec_scale[MAX_LIGHTS];
  uint4  g_light_spec_enabled[MAX_LIGHTS];
  float4 g_light_unorm_all;
  float4 g_fog_color;       // .w unused
  float4 g_fog_params;      // .x = fog thickness; .y = fog exponent
  //float4 g_light_ambo;
  //float4 g_light_ambo2;
  //float4 g_light_spec;

  // Camera/screen:
  float4 ws_eye_pos;
  float4 ws_look_dir;
  float4 ws_screen_corner_UL;
  float4 ws_screen_across;
  float4 ws_screen_down;
  float4 ws_camera_x_axis;
  float4 ws_camera_y_axis;
  float4 ws_camera_z_axis;
  float4 g_image_size;  // .xy = w, h; .zw = 1/w, 1/h
  uint4  g_mouse_info;  // .xy = client coords; .z = zoom factor (1, 2, 4...); .w = unused.

  // Misc. vectors:
  float4 g_dialog_coeffs[MAX_VECTOR_COEFFS];

  // SCALARS:
  float g_ws_pixel_size_at_depth_1;
  float max_dist;
  uint  g_cur_frame;
  uint  g_main_frame;
  uint  g_refl_frame;
  uint  g_scene_seed;
  uint  g_cur_ambo_pass;
  uint  g_main_ambo_pass;
  uint  g_refl_ambo_pass;
  uint  g_supersampling;   // 1, 2, 3...
  uint  g_intersection_set;   // 0 = we're writing to main_; 1 = we're writing to refl_.
  uint  g_geom_frames_since_geom_clear;
  uint  g_viz_mode;
  float test;

  // Scalar settings:
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

  // 4. FINAL PADDING:
  float4 __extra_padding__; //DO NOT REMOVE.

  void RandomizeFogColor()
  {
    g_fog_color = float4(FRAND, FRAND, FRAND, 0);
  }

  void RandomizeFog()
  {
    RandomizeFogColor();
    
    // Density:
    g_fog_params.x = FRAND*FRAND*FRAND;
    if (FRAND < 0.5f)
      g_fog_params.x = 0;

    // Exponent:
    g_fog_params.y = powf(2.0f, FRAND*4 - 2);

    g_fog_params.z = 0;
    g_fog_params.w = 0;
  }

  void SetLightDir(int i, float3 light_dir)
  {
    g_light_dir[i] = float4(light_dir, 0);
  }

  void RandomizeLightColor(int i) {
    if (!IsALight(i))
      return;
    g_light_col[i] = float4(FRAND, FRAND, FRAND, 0);
  }

  void RandomizeLight(int i, float str, float3 light_dir)
  {
    if (!IsALight(i))
      return;

    // Takes in new 'light_dir', and generates rest of params.
    g_light_dir[i] = float4(light_dir, 0);

    RandomizeLightColor(i);
    g_light_wrap[i] = float4(FRAND*FRAND, 0, 0, 0);
    g_light_spec_scale[i] = float4(FRAND + FRAND, 0, 0, 0);
    g_light_spec_enabled[i] = uint4(1, 0, 0, 0);
    g_light_unorm[i] = float4(FRAND, FRAND, FRAND, FRAND);

    // Desaturate
    float lum = (g_light_col[i].x + g_light_col[i].y + g_light_col[i].z)*0.33333f;
    float desat = FRAND*FRAND;
    g_light_col[i] = float4( lerp( g_light_col[i].GetXYZ(), float3(lum,lum,lum), desat ), 1 );

    // Finally, apply the requested strength.
    g_light_col[i] *= str;
  }

  void RandomizeAmboParams() {
    kLightAmboDiffuseUnLitThresh = FRAND*FRAND*0.25f;
    kLightAmboDiffuseFullyLitThresh = 0.45f + FRAND*0.05f + FRAND*0.05f;
    kLightAmboDiffuseExponent = powf(2.0f, FRAND + FRAND - 1);
    kLightAmboSpecLo = 0.20f;
    kLightAmboSpecHi = 0.45f;
    kLightAmboDirScale = 3 + 2*FRAND;    //4;
    kLightAmboDirBias  = 0 + 0.2f*FRAND; //0.1f;
    kLightUseDirectionalAmbo = 1;
  }
  void RandomizeSpecParams() {
    kLightSpecStr = saturate( FRAND*FRAND*1.3f - 0.3f ) * 1.0f;
    kLightSpecPow = FRAND*FRAND*8;// Actual exponent is 2^this.
  }

  void RandomizeCommonLightParams()
  {
    g_light_unorm_all = float4(FRAND, FRAND, FRAND, FRAND);
    RandomizeAmboParams();
    RandomizeSpecParams();
    RandomizeFog();
    kLightScale = 1;
    kLightSat   = (FRAND < 0.01f) ? 0 : 1.0f;
    kReflectivity = saturate(FRAND * 2 - 1);
    kSurfaceColorAmount = saturate(FRAND * 2 - 1);
    kFinalGamma = 2.2f;
  }

  void ApplyDefaultSettings()
  {
    memset(this, 0, sizeof(shader_constants));
    kGeomSamplesPerFrame = 4;//16;
    kGeomPercentNeighborDepthRand = 0.02f;
    kNormSampleRadiusMult = 1.0f;//0.5f;//1;             //KIV   1 = sample over entire pixel (sphere within)
    kNormPixelsAlongEyeRayToPullBack = 0;  //KIV
    kAmboRaysPerPass   = AMBO_RAYS_PER_PASS;
    kAmboSamplesPerRay = AMBO_SAMPLES_PER_RAY;  // Stay above 32 - otherwise, you get glowy edges (as rays go through the edge).
    kAmboSamplePlacementAlongRayPow = 3;  // as this goes up, we place more of the samples nearby.  Raising this value places more emphasis on reacting to nearby occluders, and less on faraway occluders; it also shrinks 'glowiness' on big, long, flat edges.
    kAmboFalloffPower = 2.5f;//1;//2.0f;   //KIV.  Bound to the perceived length of the rays (wavelen of ambo).  0.5 = very short rays; 3.0 = very long rays (makes scene look "big").
    kAmboPixelsAlongEyeRayToPullBack = 0.1f;  // Because z-refine might not be 100% perfect.
    kAmboPixelsAlongNormalToJumpFwd  = 0.2f;  // Get out away from the surface ever so slightly.
    kAmboPixelsAlongAmboRayToJumpFwd = 0.0f;
    kNoiseCubeSize  = k3DNoiseSize;
    kNoiseVoxelSize = 1.0f/(float)(k3DNoiseSize);
    g_viz_mode = 0;
  }

  void OnSeed(uint new_seed)
  {
    g_scene_seed = new_seed;

    for (int i = 0; i < SEED_ROT_MAT_COUNT; ++i)
      RandomRot(seed_rot_mat[i]);
    for (int i = 0; i < SEED_DIR_COUNT; ++i) 
    {
      float3 v = normalize(float3(FRAND*2-1, FRAND*2-1, FRAND*2-1));
      seed_dir[i] = float4(v.x, v.y, v.z, 0);      // Random unit-length vectors
    }
    for (int i = 0; i < SEED_UNORM_COUNT; ++i)
      seed_unorm[i] = float4(FRAND, FRAND, FRAND, FRAND);   // [0..1]
    for (int i = 0; i < SEED_SNORM_COUNT; ++i)
      seed_snorm[i] = float4(FRAND*2-1, FRAND*2-1, FRAND*2-1, FRAND*2-1);   // [-1..1]
    for (int i = 0; i < SEED_UINT_COUNT; ++i)
      seed_uint[i] = uint4(NRAND, NRAND, NRAND, NRAND);     // [0..2^32-1]
    //for (int i = 0; i < SEED_QUAT_COUNT; ++i)
    //  seed_quat[i] = ...;   // Quaternions (rotations)
  }
};

class RenderTarget
{
 public:
  RenderTarget() : tex(NULL), rtview(NULL), srview(NULL) { }
  ~RenderTarget() { Free(); }

  HRESULT Init(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format);
  void Free() { SAFE_RELEASE(tex); SAFE_RELEASE(rtview); SAFE_RELEASE(srview); }

  ID3D11Texture2D*          tex;
  ID3D11RenderTargetView*   rtview;
  ID3D11ShaderResourceView* srview;
};

// Contains a set of 3 RT's for a stage of intersection with a surface.
class IntersectionSet
{
 public:
  IntersectionSet() { initialized_ = false; which_depth_ = 0; }
  ~IntersectionSet() { Free(); }

  bool IsInitialized() { return initialized_; }

  HRESULT Init(ID3D11Device* pDevice, int w, int h) {
    ambo_dirty_ = false;
    ambo_pass_ = 0;
    geom_frames_since_geom_clear_ = 0;
    normals_ready_ = 0;
    frame_ = 0;
    clear_flags_ = 0xFFFFFFFF;

    gen_mode_ = GENERATE_GEOM;
    next_gen_mode_ = GENERATE_GEOM;

    which_depth_ = 0;

    HRESULT hr;
    hr = depth[0].Init(pDevice, w, h, kDepthTexStorageFmt);
    if (FAILED(hr)) return hr;
    hr = depth[1].Init(pDevice, w, h, kDepthTexStorageFmt);
    if (FAILED(hr)) return hr;
    hr = norm.Init(pDevice, w, h, kNormalTexStorageFmt);
    if (FAILED(hr)) return hr;
    hr = ambo.Init(pDevice, w, h, kAmboTexStorageFmt);
    if (FAILED(hr)) return hr;

    initialized_ = true;

    return S_OK;
  }

  void Free() {
    depth[0].Free();
    depth[1].Free();
    norm.Free();
    ambo.Free();
    initialized_ = false;
  }

  GenMode gen_mode_;
  GenMode next_gen_mode_;  // When we finish the last tile, we'll switch.

  bool initialized_;
  int  frame_;
  bool ambo_dirty_;
  uint clear_flags_;
  int  normals_ready_;
  int  ambo_pass_;   // Stays at 0 even when pass 0 is complete; only jumps to 1 when you do first tile of 2nd pass.
  int  geom_frames_since_geom_clear_;

  int  which_depth_;

  RenderTarget depth[2];
  RenderTarget norm;
  RenderTarget ambo;
};

class TextureFromDisk
{
 public:
  TextureFromDisk() : srview(NULL) { }
  ~TextureFromDisk() { SAFE_RELEASE(srview); }

  HRESULT Init(ID3D11Device* pDevice, DXGI_FORMAT format, const char* szFilename);

  ID3D11ShaderResourceView* srview;
};

class Texture1D
{
 public:
  Texture1D() : tex(NULL), srview(NULL) { }
  ~Texture1D() { Free(); }

  void Free() { SAFE_RELEASE(tex); SAFE_RELEASE(srview); }

  enum FillWith {
    Noise,
    SinCos,
    ProgressiveAmboRayDirs,
    HomogenousNormalRayDirs
  };

  HRESULT Init(ID3D11Device* pDevice, int width, DXGI_FORMAT format, FillWith fill);

  ID3D11Texture1D*          tex;
  ID3D11ShaderResourceView* srview;
};

class Texture2D
{
 public:
  Texture2D() : tex(NULL), srview(NULL) { }
  ~Texture2D() { Free(); }

  void Free() { SAFE_RELEASE(tex); SAFE_RELEASE(srview); }

  HRESULT Init(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format, bool fill_with_noise);

  ID3D11Texture2D*          tex;
  ID3D11ShaderResourceView* srview;
};

class Texture3D
{
 public:
  Texture3D() : tex(NULL), srview(NULL) { }
  ~Texture3D() { Free(); }

  void Free() { SAFE_RELEASE(tex); SAFE_RELEASE(srview); }

  HRESULT Init(ID3D11Device* pDevice, int width, int height, int depth, DXGI_FORMAT format, bool bFillWithNoise, bool bPackNeighbors);

  ID3D11Texture3D*          tex;
  ID3D11ShaderResourceView* srview;
};

class TextureCube
{
 public:
  TextureCube() : tex(NULL), srview(NULL) { }
  ~TextureCube() { Free(); }

  void Free() { SAFE_RELEASE(tex); SAFE_RELEASE(srview); }

  HRESULT Init(ID3D11Device* pDevice, const char* szDdsFile);

  ID3D11Texture2D*          tex;
  ID3D11ShaderResourceView* srview;
};

class Demo;

unsigned __stdcall LoadShaderThread1(void* arg);
unsigned __stdcall LoadShaderThread2(void* arg);
unsigned __stdcall LoadShaderThread3(void* arg);
unsigned __stdcall LoadShaderThread4(void* arg);

class Demo
{
 public:
  Demo(HWND hwnd, HINSTANCE hInstance, int tile_edge_len_pixels) 
      : hwnd_(hwnd), 
        hinstance_(hInstance),
        tile_edge_len_pixels_(tile_edge_len_pixels)
  { }
  
  ~Demo() { 
    // Release our scene-specific objects.
    SAFE_RELEASE( rasterstate_geom_ );
    FreeShaders(false);
    //cb_data_
    SAFE_RELEASE( shader_data_gpu_ );
    SAFE_RELEASE( sampler_linear_wrap_ );
    SAFE_RELEASE( sampler_linear_clamp_ );
    SAFE_RELEASE( sampler_nearest_wrap_ );
    SAFE_RELEASE( sampler_nearest_clamp_ );
    SAFE_RELEASE( blend_off_ );
    SAFE_RELEASE( blend_add_ );

    SAFE_DELETE( lighting_dialog_ );

    // Release our universal objects.
    if( immed_context_ ) immed_context_->ClearState();
    if( rtview_backbuf_ ) rtview_backbuf_->Release();
    if( swap_chain_ ) swap_chain_->Release();
    if( immed_context_ ) immed_context_->Release();
    if( device_ ) device_->Release();
  }

  HWND GetLightingDialogHwnd() 
  {
    if (lighting_dialog_ == NULL)
      return NULL;
    return lighting_dialog_->GetHwnd();
  }

  void FreeShaders(bool bRelightShaderOnly)
  {
    if (!bRelightShaderOnly) {
      SAFE_RELEASE( shader_vs_fullscreen_ );
      SAFE_RELEASE( shader_vs_tile_ );
      SAFE_RELEASE( shader_ps_geom_ );
      SAFE_RELEASE( shader_ps_norm_ );
      SAFE_RELEASE( shader_ps_ambo_ );
    }
    SAFE_RELEASE( shader_ps_relight_ );
  }

  HRESULT Init() {
    HRESULT hr = S_OK;

    GetModuleFileName(NULL, parent_dir_, sizeof(parent_dir_)/sizeof(parent_dir_[0]));
    TCHAR* p = wcsrchr(parent_dir_, L'\\');
    if (p) *p = 0;
    p = wcsrchr(parent_dir_, L'\\');
    if (p) *(p+1) = 0;
    SetCurrentDirectory(parent_dir_);

    // NULL out our universal objects.
    driver_type_ = D3D_DRIVER_TYPE_NULL;
    feature_level_ = D3D_FEATURE_LEVEL_11_0;
    device_ = NULL;
    immed_context_ = NULL;
    swap_chain_ = NULL;
    rtview_backbuf_ = NULL;

    cur_set_ = &main_;

    super_sample_ = 1;
    width_  = 1;  // of the client area.
    height_ = 1;  // of the client area.

    for (int i = 0; i < MAX_VECTOR_COEFFS; i++) {
      dialog_coeffs_[i] = float4(1,1,1,1);
    }

    autopilot_ = false;
    clear_at_end_of_pass_ = false;
    relight_whole_client_ = 0;
    save_screenshot_ = 0;
    mouse_x_ = -1;
    mouse_y_ = -1;
    mouse_zoom_level_ = 0;  // 0 = none, 1 = see pixels 1:1, 2 = see pixels double-size
    mouse_button_down_[0] = false; 
    mouse_button_down_[1] = false; 
    mouse_button_down_[2] = false; 
    mouse_click_x_ = 0; 
    mouse_click_y_ = 0; 
    mouse_shift_ = false;
    mouse_ctrl_  = false;
    mouse_light_ = -1;    // -1 = mouse camera; 9 = all lights; 0..MAX_LIGHTS-1 = one particular light.
    mouse_frame_ = &camera_;   // Alias to the CameraInfo frame we are currently mousing (a mouse or a camera), or NULL if we are rotating all lights.

    // Camera:
    camera_.pos = float3(0,0,-LOOKAT_DISTANCE);
    camera_.ws_x_axis = float3(1,0,0);
    camera_.ws_y_axis = float3(0,1,0);
    camera_.ws_z_axis = float3(0,0,1);
    camera_.fov = kFOVDegrees * (3.1415926f/180.0f);
    camera_.aspect = width_/(float)height_;
    camera_frozen_ = camera_;   // A snapshot of the camera on the previous frame.  Necessary since mouse moves will mess with internals.

    // NULL out our scene-specific objects.
    rasterstate_geom_ = NULL;
    shader_vs_fullscreen_ = NULL;
    shader_vs_tile_ = NULL;
    shader_ps_geom_ = NULL;
    shader_ps_norm_ = NULL;
    shader_ps_ambo_ = NULL;
    shader_ps_relight_ = NULL;
    temp_vs_quad_fullscreen_ = NULL;
    temp_vs_quad_tile_ = NULL;
    temp_ps_geom_ = NULL;
    temp_ps_norm_ = NULL;
    temp_ps_ambo_ = NULL;
    temp_ps_relight_ = NULL;
    cb_data_.ApplyDefaultSettings();
    shader_data_gpu_ = NULL;
    sampler_linear_wrap_ = NULL;
    sampler_linear_clamp_ = NULL;
    sampler_nearest_wrap_ = NULL;
    sampler_nearest_clamp_ = NULL;
    blend_off_ = NULL;
    blend_add_ = NULL;

    lighting_dialog_ = NULL;

    RECT r;
    GetClientRect(hwnd_, &r);
    const int client_width  = (r.right - r.left);
    const int client_height = (r.bottom - r.top);

    super_sample_ = kInitialSuperSample;
    width_  = client_width ;// * super_sample_;
    height_ = client_height;// * super_sample_;

    // Create D3D11 device.
    UINT createDeviceFlags = 0;//D3D10_CREATE_DEVICE_SINGLETHREADED;
    #ifdef _DEBUG
      // Doesn't work on Radeon?
      //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_DRIVER_TYPE driverTypes[] = {
        //D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE,
        MY_D3D_DRIVER_TYPE
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] = {
        //D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
        MY_FEATURE_LEVEL
    };
	  UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof(sd) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width  = width_ ;//client_width;
    sd.BufferDesc.Height = height_;//client_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd_;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.Flags = kSwapChainFlags;
    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        driver_type_ = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, driver_type_, NULL, 
                                            createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &swap_chain_, 
                                            &device_, &feature_level_, &immed_context_ );
        if( SUCCEEDED( hr ) )
          break;
    }
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "D3D11CreateDeviceAndSwapChain() failed.", "Error", MB_ICONERROR);
      return hr;
    }
    
    //immed_context_->OMSetRenderTargets( 1, &rtview_backbuf_, NULL );
    //SetViewport(false);  // true = big, false = small

    // Create Render Targets.
    hr = InitRenderTargets();
    if (FAILED(hr)) return hr;

    // Create static textures.  DON'T create anything random here; 
    // that stuff must be seed-driven.
    hr = sincos_.Init(device_, kSinCosLookupLength, kSinCosLookupTexFmt, Texture1D::SinCos);
    if (FAILED(hr)) return hr;    
    //hr = tex_sky_.Init(device_, "textures\\miramar_mips.dds");
    //if (FAILED(hr)) return hr;    
    hr = ambo_ray_dirs_1D_.Init(device_, kAmboRayDirCount, kAmboRayDirTexFmt, Texture1D::ProgressiveAmboRayDirs);
    if (FAILED(hr)) return hr;
    hr = normal_ray_dirs_1D_.Init(device_, kNormalRayDirCount, kNormalRayDirTexFmt, Texture1D::HomogenousNormalRayDirs);
    if (FAILED(hr)) return hr;
    hr = alt_map_1d_.Init(device_, DXGI_FORMAT_R8G8B8A8_UNORM, "textures\\h1.png");
    if (FAILED(hr)) return hr;
    
    // Create shaders.
    hr = LoadShaders(false);
    if (FAILED(hr)) return hr;

    // Create Rasterizer State Objects.
    D3D11_RASTERIZER_DESC RSDesc;
    RSDesc.FillMode = D3D11_FILL_SOLID;
    RSDesc.CullMode = D3D11_CULL_BACK;
    RSDesc.FrontCounterClockwise = FALSE;
    RSDesc.DepthBias = 0;
    RSDesc.DepthBiasClamp = 0.0f;
    RSDesc.SlopeScaledDepthBias = 0.0f;
    RSDesc.DepthClipEnable = TRUE;
    RSDesc.ScissorEnable = FALSE;
    RSDesc.MultisampleEnable = FALSE;
    RSDesc.AntialiasedLineEnable = FALSE;
    hr = device_->CreateRasterizerState( &RSDesc, &rasterstate_geom_ );
    if (FAILED(hr)) { 
      MessageBoxA(hwnd_, "CreateRasterizerState() failed.", "Error", MB_ICONERROR); 
      return hr;
    }
    //DXUT_SetDebugName( rasterstate_geom_, "geom raster state" );

    // Create constant buffer.
    D3D11_BUFFER_DESC bd;
    bd.ByteWidth = (sizeof(cb_data_) + 15) & 0xFFFFFFF0;
    bd.Usage = D3D11_USAGE_DYNAMIC;  // so cpu can access it
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    hr = device_->CreateBuffer( &bd, NULL, &shader_data_gpu_ );
    if (FAILED(hr)) { 
      MessageBoxA(hwnd_, "CreateBuffer() failed.", "Error", MB_ICONERROR); 
      return hr;
    }
    //DXUT_SetDebugName( shader_data_gpu_, "shader_data_gpu_" );

    // Create samplers.
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Two wrap samplers:
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    hr = device_->CreateSamplerState( &sampDesc, &sampler_linear_wrap_ );
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateSamplerState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = device_->CreateSamplerState( &sampDesc, &sampler_nearest_wrap_ );
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateSamplerState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }

    // Two clamp samplers:
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    hr = device_->CreateSamplerState( &sampDesc, &sampler_linear_clamp_ );
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateSamplerState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = device_->CreateSamplerState( &sampDesc, &sampler_nearest_clamp_ );
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateSamplerState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }

    // Normal blend state: (blending off):
    D3D11_BLEND_DESC bsd;
    bsd.AlphaToCoverageEnable          = FALSE;
    bsd.IndependentBlendEnable         = FALSE;
    bsd.RenderTarget[0].BlendEnable    = FALSE;
    bsd.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].DestBlend      = D3D11_BLEND_ZERO;
    bsd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    bsd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bsd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    bsd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device_->CreateBlendState(&bsd, &blend_off_);
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateBlendState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }

    // Additive blend state:
    bsd.AlphaToCoverageEnable          = FALSE;
    bsd.IndependentBlendEnable         = FALSE;
    bsd.RenderTarget[0].BlendEnable    = TRUE;
    bsd.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    bsd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    bsd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    bsd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device_->CreateBlendState(&bsd, &blend_add_);
    if (FAILED(hr)) {
        MessageBoxA(hwnd_, "CreateBlendState() failed.", "Error", MB_ICONERROR); 
        return hr;
    }

    // Set up common render state.
    immed_context_->RSSetState( rasterstate_geom_ );
    immed_context_->VSSetConstantBuffers( 0, 1, &shader_data_gpu_ ); 
    immed_context_->PSSetConstantBuffers( 0, 1, &shader_data_gpu_ );  // KIV: ok to set for both?
    immed_context_->IASetInputLayout( NULL );
    immed_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    // Share common sampler setup for all shaders:
    ID3D11SamplerState* samplers[] = { sampler_linear_wrap_, sampler_linear_clamp_, sampler_nearest_wrap_, sampler_nearest_clamp_ };
    immed_context_->PSSetSamplers(0, sizeof(samplers)/sizeof(samplers[0]), samplers);

    // Apply the default random number seed.
    OnSeed(0x713BF2CE);
    RandomizeLights();

    // Lighting dialog:
    lighting_dialog_ = new ConfigBase(hinstance_, hwnd_, IDD_LIGHTING, IDC_CLOSE);
    InitLightingDialog(lighting_dialog_->GetHwnd());
    WriteLightingDialog();

    // Set initial thread priority.
    //SetLowThreadPriority(true);

    // Setup successful.
    return S_OK;
  }

  void FillWithColor(HWND hThumb, float3 color)
  {
    HDC hdc = GetDC(hThumb);
    HBRUSH brush = CreateSolidBrush(RGB(color.x*255, color.y*255, color.z*255));
    RECT r;
    GetClientRect(hThumb, &r);
    FillRect(hdc, &r, brush);
    DeleteObject(brush);
    ReleaseDC(hThumb, hdc);
  }

  void DescribeLight(HWND h, int i, int image_ctrl_id, int text_ctrl_id)
  {
    // 1. Update text string.
    char szWhere[256];
    float3 ws_dir = cb_data_.g_light_dir[i].GetXYZ();
    
    // Now convert to eye-space:
    float3 dir;
    dir.x = dot(ws_dir, camera_.ws_x_axis);
    dir.y = dot(ws_dir, camera_.ws_y_axis);
    dir.z = dot(ws_dir, camera_.ws_z_axis);
    
    float fmax = max(fabsf(dir.x), max(fabsf(dir.y), fabsf(dir.z)));
    if (fabsf(dir.x) >= fmax) {
      if (dir.x > 0)
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "from left");
      else
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "from right");
    }
    else if (fabsf(dir.y) >= fmax) {
      if (dir.y > 0)
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "from below");
      else
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "from above");
    }
    else {
      if (dir.z > 0)
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "frontal");
      else
        strcpy_s(szWhere, sizeof(szWhere)/sizeof(szWhere[0]), "backlight");
    }
    char buf[256];
    sprintf_s(buf, sizeof(buf)/sizeof(buf[0]), "Light %d: %s %s", i+1, szWhere, (i == mouse_light_) ? " [SELECTED]" : "");
    SetWindowTextA( GetDlgItem(h, text_ctrl_id), buf);

    // 2. Update color indicator.
    HWND hThumb = GetDlgItem(h, image_ctrl_id);
    FillWithColor(hThumb, cb_data_.g_light_col[i].GetXYZ());
  }

  void SetSliderRange(HWND hwnd, int ctrl, float fmin, float fmax) {
    // Always set real slider range to [0..256], but remember a hash map
    // of ctrl_ID to fmin and fmax; then whenever Set or Get are called,
    // user can specify simple floats, and we'll translate.

    // Set trackbar limits to [0..256]
    SendMessage(GetDlgItem(hwnd, ctrl), TBM_SETRANGE, FALSE, MAKELONG(0, 256));

    // Add range to hash map:
    MinMax mm;
    mm.min = fmin;
    mm.max = fmax;
    lighting_sliders_.insert(std::pair<int, MinMax>(ctrl, mm));
  }
  void SetSlider(HWND hwnd, int ctrl, float value) {
    float fmin = lighting_sliders_[ctrl].min;
    float fmax = lighting_sliders_[ctrl].max;
    int n = (int)((value - fmin)*256/(fmax - fmin));
    SendMessage(GetDlgItem(hwnd, ctrl), TBM_SETPOS, TRUE, n);
  }
  float GetSlider(HWND hwnd, int ctrl) {
    float fmin = lighting_sliders_[ctrl].min;
    float fmax = lighting_sliders_[ctrl].max;
    int n = SendMessage(GetDlgItem(hwnd, ctrl), TBM_GETPOS, 0, 0);
    float f = fmin + (fmax - fmin)*(n/256.0f);
    return f;
  }
  void SetCheckbox(HWND hwnd, int ctrl, int checked) {
    SendMessage(GetDlgItem(hwnd, ctrl), BM_SETCHECK, (checked != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
  }
  int GetCheckbox(HWND hwnd, int ctrl) {
    return (SendMessage(GetDlgItem(hwnd, ctrl), BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
  }

  void InitLightingDialog(HWND hwnd)
  {
    SetSliderRange(hwnd, IDC_LIGHTN_R, 0, 1);
    SetSliderRange(hwnd, IDC_LIGHTN_G, 0, 1);
    SetSliderRange(hwnd, IDC_LIGHTN_B, 0, 1);
    SetSliderRange(hwnd, IDC_LIGHTN_WRAP, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_LO, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_HI, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_EXP, 0, 4);
    SetSliderRange(hwnd, IDC_AMBO_SPEC_LO, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_SPEC_HI, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_LO, 0, 1);
    SetSliderRange(hwnd, IDC_AMBO_DIR_SCALE, 0, 8);
    SetSliderRange(hwnd, IDC_AMBO_DIR_BIAS, -2, 2);
    SetSliderRange(hwnd, IDC_SPEC_STR, 0, 1.5f);
    SetSliderRange(hwnd, IDC_SPEC_EXP, 0, 8); // Actual exponent is 2^this.
    SetSliderRange(hwnd, IDC_FOG_R, 0, 1);
    SetSliderRange(hwnd, IDC_FOG_G, 0, 1);
    SetSliderRange(hwnd, IDC_FOG_B, 0, 1);
    SetSliderRange(hwnd, IDC_FOG_DENSITY, 0, 1);
    SetSliderRange(hwnd, IDC_FOG_EXP, 0, 4);
    SetSliderRange(hwnd, IDC_BRIGHTNESS, 0, 5);
    SetSliderRange(hwnd, IDC_SATURATION, 0, 4);
    SetSliderRange(hwnd, IDC_REFL, 0, 2);
    SetSliderRange(hwnd, IDC_SURF_COLOR_AMT, 0, 2);
  }

  bool Export()
  {
    TCHAR szInitialDir[1024];
    swprintf_s(szInitialDir, 
               sizeof(szInitialDir)/sizeof(szInitialDir[0]), 
               L"%s%s", parent_dir_, L"scenes\\");
    CreateDirectory(szInitialDir, NULL);

    // Figure out filename to save to.
    TCHAR filename[MAX_PATH];
    filename[0] = 0;
    OPENFILENAME ofn = { sizeof(OPENFILENAME), };
    ofn.hwndOwner = hwnd_;
    ofn.hInstance = hinstance_;
    ofn.lpstrInitialDir = szInitialDir;
    ofn.lpstrFilter = L"Scene Files\0*.scn\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename)/sizeof(filename[0]);
    ofn.lpstrTitle = L"Select file to write";
    ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER;
    if (!GetSaveFileName(&ofn))
      return false;

    // Append .scn if they didn't.
    int len = wcslen(filename);
    if (len < 4 || _wcsicmp(&filename[len-4], L".scn"))
      wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".scn");

    bool bSuccess = Export(filename);

    if (bSuccess)
      MessageBoxA(hwnd_, "Save successful.", "", MB_OK); 

    return bSuccess;
  }

  bool Export(const TCHAR* filename)
  {
    // Open the file.
    FILE* f = NULL;
    _wfopen_s(&f, filename, L"wb");
    if (!f) {
      MessageBox(hwnd_, filename, L"Unable to open file", MB_ICONERROR); 
      return false;
    }

    // Write the contents.
    DWORD version = 5;
    DWORD seed = cb_data_.g_scene_seed;
    fwrite(&version, sizeof(DWORD ), 1, f);
    fwrite(&seed   , sizeof(DWORD ), 1, f);

    // Read camera info.
    fwrite(&camera_.pos      , sizeof(float3), 1, f);
    fwrite(&camera_.ws_x_axis, sizeof(float3), 1, f);
    fwrite(&camera_.ws_y_axis, sizeof(float3), 1, f);
    fwrite(&camera_.ws_z_axis, sizeof(float3), 1, f);
    fwrite(&camera_.fov      , sizeof(float ), 1, f);
    fwrite(&camera_.aspect   , sizeof(float ), 1, f);

    // Read lights.
    DWORD num_lights = MAX_LIGHTS;
    fwrite(&num_lights                 , sizeof(DWORD ), 1, f);
    fwrite(&cb_data_.g_light_unorm_all , sizeof(float4), 1, f);
    fwrite(&cb_data_.kLightUseDirectionalAmbo       , sizeof(int  ), 1, f);
    fwrite(&cb_data_.kLightAmboDiffuseUnLitThresh   , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboDiffuseFullyLitThresh, sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboDiffuseExponent      , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboSpecLo  , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboSpecHi  , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboDirScale, sizeof(float), 1, f);
    fwrite(&cb_data_.kLightAmboDirBias , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightSpecStr     , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightSpecPow     , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightScale       , sizeof(float), 1, f);
    fwrite(&cb_data_.kLightSat         , sizeof(float), 1, f);
    for (DWORD i=0; i<num_lights; i++)
    {
      fwrite(&cb_data_.g_light_dir[i]         , sizeof(float4), 1, f); 
      fwrite(&cb_data_.g_light_col[i]         , sizeof(float4), 1, f); 
      fwrite(&cb_data_.g_light_wrap[i]        , sizeof(float4), 1, f); 
      fwrite(&cb_data_.g_light_unorm[i]       , sizeof(float4), 1, f); 
      fwrite(&cb_data_.g_light_spec_scale[i]  , sizeof(float4), 1, f); 
      fwrite(&cb_data_.g_light_spec_enabled[i], sizeof(uint4 ), 1, f); 
    }
    // Added in version 2:
    fwrite(&cb_data_.g_fog_color,  sizeof(float4), 1, f);
    fwrite(&cb_data_.g_fog_params, sizeof(float4), 1, f);
    // Added in version 3:
    fwrite(&cb_data_.kReflectivity      , sizeof(float), 1, f);
    fwrite(&cb_data_.kSurfaceColorAmount, sizeof(float), 1, f);
    // Added in version 4:
    fwrite(dialog_coeffs_, sizeof(float4), MAX_VECTOR_COEFFS, f);
    // Added in version 5:
    fwrite(&cb_data_.kFinalGamma       , sizeof(float), 1, f);

    // Close the file.
    fclose(f);

    return true;
  }

  // If bLightsOnly is true, then we only read in the light settings,
  //  and we adapt them to be the same, relative to the existing camera.
  // If it is false, we overwrite the camera position, seed, and all lights.
  void Import(bool bLightsOnly)
  {
    TCHAR szInitialDir[1024];
    swprintf_s(szInitialDir, 
               sizeof(szInitialDir)/sizeof(szInitialDir[0]), 
               L"%s%s", parent_dir_, L"scenes\\");
    CreateDirectory(szInitialDir, NULL);

    // Figure out filename to save to.
    TCHAR filename[MAX_PATH];
    filename[0] = 0;
    OPENFILENAME ofn = { sizeof(OPENFILENAME), };
    ofn.hwndOwner = hwnd_;
    ofn.hInstance = hinstance_;
    ofn.lpstrInitialDir = szInitialDir;
    ofn.lpstrFilter = L"Scene Files\0*.scn\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename)/sizeof(filename[0]);
    ofn.lpstrTitle = L"Select file to write";
    ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER;
    if (!GetOpenFileName(&ofn))
      return;

    // Open the file.
    FILE* f = NULL;
    _wfopen_s(&f, filename, L"rb");
    if (!f) {
      MessageBox(hwnd_, filename, L"Unable to open file", MB_ICONERROR); 
      return;
    }

    /*
    if (!bLightsOnly)
    {
      MessageBox(hwnd_, L"Not yet implemented.", L"Error", MB_ICONERROR); 
      return;
    }
    */

    // Read the contents.
    DWORD version = 0, seed = 0;
    fread(&version, sizeof(DWORD ), 1, f);
    fread(&seed   , sizeof(DWORD ), 1, f);
    if (version < 1)
    {
      MessageBox(hwnd_, L"Unknown version number.", L"Error", MB_ICONERROR); 
      return;
    }

    // Read camera info.
    CameraInfo cam;
    fread(&cam.pos      , sizeof(float3), 1, f);
    fread(&cam.ws_x_axis, sizeof(float3), 1, f);
    fread(&cam.ws_y_axis, sizeof(float3), 1, f);
    fread(&cam.ws_z_axis, sizeof(float3), 1, f);
    fread(&cam.fov      , sizeof(float ), 1, f);
    fread(&cam.aspect   , sizeof(float ), 1, f);

    // Read lights.
    DWORD num_lights = MAX_LIGHTS;
    fread(&num_lights                 , sizeof(DWORD ), 1, f);
    fread(&cb_data_.g_light_unorm_all , sizeof(float4), 1, f);
    fread(&cb_data_.kLightUseDirectionalAmbo       , sizeof(int  ), 1, f);
    fread(&cb_data_.kLightAmboDiffuseUnLitThresh   , sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboDiffuseFullyLitThresh, sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboDiffuseExponent      , sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboSpecLo  , sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboSpecHi  , sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboDirScale, sizeof(float), 1, f);
    fread(&cb_data_.kLightAmboDirBias , sizeof(float), 1, f);
    fread(&cb_data_.kLightSpecStr     , sizeof(float), 1, f);
    fread(&cb_data_.kLightSpecPow     , sizeof(float), 1, f);
    fread(&cb_data_.kLightScale       , sizeof(float), 1, f);
    fread(&cb_data_.kLightSat         , sizeof(float), 1, f);
    for (DWORD i=0; i<min(num_lights, MAX_LIGHTS); i++)
    {
      fread(&cb_data_.g_light_dir[i]         , sizeof(float4), 1, f); 
      fread(&cb_data_.g_light_col[i]         , sizeof(float4), 1, f); 
      fread(&cb_data_.g_light_wrap[i]        , sizeof(float4), 1, f); 
      fread(&cb_data_.g_light_unorm[i]       , sizeof(float4), 1, f); 
      fread(&cb_data_.g_light_spec_scale[i]  , sizeof(float4), 1, f); 
      fread(&cb_data_.g_light_spec_enabled[i], sizeof(uint4 ), 1, f); 

      // Make light direction same as before, *relative to the camera*.
      if (bLightsOnly) {
        float3 es_light_dir;
        es_light_dir.x = dot(cam.ws_x_axis, cb_data_.g_light_dir[i].GetXYZ());
        es_light_dir.y = dot(cam.ws_y_axis, cb_data_.g_light_dir[i].GetXYZ());
        es_light_dir.z = dot(cam.ws_z_axis, cb_data_.g_light_dir[i].GetXYZ());
        float3 new_ws_light_dir;
        new_ws_light_dir = camera_.ws_x_axis * es_light_dir.x
                          + camera_.ws_y_axis * es_light_dir.y
                           + camera_.ws_z_axis * es_light_dir.z;
        cb_data_.g_light_dir[i] = float4(new_ws_light_dir, 0);
      }
    }
    
    if (version >= 2) {       // Added in version 2:
      fread(&cb_data_.g_fog_color,  sizeof(float4), 1, f);
      fread(&cb_data_.g_fog_params, sizeof(float4), 1, f);
    } else {
      cb_data_.g_fog_color  = float4(0,0,0,0);
      cb_data_.g_fog_params = float4(0,1,0,0);
    }
    
    if (version >= 3) {
      fread(&cb_data_.kReflectivity      , sizeof(float), 1, f);
      fread(&cb_data_.kSurfaceColorAmount, sizeof(float), 1, f);
    } else {
      cb_data_.kReflectivity       = 0;
      cb_data_.kSurfaceColorAmount = 0;
    }

    memset(dialog_coeffs_, 0, sizeof(dialog_coeffs_));
    if (version >= 4) {
      fread(dialog_coeffs_, sizeof(float4), MAX_VECTOR_COEFFS, f);
    }

    if (version >= 5) {
      fread(&cb_data_.kFinalGamma       , sizeof(float), 1, f);
    } else {
      cb_data_.kFinalGamma = 1;
    }

    // Moved lights.
    for (int i=0; i<MAX_LIGHTS; i++) {
      // Fill in lights_[i], for rotation w/mouse.
      lights_[i].ws_z_axis = cb_data_.g_light_dir[i].GetXYZ();
      
      // Make up the X/Y axes.
      lights_[i].ws_y_axis = float3(0,1,0);
      lights_[i].ws_x_axis = normalize(cross(lights_[i].ws_y_axis, lights_[i].ws_z_axis));
      lights_[i].ws_y_axis = normalize(cross(lights_[i].ws_z_axis, lights_[i].ws_x_axis));
    }
    relight_whole_client_ = 1;  
    WriteLightingDialog();

    if (!bLightsOnly) {
      // Apply random number seed.
      OnSeed(seed);

      // Apply camera - except aspect ratio.
      cam.aspect = camera_frozen_.aspect;
      camera_ = cam;
      camera_frozen_ = cam;

      // Moved camera.
      clear_at_end_of_pass_ = true;
    }

    // Close the file.
    fclose(f);
  }

  void MouseLight(int i) 
  {
    // start rotating a particular light.
    if (!IsALight(i)) 
      return;
    mouse_light_ = i;
    mouse_frame_ = &lights_[i];
    WriteLightingDialog();
  }

  void MouseCamera() 
  {
    // stop rotating lights & go back to rotating camera.
    mouse_light_ = -1;
    mouse_frame_ = &camera_;
    WriteLightingDialog();
  }

  void MouseAllLights()
  {
    // start rotating ALL lights.
    mouse_light_ = 9;
    mouse_frame_ = NULL;
    WriteLightingDialog();
  }

  void HandleLightingDialogButtons()
  {
    // 1. This processes unhandled clicks on 'hot' areas
    // (i.e. clicks on bitmaps, but not on buttons).
    int x, y;
    if (lighting_dialog_->GetClick(x, y))
    {
      lighting_dialog_->ClearClick();
      DWORD clickables[] = { 
        IDC_LIGHT0_COLOR_PREVIEW,
        IDC_LIGHT1_COLOR_PREVIEW,
        IDC_LIGHT2_COLOR_PREVIEW,
        IDC_LIGHT3_COLOR_PREVIEW,
        IDC_LIGHT4_COLOR_PREVIEW
      };
      for (int i=0; i<sizeof(clickables)/sizeof(clickables[0]); i++) {
        HWND h = GetDlgItem(lighting_dialog_->GetHwnd(), clickables[i]);
        if (h) {
          RECT r;
          GetWindowRect(h, &r);
          POINT lt, rb;
          lt.x = r.left;
          lt.y = r.top;
          rb.x = r.right;
          rb.y = r.bottom;
          ScreenToClient(lighting_dialog_->GetHwnd(), &lt);
          ScreenToClient(lighting_dialog_->GetHwnd(), &rb);
          if (x >= lt.x && x < rb.x && 
              y >= lt.y && y < rb.y)
          {
            // start rotating a particular light.
            MouseLight(i);
          }
        }
      }
    }

    // 2. Handle regular buttons:
    if (lighting_dialog_->ButtonPushed()) {
      int button_id = lighting_dialog_->ButtonPushed();
      lighting_dialog_->SetButtonPushed(0);
      switch(button_id) {

      // ACTUAL BUTTONS:
      case IDC_LIGHTN_EDIT_COL:  // Caution: will this block?
        break;
      case IDC_LIGHTN_RAND_COL: RandomizeLightColor(mouse_light_); break;
      case IDC_LIGHTN_RAND_DIR: RandomizeLightDir(mouse_light_); break;
      case IDC_LIGHTN_RAND_ALL: RandomizeLight(mouse_light_, 1.0f); break;
      case IDC_RAND_AMBO: cb_data_.RandomizeAmboParams(); break;
      case IDC_RAND_SPEC: cb_data_.RandomizeSpecParams(); break;
      case IDC_RAND_ALL_LIGHTS: RandomizeLights(); break;
      case IDC_EXPORT: Export(); break;
      case IDC_IMPORT: Import(true); break;
      case IDC_IMPORT_ALL: Import(false); break;
      case IDC_ROTATE_ALL_LIGHTS: MouseAllLights(); break;
      case IDC_MOUSE_CAMERA: MouseCamera(); break;
      case IDC_FOG_RAND_COLOR: cb_data_.RandomizeFogColor(); break;
      case IDC_FOG_RAND_ALL:   cb_data_.RandomizeFog(); break;
      
      // CHECKBOXES:
      case IDC_LIGHTN_BSPEC: if (IsALight(mouse_light_)) cb_data_.g_light_spec_enabled[mouse_light_].x = 1 - cb_data_.g_light_spec_enabled[mouse_light_].x; break;
      case IDC_AMBO_BDIR:    cb_data_.kLightUseDirectionalAmbo = 1 - cb_data_.kLightUseDirectionalAmbo; break;
      }
      relight_whole_client_ = 1;
      WriteLightingDialog();      
    }
  }

  void ReadLightingDialog()
  {
    if (!lighting_dialog_->IsDirty())
      return;

    HWND h = lighting_dialog_->GetHwnd();

    // TODO: read slider positions & checkbox states;
    // write new color bitmaps.
    if (IsALight(mouse_light_)) {
      cb_data_.g_light_col[mouse_light_].x  = GetSlider(h, IDC_LIGHTN_R   );
      cb_data_.g_light_col[mouse_light_].y  = GetSlider(h, IDC_LIGHTN_G   );
      cb_data_.g_light_col[mouse_light_].z  = GetSlider(h, IDC_LIGHTN_B   );
      cb_data_.g_light_wrap[mouse_light_].x = GetSlider(h, IDC_LIGHTN_WRAP);
      //cb_data_.g_light_spec_enabled[mouse_light_].x = GetCheckbox(h, IDC_LIGHTN_BSPEC);
    }
    cb_data_.kLightAmboDiffuseUnLitThresh    = GetSlider(h, IDC_AMBO_LO);
    cb_data_.kLightAmboDiffuseFullyLitThresh = GetSlider(h, IDC_AMBO_HI);
    cb_data_.kLightAmboDiffuseExponent       = GetSlider(h, IDC_AMBO_EXP); 
    cb_data_.kLightAmboSpecLo                = GetSlider(h, IDC_AMBO_SPEC_LO);
    cb_data_.kLightAmboSpecHi                = GetSlider(h, IDC_AMBO_SPEC_HI);
    cb_data_.kLightAmboDirScale              = GetSlider(h, IDC_AMBO_DIR_SCALE);
    cb_data_.kLightAmboDirBias               = GetSlider(h, IDC_AMBO_DIR_BIAS);
    cb_data_.kLightSpecStr                   = GetSlider(h, IDC_SPEC_STR);
    cb_data_.kLightSpecPow                   = GetSlider(h, IDC_SPEC_EXP);
    cb_data_.g_fog_color.x                   = GetSlider(h, IDC_FOG_R);
    cb_data_.g_fog_color.y                   = GetSlider(h, IDC_FOG_G);
    cb_data_.g_fog_color.z                   = GetSlider(h, IDC_FOG_B);
    cb_data_.g_fog_params.x                  = GetSlider(h, IDC_FOG_DENSITY);
    cb_data_.g_fog_params.y                  = GetSlider(h, IDC_FOG_EXP);
    cb_data_.kLightScale                     = GetSlider(h, IDC_BRIGHTNESS);
    cb_data_.kLightSat                       = GetSlider(h, IDC_SATURATION);
    cb_data_.kReflectivity                   = GetSlider(h, IDC_REFL);
    cb_data_.kSurfaceColorAmount             = GetSlider(h, IDC_SURF_COLOR_AMT);
    //cb_data_.kLightUseDirectionalAmbo        = GetCheckbox(h, IDC_AMBO_BDIR);
    cb_data_.kFinalGamma = 2.2f;

    // Update the lighting dialog.  For example, if they adjusted
    // the red slider, this will cause other displays of the same
    // info (such as the color thumbs) to update.
    WriteLightingDialog();

    // Do this at the very end, in case we updated some
    // stuff on the dialog as a result of value changes.
    lighting_dialog_->ClearDirtyFlag();
    
    relight_whole_client_ = true;
  }

  void WriteLightingDialog()
  {
    HWND h = lighting_dialog_->GetHwnd();
    
    //TODO: set slider positions & checkbox states;
    // update color boxes; write text strings to describe each light.

    SetSlider(h, IDC_LIGHTN_R      , (!IsALight(mouse_light_)) ? 0 : cb_data_.g_light_col[mouse_light_].x);
    SetSlider(h, IDC_LIGHTN_G      , (!IsALight(mouse_light_)) ? 0 : cb_data_.g_light_col[mouse_light_].y);
    SetSlider(h, IDC_LIGHTN_B      , (!IsALight(mouse_light_)) ? 0 : cb_data_.g_light_col[mouse_light_].z);
    SetSlider(h, IDC_LIGHTN_WRAP   , (!IsALight(mouse_light_)) ? 0 : cb_data_.g_light_wrap[mouse_light_].x);
    SetCheckbox(h, IDC_LIGHTN_BSPEC, (!IsALight(mouse_light_)) ? 0 : cb_data_.g_light_spec_enabled[mouse_light_].x);
    SetSlider(h, IDC_AMBO_LO       , cb_data_.kLightAmboDiffuseUnLitThresh);
    SetSlider(h, IDC_AMBO_HI       , cb_data_.kLightAmboDiffuseFullyLitThresh);
    SetSlider(h, IDC_AMBO_EXP      , cb_data_.kLightAmboDiffuseExponent);
    SetSlider(h, IDC_AMBO_SPEC_LO  , cb_data_.kLightAmboSpecLo);
    SetSlider(h, IDC_AMBO_SPEC_HI  , cb_data_.kLightAmboSpecHi);
    SetSlider(h, IDC_AMBO_DIR_SCALE, cb_data_.kLightAmboDirScale);
    SetSlider(h, IDC_AMBO_DIR_BIAS , cb_data_.kLightAmboDirBias);
    SetSlider(h, IDC_SPEC_STR      , cb_data_.kLightSpecStr);
    SetSlider(h, IDC_SPEC_EXP      , cb_data_.kLightSpecPow);
    SetSlider(h, IDC_FOG_R         , cb_data_.g_fog_color.x);
    SetSlider(h, IDC_FOG_G         , cb_data_.g_fog_color.y);
    SetSlider(h, IDC_FOG_B         , cb_data_.g_fog_color.z);
    SetSlider(h, IDC_FOG_DENSITY   , cb_data_.g_fog_params.x);
    SetSlider(h, IDC_FOG_EXP       , cb_data_.g_fog_params.y);
    SetSlider(h, IDC_BRIGHTNESS    , cb_data_.kLightScale);
    SetSlider(h, IDC_SATURATION    , cb_data_.kLightSat);
    SetSlider(h, IDC_REFL          , cb_data_.kReflectivity      );
    SetSlider(h, IDC_SURF_COLOR_AMT, cb_data_.kSurfaceColorAmount);
    SetCheckbox(h, IDC_AMBO_BDIR, cb_data_.kLightUseDirectionalAmbo);  

    DescribeLight(h, 0, IDC_LIGHT0_COLOR_PREVIEW, IDC_LIGHT_DESC0);
    DescribeLight(h, 1, IDC_LIGHT1_COLOR_PREVIEW, IDC_LIGHT_DESC1);
    DescribeLight(h, 2, IDC_LIGHT2_COLOR_PREVIEW, IDC_LIGHT_DESC2);
    DescribeLight(h, 3, IDC_LIGHT3_COLOR_PREVIEW, IDC_LIGHT_DESC3);
    DescribeLight(h, 4, IDC_LIGHT4_COLOR_PREVIEW, IDC_LIGHT_DESC4);

    // 2. Update color indicator.
    FillWithColor(GetDlgItem(h, IDC_LIGHTN_COL_PREVIEW), (IsALight(mouse_light_)) ? cb_data_.g_light_col[mouse_light_].GetXYZ() : float3(0,0,0));
    FillWithColor(GetDlgItem(h, IDC_FOG_COLOR_PREVIEW), cb_data_.g_fog_color.GetXYZ());

    lighting_dialog_->ClearDirtyFlag();
  }

  void RandomizeLightColor(int i)
  {
    if (!IsALight(i))
      return;
    cb_data_.RandomizeLightColor(i);
  }

  void RandomizeLightDir(int i)
  {
    if (!IsALight(i))
      return;

    lights_[i].ws_z_axis = normalize(float3(FRAND*2-1, FRAND*2-1, FRAND*2-1));

    // Lerp toward viewer
    float lerp_to_look_dir = FRAND*0.5f + FRAND*0.3f;
    lights_[i].ws_z_axis = normalize( lerp( lights_[i].ws_z_axis, 
                                            camera_.ws_z_axis, 
                                            lerp_to_look_dir ));

    // Create X/Y axes.
    lights_[i].ws_y_axis = float3(0,1,0);
    lights_[i].ws_x_axis = normalize(cross(lights_[i].ws_y_axis, lights_[i].ws_z_axis));
    lights_[i].ws_y_axis = normalize(cross(lights_[i].ws_z_axis, lights_[i].ws_x_axis));

    // Update CB:
    cb_data_.g_light_dir[i] = float4(lights_[i].ws_z_axis, 0);
  }

  void RandomizeLight(int i, float str)
  {
    if (!IsALight(i))
      return;

    RandomizeLightDir(i);  // modifies lights_[i]
    cb_data_.RandomizeLight(i, str, lights_[i].ws_z_axis);
  }

  void RandomizeLights()
  {
    int num_primary_lights = 3 + (NRAND % 3);
    float str_per_light = 3.0f/(float)num_primary_lights;
    for (int i=0; i<MAX_LIGHTS; i++) {
      // By default, dim all but the first 3 lights.
      float str = (i < num_primary_lights) ? str_per_light : 0;
      RandomizeLight(i, str);
    }
    cb_data_.RandomizeCommonLightParams();
  }

  HRESULT InitNoiseTextures(uint new_seed) 
  {
    HRESULT hr;

    // Create noise textures.
    for (int i=0; i<kNumNoiseCubes; i++) {
      hr = noise3D_full_[i].Init(device_, k3DNoiseSize, k3DNoiseSize, k3DNoiseSize, k3DNoiseTexFmt, true, false);
      if (FAILED(hr)) return hr;
      hr = noise3D_packed_[i].Init(device_, k3DNoiseSize, k3DNoiseSize, k3DNoiseSize, k3DNoiseTexFmt, true, true);
      if (FAILED(hr)) return hr;
    }
    hr = noise1D_.Init(device_, k1DNoiseLength, k1DNoiseTexFmt, Texture1D::Noise);
    if (FAILED(hr)) return hr;
    hr = tex_noise_unorm_2d_.Init(device_, NOISE_2D_SIZE, NOISE_2D_SIZE, kNoise2DTexFmt, true);
    if (FAILED(hr)) return hr;    

    return S_OK;
  }

  void FreeNoiseTextures() {
    for (int i=0; i<kNumNoiseCubes; i++) {
      noise3D_full_[i].Free();
      noise3D_packed_[i].Free();
    }
    noise1D_.Free();
    tex_noise_unorm_2d_.Free();
  }

  void OnSeed(uint new_seed) 
  {
    g_rand.seed(new_seed);//srand(new_seed);
    cb_data_.OnSeed(new_seed);

    FreeNoiseTextures();
    HRESULT hr = InitNoiseTextures(new_seed);
    if (hr != S_OK) {
      MessageBoxA(hwnd_, "InitNoiseTextures() failed.", "Error", MB_ICONERROR); 
    }
  }

  void SetViewport(bool big)
  {
    D3D11_VIEWPORT vp;
    vp.Width  = (FLOAT)(width_  * (big ? super_sample_ : 1));
    vp.Height = (FLOAT)(height_ * (big ? super_sample_ : 1));
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    immed_context_->RSSetViewports( 1, &vp );
  }

  HRESULT LoadShaders(bool bRelightShaderOnly)
  {
    HRESULT hr = S_OK;
    HRESULT hr_last = S_OK;

    // Try to create all of the shaders.  If any one fails,
    // revert all of them to the old set.  This gives the user
    // a chance to fix the error(s), rather than crashing the program.

    SetCurrentDirectory(parent_dir_);
    unsigned int thread_id;
    if (bRelightShaderOnly) {
      LoadShaderThread4(this);
      hr_last = hr_load_shader_4;
    } else {
      HANDLE h1 = (HANDLE) _beginthreadex(NULL, 0, LoadShaderThread1, this, 0, &thread_id);
      HANDLE h2 = (HANDLE) _beginthreadex(NULL, 0, LoadShaderThread2, this, 0, &thread_id);
      HANDLE h3 = (HANDLE) _beginthreadex(NULL, 0, LoadShaderThread3, this, 0, &thread_id);
      HANDLE h4 = (HANDLE) _beginthreadex(NULL, 0, LoadShaderThread4, this, 0, &thread_id);
      WaitForSingleObject( h1, INFINITE );
      WaitForSingleObject( h2, INFINITE );
      WaitForSingleObject( h3, INFINITE );
      WaitForSingleObject( h4, INFINITE );
      CloseHandle(h1);
      CloseHandle(h2);
      CloseHandle(h3);
      CloseHandle(h4);
      if (hr_load_shader_1 != S_OK) hr_last = hr_load_shader_1;
      if (hr_load_shader_2 != S_OK) hr_last = hr_load_shader_2;
      if (hr_load_shader_3 != S_OK) hr_last = hr_load_shader_3;
      if (hr_load_shader_4 != S_OK) hr_last = hr_load_shader_4;
    }

    if (hr_last != S_OK) 
    {
      // Release all of the new ones, if one didn't work.
      if (!bRelightShaderOnly) {
        SAFE_RELEASE( temp_vs_quad_fullscreen_ );
        SAFE_RELEASE( temp_vs_quad_tile_ );
        SAFE_RELEASE( temp_ps_geom_ );
        SAFE_RELEASE( temp_ps_norm_ );
        SAFE_RELEASE( temp_ps_ambo_ );
      }
      SAFE_RELEASE( temp_ps_relight_ );
      return hr_last;
    } 
    else 
    {
      // Free all the old ones, and apply all the new ones, together.
      FreeShaders(bRelightShaderOnly);
      if (!bRelightShaderOnly) {
        shader_vs_fullscreen_ = temp_vs_quad_fullscreen_;
        shader_vs_tile_ = temp_vs_quad_tile_;
        shader_ps_geom_ = temp_ps_geom_;
        shader_ps_norm_ = temp_ps_norm_;
        shader_ps_ambo_ = temp_ps_ambo_;
      }
      shader_ps_relight_ = temp_ps_relight_;
    }

    return hr;
  }

  /*
  void SetLowThreadPriority(bool bSet)
  {
    HANDLE h = GetCurrentThread();
    SetThreadPriority(h, bSet ? THREAD_PRIORITY_BELOW_NORMAL : THREAD_PRIORITY_NORMAL);
    CloseHandle(h);
  }
  */

  HRESULT InitRenderTargets()
  {
    HRESULT hr;

    // Create a render target view for the [client-sized] backbuffer.
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "SwapChain::GetBuffer() failed.", "Error", MB_ICONERROR); 
      return hr;
    }
    hr = device_->CreateRenderTargetView( pBackBuffer, NULL, &rtview_backbuf_ );
    pBackBuffer->Release();
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "CreateRenderTargetView() failed.", "Error", MB_ICONERROR); 
      return hr;
    }

    // Create full-sized final color rendertarget (for screenshots only).
    final_color_.Init(device_, width_ * super_sample_, height_ * super_sample_, kFinalColorTexFmt);

    // Create full-sized render targets & corresponding views.
    hr = main_.Init(device_, width_ * super_sample_, height_ * super_sample_);
    if (FAILED(hr)) return hr;

    // For this one, wait until first use.  (Note that this can be called from OnResize!)
    assert(cur_set_ != NULL);
    if (cur_set_ == &refl_) {
        hr = refl_.Init(device_, width_ * super_sample_, height_ * super_sample_);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
  }

  void FreeRenderTargets()
  {
    SAFE_RELEASE( rtview_backbuf_ );

    final_color_.Free();

    main_.Free();
    refl_.Free();
  }

  HRESULT OnResize(int w, int h, int supersample) 
  {
    if (w <= 0 || h <= 0) {
      RECT r;
      GetClientRect(hwnd_, &r);
      w = r.right - r.left;
      h = r.bottom - r.top;
    }
    if (supersample <= 0) {
      supersample = super_sample_;
    }
    if (w == width_ && h == height_ && supersample == super_sample_) {
      // This happens when you move the window, w/o resizing.
      return S_OK;
    }

    width_  = w;// * supersample;
    height_ = h;// * supersample;
    super_sample_ = supersample;

    camera_.aspect = width_/(float)height_;
    camera_frozen_ = camera_;

    FreeRenderTargets();

    HRESULT hr = swap_chain_->ResizeBuffers(1, width_, height_, DXGI_FORMAT_R8G8B8A8_UNORM, kSwapChainFlags);
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "SwapChain::GetBuffer() failed.", "Error", MB_ICONERROR); 
      return hr;
    }

    hr = InitRenderTargets();
    if (FAILED(hr))
      return hr;

    return S_OK;
  }

  void OnButtonDown(int button, int x, int y, bool shift, bool ctrl) 
  {
    // button: 0=left, 1=middle, 2=right.
    mouse_button_down_[button] = true; 
    mouse_x_ = x;
    mouse_y_ = y;
    mouse_click_x_ = x; 
    mouse_click_y_ = y; 
    mouse_shift_ = shift;
    mouse_ctrl_  = ctrl;
  }

  void OnButtonUp(int button, int x, int y, bool shift, bool ctrl) 
  {
    // button: 0=left, 1=middle, 2=right.
    mouse_button_down_[button] = false; 
    WriteLightingDialog();
  }

  void OnMouseMove(int x, int y, bool shift, bool ctrl) 
  {
    float dx = -(x - mouse_x_)/(float)(width_ );
    float dy =  (y - mouse_y_)/(float)(height_);

    mouse_x_ = x;
    mouse_y_ = y;

    if (mouse_zoom_level_ != 0) {
      relight_whole_client_ = 1;
      return;
    }

    if (!(mouse_button_down_[0] || mouse_button_down_[1] || mouse_button_down_[2]))
      return;
    if (super_sample_ > 1 && mouse_light_ == -1)    // Don't allow camera mvmt when supersampling.
      return;

    // LMB: spin
    if (mouse_button_down_[0]) {
      if (mouse_ctrl_) {
        // axial rotation
        if (mouse_frame_)
          mouse_frame_->RotateOnZ((dx - dy) * 2);
      }
      else if (mouse_shift_) {
        // pan
        float scale = 4.0f;
        if (mouse_frame_)
          mouse_frame_->pos += mouse_frame_->ws_x_axis * dx * scale + 
                               mouse_frame_->ws_y_axis * dy * scale;
      }
      else {
        // spin
        float scale = (mouse_light_ == -1) ? 1.0f : 4.0f;
        float radians_horiz =  dx * scale;
        float radians_vert  = -dy * scale;

        if (mouse_light_ == -1) {
          // rotate the camera, about its local X/Y axes, and adjust the lookat pos.
          float radius = LOOKAT_DISTANCE;
          float3 lookat = (mouse_frame_) ? mouse_frame_->pos + mouse_frame_->ws_z_axis * radius : float3(0,0,0);
          //float3 pivot_delta = mouse_frame_->ws_z_axis * -3;
          //mouse_frame_->pos += pivot_delta;
          mouse_frame_->RotateOnY(radians_horiz);
          mouse_frame_->RotateOnX(radians_vert );
          mouse_frame_->pos = lookat - mouse_frame_->ws_z_axis * radius;
          //mouse_frame_->pos -= pivot_delta;
        }
        else if (mouse_light_ == 9) {
          // rotate ALL lights, together, about WORLD-SPACE X/Y axes.
          for (int i=0; i<MAX_LIGHTS; i++) {
            lights_[i].RotateOnWSY(radians_horiz);
            lights_[i].RotateOnWSX(radians_vert );
          }
        }
        else {
          // rotate just one light (or the camera), about WORLD-SPACE X/Y axes.
          mouse_frame_->RotateOnWSY(radians_horiz);
          mouse_frame_->RotateOnWSX(radians_vert );
        }
      }
    }

    // RMB: zoom; +shift = fine
    if (mouse_button_down_[2])// && !m_bLastButtonDown_Shift && !m_bLastButtonDown_Ctrl)
    {
        float scale = mouse_shift_ ? 1.0f : 8.0f;
        if (mouse_frame_)
          mouse_frame_->pos -= mouse_frame_->ws_z_axis * (dx - dy) * scale;
    }

    if (mouse_light_ == -1) {
      // moved camera
      clear_at_end_of_pass_ = true;
    } else {
      // moved light(s)
      if (mouse_light_ == 9) {
        for (int i=0; i<MAX_LIGHTS; i++)
          cb_data_.SetLightDir(i, lights_[i].ws_z_axis);
      }
      else {
        cb_data_.SetLightDir(mouse_light_, lights_[mouse_light_].ws_z_axis);
      }
      relight_whole_client_ = 1;  
      WriteLightingDialog();
    }
  }

  void clear() 
  {
    cur_set_ = &main_;
    main_.clear_flags_ = 0xFFFFFFFF;
    refl_.clear_flags_ = 0xFFFFFFFF;
    autopilot_ = false;
    main_.next_gen_mode_ = GENERATE_GEOM;
    refl_.next_gen_mode_ = GENERATE_GEOM;
    relight_whole_client_ = 1;
  }

  void SwitchToMain() {
    cur_set_ = &main_;
  }

  void SwitchToRefl() {
    if (!refl_.IsInitialized()) {
      refl_.Init(device_, width_ * super_sample_, height_ * super_sample_);
    }
    cur_set_ = &refl_;
  }

  void ShowHelpScreen() {
    static const char kHelpText[] = 
      "H\t show this help screen\n"
      "\n"
      "F5\t reload all shaders\n"
      "  SHIFT+F5 = reload only 'relight.hlsl'\n"
      "\n"
      "COMPUTATION MODE:\n"
      "  p\t start autopilot (script for # of regular + reflection passes)\n"
      "  g\t solve for geometry (scene depth; step 1)\n"
      "  n\t solve for normals (step 2)\n"
      "  a\t solve for ambient occlusion (step 3)\n"
      "  ESC\t cancel autopilot (if on), or stop rendering after current pass finishes (go to idle mode)\n"
      "  f\t switch (immediately!) between regular solving and reflection solving.\n"
      "   \t   (note: recommend using ESC and waiting for idle first.)\n"
      "\n"
      "MOUSE MODE:\n"
      "  0   \t use mouse to rotate/move camera.\n"
      "  1..5\t use mouse to rotate a specific light\n"
      "  9   \t use mouse to rotate all lights, simultaneously\n"
      "\n"
      "LIGHTING:\n"
      "  CTRL+L\t show/hide lighting dialog\n"
      "  L\t apply random lighting\n"
      "  +\t brighten scene\n"
      "  -\t darken scene\n"
      "\n"
      "SUPERSAMPLING:\n"
      "  SHIFT + #\t Set oversampling to # (...to render HUGE images).\n"
      "  z\t toggle mouse zoom\n"
      "\n"
      "MISC:\n"
      "  r\t randomize scene (with new random number seed), and clear\n"
      "  c\t clear scene (reset all {geom, normals, ambo})\n"
      "  CTRL + S\t save screenshot, plus a snapshot of the shaders and code.\n"
      "  F1\t previous visualization (debug) mode\n"
      "  F2\t next visualization (debug) mode\n"
    ;

    MessageBoxA(hwnd_, kHelpText, "Keyboard commands:", MB_OK);
  }

  void CoeffsDialog(int index) {
    if (index < 0 || index >= MAX_VECTOR_COEFFS) {
      return;
    }

    const int max_chars = 64;
    const char subindex_to_char[4] = { 'x', 'y', 'z', 'w' };
    char label[4][max_chars];
    char value[4][max_chars];
    for (int i = 0; i < 4; i++) {
      sprintf(label[i], "g_c%d.%c", index, subindex_to_char[i]);
      sprintf(value[i], "%f", *((float*)(&dialog_coeffs_[index]) + i));
    }

    bool success = GetStringFromUser(hinstance_, hwnd_, "Coefficients:",
                           label[0], value[0],
                           label[1], value[1],
                           label[2], value[2],
                           label[3], value[3],
                           NULL, NULL, max_chars);

    if (success) {
      for (int i = 0; i < 4; i++) {
        sscanf(value[i], "%f", ((float*)(&dialog_coeffs_[index]) + i));
      }
    }

    SetFocus(hwnd_);
    SetForegroundWindow(hwnd_);
  }

  bool On_WM_KEYDOWN(WPARAM wParam, LPARAM lParam)
  {
    // Return 'true' if you consume the key, otherwise 'false'.
    int  repeat_count = LOWORD(lParam) + 1;
    bool shift = (GetKeyState(VK_SHIFT  ) & (1 << (sizeof(SHORT)*8 - 1))) != 0;
    bool ctrl  = (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8 - 1))) != 0;
    //bool bAltHeldDown: most keys come in under WM_SYSKEYDOWN when ALT is depressed.

    switch (wParam) {
      //case VK_ESCAPE: return true;
      /*
      case VK_PRIOR:  
        SetLowThreadPriority(false);
        return true;
      case VK_NEXT:   
        SetLowThreadPriority(true);
        return true;
        */
      case VK_F1:
        cb_data_.g_viz_mode = (cb_data_.g_viz_mode - 1 + NUM_VIZ_MODES) % NUM_VIZ_MODES;
        relight_whole_client_ = 1;
        return true;
      case VK_F2:
        cb_data_.g_viz_mode = (cb_data_.g_viz_mode + 1) % NUM_VIZ_MODES;
        relight_whole_client_ = 1;
        return true;
      case VK_F5:
        LoadShaders(shift);
        return true;
      case VK_ADD:    
      case VK_OEM_PLUS:  
        cb_data_.kLightScale *= powf(1.025f, (float)repeat_count);
        relight_whole_client_ = 1;
        WriteLightingDialog();
        return true;
      case VK_SUBTRACT:  
      case VK_OEM_MINUS: 
        cb_data_.kLightScale *= powf(0.975f, (float)repeat_count);
        relight_whole_client_ = 1;
        WriteLightingDialog();
        return true;
      case '0':
      case '1': 
      case '2': 
      case '3': 
      case '4': 
      case '5': 
      case '6': 
      case '7': 
      case '8': 
      case '9': 
        {
          int i = wParam - '0';
          if (ctrl) {
            CoeffsDialog(i);
          } else if (shift) {
            if (i > 0 && super_sample_ != i) {
              // Resize image.
              OnResize(width_, height_, (wParam - '0')); 
              clear();
              return true;   
            }
          } else {
            // rotate lights.
            if (i == 0) {
              // stop rotating lights & go back to rotating camera.
              MouseCamera();
            } else if (i == 9) {
              // start rotating ALL lights.
              MouseAllLights();
            } else if (i > 0 && i <= MAX_LIGHTS) {
              // start rotating a particular light.
              MouseLight(i-1);
            }
          }
        }
        return true;
      case 'H':
        ShowHelpScreen();
        return true;
      //case '0': return true;
      //case 'X': return true;
      case 'F':  // reFlect
        autopilot_ = false;
        if (cur_set_ == &main_)
          SwitchToRefl();
        else
          SwitchToMain();
        return true;
      case 'P':  // autoPilot
        {
          // go to autopilot
          char szText1[64] = "192";
          char szText2[64] = "32";
          char szText3[64] = "192";
          char szText4[64] = "16";
          if (GetStringFromUser(hinstance_, hwnd_, 
                "AutoPilot parameters:",
                "Main geom passes", szText1, 
                "Main ambo passes", szText2, 
                "Refl geom passes", szText3, 
                "Refl ambo passes", szText4, 
                NULL, NULL,
                32))
          {
            int main_geom_passes = 0;
            int main_ambo_passes = 0;
            int refl_geom_passes = 0;
            int refl_ambo_passes = 0;
            if (sscanf_s(szText1, "%d", &main_geom_passes) != 1 ||
                sscanf_s(szText2, "%d", &main_ambo_passes) != 1 ||
                sscanf_s(szText3, "%d", &refl_geom_passes) != 1 ||
                sscanf_s(szText4, "%d", &refl_ambo_passes) != 1) {
              MessageBoxA(hwnd_, "Not an integer.", "Error", MB_ICONERROR);
            }
            else {
              autopilot_ = true;
              autopilot_main_geom_passes_left_ = max(1, main_geom_passes - main_.geom_frames_since_geom_clear_);
              autopilot_main_ambo_passes_left_ = max(1, main_ambo_passes - main_.ambo_pass_);
              autopilot_refl_geom_passes_left_ = max(1, refl_geom_passes - refl_.geom_frames_since_geom_clear_);
              autopilot_refl_ambo_passes_left_ = max(1, refl_ambo_passes - refl_.ambo_pass_);
            }
          }
        }
        return true;
      case 'L': 
        if (ctrl)
        {
          lighting_dialog_->Toggle();
        }
        else
        {
          RandomizeLights();
          relight_whole_client_ = 1;
          WriteLightingDialog();
        }
        return true;
      //case 'M': return true;
      case 'R':
        OnSeed(NRAND);
        main_.clear_flags_ = 0xFFFFFFFF;
        refl_.clear_flags_ = 0xFFFFFFFF;
        return true;
      case 'C':
        clear();
        return true;
      case VK_ESCAPE:
        // stop crunching real data & go into 'relax' mode.
        if (autopilot_)
          autopilot_ = false;
        cur_set_->next_gen_mode_ = GENERATE_NONE;
        return true;
      case 'A': 
        if (cur_set_->normals_ready_ || cur_set_->gen_mode_ == GENERATE_NORMALS)
          cur_set_->next_gen_mode_ = GENERATE_AMBO;
        return true;
      case 'N': 
        cur_set_->next_gen_mode_ = GENERATE_NORMALS;
        return true;
      case 'G': 
        cur_set_->next_gen_mode_ = GENERATE_GEOM;
        return true;
      case 'Z':
        if (super_sample_ > 1)
          mouse_zoom_level_ = (mouse_zoom_level_ + 1) % 3;
        else
          mouse_zoom_level_ = (mouse_zoom_level_ == 2) ? 0 : 2;
        relight_whole_client_ = 1;   // make sure you relight at least 1 extra whole frame, for when it shuts off.
        return true;
      //case 'C': return true;
      //case 'G': return true;
      //case 'B': return true;
      //case VK_F4: return true;
      //case VK_F5: return true;
      //case VK_F7: return true;
      case 'S': 
        if (ctrl) { 
          if (cur_set_->gen_mode_ == GENERATE_GEOM) {
            MessageBoxA(hwnd_, "You can't save a screenshot while generating geometry,\nsince both depth buffers are in use, and one of them\nmust be used as a temp buffer (for a final, full-sized relight)\nin order to save a screenshot.\n\nPlease hit ESC to stop generating geometry, then try again.", "Warning", MB_ICONERROR); 
            return true;
          } 
          if (mouse_zoom_level_ != 0) {
            MessageBoxA(hwnd_, "Since you are zoomed in, this will also affect\nyour screenshot - just FYI.", "Warning", MB_ICONERROR); 
            //return true;
          }
          save_screenshot_ = 1; //SaveImageToDiskUncompressed(); 
          return true; 
        }
        return false;
      //case 'D': return true;
      //case 'T': return true;
      //case 'R': return true;
      //case 'L': return true;
      //case VK_SPACE: return true;
      //case VK_LEFT:  return true;
      //case VK_RIGHT: return true;
      //case VK_UP:    return true;
      //case VK_DOWN:  return true;
      //case VK_UP:    return true;
      //case VK_DOWN:  return true;
      //case VK_F1: return true;
      //case VK_F2: return true;
      //case VK_F3: return true;
      //case VK_F4: return true;
      //case VK_F5: return true;
      //case VK_F6: return true;
      //case VK_F7: return true;
      //case VK_F8: return true;
      //case VK_F9: return true;
    }
    return false;
  }
  bool On_WM_KEYUP(WPARAM wParam, LPARAM lParam)
  {
    // Return 'true' if you consume the key, otherwise 'false'.
    return false;
  }
  bool On_WM_CHAR(WPARAM wParam, LPARAM lParam)
  {
    // Return 'true' if you consume the key, otherwise 'false'.
    return false;
  }

  void onImageReady() 
  {
    cur_set_->next_gen_mode_ = GENERATE_NONE;
    autopilot_ = false;
    // Rendering is finished -> emit some audible chirps.
    for (int i=0; i<5; i++) {
      Beep(900, 90 - i*10);
      Sleep(24 - i*2);
    }
  }

  void ApplyAutoPilot() {
    if (autopilot_) {
      if (cur_set_ == &main_) {
        if (cur_set_->gen_mode_ == GENERATE_NONE) {
          cur_set_->next_gen_mode_ = GENERATE_GEOM;
        } 
        else if (cur_set_->gen_mode_ == GENERATE_GEOM) {
          autopilot_main_geom_passes_left_--;
          if (autopilot_main_geom_passes_left_ > 0)
            cur_set_->next_gen_mode_ = GENERATE_GEOM;
          else 
            cur_set_->next_gen_mode_ = GENERATE_NORMALS;
        }
        else if (cur_set_->gen_mode_ == GENERATE_NORMALS) {
          cur_set_->next_gen_mode_ = GENERATE_AMBO;
        }
        else if (cur_set_->gen_mode_ == GENERATE_AMBO) {
          if (cur_set_->ambo_pass_ < autopilot_main_ambo_passes_left_)
            cur_set_->next_gen_mode_ = GENERATE_AMBO;
          else {
            if (autopilot_refl_geom_passes_left_ > 0) {
              cur_set_->next_gen_mode_ = GENERATE_GEOM;
              SwitchToRefl();
            } else {
              // Done.
              onImageReady();
            }
          }
        }
      } else if (cur_set_ == &refl_) {
        if (cur_set_->gen_mode_ == GENERATE_NONE) {
          cur_set_->next_gen_mode_ = GENERATE_GEOM;
        } 
        else if (cur_set_->gen_mode_ == GENERATE_GEOM) {
          autopilot_refl_geom_passes_left_--;
          if (autopilot_refl_geom_passes_left_ > 0)
            cur_set_->next_gen_mode_ = GENERATE_GEOM;
          else 
            cur_set_->next_gen_mode_ = GENERATE_NORMALS;
        }
        else if (cur_set_->gen_mode_ == GENERATE_NORMALS) {
          cur_set_->next_gen_mode_ = GENERATE_AMBO;
        }
        else if (cur_set_->gen_mode_ == GENERATE_AMBO) {
          if (cur_set_->ambo_pass_ < autopilot_refl_ambo_passes_left_)
            cur_set_->next_gen_mode_ = GENERATE_AMBO;
          else {
            cur_set_->next_gen_mode_ = GENERATE_NONE;
            SwitchToMain();
            onImageReady();
          }
        }
      }
    }
  }

  void Render() { 
    #ifdef _DEBUG
      // Clear the backbuffer.
      //float AlarmClearColor[4] = { 1, 0, 1, 0 }; // bright pink
      //immed_context_->ClearRenderTargetView( rtview_backbuf_, AlarmClearColor );
    #endif

    // Clear other buffers.
    for (int i = 0; i < 2; i++) {
      IntersectionSet* s = (i==0) ? &main_ : &refl_;
      if (s->clear_flags_)
      {
        float ClearColor[4] = { 0, 0, 0, 0 }; //red,green,blue,alpha
        if (s->clear_flags_ & CLEAR_GEOM) 
        {
          immed_context_->ClearRenderTargetView( s->depth[0].rtview, ClearColor );
          immed_context_->ClearRenderTargetView( s->depth[1].rtview, ClearColor );
          s->geom_frames_since_geom_clear_ = 0;
        }
        if (s->clear_flags_ & CLEAR_NORM)
        {
          immed_context_->ClearRenderTargetView( s->norm.rtview, ClearColor );
          s->normals_ready_ = 0;
        }
        if (s->clear_flags_ & CLEAR_AMBO)
        {
          immed_context_->ClearRenderTargetView( s->ambo.rtview, ClearColor );
          s->ambo_pass_ = 0;
        }
        if (s->clear_flags_ == 0xFFFFFFFF)
        {
          s->frame_ = 0;  // reset tiles to UL corner
          s->gen_mode_ = GENERATE_GEOM;
          s->next_gen_mode_ = GENERATE_GEOM;
        }
        s->clear_flags_ = 0;
      }
    }

    // If generating ambo, don't increment the # of passes until we actually
    // start on the next pass - otherwise, on FS relight, stuff will look too dark.
    if (cur_set_->gen_mode_ == GENERATE_AMBO && (cur_set_->frame_ % cb_data_.kTilesTotal) == 0)
      cur_set_->ambo_pass_++;

    // Get current time.
    //float time = 4.0f;//(float)GetTimeInSeconds();

    // Process any button pushes on the lighting dialog.
    HandleLightingDialogButtons();

    // Check if the lighting dialog has been changed.
    ReadLightingDialog();

    // Update CPU copy of cb_data_.
    {
      // Compute view frustum:
      mat4x4 view;
      {
        view.col[0] = float4(camera_frozen_.ws_x_axis, 0);
        view.col[1] = float4(camera_frozen_.ws_y_axis, 0);
        view.col[2] = float4(camera_frozen_.ws_z_axis, 0);
        view.col[3] = float4(camera_frozen_.pos, 1); //KIV
      }

      float dx, dy;
      if (height_ > width_) {
        dy = tanf(camera_frozen_.fov * 0.5f);
        dx = dy * camera_frozen_.aspect;
      } else {
        dx = tanf(camera_frozen_.fov * 0.5f);
        dy = dx / camera_frozen_.aspect;
      }

      // set up virtual screen, in eye space.
      // camera is at (0,0,0), and virtual screen is in XY plane at z==1.
      float3 ss_vs_corners[4] = {   
        float3(-dx,  dy, 1),  //UL
        float3( dx,  dy, 1),  //UR
        float3(-dx, -dy, 1),  //LL
        float3( dx, -dy, 1),  //LR
      };

      float3 ws_vs_corners[4];
      for (int i=0; i<4; i++) {
        #ifdef APPLY_CAMERA_ON_GPU
          ws_vs_corners[i] = ss_vs_corners[i];
        #else
          ws_vs_corners[i] = view.TransformPnt(ss_vs_corners[i]);
        #endif
      }

      cb_data_.ws_eye_pos = float4(camera_frozen_.pos, 0);
      cb_data_.ws_screen_corner_UL = float4(ws_vs_corners[0], 0);
      cb_data_.ws_screen_across    = float4(ws_vs_corners[1] - ws_vs_corners[0], 0);
      cb_data_.ws_screen_down      = float4(ws_vs_corners[2] - ws_vs_corners[0], 0);
      cb_data_.ws_camera_x_axis = float4(camera_frozen_.ws_x_axis, 0);
      cb_data_.ws_camera_y_axis = float4(camera_frozen_.ws_y_axis, 0);
      cb_data_.ws_camera_z_axis = float4(camera_frozen_.ws_z_axis, 0);

      float3 ws_screen_center = (ws_vs_corners[0] +
                                 ws_vs_corners[1] +
                                 ws_vs_corners[2] +
                                 ws_vs_corners[3]) * 0.25f;
      float dist_across = cb_data_.ws_screen_across.GetXYZ().Length();
      #ifdef APPLY_CAMERA_ON_GPU
        float dist_to_screen_center = ws_screen_center.Length();
        cb_data_.ws_look_dir = float4(camera_frozen_.ws_z_axis, 0);//float4( normalize(float3(ws_screen_center - cb_data_.ws_eye_pos.GetXYZ())), 1 );
      #else
        float dist_to_screen_center = float3(ws_screen_center - cb_data_.ws_eye_pos.GetXYZ()).Length();
        cb_data_.ws_look_dir = float4( normalize(float3(ws_screen_center - cb_data_.ws_eye_pos.GetXYZ())), 1 );
      #endif
      cb_data_.g_ws_pixel_size_at_depth_1 = fabsf(dist_across) / (dist_to_screen_center * (float)width_ * super_sample_);
    }

    int mouse_zoom = 1;
    switch (mouse_zoom_level_) {
      case 0: mouse_zoom = 1; break;
      case 1: mouse_zoom = super_sample_; break;
      case 2: mouse_zoom = super_sample_*2; break;
    }
    cb_data_.g_mouse_info = uint4(mouse_x_, mouse_y_, mouse_zoom, 0);
    
    // Misc CB data:
    cb_data_.kTilesAcross = width_ * super_sample_  / tile_edge_len_pixels_;
    cb_data_.kTilesDown   = height_ * super_sample_ / tile_edge_len_pixels_;
    cb_data_.kTilesTotal  = cb_data_.kTilesAcross * cb_data_.kTilesDown;
    cb_data_.kInvTilesAcross = 1.0f/(float)cb_data_.kTilesAcross;
    cb_data_.kInvTilesDown   = 1.0f/(float)cb_data_.kTilesDown;
    cb_data_.g_image_size = float4((float)width_ * super_sample_, (float)height_ * super_sample_, (float)(1.0/width_), (float)(1.0/height_));
    cb_data_.max_dist = kMaxDist;
    cb_data_.g_cur_frame      = cur_set_->frame_;
    cb_data_.g_main_frame     = main_.frame_;
    cb_data_.g_refl_frame     = refl_.frame_;
    cb_data_.g_cur_ambo_pass  = cur_set_->ambo_pass_;
    cb_data_.g_main_ambo_pass = main_.ambo_pass_;
    cb_data_.g_refl_ambo_pass = refl_.ambo_pass_;
    cb_data_.g_supersampling = super_sample_;
    cb_data_.g_intersection_set = (cur_set_ == &main_) ? 0 : 1;
    for (int i = 0; i < TIME_UNORM_COUNT; ++i)
      cb_data_.time_unorm[i] = float4(FRAND, FRAND, FRAND, FRAND);   // [0..1]
    for (int i = 0; i < TIME_SNORM_COUNT; ++i)
      cb_data_.time_snorm[i] = float4(FRAND*2-1, FRAND*2-1, FRAND*2-1, FRAND*2-1);   // [-1..1]
    for (int i = 0; i < TIME_UINT_COUNT; ++i)
      cb_data_.time_uint[i] = uint4(NRAND, NRAND, NRAND, NRAND);     // [0..2^32-1]
    cb_data_.g_geom_frames_since_geom_clear = cur_set_->geom_frames_since_geom_clear_;
    memcpy(cb_data_.g_dialog_coeffs, dialog_coeffs_, sizeof(dialog_coeffs_[0]) * MAX_VECTOR_COEFFS);

    // Copy 'cb_data_' to 'shader_data_gpu_':
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    immed_context_->Map( shader_data_gpu_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ); 
    memcpy(MappedResource.pData, &cb_data_, sizeof(cb_data_));
    immed_context_->Unmap(shader_data_gpu_, 0);

    // 1. Render a small tiled quad to tex_depth_.
    if (cur_set_->gen_mode_ == GENERATE_GEOM)
    {
      ID3D11RenderTargetView*   rtargets[] = { cur_set_->depth[cur_set_->which_depth_].rtview };
      ID3D11ShaderResourceView* textures[] = { noise3D_full_[0].srview, 
                                               noise3D_full_[1].srview, 
                                               noise3D_full_[2].srview, 
                                               noise3D_full_[3].srview, 
                                               alt_map_1d_.srview, 
                                               noise1D_.srview, 
                                               main_.depth[1 - main_.which_depth_].srview,
                                               sincos_.srview,
                                               tex_noise_unorm_2d_.srview,
                                               noise3D_packed_[0].srview,
                                               noise3D_packed_[1].srview,
                                               noise3D_packed_[3].srview,
                                               noise3D_packed_[4].srview,
                                               cur_set_->depth[1 - cur_set_->which_depth_].srview,
                                               main_.norm.srview,
      };
      immed_context_->OMSetRenderTargets( sizeof(rtargets)/sizeof(rtargets[0]), rtargets, NULL );
      SetViewport(true);  // true = big image (supersampled), small = small image (backbuf-sized)
      immed_context_->VSSetShader( shader_vs_tile_, NULL, 0 );
      immed_context_->PSSetShader( shader_ps_geom_, NULL, 0 );
      immed_context_->PSSetShaderResources(0, sizeof(textures)/sizeof(textures[0]), textures);
      immed_context_->Draw(4, 0);
    }

    // 2. Render a small tiled quad to tex_normals_.
    if (cur_set_->gen_mode_ == GENERATE_NORMALS)
    {
      ID3D11RenderTargetView*   rtargets[] = { cur_set_->norm.rtview };
      ID3D11ShaderResourceView* textures[] = { noise3D_full_[0].srview, 
                                               noise3D_full_[1].srview, 
                                               noise3D_full_[2].srview, 
                                               noise3D_full_[3].srview, 
                                               alt_map_1d_.srview, 
                                               noise1D_.srview, 
                                               main_.depth[main_.which_depth_].srview, 
                                               sincos_.srview,
                                               noise3D_packed_[0].srview,
                                               noise3D_packed_[1].srview,
                                               noise3D_packed_[3].srview,
                                               noise3D_packed_[4].srview,
                                               refl_.depth[refl_.which_depth_].srview, 
                                               (cur_set_ == &refl_) ? main_.norm.srview : NULL, 
                                               normal_ray_dirs_1D_.srview
      };
      immed_context_->OMSetRenderTargets( sizeof(rtargets)/sizeof(rtargets[0]), rtargets, NULL );
      SetViewport(true);  // true = big image (supersampled), small = small image (backbuf-sized)
      immed_context_->VSSetShader( shader_vs_tile_, NULL, 0 );
      immed_context_->PSSetShader( shader_ps_norm_, NULL, 0 );
      immed_context_->PSSetShaderResources(0, sizeof(textures)/sizeof(textures[0]), textures);
      immed_context_->Draw(4, 0);
    }

    // 3. Render a small tiled quad to tex_ambo_.
    if (cur_set_->gen_mode_ == GENERATE_AMBO)
    {
      // Turn on additive alpha blending:  
      float blend_factor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; 
      immed_context_->OMSetBlendState( blend_add_, blend_factor, 0xffffffff );

      ID3D11RenderTargetView*   rtargets[] = { cur_set_->ambo.rtview };
      ID3D11ShaderResourceView* textures[] = { noise3D_full_[0].srview, 
                                               noise3D_full_[1].srview, 
                                               noise3D_full_[2].srview, 
                                               noise3D_full_[3].srview, 
                                               alt_map_1d_.srview, 
                                               noise1D_.srview, 
                                               main_.depth[main_.which_depth_].srview,
                                               main_.norm.srview,
                                               sincos_.srview,
                                               tex_noise_unorm_2d_.srview,
                                               ambo_ray_dirs_1D_.srview,
                                               noise3D_packed_[0].srview,
                                               noise3D_packed_[1].srview,
                                               noise3D_packed_[3].srview,
                                               noise3D_packed_[4].srview,
                                               refl_.depth[refl_.which_depth_].srview, 
                                               refl_.norm.srview, 
      };
      immed_context_->OMSetRenderTargets( sizeof(rtargets)/sizeof(rtargets[0]), rtargets, NULL );
      SetViewport(true);  // true = big image (supersampled), small = small image (backbuf-sized)
      immed_context_->VSSetShader( shader_vs_tile_, NULL, 0 );
      immed_context_->PSSetShader( shader_ps_ambo_, NULL, 0 );
      immed_context_->PSSetShaderResources(0, sizeof(textures)/sizeof(textures[0]), textures);
      immed_context_->Draw(4, 0);

      // Turn alpha blending back off:
      immed_context_->OMSetBlendState( blend_off_, blend_factor, 0xffffffff );
    }

    // 4. Relight: 
    //      Render a quad to the client area (to relight the scene, using depths + normals + ambo).
    //      It's usually a small quad, but can be full-screen if (relight_whole_client_==1).
    //    NOTE: If save_screenshot_==1, we'll instead render an FS quad to a large 
    //      temp buffer (the spare depth buffer, but as RGBA8888), and save that.
    if (relight_whole_client_ || cur_set_->gen_mode_ != GENERATE_NONE || save_screenshot_ == 1)
    {
      ID3D11ShaderResourceView* textures[] = { main_.depth[main_.which_depth_].srview, 
                                               main_.norm.srview, 
                                               main_.ambo.srview, 
                                               noise3D_full_[0].srview, 
                                               noise3D_full_[1].srview, 
                                               noise3D_full_[2].srview, 
                                               noise3D_full_[3].srview, 
                                               alt_map_1d_.srview, 
                                               noise1D_.srview,
                                               sincos_.srview,
                                               noise3D_packed_[0].srview,
                                               noise3D_packed_[1].srview,
                                               noise3D_packed_[3].srview,
                                               noise3D_packed_[4].srview,
                                               refl_.depth[refl_.which_depth_].srview, 
                                               refl_.norm.srview, 
                                               refl_.ambo.srview
                                               //tex_sky_.srview
      };
      ID3D11RenderTargetView* rtargets[] = { save_screenshot_ ? final_color_.rtview : rtview_backbuf_ };
      immed_context_->OMSetRenderTargets( sizeof(rtargets)/sizeof(rtargets[0]), rtargets, NULL );
      SetViewport(save_screenshot_ != 0);  // true = big image, small = client area
      immed_context_->VSSetShader( (relight_whole_client_ || save_screenshot_) ? shader_vs_fullscreen_ : shader_vs_tile_, NULL, 0 );
      immed_context_->PSSetShader( shader_ps_relight_, NULL, 0 );
      immed_context_->PSSetShaderResources(0, sizeof(textures)/sizeof(textures[0]), textures);
      immed_context_->Draw(4, 0);

      relight_whole_client_ = 0;

      if (save_screenshot_) {
        SaveImageToDiskUncompressed();
        save_screenshot_ = 0;
        SetFocus(hwnd_);
        SetForegroundWindow(hwnd_);
      }
    }

    /*
    // For now, always redraw the full client area.
    //TODO: only do it when (redraw_whole_client_ == 1), or, if cur_set_->gen_mode_ != GENERATE_NONE, 
    // also copy draw the tiny quad that was drawn, to the client area.
    if (true)//redraw_whole_client_ || cur_set_->gen_mode_ != GENERATE_NONE)
    {
      bool bEntireScreen = true;//(relight_whole_client_) ? 1 : 0;
      ID3D11RenderTargetView*   rtargets[] = { rtview_backbuf_ };
      ID3D11ShaderResourceView* textures[] = { full_size_color_.srview };
      immed_context_->OMSetRenderTargets( sizeof(rtargets)/sizeof(rtargets[0]), rtargets, NULL );
      SetViewport(false);  // true = big image (supersampled), small = small image (backbuf-sized)
      immed_context_->VSSetShader( bEntireScreen ? shader_vs_fullscreen_ : shader_vs_tile_, NULL, 0 );
      immed_context_->PSSetShader( shader_ps_client_, NULL, 0 );
      immed_context_->PSSetShaderResources(0, sizeof(textures)/sizeof(textures[0]), textures);
      immed_context_->Draw(4, 0);
      redraw_whole_client_ = 0;
    }
    */

    // Un-bind the textures when done with them, so we can render to them again later:
    ID3D11ShaderResourceView* null_textures[16] = { 0 };
    immed_context_->PSSetShaderResources(0, 16, null_textures);

    // Increment frame counter.
    cur_set_->frame_++;

    if (cur_set_->gen_mode_ == GENERATE_GEOM || cur_set_->gen_mode_ == GENERATE_NORMALS)
      cur_set_->ambo_dirty_ = true;

    // If we just finished the last tile, 
    // swap depth buffers & (maybe) render modes.
    if ((cur_set_->frame_ % cb_data_.kTilesTotal) == 0)
    {
      if (clear_at_end_of_pass_) {
        clear_at_end_of_pass_ = false;
        main_.clear_flags_ = 0xFFFFFFFF;
        refl_.clear_flags_ = 0xFFFFFFFF;
      }

      // Do end-of-frame bookkeeping based on what 'gen_mode' was
      // (before we go changing it).
      if (cur_set_->gen_mode_ == GENERATE_GEOM)
        cur_set_->geom_frames_since_geom_clear_++;
      if (cur_set_->gen_mode_ == GENERATE_NORMALS)
        cur_set_->normals_ready_ = 1;

      // Override 'next_gen_mode_' when appropriate.
      if (cur_set_->gen_mode_ == GENERATE_NORMALS && cur_set_->next_gen_mode_ == GENERATE_NORMALS)  // Auto switch back to NULL, after 1 normal pass, so that the depths & normals are in perfect sync.
        cur_set_->next_gen_mode_ = GENERATE_NONE;
      if (cur_set_->gen_mode_ == GENERATE_AMBO && cur_set_->next_gen_mode_ == GENERATE_AMBO && 
          cur_set_->ambo_pass_ >= kMaxAmboPasses) {
        onImageReady();
        cur_set_->next_gen_mode_ = GENERATE_NONE;
      }

      ApplyAutoPilot();

      //-----------------
      // Apply 'next_gen_mode_'.
      cur_set_->gen_mode_ = cur_set_->next_gen_mode_;  // Do this first, so we don't swap depth buffers if they're being frozen.
      //-----------------

      // Other setup based on the new 'gen_mode_'.
      if (cur_set_->gen_mode_ == GENERATE_GEOM)
        cur_set_->which_depth_ = 1 - cur_set_->which_depth_;
      if (cur_set_->gen_mode_ == GENERATE_AMBO && cur_set_->ambo_dirty_) {  // Auto clear *prev* ambo, if entering ambo mode @ end of a pass.
        cur_set_->clear_flags_ |= CLEAR_AMBO;
        cur_set_->ambo_dirty_ = false;
      }

      camera_frozen_ = camera_;
    } else if (cur_set_->gen_mode_ == GENERATE_NONE) {
      ApplyAutoPilot();
    }

    // Update window title.
    static const char anim[8][8] = { "o....", ".o...", "..o..", "...o.", "....o", "...o.", "..o..", ".o..." };
    char buf[256];
    sprintf_s(buf, sizeof(buf), "GpuCaster - %.1f MP - %d x %d - ss:%d - [%s] passes:[G=%d, A=%d] - [%s] %s %s viz=%s", 
              width_ * super_sample_*height_ * super_sample_*0.000001f,
              width_ * super_sample_, 
              height_ * super_sample_, 
              super_sample_, 
              (cur_set_ == &main_) ? "MAIN" : "REFL",
              cur_set_->geom_frames_since_geom_clear_,
              max(0, cur_set_->ambo_pass_ - 1),
              g_mode_names[cur_set_->gen_mode_], 
              anim[cur_set_->frame_ & 7],
              autopilot_ ? "AUTOPILOT" : "",
              g_vizmode_names[cb_data_.g_viz_mode]
    );
    SetWindowTextA(hwnd_, buf);

    // Sleep.
    if (cur_set_->gen_mode_ == GENERATE_NONE && cur_set_->next_gen_mode_ == GENERATE_NONE) {
      if (mouse_button_down_[0] || mouse_button_down_[1] || mouse_button_down_[2] || mouse_zoom_level_ > 0)
        Sleep(10);  // limit to ~100 Hz.
      else
        Sleep(200);  // limit to ~5 Hz.
    }

    swap_chain_->Present( 0, 0 );
  }

 protected:

  // Methods.
  double GetTimeInSeconds();
  //bool   SaveImageToDiskUsingD3DX();
  bool   SaveImageToDiskUncompressed();
  //bool   SaveImageToDiskCompressed();

  // Windows Data
  HWND hwnd_;
  HINSTANCE hinstance_;
  const int tile_edge_len_pixels_;
  TCHAR parent_dir_[1024];   // example: "c:\work\code\gpucaster\"

  int width_;   // The current size of the internal image.
  int height_;  // Client window size is this divided by super_sample_.
  int super_sample_;

  bool autopilot_;
  int  autopilot_main_geom_passes_left_;
  int  autopilot_main_ambo_passes_left_;
  int  autopilot_refl_geom_passes_left_;
  int  autopilot_refl_ambo_passes_left_;
  
  bool clear_at_end_of_pass_;
  int  relight_whole_client_;
  int  save_screenshot_;
  int  mouse_x_;
  int  mouse_y_;
  int  mouse_zoom_level_;
  bool mouse_button_down_[3];
  int  mouse_click_x_;
  int  mouse_click_y_;
  bool mouse_shift_;
  bool mouse_ctrl_;
  int  mouse_light_;    // 0..MAX_LIGHTS-1, or -1 if controlling the camera.
  CameraInfo* mouse_frame_;   // Alias to the CameraInfo frame we are currently mousing.

  // Misc. objects
  CameraInfo camera_;
  CameraInfo camera_frozen_;
  CameraInfo lights_[MAX_LIGHTS];
  ConfigBase* lighting_dialog_;
  stdext::hash_map<int, MinMax> lighting_sliders_;
  float4 dialog_coeffs_[MAX_VECTOR_COEFFS];

  // DX11 Data
  D3D_DRIVER_TYPE         driver_type_;
  D3D_FEATURE_LEVEL       feature_level_;
  ID3D11Device*           device_;
  ID3D11DeviceContext*    immed_context_;
  IDXGISwapChain*         swap_chain_;
  ID3D11RenderTargetView* rtview_backbuf_;

  // Shader-loading
 public:
  ID3D11Device* getDevice() { return device_; }
  ID3D11VertexShader*       shader_vs_fullscreen_;
  ID3D11VertexShader*       shader_vs_tile_;
  ID3D11PixelShader*        shader_ps_geom_;
  ID3D11PixelShader*        shader_ps_norm_;
  ID3D11PixelShader*        shader_ps_ambo_;
  ID3D11PixelShader*        shader_ps_relight_;
  ID3D11VertexShader*       temp_vs_quad_fullscreen_;
  ID3D11VertexShader*       temp_vs_quad_tile_;
  ID3D11PixelShader*        temp_ps_geom_;
  ID3D11PixelShader*        temp_ps_norm_;
  ID3D11PixelShader*        temp_ps_ambo_;
  ID3D11PixelShader*        temp_ps_relight_;
  HRESULT hr_load_shader_1;
  HRESULT hr_load_shader_2;
  HRESULT hr_load_shader_3;
  HRESULT hr_load_shader_4;
 protected:

  // Other DX11 demo-specific data
  ID3D11RasterizerState*    rasterstate_geom_;
  shader_constants          cb_data_;
  ID3D11Buffer*             shader_data_gpu_;
  RenderTarget              final_color_;
  IntersectionSet           main_;   // Initial (main) intersection from camera to surface.
  IntersectionSet           refl_;   // Depth at which reflection ray hit the surface.
  IntersectionSet*          cur_set_;
  Texture3D                 noise3D_full_[kNumNoiseCubes];
  Texture3D                 noise3D_packed_[kNumNoiseCubes];
  Texture1D                 noise1D_;
  TextureFromDisk           alt_map_1d_;
  Texture1D                 sincos_;
  Texture2D                 tex_noise_unorm_2d_;
  //TextureCube               tex_sky_;
  Texture1D                 ambo_ray_dirs_1D_;
  Texture1D                 normal_ray_dirs_1D_;
  ID3D11SamplerState*       sampler_linear_wrap_;
  ID3D11SamplerState*       sampler_linear_clamp_;
  ID3D11SamplerState*       sampler_nearest_wrap_;
  ID3D11SamplerState*       sampler_nearest_clamp_;
  ID3D11BlendState*         blend_off_;
  ID3D11BlendState*         blend_add_;
};

}  // namespace gpucaster

#endif  // _CODE_GPUCASTER_DEMO_H_