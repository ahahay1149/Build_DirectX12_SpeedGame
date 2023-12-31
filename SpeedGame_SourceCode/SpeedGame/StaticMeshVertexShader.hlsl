#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    //======Normal Map
    float3 tan : TANGENT0;
    //======Normal Map End
    float4 color : COLOR0;  //Albedo�ɑ���
    float2 uv : TEXTURE0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
	
    output.pos = float4(input.pos, 1.0f);
	
    output.pos = mul(output.pos, Model);
    //======Specular
    //�r���[���W�ɂ���O�̍��W�l��output�ɕۑ�
    output.wpos = output.pos;
    //======Specular End
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //�@�������[���h���W�n�ɕϊ�
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(normal, Model);
    normal = normalize(normal);     //���K��
    
    output.wnml = normal;   //PS�ɓn���p
    
    //======Normal Map
    float4 tangent = float4(input.tan, 0.0f);
    tangent = mul(tangent, Model);
    tangent = normalize(tangent); //���K��

    output.wtan = tangent; //PS�ɓn���l
    output.wbnml = float4(normalize(cross(normal.xyz, tangent.xyz)), 0.0f);
    //======Normal Map End

    //======Depth Shadow
    // ���C�g���猩���v���W�F�N�V�������W��ۑ�
    output.lightPos = mul(output.wpos, lightViewProjection);
    //======Depth Shadow End
    
    //PixelShader�ŏ�������̂ł��̂܂ܓn��
    output.color = input.color;
    
    //Texture���W�w��
    output.uv = input.uv;
	
	return output;
}