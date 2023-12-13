
cbuffer cbAmbientColor : register(b4)
{
    float4 AmbientColor;
}

Texture2D Texture : register(t0, space0);       //Texture���X���b�g0��0�Ԗڂ̃e�N�X�`�����W�X�^�ɐݒ�
SamplerState Sampler : register(s0, space0);    //Sampler���X���b�g0��0�Ԗڂ̃T���v�����W�X�^�ɐݒ�

//======Specular + Normal
// �e�N�X�`���ǉ�
Texture2D SpecularTex : register(t1, space0);   //Texture���X���b�g0��1�Ԗڂ̃e�N�X�`�����W�X�^�ɐݒ�
Texture2D NormalTex : register(t2, space0);     //Texture���X���b�g0��2�Ԗڂ̃e�N�X�`�����W�X�^�ɐݒ�
//======Specular + Normal End

// �ǂ̃e�N�X�`���������Ă��邩�̃t���O
#define TEX_DIFFUSE 0x01
#define TEX_NORMAL 0x02
#define TEX_SPECULAR 0x04

//View�s��ɓ������Ă��������̂́A�g�����̂��ߍ���͕ʁX�ɂ��Ďg�p
cbuffer cbCameraPosition : register(b5)
{
    float3 CameraPos; //�J�������W
}

//======Specular + Normal
// normalize & cpecular texture + Material color
cbuffer cbMaterialInfo : register(b6)
{
    float4 MaterialDiffuse;
    float4 MaterialAmbient;
    float4 MaterialSpecular;
    
    uint TextureFlag;
}
//======Specular + Normal End