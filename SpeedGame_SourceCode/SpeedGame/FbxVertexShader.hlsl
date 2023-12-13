struct VS_IN
{
	float3 pos : POSITION0;
	float3 nor : NORMAL0;
	//======Normal Map
    float3 tan : TANGENT0;
	//======Normal Map End
	float4 color : COLOR0;
	float2 uv : TEXTURE0;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE0;
};

cbuffer ViewBuffer : register(b0)		//����̓J�����̈ʒu�Ō��܂�
{
	float4x4	View;					// �r���[�ϊ��s��
}

cbuffer ProjectionBuffer : register(b1)	//����̓J�����̈ʒu�Ō��܂�
{
	float4x4	Projection;				// �����ˉe�ϊ��s��
}

cbuffer ModelBuffer : register(b2)		// ���b�V���������g�̃}�g���N�X
{
    float4x4 Model; // ���[���h�ϊ��s��
}

VS_OUT main(VS_IN input)
{
	VS_OUT output;

	// �s��ϊ��p��4�v�f�ɂӂ₵�Ė����iw�j��1.0f��ǉ�
	output.pos = float4(input.pos, 1.0f);

	// ���[�J�����W * ���[���h���W�ϊ��s��
    output.pos = mul(output.pos, Model);

	// ���[���h���W * �r���[���W�ϊ��s��
	output.pos = mul(output.pos, View);
	// �r���[���W * �v���W�F�N�V�������W�ϊ��s��
	output.pos = mul(output.pos, Projection);

	// ���_�J���[
	output.color = input.color;

	// Texture���W�w��
	output.uv = input.uv;

	return output;
}
