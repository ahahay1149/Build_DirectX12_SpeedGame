
struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float3 tan : TANGENT0;
    float4 color : COLOR0;
    float2 uv : TEXTURE0;
};

cbuffer ViewBuffer : register(b0)
{
    float4x4 View;          //ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1)
{
    float4x4 Projection;    //透視投影変換行列
}

cbuffer ModelBuffer : register(b2)
{
    float4x4 Model;         //ワールド変換行列
}

float4 main(VS_IN input) : SV_Position
{
    float4 pos;
    
    //座標変換
    pos = float4(input.pos, 1.0f);
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    return pos;
}