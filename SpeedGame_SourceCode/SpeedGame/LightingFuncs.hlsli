
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

//======Normal Map
float4 GetNormalVect(float3 baseNormal, float3 baseTangent, float3 binormal, float3 nmlTex)
{
    float4 normal; //ワールド座標系になったnmlTex

    float3x3 tanMtx =
    {
        baseTangent,
        binormal,
        baseNormal,
    };

    //transposeで逆行列化
    tanMtx = transpose(tanMtx);

    nmlTex.xy = nmlTex.xy * 2.0f - 1.0f;    //XYは-1.0から1.0
    nmlTex.x = 0.5f + nmlTex.z * 0.5f;      //Zは0.5から1.0    
    nmlTex = normalize(nmlTex);             //法線マップはTangent空間
    normal = float4(mul(nmlTex, tanMtx), 0); //ワールド座標系に変換

    return normal;
}
//======Normal Map End