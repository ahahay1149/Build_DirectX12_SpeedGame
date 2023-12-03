
cbuffer cbAmbientColor : register(b4)
{
    float4 AmbientColor;
}

Texture2D Texture : register(t0, space0);       //Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0, space0);    //Samplerをスロット0の0番目のサンプラレジスタに設定