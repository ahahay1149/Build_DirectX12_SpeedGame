Texture2D<float4> SrcTexture : register(t0, space0);    //元テクスチャ
RWTexture2D<float4> DstTexture : register(u0, space0);  //MipMap生成先
SamplerState BilinearClamp : register(s0, space0);      //サンプラー

cbuffer CB : register(b0, space0)
{
    float2 TexelSize;
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //TexelSizeをUV値に変換
    float2 texcoords = TexelSize * (DTid.xy + 0.5);
    
    //Bilinearフィルタに設定しているのでサンプル座標の上下４点をサンプリング
    float4 color = SrcTexture.SampleLevel(BilinearClamp, texcoords, 0);

    //宛先テクスチャのピクセル座標位置に色データを書き込み
    DstTexture[DTid.xy] = color;
}