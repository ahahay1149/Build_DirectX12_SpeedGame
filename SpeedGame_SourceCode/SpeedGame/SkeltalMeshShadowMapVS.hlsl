
struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float3 tan : TANGENT0;
    float4 color : COLOR0;
    float2 uv : TEXTURE0;
    uint4 indices0 : BLENDINDICES0;
    uint4 indices1 : BLENDINDICES1;
    float4 weights0 : BLENDWEIGHT0;
    float4 weights1 : BLENDWEIGHT1;
};

cbuffer ViewBuffer : register(b0)
{
    float4x4 View;      //ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1)
{
    float4x4 Projection;//透視投影変換行列
}

cbuffer ModelBuffer : register(b2)
{
    float4x4 Model;     //ワールド変換行列
}

cbuffer MeshBone : register(b3)
{
    float4x4 bones[2];  //ボーン用配列
}

float4 main(VS_IN input) : SV_Position
{
    float4 pos;
    pos = float4(input.pos, 1.0f);
    
    //スキンアニメマトリクス反映
    float4x4 boneMtx;
    boneMtx =  bones[input.indices0[0]] * input.weights0[0];
    boneMtx += bones[input.indices0[1]] * input.weights0[1];
    boneMtx += bones[input.indices0[2]] * input.weights0[2];
    boneMtx += bones[input.indices0[3]] * input.weights0[3];
    boneMtx += bones[input.indices1[0]] * input.weights1[0];
    boneMtx += bones[input.indices1[1]] * input.weights1[1];
    pos = mul(boneMtx, pos);
    
    //座標変換
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    
    return pos;
}