
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

//======Depth Shadow & Shadow Bler
float4 GetShadowColor(float4 pos, Texture2D lightDepth, SamplerComparisonState samp)
{
    float4 color = { 1,1,1,1 };

    float3 screenPos = pos.xyz / pos.w;     //スクリーン座標 = プロジェクション座標 / w
    float2 uv = (screenPos.xy + float2(1, -1)) * float2(0.5, -0.5);

    if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)   //UVが0.0 - 1.0におさまっているかチェック
        return color;

    float4 depth = lightDepth.SampleCmp(samp, uv, screenPos.z - 0.001); //0.001でシャドウアクネ対策
    float shadowWeight = lerp(0.5, 1.0, depth);                         //影補正値が0.5より小さいと黒すぎるので0.5に強制

    if (depth.r < screenPos.z)
    {
        //何かに光がさえぎられている
        color.xyz = shadowWeight;
    }

    return color;
}
//======Depth Shadow & Shadow Bler End