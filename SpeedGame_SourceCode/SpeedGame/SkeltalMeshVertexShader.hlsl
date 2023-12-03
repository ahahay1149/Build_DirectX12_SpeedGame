#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float4 color : COLOR0;  //Albedo�ɑ���  
    float2 uv : TEXTURE0;
    
    uint4 indices0 : BLENDINDICES0;
    uint4 indices1 : BLENDINDICES1;
    float4 weights0 : BLENDWEIGHT0;
    float4 weights1 : BLENDWEIGHT1;
};

cbuffer MeshBone : register(b4)
{
    float4x4 bones[2];  //�{�[���p�z��
}

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.pos = float4(input.pos, 1.0f);
    
    //�X�P���^�����b�V���E�A�j���}�g���N�X���f
    float4x4 boneMtx;
    
    boneMtx =  bones[input.indices0[0]] * input.weights0[0];
    boneMtx += bones[input.indices0[1]] * input.weights0[1];
    boneMtx += bones[input.indices0[2]] * input.weights0[2];
    boneMtx += bones[input.indices0[3]] * input.weights0[3];
    boneMtx += bones[input.indices1[0]] * input.weights1[0];
    boneMtx += bones[input.indices1[1]] * input.weights1[1];
    
    output.pos = mul(boneMtx, output.pos);
    
    output.pos = mul(output.pos, Model);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //Lambert���˃��f������
    //�@�������[���h���W�n�ɕϊ�
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(boneMtx, normal);
    normal = mul(normal, Model);
    normal = normalize(normal); //���K��
    output.wnml = normal;       //PS�ɓn���p
    
    //�{�̏����Ăяo��
    output.color = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    //Texture���W�w��
    output.uv = input.uv;
    
    return output;
}