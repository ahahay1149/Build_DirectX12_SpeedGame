#include "BaseConstantBuffers.hlsli"
#include "PSConstantBufferHeader.hlsli"
#include "LightingFuncs.hlsli"

float4 main(VS_OUT input) : SV_Target
{
    //テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, input.uv);

    //頂点シェーダで計算されたDiffuse*Albedoを取得
    float4 diffuse = MakeDiffuseColor(input.wnml, input.color, dLightColor, dLightVector);
    
    //ディフューズカラー(Lambert + Ambient) * テクスチャカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor);
    
    return finalColor;
}