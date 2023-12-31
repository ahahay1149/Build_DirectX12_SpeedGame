#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    //======Normal Map
    float3 tan : TANGENT0;
    //======Normal Map End
    float4 color : COLOR0;  //Albedoに相当
    float2 uv : TEXTURE0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
	
    output.pos = float4(input.pos, 1.0f);
	
    output.pos = mul(output.pos, Model);
    //======Specular
    //ビュー座標にする前の座標値をoutputに保存
    output.wpos = output.pos;
    //======Specular End
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //法線をワールド座標系に変換
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(normal, Model);
    normal = normalize(normal);     //正規化
    
    output.wnml = normal;   //PSに渡す用
    
    //======Normal Map
    float4 tangent = float4(input.tan, 0.0f);
    tangent = mul(tangent, Model);
    tangent = normalize(tangent); //正規化

    output.wtan = tangent; //PSに渡す値
    output.wbnml = float4(normalize(cross(normal.xyz, tangent.xyz)), 0.0f);
    //======Normal Map End

    //======Depth Shadow
    // ライトから見たプロジェクション座標を保存
    output.lightPos = mul(output.wpos, lightViewProjection);
    //======Depth Shadow End
    
    //PixelShaderで処理するのでそのまま渡す
    output.color = input.color;
    
    //Texture座標指定
    output.uv = input.uv;
	
	return output;
}