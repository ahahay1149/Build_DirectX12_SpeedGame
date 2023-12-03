#include "BaseConstantBuffers.hlsli"
#include "PSConstantBufferHeader.hlsli"
#include "LightingFuncs.hlsli"

float4 main(VS_OUT input) : SV_Target
{
    //�e�N�X�`���J���[�̎擾
    float4 tex_color = Texture.Sample(Sampler, input.uv);

    //���_�V�F�[�_�Ōv�Z���ꂽDiffuse*Albedo���擾
    float4 diffuse = input.color;
    
    //Diffuse * Specular
    float4 finalColor = tex_color * (diffuse + AmbientColor);
    
    return finalColor;
}