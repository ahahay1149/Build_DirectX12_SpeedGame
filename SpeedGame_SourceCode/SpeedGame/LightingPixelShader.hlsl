#include "BaseConstantBuffers.hlsli"
#include "PSConstantBufferHeader.hlsli"
#include "LightingFuncs.hlsli"

float4 main(VS_OUT input) : SV_Target
{
    //テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, input.uv);

    //頂点シェーダで計算されたDiffuse*Albedoを取得
    float4 diffuse = MakeDiffuseColor(input.wnml, input.color, dLightColor, dLightVector);
    
    //======Depth Shadow
    float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);		//深度バッファシャドウ計算
    tex_color *= shadowColor;	//テクスチャ自体の色に補正値

    if (shadowColor.x < 1.0f)
    {
        tex_color.z *= 0.2f;	//最も暗くする
    }
    //======Depth Shadow End

    //ディフューズカラー(Lambert + Ambient) * テクスチャカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor);
    
    return finalColor;
}