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
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, uv).xyz);
    }
    //======Normal Map End
    
    //テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    //Diffuse * Albedo カラー計算
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    float4 specular = 0;
    
    //======Specular Map
    if (TextureFlag & TEX_SPECULAR)
    {
        float4 sptex; //スペキュラーマップの色
        sptex = SpecularTex.Sample(Sampler, uv);
    
        //======Blinn-Phong反射モデル
        //視線ベクトル
        float3 viewVect = normalize(CameraPos - input.wpos.xyz); //正規化
        
        //ハーフベクトル
        //反射点基準の視線ベクトルと入射光ベクトルを足して正規化
        float3 halfVect = normalize(-dLightVector + viewVect);
        
        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz * pow(saturate(dot(halfVect, normal.xyz)), MaterialSpecular.w);
        //======Blinn-Phong反射モデル End
    }
    //======Specular Map End
    
    //テクスチャカラー * (ディフューズカラー(Light + Ambient)) + スペキュラーカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    finalColor.w = tex_color.w;
    
    return finalColor;
}