#include "Header.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    //return color;
    color.xyz = color.xyz * input.Col.xyz;  //RGB���|���Z
    color.w *= input.Col.w;                 //A�͊|���Z
    return color;
}
