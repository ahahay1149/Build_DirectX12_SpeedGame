#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"
#include "PSConstantBufferHeader.hlsli"

struct TONE_OUT	//�s�N�Z���V�F�[�_�o�͂Ɏg���\����
{
	float4 outColor : SV_TARGET0;
	//======Edge Draw
	float4 outNormal : SV_TARGET1;	//�@��
	//======Edge Draw End
};

Texture2D ToneTex : register(t3, space0);	//Texture���X���b�g0��3�Ԗڂ̃e�N�X�`�����W�X�^�ɐݒ�

TONE_OUT main(VS_OUT input)
{
	TONE_OUT res;
	
	float2 uv = input.uv;
	float4 normal = input.wnml;

	//�e�N�X�`���J���[�̎擾
	float4 tex_color = Texture.Sample(Sampler, uv);

	//Alpha Test
	if (tex_color.w < 0.1f)
		discard;

	//Normal Check
	if (TextureFlag & TEX_NORMAL)
	{
		normal = GetNormalVect(input.wnml.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, uv).xyz);
	}

	float p;

	//======Depth Shadow
	float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);			//�[�x�o�b�t�@�V���h�E�v�Z
	tex_color *= shadowColor;	//�e�N�X�`�����̂̐F�ɕ␳�l

	if (shadowColor.x < 1.0f)
	{
		p = 0.2f;	//�ł��Â�����
	}
	else
	{
		p = dot(input.wnml.xyz, -dLightVector.xyz);
		p = p * 0.5f + 0.5;		//HalfLambert����
		p = p * p;
	}
	//======Depth Shadow End

	float4 toneCL = ToneTex.Sample(Sampler, float2(p, 0.0f));
	res.outColor.xyz = tex_color.xyz * toneCL.xyz;

	res.outColor.w = tex_color.w * input.color.w;

	//======Edge Draw
	res.outNormal = float4(normal.x, normal.y, normal.z, 1.0f);
	//======Edge Draw End

    return res;
}