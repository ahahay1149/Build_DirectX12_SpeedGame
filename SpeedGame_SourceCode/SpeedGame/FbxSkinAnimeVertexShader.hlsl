struct VS_IN
{
	float3 pos : POSITION0;
	float3 nor : NORMAL0;
	float4 color : COLOR0;
	float2 uv : TEXTURE0;

	//SkinMesh Unity�����͂T�_�ȏ�̉e���{�[��������N���X�^������̂œ�i�ő�W�j�K�v
	uint4	indices0 : BLENDINDICES0;
	uint4	indices1 : BLENDINDICES1;
	float4	weights0 : BLENDWEIGHT0;
	float4	weights1 : BLENDWEIGHT1;

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
    float4x4	Model;					// ���[���h�ϊ��s��
}

cbuffer MeshBone : register(b3)
{
	float4x4	bones[2];			//�{�[���p�z��B�f�[�^���I�[�o�[����Ɖϒ��ɂȂ�̂ő��v�B�ł��P���ƌŒ蒷�ɂȂ��Ă��܂��B
}

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	output.pos = float4(input.pos, 1.0f);

	// �X�L���A�j���}�g���N�X���f
	float4x4 boneMtx;

	boneMtx = bones[input.indices0[0]] * input.weights0[0];
	boneMtx += bones[input.indices0[1]] * input.weights0[1];
	boneMtx += bones[input.indices0[2]] * input.weights0[2];
	boneMtx += bones[input.indices0[3]] * input.weights0[3];
	boneMtx += bones[input.indices1[0]] * input.weights1[0];
	boneMtx += bones[input.indices1[1]] * input.weights1[1];

	output.pos = mul(boneMtx, output.pos);

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
