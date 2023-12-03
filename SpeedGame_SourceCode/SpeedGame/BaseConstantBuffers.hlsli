struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 wnml : NORMAL0;      //ワールド座標系の法線
    
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer ViewBuffer : register(b0)
{
    float4x4 View;  //ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1)
{
    float4x4 Projection;    //透視投影(Proj)変換行列
}

cbuffer ModelBuffer : register(b2)
{
    float4x4 Model; //モデル(ワールド)変換行列
}

cbuffer DirectionalLight : register(b3)
{
    float3 dLightColor;     //色
    float3 dLightVector;    //向き
}