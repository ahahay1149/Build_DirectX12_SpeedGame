#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>

#include <wrl/client.h>

#include <PipeLineManager.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class GraphicsPipeLineObjectBase;

class CharacterData
{
protected:
	XMFLOAT3	position;			//キャラクタの中心位置
	XMFLOAT3	rotation;
	XMFLOAT3	scale;

	//=========移動する地形　対応
	XMMATRIX	m_worldMtx;		//変換行列
	XMMATRIX	m_worldMtxInv;	//逆行列
	bool		m_mtxChange;	//行列の修正が必要な際にtrueになるフラグ
	//=========移動する地形　対応 END

	std::vector<ComPtr<ID3D12Resource>> m_constantBuffers;

	GraphicsPipeLineObjectBase* m_pPipeLine = nullptr;

	UINT m_cbuffCount;

	//=========カメラチェンジ対応Phase1
	std::wstring m_camera;
	//=========カメラチェンジ対応Phase1 End

public:
	CharacterData()
	{
		setPosition(0.0f, 0.0f, 0.0f);
		setRotation(0.0f, 0.0f, 0.0f);
		setScale(1.0f, 1.0f, 1.0f);

		m_constantBuffers.clear();
		m_cbuffCount = 0;

		//=========カメラチェンジ対応Phase1
		m_camera = L"MainCamera";
		//=========カメラチェンジ対応Phase1 End
	}

	virtual ~CharacterData() {}

	void AddConstantBuffer(UINT buffSize, const void* initData);

	ID3D12Resource* GetConstantBuffer(UINT index)
	{
		if (m_cbuffCount > index)
		{
			return m_constantBuffers[index].Get();
		}

		return nullptr;
	}

	void setPosition(float x, float y, float z)
	{
		position.x = x;
		position.y = y;
		position.z = z;

		//=========移動する地形　対応
		m_mtxChange = true;
		//=========移動する地形　対応 END
	}

	void setRotation(float x, float y, float z)
	{
		rotation.x = x;
		rotation.y = y;
		rotation.z = z;

		//=========移動する地形　対応
		m_mtxChange = true;
		//=========移動する地形　対応 END
	}

	void setScale(float x, float y, float z)
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;

		//=========移動する地形　対応
		m_mtxChange = true;
		//=========移動する地形　対応 END
	}

	const XMFLOAT3 getPosition()
	{
		return position;
	}

	const XMFLOAT3 getRotation()
	{
		return rotation;
	}

	const XMFLOAT3 getScale()
	{
		return scale;
	}

	void SetGraphicsPipeLine(std::wstring pipelineName);

	GraphicsPipeLineObjectBase* GetPipeline()
	{
		return m_pPipeLine;
	}

	//=========カメラチェンジ対応Phase1
	void SetCameraLabel(std::wstring label)
	{
		m_camera = label;
	}

	std::wstring GetCameraLabel()
	{
		return m_camera;
	}
	//=========カメラチェンジ対応Phase1 End

	//=========移動する地形　対応
	XMMATRIX& GetWorldMatrix();
	XMMATRIX& GetInverseWorldMatrix();
	//=========移動する地形　対応 END

};

