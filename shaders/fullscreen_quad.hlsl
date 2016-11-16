#ifndef _FULLSCREEN_QUAD_HLSL_
#define _FULLSCREEN_QUAD_HLSL_

//--------------------------------------------------------------------------------------
// Vertex Shader to draw a fullscreen quad (no input geometry necessary).
//--------------------------------------------------------------------------------------

#include "constants.hlsl"

struct FSQuadVSIn
{
    uint Pos : SV_VertexID;
};

struct FSQuadVsOut
{
    float4 pos                  : SV_Position;  // Screen-space position [-1..1] of pixel
    float2 uv                   : TEXCOORD;     // Interesting... PS can't read 'pos'.
    float3 some_ws_point_on_ray : TEXCOORD2;    // WS pos of some point on the ray.
};

FSQuadVsOut VS(FSQuadVSIn input)
{
    float u = (float)(input.Pos % 2);   // [0..1]
    float v = (float)(input.Pos / 2);   // [0..1]
    
    #ifndef RENDER_ENTIRE_FULLSCREEN_QUAD
      u = (u + ((g_frame            ) % (TILES_X))) * (INV_TILES_X);
      v = (v + ((g_frame / (TILES_X)) % (TILES_Y))) * (INV_TILES_Y);
    #endif
    
    FSQuadVsOut Output;
    
    // Note that screen-space y is flipped, in D3D.
    Output.pos = float4(u*2-1, 1-v*2, 0.5f, 1);     //mul( vCreatedPosition, g_mWorldViewProj );
    Output.uv  = float2(u, v);
    Output.some_ws_point_on_ray = lerp( lerp(ws_screen_corner_UL, ws_screen_corner_UR, u),
                                        lerp(ws_screen_corner_LL, ws_screen_corner_LR, u),
                                        v );
    return Output;
}

#endif  // _FULLSCREEN_QUAD_HLSL_