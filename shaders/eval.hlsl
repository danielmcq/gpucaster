#ifndef _EVAL_HLSL_
#define _EVAL_HLSL_

#include "master_include.hlsl"
#include "helper.hlsl"
#include "noise.hlsl"

// Here's a great reference for HLSL, the shader language that this function is written in: 
// http://msdn.microsoft.com/en-us/library/bb509638(v=VS.85).aspx
//
// Some built-in HLSL functions you might find very useful:
//   abs()
//   frac()
//   ceil()
//   floor()
//   lerp()
//   normalize()
//   saturate()
//   ...and so on.
//
// Also check the included headers above (helper, noise, etc.) for other useful functions:
//   RotatePointByQuaternion()
//   ReflectAboutPlane
//   MakeQuaternion
//   <functions to build dodecahedrons>
//   ...and so on.

struct EvalResult {
  float  density;
  float3 surface_color;
  float  reflectivity;
};

//--------------------------------------------------------------------------------------
// Eval - evaluates our implicit surface at a point in space.
//--------------------------------------------------------------------------------------
EvalResult Eval(
  // INPUTS:
  // There is only one input to our world-defining function:
  // 'ws', the 3D world-space coordinate of the point we're evaluating.
  float3 ws   
) {

  // This structure will hold our return value(s).
  // They describe the 'density' value at this point in space (positive = solid, negative or zero = air;
  //   the surface is formed along the boundary where this value crosses from positive to negative).
  EvalResult ret;
  ret.density = 0;
  ret.surface_color = float3(1, 1, 1);
  ret.reflectivity = 1;
   
  float ws_pixel_size = length(ws_eye_pos - ws) * g_ws_pixel_size_at_depth_1;

  // Each time we call the OctaveNoise() function, we'll pass in a new (incremented) noiseIndex 
  // value; this invokes a new instance of the noise, and keeps successive calls from returning 
  // the same value over and over.
  int noiseIndex = 0;
  
  // Let's hang onto a (const) version of the original world-space point, which you can always
  // refer back to if needed.  (You might want to push the ws coordinate around, for example,
  // using the OctaveNoise function, to create natural and rope-like warps in space.)
  const float3 ws_orig = ws;



  #if 0
    // OPTIONAL: Pre-warp the entire world by perturbing the coordinate (ws) by some 3D fractal noise.
    // Note: Using OctaveNoise() (even once) generally slows down shader load (compile) time A LOT,
    //   although after that, the actual GPU work (pixel shading) is pretty fast.
    {
      float wavelen = 0.5;
      float strength = 0.05;
      float3 delta = OctaveNoise(
          NOISE_HQ,                   // NOISE_SNAP, NOISE_REG, or NOISE_HQ.
          ws_orig,                    // *real* world-space pos (...otherwise nyquist gets botched)
          wavelen,                    // world-space wavelength of lowest frequency (ws size of 1 texel)
          ws_pixel_size,              // the ws size of a pixel (at this depth)
          4.0,                        // the min feature size to loop until, in pixels.
          false,                      // bSmoothBasedOnLowerFreqs
          false,                      // bCreased
          noiseIndex++,               // put 'noiseIndex++' here.
          MAX_OCTAVES                 // max # of octaves.
      ).xyz;
      ws += delta * strength;
    }
  #endif
  


  // Pick one of the following examples to render.
  // The first block with "#if 1" or "#elif 1" will be active, and the others 
  //   will be disabled.

  #if 0
    // A simple sphere at the origin, sitting on a plane:
    float sphere_radius = 0.3;
    float sphere = sphere_radius - length(ws);
    float plane  = -sphere_radius - ws.y;
    ret.density = max(sphere, plane);
  #elif 1
    // Blobs.
    ret.density = 0;
    for (int i = 0; i < 8; i++) {
      float3 blob_pos = seed_snorm[i].xyz * 0.4;
      float blob_field_rad = 0.4 + seed_unorm[i].w * 0.2;
      float t = length(ws - blob_pos) / blob_field_rad;
      // Pick one of the following:
      // (1) Cheap polynomial blobs:
      //ret.density += SmoothL2(1 - saturate(t)) - 0.25;
      // (2) High-quality blobs:
      ret.density += 0.1 / (t * t + 0.01) - 0.25;
    }  
    
    // Force everything below y == -0.2 to solid earth.
    if (ws.y < -0.2) ret.density = 1;
  #elif 0
    // Some reflective spheres:
    if (length(ws) < 0.9) {
      float3 ws2 = (ws + seed_snorm[7].xyz) * 10;
      
      float4 sn = SnapNoise4(ws2);
      float  sph_rad = 0.2 + 0.1*sn.w;
      float3 sph_ctr = floor(ws2) + 0.5 + sn.xyz*(0.5 - sph_rad);
      if (length(ws2 - sph_ctr) < sph_rad)
      {
        ret.density = 10;
        ret.surface_color = 0;
        ret.reflectivity = frac(sn.w * 17.31);
      }
    }        
  #elif 0
    // An infinite plane of dodecahedra and icosahedra.
    float  size = 0.2;
    float3 icosa_center = float3(0.25, -0.5, 0.25);
    float3 dodec_center = float3(-0.25, -0.5, -0.25);
    icosa_center.xz += floor(ws.xz) + 0.5;
    dodec_center.xz += floor(ws.xz) + 0.5;
    
    int i1 = 0;
    float closest_distance1 = 9999;
    float3 ws_dodec = (ws - dodec_center) / size;
    for (int i = 0; i < 12; i++) {
      float dist = length(dodec[i] - ws_dodec);
      if (dist < closest_distance1) {
        closest_distance1 = dist;
        i1 = i;
      }
    }
    
    int i2 = 0;
    float closest_distance2 = 9999;
    float3 ws_icosa = (ws - icosa_center) / size;
    for (i = 0; i < 20; i++) {
      float dist = length(icosa[i] - ws_icosa);
      if (dist < closest_distance2 && i != i2) {
        closest_distance2 = dist;
        i2 = i;
      }
    }

    float proj1 = dot(dodec[i1], ws_dodec);
    float proj2 = dot(icosa[i2], ws_icosa);
    ret.density = (proj1 < 1 || proj2 < 1) ? 1 : -1;
  #elif 0
    // A very cool, ropey-looking planet surface:
    float3 bump_wavelen = 0.7f;
    float3 pivot_delta = float3( RegNoise4(ws * 0.25f/bump_wavelen + seed_snorm[0].xyz*8).x,
                                 RegNoise4(ws * 0.25f/bump_wavelen + seed_snorm[1].xyz*8).y,
                                 RegNoise4(ws * 0.25f/bump_wavelen + seed_snorm[2].xyz*8).z ) * 12 * bump_wavelen;
    float wavelen = 4;
    float amp = 0.1;
    // Note: Prefixing for loops with 'LOOP_HINT' might help speed up shader compilation time.
    LOOP_HINT for (int i=0; i<8; i++)
    {
        float3 axis   = seed_dir[i].xyz;
        float  amount = RegNoise4( ws * (1.0/wavelen)*3 ).w * wavelen * amp;
        
        float3 pivot = ws + pivot_delta;
        ws = RotatePointAboutAxis(axis, pivot, ws, amount);
        
        wavelen = wavelen * 0.5;
        amp = amp * 0.9;
        //pivot_delta *= 0.5;
    }
    ret.density = -1 - ws.y;
  #elif 0
    // Honeycomb floor.
    float2 st;
    float2 xy_base = float2(floor(ws.x*0.5)*2, floor(ws.z));
    float x = frac(ws.x*0.5)*2;
    float y = frac(ws.z);
    if (abs(y-0.5)*2 < (abs(x-1.3333)-0.3333)*3)
    {
      if (x < 1.333)
        st = float2(0,0);
      else
        st = float2(2,0);
    }
    else if (y < 0.5)
      st = float2(1,0);
    else
      st = float2(1,1);
    float floor = tex_noise3D_full_a.SampleLevel(sam_nearest_wrap, float3(xy_base + st, 0) * (1.0/16.0), 0);
    ret.density = floor*0.2 - ws.y - 1;
    ret.reflectivity = 1;
  #elif 0         
    // Standard Mandelbox.
    const float3 c = ws * 20;
    const float scale = 2;     // TRY: -1.5, 1.5, 2
    const float r = 0.5f;
    const float f = 1;
    const int its = 8;
    float3 v = 0;    // c or 0?

    ret.density = -1;
    
    LOOP_HINT for (int i=0; i<its; i++) {
      
      // Box fold:
      if (v.x > 1) 
        v.x = 2 - v.x;
      else if (v.x < -1) 
        v.x = -2 - v.x;
        
      if (v.y > 1) 
        v.y = 2 - v.y;
      else if (v.y < -1) 
        v.y = -2 - v.y;
        
      if (v.z > 1) 
        v.z = 2 - v.z;
      else if (v.z < -1) 
        v.z = -2 - v.z;

      v = v*f;
      
      // Ball fold:
      float mag = length(v);
      if (mag < r) 
        v *= 1.0/(r*r);
      else if (mag < 1) 
        v /= mag*mag;
      
      v = v*scale + c;
    }
    
    float max_dim = max(abs(v.x), max(abs(v.y), abs(v.z)));
    
    //ret.density = (length(v) > 3) ? -1 : 1;
    ret.density = (max_dim > 4) ? -1 : 1;      // thresh determines the fullness of the volume.
    ret.reflectivity = 1;
  #elif 0
    // Standard mandelbulb.
    float n = 8;
    ws = ws.yzx * 2.5;    // Scale it down (by scaling space UP), and rotate it (by rotating space).
    const float3 ws_start = ws;
    
    LOOP_HINT for (int i = 0; i < 6; i++) {
      float r = length(ws);
      float theta = atan2(ws.y, ws.x);
      float phi = atan2(ws.z, length(ws.xy));
      
      float3 ws2;
      ws2.x = pow(r, n) * sin(theta*n) * cos(phi*n);
      ws2.y = pow(r, n) * sin(theta*n) * sin(phi*n);
      ws2.z = pow(r, n) * cos(theta*n);
      ws = ws2 + ws_start * seed_unorm[0].x;
    } 
    
    if (length(ws) < 1) {
      ret.surface_color = 0.9 + 0.3*g_light_unorm[0].xyz;
      ret.density = 1;
    }      
  #elif 0
    float str = 0.1;
    
    // Double pre-warp, ** driven by values you can manually adjust, using a dialog box. **
    // Compiling shaders with OctaveNoise calls is very slow.  So, to get around
    //   this, you can set up your shader (this function) with the code you want,
    //   and then interactively update the numeric values used in the shader --
    //   without recompiling it.  Just hit CTRL+0, for example, to bring up a 
    //   dialog that lets you edit 4 values, which you can access here - in the
    //   shader - using g_c0.xyzw.  Similarly, hit CTRL+1 to edit g_c1.xyzw,
    //   and so on.  Once you've edited the values, hit OK, then back in the 
    //   rendering window, it 'C' (clear) to restart the rendering, using the
    //   new values, but *without* recompiling any shaders.
    
    // Pre-warp 'ws'.
    float3 delta = OctaveNoise(
        NOISE_HQ,                   // NOISE_SNAP, NOISE_REG, or NOISE_HQ.
        ws,                         // *real* world-space pos (...otherwise nyquist gets botched)
        g_c1.x,                     // wavelen (ws size of 1 texel)
        ws_pixel_size,              // the ws size of a pixel (at this depth)
        g_c0.z,                     // the min feature size to loop until, in pixels.
        false,                      // bSmoothBasedOnLowerFreqs
        false,                      // bCreased
        noiseIndex++,               // put 'noiseIndex++' here.
        MAX_OCTAVES                 // max # of octaves.
    ).xyz;
    ws += delta * str * g_c1.y;

    // Do it again.
    delta = OctaveNoise(
        NOISE_HQ,                   // NOISE_SNAP, NOISE_REG, or NOISE_HQ.
        ws,                         // *real* world-space pos (...otherwise nyquist gets botched)
        g_c1.z,                     // wavelen (ws size of 1 texel)
        ws_pixel_size,              // the ws size of a pixel (at this depth)
        1,                          // the min feature size to loop until, in pixels.
        false,                      // bSmoothBasedOnLowerFreqs
        false,                      // bCreased
        noiseIndex++,               // put 'noiseIndex++' here.
        MAX_OCTAVES                 // max # of octaves.
    ).xyz;
    ws += delta * str * g_c1.w;

    // ...plus a simple ground plane, to show it.
    ret.density = -0.5 - ws.y;
  #elif 1
    // Fractal perturbations on the surface of a dodecahedron.  (Slow)
    ws *= 3.0f;   
    int iters = 10;
    for (int loop = 0; loop < iters; loop++) {
      // To which face center on the dodecahedron are we closest?
      int i1 = 0;
      float closest_distance1 = 9999;
      for (int i = 0; i < 12; i++) {
        float dist = length(dodec[i] - ws);
        if (dist < closest_distance1) {
          closest_distance1 = dist;
          i1 = i;
        }
      }
      float3 nearest_dodec_face_center = dodec[i1];

      // To which vertex on the dodecahedron (or face center on the icosahedron) are we closest?
      int i2 = 0;
      float closest_distance2 = 9999;
      for (i = 0; i < 20; i++) {
        float dist = length(icosa[i] - ws);
        if (dist < closest_distance2 && i != i2) {
          closest_distance2 = dist;
          i2 = i;
        }
      }
      float3 nearest_dodec_vertex = icosa[i2];

      float proj1 = dot(dodec[i1], ws);
      float proj2 = dot(icosa[i2], ws);
      if (proj1 < 1
          && loop == iters - 1) {
        ret.density = 1;
      }
      
      float3 pivot = nearest_dodec_face_center;
      float3 axis0 = normalize(nearest_dodec_face_center);
      float3 axis1 = normalize(nearest_dodec_face_center - nearest_dodec_vertex);
      float3 axis2 = cross(axis0, axis1);
      ws = RotatePointAboutAxis(axis2, pivot, ws, seed_snorm[0].y * 0.5);
      
      float st = length(ws - axis0 * dot(ws, axis0)) * 0.3f;
      ws = RotatePointAboutAxis(axis0, nearest_dodec_face_center, ws, st);
      
      ws = (ws - nearest_dodec_vertex) * 2.3 + nearest_dodec_vertex;
      
      // Avoid +INF's causing issues at high # of iters.
      if (length(ws) > 10) {
        ws = normalize(ws) * 10;
      }
    }

    ret.density = (length(ws) < 1.0) ? 1 : -1;     
  #endif    

  return ret;
}

#endif  // _EVAL_HLSL_