#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"
#include "PSConstantBufferHeader.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{
    float2 uv = input.uv;
    float4 normal = input.wnml;
    
    //======Normal Map
    // Normal Textureを持っている場合のみノーマルマップによる法線修正を計算
    if (TextureFlag & TEX_NORMAL)
    {
        float3 nmlTex = NormalTex.Sample(Sampler, uv);
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, nmlTex.xyz);
    }
    //======Normal Map End
    
    //テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    //Diffuse * Albedo カラー計算
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    float4 specular = 0;
    
    //======Specular Map
    if (TextureFlag & TEX_SPECULAR)  //テクスチャ所持チェック
    {
        float4 sptex; //スペキュラーマップの色
        sptex = SpecularTex.Sample(Sampler, uv);
    
        //======Phong反射モデル
        // Phong反射の反射光計算式
        float3 refVect = dLightVector + 2.0f * dot(-dLightVector, normal.xyz) * normal.xyz;
        
        // 視線ベクトルの作成
        float3 viewRect = normalize(CameraPos - input.wpos.xyz); //正規化
        
        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz * pow(saturate(dot(refVect, viewRect)), MaterialSpecular.w);
        //======Phong反射モデル End
    }
    //======Specular Map End

    //ディフューズカラー(Light * Ambient) * テクスチャカラー + スペキュラーカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    finalColor.w = tex_color.w;
    
    return finalColor;
}