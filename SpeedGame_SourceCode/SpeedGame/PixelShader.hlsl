#include "Header.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    //return color;
    color.xyz = color.xyz * input.Col.xyz;  //RGB‚ðŠ|‚¯ŽZ
    color.w *= input.Col.w;                 //A‚ÍŠ|‚¯ŽZ
    return color;
}
