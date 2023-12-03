#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float4 color : COLOR0;  //Albedoに相当
    float2 uv : TEXTURE0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
	
    output.pos = float4(input.pos, 1.0f);
	
    output.pos = mul(output.pos, Model);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //法線をワールド座標系に変換
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(normal, Model);
    normal = normalize(normal);     //正規化
    
    output.wnml = normal;   //PSに渡す用
    
    //本体処理呼び出し
    output.color = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    //Texture座標指定
    output.uv = input.uv;
	
	return output;
}