#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"
#include "PSConstantBufferHeader.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{
    float2 uv = input.uv;
    float4 normal = input.wnml;
    
    //======Normal Map
    // Normal Texture�������Ă���ꍇ�̂݃m�[�}���}�b�v�ɂ��@���C�����v�Z
    if (TextureFlag & TEX_NORMAL)
    {
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, uv).xyz);
    }
    //======Normal Map End
    
    //�e�N�X�`���J���[�̎擾
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    //Diffuse * Albedo �J���[�v�Z
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    float4 specular = 0;
    
    //======Specular Map
    if (TextureFlag & TEX_SPECULAR)
    {
        float4 sptex; //�X�y�L�����[�}�b�v�̐F
        sptex = SpecularTex.Sample(Sampler, uv);
    
        //======Blinn-Phong���˃��f��
        //�����x�N�g��
        float3 viewVect = normalize(CameraPos - input.wpos.xyz); //���K��
        
        //�n�[�t�x�N�g��
        //���˓_��̎����x�N�g���Ɠ��ˌ��x�N�g���𑫂��Đ��K��
        float3 halfVect = normalize(-dLightVector + viewVect);
        
        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz * pow(saturate(dot(halfVect, normal.xyz)), MaterialSpecular.w);
        //======Blinn-Phong���˃��f�� End
    }
    //======Specular Map End

    //======Depth Shadow
    float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);		//�[�x�o�b�t�@�V���h�E�v�Z
    tex_color *= shadowColor;	//�e�N�X�`�����̂̐F�ɕ␳�l

    if (shadowColor.x < 1.0f)
    {
        tex_color.z *= 0.2f;	//�ł��Â�����
    }
    //======Depth Shadow End

    //�e�N�X�`���J���[ * (�f�B�t���[�Y�J���[(Light + Ambient)) + �X�y�L�����[�J���[
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    finalColor.w = tex_color.w;
    
    return finalColor;
}