
struct VS_INPUT
{
    float4 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 tex0 : TEXCOORD0;    //������UV�݂̂Ȃ̂�float2
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
    float4 tex8 : TEXCOORD8;
};

Texture2D NormalTex : register(t0, space0);     //Texture���X���b�g0��0�Ԗڂ̃e�N�X�`�����W�X�^�ɖ@���o�b�t�@�ݒ�
Texture2D DepthTex : register(t1, space0);      //Texture���X���b�g0��1�Ԗڂ̃e�N�X�`�����W�X�^�ɐ[�x�o�b�t�@�ݒ�
SamplerState Sampler : register(s0, space0);    //Sampler���X���b�g0��0�Ԗڂ̃T���v�����W�X�^�ɐݒ�