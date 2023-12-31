
float4 MakeDiffuseColor(float4 normal, float4 inColor, float3 lightColor, float3 lightVect)
{
    //���s����
    float3 dlight = lightColor * saturate(dot(normal.xyz, -lightVect));
    
    float4 output;
    
    //�|���S���̒��_�J���[��Directional Light�̐ς��V�������_�J���[
    output.xyz = inColor.xyz * dlight;
    output.w = inColor.w;    //w�͓��͒l�̂܂�
    
    return output;
}

//======Normal Map
float4 GetNormalVect(float3 baseNormal, float3 baseTangent, float3 binormal, float3 nmlTex)
{
    float4 normal; //���[���h���W�n�ɂȂ���nmlTex

    float3x3 tanMtx =
    {
        baseTangent,
        binormal,
        baseNormal,
    };

    //transpose�ŋt�s��
    tanMtx = transpose(tanMtx);

    nmlTex.xy = nmlTex.xy * 2.0f - 1.0f;    //XY��-1.0����1.0
    nmlTex.x = 0.5f + nmlTex.z * 0.5f;      //Z��0.5����1.0    
    nmlTex = normalize(nmlTex);             //�@���}�b�v��Tangent���
    normal = float4(mul(nmlTex, tanMtx), 0); //���[���h���W�n�ɕϊ�

    return normal;
}
//======Normal Map End

//======Depth Shadow & Shadow Bler
float4 GetShadowColor(float4 pos, Texture2D lightDepth, SamplerComparisonState samp)
{
    float4 color = { 1,1,1,1 };

    float3 screenPos = pos.xyz / pos.w;     //�X�N���[�����W = �v���W�F�N�V�������W / w
    float2 uv = (screenPos.xy + float2(1, -1)) * float2(0.5, -0.5);

    if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)   //UV��0.0 - 1.0�ɂ����܂��Ă��邩�`�F�b�N
        return color;

    float4 depth = lightDepth.SampleCmp(samp, uv, screenPos.z - 0.001); //0.001�ŃV���h�E�A�N�l�΍�
    float shadowWeight = lerp(0.5, 1.0, depth);                         //�e�␳�l��0.5��菬�����ƍ�������̂�0.5�ɋ���

    if (depth.r < screenPos.z)
    {
        //�����Ɍ������������Ă���
        color.xyz = shadowWeight;
    }

    return color;
}
//======Depth Shadow & Shadow Bler End