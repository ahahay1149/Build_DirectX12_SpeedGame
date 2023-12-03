#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float4 color : COLOR0;  //Albedo�ɑ���
    float2 uv : TEXTURE0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
	
    output.pos = float4(input.pos, 1.0f);
	
    output.pos = mul(output.pos, Model);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //�@�������[���h���W�n�ɕϊ�
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(normal, Model);
    normal = normalize(normal);     //���K��
    
    output.wnml = normal;   //PS�ɓn���p
    
    //�{�̏����Ăяo��
    output.color = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    //Texture���W�w��
    output.uv = input.uv;
	
	return output;
}