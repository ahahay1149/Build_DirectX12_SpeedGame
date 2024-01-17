#include "PEEdgeDrawHeader.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
    float4 color;
    
    float3 normal;  //�@���x�N�g���̃��v���V�A�����v
    float depth;    //�[�x�l�̃��v���V�A�����v
    
    //������-8�{���Ď���𑫂��B���ӂ�UV�l��xy
    normal = -8.0 * NormalTex.Sample(Sampler, input.tex0).xyz;  //�����̖@���x�N�g��
    normal += NormalTex.Sample(Sampler, input.tex1.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex2.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex3.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex4.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex5.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex6.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex7.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex8.xy).xyz;
    
    depth = -8.0f * DepthTex.Sample(Sampler, input.tex0).x;     //�����̐[�x�l
    depth += DepthTex.Sample(Sampler, input.tex1.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex2.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex3.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex4.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex5.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex6.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex7.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex8.zw).x;
    
    //�l�`�F�b�N
    //�����@���̒�����0.8�ȏ�A�܂��͍����[�x�l��+-0.003�𒴂��Ă�����[�Ɣ���
    if(length(normal) >= 0.8f || abs(depth) > 0.003f)
    {
        color = float4(0.6f, 0.6f, 0.6f, 1);    //�[�Ȃ̂ŗ֊s���B�F���Â�����
    }
    else
    {
        color = float4(1, 1, 1, 1);             //���̂܂܂̐F���o��
    }
    
    return color;
}