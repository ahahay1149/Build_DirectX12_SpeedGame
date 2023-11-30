struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE0;
};

Texture2D    Texture : register(t0, space0); // Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0, space0); // Samplerをスロット0の0番目のサンプラレジスタに設定

float4 main(PS_IN input) : SV_Target
{
	// テクスチャカラーの取得
	float4 tex_color = Texture.Sample(Sampler, input.uv);

	// アンビエントカラー + ディフューズカラー + テクスチャカラー
	float4 finalColor = tex_color * input.color;
	
	return finalColor;
}
