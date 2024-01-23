Texture2D<float4> SrcTexture : register(t0, space0);    //���e�N�X�`��
RWTexture2D<float4> DstTexture : register(u0, space0);  //MipMap������
SamplerState BilinearClamp : register(s0, space0);      //�T���v���[

cbuffer CB : register(b0, space0)
{
    float2 TexelSize;
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //TexelSize��UV�l�ɕϊ�
    float2 texcoords = TexelSize * (DTid.xy + 0.5);
    
    //Bilinear�t�B���^�ɐݒ肵�Ă���̂ŃT���v�����W�̏㉺�S�_���T���v�����O
    float4 color = SrcTexture.SampleLevel(BilinearClamp, texcoords, 0);

    //����e�N�X�`���̃s�N�Z�����W�ʒu�ɐF�f�[�^����������
    DstTexture[DTid.xy] = color;
}