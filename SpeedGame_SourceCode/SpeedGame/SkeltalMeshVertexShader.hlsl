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
    
    uint4 indices0 : BLENDINDICES0;
    uint4 indices1 : BLENDINDICES1;
    float4 weights0 : BLENDWEIGHT0;
    float4 weights1 : BLENDWEIGHT1;
};

cbuffer MeshBone : register(b4)
{
    float4x4 bones[2];  //ボーン用配列
}

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.pos = float4(input.pos, 1.0f);
    
    //スケルタルメッシュ・アニメマトリクス反映
    float4x4 boneMtx;
    
    boneMtx =  bones[input.indices0[0]] * input.weights0[0];
    boneMtx += bones[input.indices0[1]] * input.weights0[1];
    boneMtx += bones[input.indices0[2]] * input.weights0[2];
    boneMtx += bones[input.indices0[3]] * input.weights0[3];
    boneMtx += bones[input.indices1[0]] * input.weights1[0];
    boneMtx += bones[input.indices1[1]] * input.weights1[1];
    
    output.pos = mul(boneMtx, output.pos);
    
    output.pos = mul(output.pos, Model);
    //======Specular
    //ビュー座標にする前の座標値をoutputに保存
    output.wpos = output.pos;
    //======Specular End
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    
    //Lambert反射モデル実装
    //法線をワールド座標系に変換
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(boneMtx, normal);
    normal = mul(normal, Model);
    normal = normalize(normal); //正規化
    
    output.wnml = normal;       //PSに渡す用
    
    //======Normal Map
    float4 tangent = float4(input.tan, 0.0f);
    tangent = mul(boneMtx, tangent);
    tangent = mul(tangent, Model);
    tangent = normalize(tangent); //正規化

    output.wtan = tangent; //PSに渡す値
    output.wbnml = float4(normalize(cross(normal.xyz, tangent.xyz)), 0.0f);
    //======Normal Map End
    
    //本体処理呼び出し
    output.color = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    //Texture座標指定
    output.uv = input.uv;
    
    return output;
}