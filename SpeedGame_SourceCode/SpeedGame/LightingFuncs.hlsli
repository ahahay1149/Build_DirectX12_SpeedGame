
float4 MakeDiffuseColor(float4 normal, float4 inColor, float3 lightColor, float3 lightVect)
{
    //平行光源
    float3 dlight = lightColor * saturate(dot(normal.xyz, -lightVect));
    
    float4 output;
    
    //ポリゴンの頂点カラーとDirectional Lightの積が新しい頂点カラー
    output.xyz = inColor.xyz * dlight;
    output.w = inColor.w;    //wは入力値のまま
    
    return output;
}