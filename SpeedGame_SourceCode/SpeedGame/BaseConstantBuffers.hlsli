struct VS_OUT
{
    float4 pos : SV_POSITION;
    
    //======Specular
    float4 wpos : POSITION0;    //���[���h���W�n�̍��W
    //======Specular End
    
    float4 wnml : NORMAL0;      //���[���h���W�n�̖@��
    
    //======Normal Map
    float4 wtan : TANGENT0;     //���[���h���W�n�ł̖@���ɑ΂���tangent�x�N�g��
    float4 wbnml : NORMAL1;     //B1Normal
    //======Normal Map End
    
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer ViewBuffer : register(b0)
{
    float4x4 View;  //�r���[�ϊ��s��
}

cbuffer ProjectionBuffer : register(b1)
{
    float4x4 Projection;    //�������e(Proj)�ϊ��s��
}

cbuffer ModelBuffer : register(b2)
{
    float4x4 Model; //���f��(���[���h)�ϊ��s��
}

cbuffer DirectionalLight : register(b3)
{
    float3 dLightColor;     //�F
    float3 dLightVector;    //����
}