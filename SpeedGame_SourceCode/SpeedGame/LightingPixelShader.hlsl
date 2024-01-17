#include "BaseConstantBuffers.hlsli"
#include "PSConstantBufferHeader.hlsli"
#include "LightingFuncs.hlsli"

float4 main(VS_OUT input) : SV_Target
{
    //�e�N�X�`���J���[�̎擾
    float4 tex_color = Texture.Sample(Sampler, input.uv);

    //���_�V�F�[�_�Ōv�Z���ꂽDiffuse*Albedo���擾
    float4 diffuse = MakeDiffuseColor(input.wnml, input.color, dLightColor, dLightVector);
    
    //======Depth Shadow
    float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);		//�[�x�o�b�t�@�V���h�E�v�Z
    tex_color *= shadowColor;	//�e�N�X�`�����̂̐F�ɕ␳�l

    if (shadowColor.x < 1.0f)
    {
        tex_color.z *= 0.2f;	//�ł��Â�����
    }
    //======Depth Shadow End

    //�f�B�t���[�Y�J���[(Lambert + Ambient) * �e�N�X�`���J���[
    float4 finalColor = tex_color * (diffuse + AmbientColor);
    
    return finalColor;
}