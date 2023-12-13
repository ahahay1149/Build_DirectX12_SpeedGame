
cbuffer cbAmbientColor : register(b4)
{
    float4 AmbientColor;
}

Texture2D Texture : register(t0, space0);       //Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0, space0);    //Samplerをスロット0の0番目のサンプラレジスタに設定

//======Specular + Normal
// テクスチャ追加
Texture2D SpecularTex : register(t1, space0);   //Textureをスロット0の1番目のテクスチャレジスタに設定
Texture2D NormalTex : register(t2, space0);     //Textureをスロット0の2番目のテクスチャレジスタに設定
//======Specular + Normal End

// どのテクスチャを持っているかのフラグ
#define TEX_DIFFUSE 0x01
#define TEX_NORMAL 0x02
#define TEX_SPECULAR 0x04

//View行列に統合してもいいものの、拡張性のため今回は別々にして使用
cbuffer cbCameraPosition : register(b5)
{
    float3 CameraPos; //カメラ座標
}

//======Specular + Normal
// normalize & cpecular texture + Material color
cbuffer cbMaterialInfo : register(b6)
{
    float4 MaterialDiffuse;
    float4 MaterialAmbient;
    float4 MaterialSpecular;
    
    uint TextureFlag;
}
//======Specular + Normal End