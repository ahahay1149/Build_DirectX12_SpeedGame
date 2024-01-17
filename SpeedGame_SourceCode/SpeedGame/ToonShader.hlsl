#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"
#include "PSConstantBufferHeader.hlsli"

struct TONE_OUT	//ピクセルシェーダ出力に使う構造体
{
	float4 outColor : SV_TARGET0;
	//======Edge Draw
	float4 outNormal : SV_TARGET1;	//法線
	//======Edge Draw End
};

Texture2D ToneTex : register(t3, space0);	//Textureをスロット0の3番目のテクスチャレジスタに設定

TONE_OUT main(VS_OUT input)
{
	TONE_OUT res;
	
	float2 uv = input.uv;
	float4 normal = input.wnml;

	//テクスチャカラーの取得
	float4 tex_color = Texture.Sample(Sampler, uv);

	//Alpha Test
	if (tex_color.w < 0.1f)
		discard;

	//Normal Check
	if (TextureFlag & TEX_NORMAL)
	{
		normal = GetNormalVect(input.wnml.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, uv).xyz);
	}

	float p;

	//======Depth Shadow
	float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);			//深度バッファシャドウ計算
	tex_color *= shadowColor;	//テクスチャ自体の色に補正値

	if (shadowColor.x < 1.0f)
	{
		p = 0.2f;	//最も暗くする
	}
	else
	{
		p = dot(input.wnml.xyz, -dLightVector.xyz);
		p = p * 0.5f + 0.5;		//HalfLambert公式
		p = p * p;
	}
	//======Depth Shadow End

	float4 toneCL = ToneTex.Sample(Sampler, float2(p, 0.0f));
	res.outColor.xyz = tex_color.xyz * toneCL.xyz;

	res.outColor.w = tex_color.w * input.color.w;

	//======Edge Draw
	res.outNormal = float4(normal.x, normal.y, normal.z, 1.0f);
	//======Edge Draw End

    return res;
}