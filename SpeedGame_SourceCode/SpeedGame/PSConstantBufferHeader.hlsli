
cbuffer cbAmbientColor : register(b4)
{
    float4 AmbientColor;
}

Texture2D Texture : register(t0, space0);       //Texture���X���b�g0��0�Ԗڂ̃e�N�X�`�����W�X�^�ɐݒ�
SamplerState Sampler : register(s0, space0);    //Sampler���X���b�g0��0�Ԗڂ̃T���v�����W�X�^�ɐݒ�