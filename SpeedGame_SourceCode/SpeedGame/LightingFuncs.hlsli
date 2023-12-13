
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