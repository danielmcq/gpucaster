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

FSQuadVsOut VS(FSQuadVSIn input)
{
    float u = (float)(input.Pos % 2);   // [0..1]
    float v = (float)(input.Pos / 2);   // [0..1]
    
    #ifndef RENDER_ENTIRE_FULLSCREEN_QUAD
      u = (u + ((g_cur_frame                 ) % (kTilesAcross))) * (kInvTilesAcross);
      v = (v + ((g_cur_frame / (kTilesAcross)) % (kTilesDown  ))) * (kInvTilesDown  );
    #endif
    
    FSQuadVsOut Output;
    
    // Note that screen-space y is flipped, in D3D.
    Output.pos = float4(u*2-1, 1-v*2, 0.5f, 1);     //mul( vCreatedPosition, g_mWorldViewProj );
    Output.uv  = float2(u, v);
    return Output;
}

#endif  // _FULLSCREEN_QUAD_HLSL_