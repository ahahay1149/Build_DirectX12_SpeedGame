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
        float3 nmlTex = NormalTex.Sample(Sampler, uv);
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, nmlTex.xyz);
    }
    //======Normal Map End
    
    //�e�N�X�`���J���[�̎擾
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    //Diffuse * Albedo �J���[�v�Z
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    float4 specular = 0;
    
    //======Specular Map
    if (TextureFlag & TEX_SPECULAR)  //�e�N�X�`�������`�F�b�N
    {
        float4 sptex; //�X�y�L�����[�}�b�v�̐F
        sptex = SpecularTex.Sample(Sampler, uv);
    
        //======Phong���˃��f��
        // Phong���˂̔��ˌ��v�Z��
        float3 refVect = dLightVector + 2.0f * dot(-dLightVector, normal.xyz) * normal.xyz;
        
        // �����x�N�g���̍쐬
        float3 viewRect = normalize(CameraPos - input.wpos.xyz); //���K��
        
        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz * pow(saturate(dot(refVect, viewRect)), MaterialSpecular.w);
        //======Phong���˃��f�� End
    }
    //======Specular Map End

    //�f�B�t���[�Y�J���[(Light * Ambient) * �e�N�X�`���J���[ + �X�y�L�����[�J���[
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    finalColor.w = tex_color.w;
    
    return finalColor;
}