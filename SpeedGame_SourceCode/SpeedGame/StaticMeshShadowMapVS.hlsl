
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
    float4x4 View;          //�r���[�ϊ��s��
}

cbuffer ProjectionBuffer : register(b1)
{
    float4x4 Projection;    //�������e�ϊ��s��
}

cbuffer ModelBuffer : register(b2)
{
    float4x4 Model;         //���[���h�ϊ��s��
}

float4 main(VS_IN input) : SV_Position
{
    float4 pos;
    
    //���W�ϊ�
    pos = float4(input.pos, 1.0f);
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    return pos;
}