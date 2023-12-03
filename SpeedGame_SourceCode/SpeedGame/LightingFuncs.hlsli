
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