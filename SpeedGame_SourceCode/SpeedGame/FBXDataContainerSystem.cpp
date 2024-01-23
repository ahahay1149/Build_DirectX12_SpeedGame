#include <MyAccessHub.h>
#include <MyGameEngine.h>

#include "FBXDataContainerSystem.h"

void convertFbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbxamatrix, DirectX::XMFLOAT4X4& xmfloat4x4)
{
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
		}
	}
}

FBX_TEXTURE_TYPE FBXDataContainer::GetTextureType(const fbxsdk::FbxBindingTableEntry& entryTable)
{
	std::string texStr = entryTable.GetSource();

	if (texStr == "Maya|DiffuseTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_DIFFUSE;
	}
	else if (texStr == "Maya|NormalTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_NORMAL;
	}
	else if (texStr == "Maya|SpecularTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_SPECUAR;
	}
	else if (texStr == "Maya|FalloffTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_FALLOFF;
	}
	else if (texStr == "Maya|ReflectionMapTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_REFLECTIONMAP;
	}

	return FBX_TEXTURE_TYPE::FBX_UNKNOWN;
}

void FinishFBXLoad(fbxsdk::FbxManager** man, fbxsdk::FbxImporter** imp, fbxsdk::FbxScene** sc)
{

	if ((*imp) != nullptr)
	{
		(*imp)->Destroy();
		*imp = nullptr;
	}

	if ((*sc) != nullptr)
	{
		(*sc)->Destroy();
		*sc = nullptr;
	}

	if ((*man) != nullptr)
	{
		(*man)->Destroy();
		*man = nullptr;
	}

}

HRESULT FBXDataContainer::ReadFbxToMeshContainer(const std::wstring id, FbxMesh* pMesh)
{
	HRESULT hr = S_OK;

	unique_ptr<MeshContainer> meshCont = make_unique<MeshContainer>();

	//ノード数
	int nodecount = pMesh->GetNodeCount();

	//MeshContainerに元データポインタ保存
	meshCont->setFbxMesh(pMesh);

	// VertexBuffer
	// ボーン点も含むのでコントロールポイントにもなる
	FbxVector4* controllPoints = nullptr;

	// IndexBuffer
	// インデックスデータ
	int* indices = nullptr;

	// 頂点数
	int vertexCount;
	// コントロールポイント数
	int contCount;

	//08:メッシュ基本データ読み込み 
	// VertexBuffer
	controllPoints = pMesh->GetControlPoints();

	// IndexBuffer
	// コントロールポイントの中からポリゴン頂点のインデックスリスト
	indices = pMesh->GetPolygonVertices();

	// 頂点数
	vertexCount = pMesh->GetPolygonVertexCount();

	// コントロールポイント数
	contCount = pMesh->GetControlPointsCount();
	//08: ここまで

	meshCont->m_vertexData.clear();
	meshCont->m_vertexData.resize(vertexCount);

	//メッシュごとの当たり判定エリア値初期化
	meshCont->m_vtxMin.x = FLT_MAX;
	meshCont->m_vtxMin.y = FLT_MAX;
	meshCont->m_vtxMin.z = FLT_MAX;

	meshCont->m_vtxMax.x = FLT_MIN;
	meshCont->m_vtxMax.y = FLT_MIN;
	meshCont->m_vtxMax.z = FLT_MIN;



	for (int i = 0; i < vertexCount; i++)
	{
		// Vertexデータを初期化
		ZeroMemory(&meshCont->m_vertexData[i], sizeof(FbxVertex));

		//09:頂点データをFBXから自前の頂点バッファへ 
		// インデックスバッファから頂点番号を取得
		int index = indices[i];

		// 頂点座標リストから座標を取得する
		meshCont->m_vertexData[i].position.x = (float)controllPoints[index][0];
		meshCont->m_vertexData[i].position.y = (float)controllPoints[index][1];
		meshCont->m_vertexData[i].position.z = (float)controllPoints[index][2];
		//9:ここまで

		//max & min check
		if (meshCont->m_vertexData[i].position.x < meshCont->m_vtxMin.x)
		{
			meshCont->m_vtxMin.x = meshCont->m_vertexData[i].position.x;
		}
		else if (meshCont->m_vertexData[i].position.x > meshCont->m_vtxMax.x)
		{
			meshCont->m_vtxMax.x = meshCont->m_vertexData[i].position.x;
		}

		if (meshCont->m_vertexData[i].position.y < meshCont->m_vtxMin.y)
		{
			meshCont->m_vtxMin.y = meshCont->m_vertexData[i].position.y;
		}
		else if (meshCont->m_vertexData[i].position.y > meshCont->m_vtxMax.y)
		{
			meshCont->m_vtxMax.y = meshCont->m_vertexData[i].position.y;
		}

		if (meshCont->m_vertexData[i].position.z < meshCont->m_vtxMin.z)
		{
			meshCont->m_vtxMin.z = meshCont->m_vertexData[i].position.z;
		}
		else if (meshCont->m_vertexData[i].position.z > meshCont->m_vtxMax.z)
		{
			meshCont->m_vtxMax.z = meshCont->m_vertexData[i].position.z;
		}
	}

	//保存したminとmaxを全体のmin、maxと比較して更新
	if (m_vtxTotalMax.x < meshCont->m_vtxMax.x) m_vtxTotalMax.x = meshCont->m_vtxMax.x;
	if (m_vtxTotalMax.y < meshCont->m_vtxMax.y) m_vtxTotalMax.y = meshCont->m_vtxMax.y;
	if (m_vtxTotalMax.z < meshCont->m_vtxMax.z) m_vtxTotalMax.z = meshCont->m_vtxMax.z;

	if (m_vtxTotalMin.x > meshCont->m_vtxMin.x) m_vtxTotalMin.x = meshCont->m_vtxMin.x;
	if (m_vtxTotalMin.y > meshCont->m_vtxMin.y) m_vtxTotalMin.y = meshCont->m_vtxMin.y;
	if (m_vtxTotalMin.z > meshCont->m_vtxMin.z) m_vtxTotalMin.z = meshCont->m_vtxMin.z;

	FbxStringList uvset_names;
	// UVSetの名前リストを取得
	pMesh->GetUVSetNames(uvset_names);

	FbxArray<FbxVector2> uv_buffer;

	// UVSetの名前からUVSetを取得する
	pMesh->GetPolygonVertexUVs(uvset_names.GetStringAt(0), uv_buffer);

	for (int i = 0; i < uv_buffer.Size(); i++)
	{
		FbxVector2& uv = uv_buffer[i];

		//10:UVデータを自前頂点データに反映 
		meshCont->m_vertexData[i].uv.x = (float)uv[0];
		meshCont->m_vertexData[i].uv.y = (float)(1.0 - uv[1]);	//FBXのVはひっくり返っているので1.0
		//10: ここまで
	}


	//indexの作成
	meshCont->m_indexData.clear();
	meshCont->m_indexData.resize(vertexCount);

	//ポリゴン数を取得
	int length = pMesh->GetPolygonCount();

	//ポリゴン数でループ
	for (int i = 0; i < length; i++)
	{
		//11:インデックスデータの作成 
		int baseIndex = i * 3;
		//データが右手系でひっくり返っている場合は、ここでインデックスの一つ目と三つ目の値ひっくり返して処理する
		meshCont->m_indexData[baseIndex] = static_cast<ULONG>(baseIndex);
		meshCont->m_indexData[baseIndex + 1] = static_cast<ULONG>(baseIndex + 1);
		meshCont->m_indexData[baseIndex + 2] = static_cast<ULONG>(baseIndex + 2);
		//11:ここまで 
	}

	//作成したインデックスデータでシェーダリソースを作成
	MyAccessHub::getMyGameEngine()->GetMeshManager()->AddIndexBuffer(id, meshCont->m_indexData.data(), sizeof(ULONG), vertexCount);

	//法線
	FbxArray<FbxVector4> normals;
	// 法線リストの取得
	pMesh->GetPolygonVertexNormals(normals);

	//法線登録
	for (int i = 0; i < normals.Size(); i++)
	{
		meshCont->m_vertexData[i].normal.x = normals[i][0];
		meshCont->m_vertexData[i].normal.y = normals[i][1];
		meshCont->m_vertexData[i].normal.z = normals[i][2];

		//======Normal Map
		// normalと上方向ベクトル(0,1,0)の外積でTangentベクトルを取得
		XMFLOAT3 normal = meshCont->m_vertexData[i].normal;
		XMFLOAT3 tanVect = { 1.0f,0.0f,0.0f };	//normalが0,1,0だった場合のTangent

		//外積計算
		if (normal.y < 0.99f)
		{
			tanVect = { -normal.z, 0.0f, normal.x };
		}
		meshCont->m_vertexData[i].tangent = tanVect;
		//======Normal Map End
	}

	//頂点カラー
	{

		// 頂点カラーデータの数を確認
		int color_count = pMesh->GetElementVertexColorCount();

		// 頂点カラーデータの取得
		// 今回はDiffuseだけで行う
		FbxGeometryElementVertexColor* vertex_colors = pMesh->GetElementVertexColor(0);

		if (color_count == 0 || vertex_colors == nullptr)
		{
			//白で補正
			for (int i = 0; i < vertexCount; i++)
			{
				meshCont->m_vertexData[i].color.x = 1.0f;
				meshCont->m_vertexData[i].color.y = 1.0f;
				meshCont->m_vertexData[i].color.z = 1.0f;
				meshCont->m_vertexData[i].color.w = 1.0f;
			}
		}
		else
		{
			FbxLayerElement::EMappingMode mapping_mode = vertex_colors->GetMappingMode();
			FbxLayerElement::EReferenceMode reference_mode = vertex_colors->GetReferenceMode();

			//頂点カラーはカラー設定モードで値の取り方が変わる。注意
			if (mapping_mode == FbxLayerElement::eByPolygonVertex)
			{
				if (reference_mode == FbxLayerElement::eIndexToDirect)
				{
					// 頂点カラーバッファ取得
					FbxLayerElementArrayTemplate<FbxColor>& colors = vertex_colors->GetDirectArray();
					// 頂点カラーインデックスバッファ取得
					FbxLayerElementArrayTemplate<int>& indeces = vertex_colors->GetIndexArray();
					for (int i = 0; i < indeces.GetCount(); i++)
					{
						int id = indeces.GetAt(i);
						FbxColor color = colors.GetAt(id);
						meshCont->m_vertexData[i].color.x = (float)color.mAlpha;
						meshCont->m_vertexData[i].color.y = (float)color.mRed;
						meshCont->m_vertexData[i].color.z = (float)color.mGreen;
						meshCont->m_vertexData[i].color.w = (float)color.mBlue;
					}
				}
			}
		}

	}

	meshCont->m_MeshId = id;

	FbxLayerElementMaterial* elMat = pMesh->GetElementMaterial(0);
	int matIndex = elMat->GetIndexArray().GetAt(0);
	FbxSurfaceMaterial* srfMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(matIndex);
	if (srfMat != nullptr)
	{
		wchar_t namebuff[64] = {};
		auto mtname = srfMat->GetName();
		size_t conv = 0;
		mbstowcs_s(&conv, namebuff, mtname, strlen(mtname));
		meshCont->m_MaterialId = std::wstring(namebuff);
	}
	else
	{
		meshCont->m_MaterialId = L"";
	}

	//頂点数保存
	meshCont->m_vertexCount = vertexCount;

	//SkinAnime09
	//ボーン、スキンの読み込み
	//メッシュに設定されているスキンの数を取得
	int skinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	if (skinCount > 0)
	{

		// ボーン
		meshCont->InitSkinList(skinCount);
		std::vector< std::vector< std::pair<UINT, float> > > tempWeightVect;

		std::vector< std::vector<FbxSkinAnimeParams> >	skinWeights;
		skinWeights.resize(skinCount);

		//スキンの数だけループ
		for (int skinloop = 0; skinloop < skinCount; skinloop++)
		{
			//スキンのウェイトデータ保存配列初期化
			tempWeightVect.clear();
			tempWeightVect.resize(contCount);

			//スキンを取得
			FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(skinloop, FbxDeformer::eSkin);

			skinWeights[skinloop].clear();
			skinWeights[skinloop].resize(contCount);    //WeightデータはVertexではなくControllPoint用

			//スキンの中に幾つのボーン構造があるのかを取得
			int clusterCount = pSkin->GetClusterCount();

			//クラスターの数だけループ
			for (int clusterloop = 0; clusterloop < clusterCount; clusterloop++)
			{
				FbxCluster* pCluster = pSkin->GetCluster(clusterloop);

				//ボーンのID番号を取得。getClusterId内で新しいボーンだった場合は初期状態の逆行列も保存
				int clIndex = GetClusterId(pCluster);

				int pointCnt = pCluster->GetControlPointIndicesCount();		//このボーンが影響するコントロールポイントの数
				int* pPointArray = pCluster->GetControlPointIndices();		//このボーンが影響するコントロールポイントのインデックス配列
				double* pPointWeights = pCluster->GetControlPointWeights();	//このボーンが影響するコントロールポイントのウェイト配列、インデックスと組

				for (int pointloop = 0; pointloop < pointCnt; pointloop++)
				{
					int cpIndex = pPointArray[pointloop];				//コントロールポイントのインデックス
					double boneWeight = pPointWeights[pointloop];		//そのコントロールポイントに与えるウェイト値

					//一つのコントロールポイントに最大8つまでウェイトデータを入れられる
					tempWeightVect[cpIndex].push_back({ clIndex, static_cast<float>(boneWeight) });
				}

			}

			//収集したControllPointごとのWeightデータを頂点用データに成形
			for (int cntLoop = 0; cntLoop < contCount; cntLoop++)
			{
				int wCnt = tempWeightVect[cntLoop].size();
				FbxSkinAnimeParams* skinparam = &skinWeights[skinloop][cntLoop];

				//最大の8に足らない分は0で埋めておく
				for (; wCnt < 8; wCnt++)
				{
					tempWeightVect[cntLoop].push_back({ 0, 0.0f });
				}

				skinparam->indices0 = { tempWeightVect[cntLoop][0].first, tempWeightVect[cntLoop][1].first, tempWeightVect[cntLoop][2].first, tempWeightVect[cntLoop][3].first };
				skinparam->indices1 = { tempWeightVect[cntLoop][4].first, tempWeightVect[cntLoop][5].first, tempWeightVect[cntLoop][6].first, tempWeightVect[cntLoop][7].first };
				skinparam->weight0 = { tempWeightVect[cntLoop][0].second, tempWeightVect[cntLoop][1].second, tempWeightVect[cntLoop][2].second, tempWeightVect[cntLoop][3].second };
				skinparam->weight1 = { tempWeightVect[cntLoop][4].second, tempWeightVect[cntLoop][5].second, tempWeightVect[cntLoop][6].second, tempWeightVect[cntLoop][7].second };
			}
		}

		//結合したvertexデータを作成
		std::vector<FbxSkinAnimeVertex> skinVertex;
		skinVertex.clear();
		skinVertex.resize(vertexCount);

		for (int i = 0; i < vertexCount; i++)
		{
			int index = indices[i];	//skinWeightsがControllPoint用なのでインデックスから頂点番号を取り出す必要あり
			skinVertex[i].vertex = meshCont->m_vertexData[i];
			skinVertex[i].skinvalues = skinWeights[0][index];	//今回はシェーダ側の対応スキンは1つだけ
		}

		MyAccessHub::getMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, skinVertex.data(), sizeof(FbxSkinAnimeVertex), vertexCount);
	}
	else
	{

		// スキンメッシュなし
		if (pMesh->GetScene()->GetSrcObjectCount<FbxMesh>() > 1)
		{
			std::vector<FbxSkinAnimeVertex> skinVertex;
			skinVertex.clear();
			skinVertex.resize(vertexCount);

			UINT boneId = GetClusterId(pMesh->GetNode());

			for (int i = 0; i < vertexCount; i++)
			{
				skinVertex[i].vertex = meshCont->m_vertexData[i];
				skinVertex[i].skinvalues.indices0 = { boneId, 0, 0, 0 };	//基底NodeのIDをセット
				skinVertex[i].skinvalues.indices1 = { 0, 0, 0, 0 };
				skinVertex[i].skinvalues.weight0 = { 1.0f, 0, 0, 0 };		//Weight(影響度)を1.0f(100%)にする
				skinVertex[i].skinvalues.weight1 = { 0, 0, 0, 0 };
			}
			MyAccessHub::getMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, skinVertex.data(), sizeof(FbxSkinAnimeVertex), vertexCount);
		}
		else
		{
			// シンプルな頂点シェーダ
			MyAccessHub::getMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, meshCont->m_vertexData.data(), sizeof(FbxVertex), vertexCount);
		}
	}

	//ここまで

	//12:出来上がったMeshContainerオブジェクトをm_pMeshContainerに追加
	m_pMeshContainer.push_back(move(meshCont));
	//12:ここまで

	return hr;
}

HRESULT FBXDataContainer::LoadMaterial(const std::wstring id, FbxSurfaceMaterial* material)
{
	HRESULT hr = S_OK;

	enum class MaterialOrder
	{
		Ambient,
		Diffuse,
		Specular,
		MaxOrder,
	};

	const fbxsdk::FbxImplementation* implementation = GetImplementation(material, FBXSDK_IMPLEMENTATION_CGFX);

	wchar_t namebuff[64] = {};
	auto mtname = material->GetName();
	size_t conv = 0;
	mbstowcs_s(&conv, namebuff, mtname, strlen(mtname));
	std::wstring matName = namebuff;
	m_pMaterialContainer[matName] = make_unique<MaterialContainer>();

	FbxDouble3 colors[(int)MaterialOrder::MaxOrder];
	FbxDouble factors[(int)MaterialOrder::MaxOrder];
	FbxProperty fbxProp = material->FindProperty(FbxSurfaceMaterial::sAmbient);
	if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		const char* element_check_list[] =
		{
			FbxSurfaceMaterial::sAmbient,
			FbxSurfaceMaterial::sDiffuse,

			//=====Specular
			FbxSurfaceMaterial::sSpecular,
			//=====Specular End
		};

		const char* factor_check_list[] =
		{
			FbxSurfaceMaterial::sAmbientFactor,
			FbxSurfaceMaterial::sDiffuseFactor,

			//=====Specular
			FbxSurfaceMaterial::sSpecularFactor,
			//=====Specular End
		};

		for (int i = 0; i < 3; i++)
		{
			fbxProp = material->FindProperty(element_check_list[i]);
			if (fbxProp.IsValid())
			{
				colors[i] = fbxProp.Get<FbxDouble3>();

			}
			else
			{
				colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			}

			fbxProp = material->FindProperty(factor_check_list[i]);
			if (fbxProp.IsValid())
			{
				factors[i] = fbxProp.Get<FbxDouble>();
			}
			else
			{
				factors[i] = 1.0;
			}
		}
	}
	//=====Specular
	else
	{
		for (int i = 0; i < 3; i++)
		{
			colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			factors[i] = 1.0;
		}
	}
	//=====Specular End

	FbxDouble3 color = colors[(int)MaterialOrder::Ambient];
	FbxDouble factor = factors[(int)MaterialOrder::Ambient];
	m_pMaterialContainer[matName]->setAmbient((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Diffuse];
	factor = factors[(int)MaterialOrder::Diffuse];
	m_pMaterialContainer[matName]->setDiffuse((float)color[0], (float)color[1], (float)color[2], (float)factor);

	//=====Specular
	color = colors[(int)MaterialOrder::Specular];
	factor = factors[(int)MaterialOrder::Specular];
	m_pMaterialContainer[matName]->setSpecular((float)color[0], (float)color[1], (float)color[2], (float)factor);
	//=====Specular End

	// テクスチャ読み込み(シングル対応)
	// マルチテクスチャはシェーダから別。今回は未対応。対応させたいが…

	if (implementation == nullptr)
	{
		fbxProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		LoadTextureFromMaterial(matName, id, FBX_TEXTURE_TYPE::FBX_DIFFUSE, &fbxProp);
	}
	else
	{
		const FbxBindingTable* rootTable = implementation->GetRootTable();
		size_t entryCount = rootTable->GetEntryCount();

		for (int ent = 0; ent < entryCount; ent++)
		{
			const FbxBindingTableEntry entry = rootTable->GetEntry(ent);

			fbxProp = material->FindPropertyHierarchical(entry.GetSource());
			if (!fbxProp.IsValid())
			{
				fbxProp = material->RootProperty.FindHierarchical(entry.GetSource());
			}

			LoadTextureFromMaterial(matName, id, GetTextureType(entry), &fbxProp);
		}
	}

	//======Specular 
	m_pMaterialContainer[matName]->UpdateD3DResource();
	//======Specular End 

	return hr;
}

HRESULT FBXDataContainer::LoadTextureFromMaterial(const std::wstring matName, const std::wstring id, FBX_TEXTURE_TYPE texType, const fbxsdk::FbxProperty* fbxProp)
{
	HRESULT hr = S_OK;
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();

	fbxsdk::FbxFileTexture* texture = nullptr;
	std::string keyword;
	int numOfTex = fbxProp->GetSrcObjectCount<FbxFileTexture>();
	if (numOfTex > 0)
	{
		numOfTex = 1;
	}
	else
	{
		// FbxLayeredTextureからFbxFileTextureを取得	
		numOfTex = fbxProp->GetSrcObjectCount<FbxLayeredTexture>();
	}

	TextureManager* texMng = MyAccessHub::getMyGameEngine()->GetTextureManager();
	for (int i = 0; i < numOfTex; i++)
	{
		texture = fbxProp->GetSrcObject<FbxFileTexture>(i);
		if (texture != nullptr)
		{
			std::string filePath = texture->GetRelativeFileName();

			// ファイルパス分解
			// まず区切り文字を/に統一
			std::string::size_type position(filePath.find('\\'));
			std::string fileName;

			while (position != std::string::npos)
			{
				filePath.replace(position, 1, "/");
				position = filePath.find('\\', position + 1);
			}

			// 最後の/を見つけて「ファイル名」だけを取り出す
			std::string::size_type offset = std::string::size_type(0);
			position = filePath.find('/');

			while (position != std::string::npos)
			{
				offset = position + 1;
				position = filePath.find('/', offset);
			}

			fileName = filePath.substr(offset);

			//psdデータ
			position = fileName.find(".psd");
			if (position != std::string::npos)
			{
				//最後の.psdを取得
				while (position != std::string::npos)
				{
					offset = position;
					position = fileName.find(".psd", position + 4);
				}
				fileName.replace(offset, 4, ".tga");
			}

			// fileNameをワイド文字列に変換
			wchar_t namebuff[64] = {};
			wchar_t texturePath[128];
			size_t conv = 0;
			mbstowcs_s(&conv, namebuff, fileName.c_str(), fileName.length());

			// ゲームシステムとしてのパスに変換
			wsprintfW(texturePath, L"Resources/textures/%ls/%ls", id.c_str(), namebuff);

			//テクスチャの登録IDはid_ファイル名
			wchar_t idName[128];
			wsprintfW(idName, L"%ls_%ls", id.c_str(), namebuff);
			//======MipMap
			//FBXからのテクスチャを強制的にMipMap対応する
			hr = texMng->CreateTextureFromFile(engine->GetDirect3DDevice(), idName, texturePath, 8);	//全てのFBX用テクスチャをMipMapLV8に
			//======MipMap End


			if (FAILED(hr))
			{
				break;
			}

			//一旦全部セット(拡張する場合に備える)
			switch (texType)
			{
			case FBX_TEXTURE_TYPE::FBX_DIFFUSE:
				m_pMaterialContainer[matName]->m_diffuseTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_NORMAL:
				m_pMaterialContainer[matName]->m_normalTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_SPECUAR:
				m_pMaterialContainer[matName]->m_specularTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_FALLOFF:
				m_pMaterialContainer[matName]->m_falloffTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_REFLECTIONMAP:
				m_pMaterialContainer[matName]->m_reflectionMapTextures.push_back(idName);
				break;

			default:	//Unknown
				break;
			}

			//=====Specular
			m_pMaterialContainer[matName]->materialInfo.TextureFlag |= (unsigned int)texType;
			//=====Specular End
		}
	}

	return hr;
}

HRESULT FBXDataContainer::LoadFBX(const std::wstring fileName, const std::wstring id, bool noMaterial = false)
{
	HRESULT hr = S_OK;

	fbxsdk::FbxManager* fbx_manager = nullptr;
	FbxScene* fbx_scene = nullptr;
	FbxImporter* fbx_importer = nullptr;
	FbxNode* fbx_node = nullptr;

	//fbxsdkがchar*を引数にするので変換
	char* c_filename = nullptr;
	int wcSize = sizeof(wchar_t) * wcslen(fileName.c_str()) + 1;

	c_filename = new char[wcSize];
	size_t retVal = 0;
	wcstombs_s(&retVal, c_filename, wcSize, fileName.c_str(), wcSize);

	fbx_manager = FBXDataContainer::GetFbxManager();

	//01: FbxImporterの作成
	fbx_importer = FbxImporter::Create(fbx_manager, c_filename);
	if (fbx_importer == nullptr)
	{
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}
	//01: ここまで

	//02: FbxSceneの作成
	fbx_scene = FbxScene::Create(fbx_manager, c_filename);
	if (fbx_scene == nullptr)
	{
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}
	//02: ここまで

	//03: ファイルの初期化(ファイルオープン)
	bool res = fbx_importer->Initialize(c_filename);	//どちらにせよdeleteが必要
	delete[] c_filename;
	if (res == false)
	{
		// 初期化失敗
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}

	// sceneにインポート
	if (fbx_importer->Import(fbx_scene) == false)
	{
		// インポート失敗
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}
	//03: ここまで

	FbxAxisSystem dx = FbxAxisSystem::DirectX;
	if (dx != fbx_scene->GetGlobalSettings().GetAxisSystem())
	{
		dx.DeepConvertScene(fbx_scene);
	}

	// ここから先の処理は重い。マルチスレッドか非同期処理の実装を検討
	// 今回はクラス内での実装

	//04: ノード読み込み
	fbx_node = fbx_scene->GetRootNode();
	//04: ここまで

	if (fbx_node != nullptr)
	{
		//ROOTがnullptrの時は読み取り失敗。もしくは中身が空なのでそのまま終了

		//05: ノード検索用リスト作成
		int nodes = fbx_scene->GetNodeCount();
		m_nodeNameList.clear();
		m_nodeNameList.resize(nodes);
		for (int i = 0; i < nodes; i++)
		{
			m_nodeNameList[i] = fbx_scene->GetNode(i)->GetName();
		}
		//05: ここまで

		//06: FBXのモデルデータを三角形ポリゴンデータに変換
		FbxGeometryConverter converter(fbx_manager);
		converter.SplitMeshesPerMaterial(fbx_scene, true);	//マテリアルごとにメッシュを分解
		converter.Triangulate(fbx_scene, true);             //四角とか混ざってるかもしれないFBXを全部三角形だけに変換
		//06: ここまで

		// SkinAnime01
		//アニメデータチェック
		if (fbx_importer->GetAnimStackCount() > 0)
		{
			auto stack = fbx_scene->GetCurrentAnimationStack();

			m_startTime = stack->GetLocalTimeSpan().GetStart().GetSecondDouble();
			m_endTime = stack->GetLocalTimeSpan().GetStop().GetSecondDouble();

			m_timePeriod = 1.0 / 60.0;	//16.66666ms
			m_animeFrames = floorl((m_endTime - m_startTime) / m_timePeriod);

			m_animeStack = stack;
			fbx_scene->SetCurrentAnimationStack(m_animeStack);
		}
		else
		{
			m_animeStack = nullptr;
			m_animeFrames = 0;
		}
		// ここまで

		//一つのFBXは複数のMeshを持っている事があるので総数を取得してループ
		m_pMeshContainer.clear();	//MeshContainerをクリア

		int materialCnt;
		if (noMaterial)
		{
			m_pFbxScene = fbx_scene;
			return S_OK;
		}
		else
			materialCnt = fbx_scene->GetMaterialCount();

		for (int i = 0; i < materialCnt; i++)
		{
			LoadMaterial(id, fbx_scene->GetSrcObject<FbxSurfaceMaterial>(i));
		}

		//メッシュ読み込み処理
		int meshCount = fbx_scene->GetSrcObjectCount<FbxMesh>();

		m_vtxTotalMax.x = FLT_MIN;
		m_vtxTotalMax.y = FLT_MIN;
		m_vtxTotalMax.z = FLT_MIN;

		m_vtxTotalMin.x = FLT_MAX;
		m_vtxTotalMin.y = FLT_MAX;
		m_vtxTotalMin.z = FLT_MAX;

		wchar_t idName[128];
		for (int i = 0; i < meshCount; i++)
		{
			wsprintfW(idName, L"%ls_%02d", id.c_str(), i);	//メッシュごとに名前をつける。元ファイル名+番号
			if (FAILED(ReadFbxToMeshContainer(idName, fbx_scene->GetSrcObject<FbxMesh>(i))))
			{
				hr = E_FAIL;

				FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
				return hr;
			}
		}

		// SkinAnime02
		//clusterの数からボーン用マトリクスのコンスタントバッファ用CPU側メモリを確保
		m_clusterCount = m_boneNameList.size();

		m_pFbxScene = fbx_scene;
		// ここまで

	}

	//07: FbxImporter終了
	fbx_importer->Destroy();
	//07: ここまで

	return hr;
}

int FBXDataContainer::GetNodeId(const char* nodeName)
{
	int length = m_nodeNameList.size();
	for (int i = 0; i < length; i++)
	{
		if (strcmp(m_nodeNameList[i], nodeName) == 0)
		{
			return i;
		}
	}

	return -1;
}

int FBXDataContainer::GetMeshId(const char* meshName)
{
	int len = m_pMeshContainer.size();

	for (int i = 0; i < len; i++)
	{
		if (strcmp(m_pMeshContainer[i]->getMeshNodeName(), meshName) == 0)
		{
			return i;
		}
	}

	return -1;
}

const char* FBXDataContainer::GetBoneName(int id)
{
	if (id < m_clusterCount)
	{
		return m_boneNameList[id];
	}
	return nullptr;
}

int FBXDataContainer::GetBoneId(const char* boneName)
{

	int len = m_boneNameList.size();

	for (int i = 0; i < len; i++)
	{
		if (strcmp(m_boneNameList[i], boneName) == 0)
		{
			return i;
		}
	}


	return -1;
}

FbxNode* FBXDataContainer::GetMeshNode(int id)
{
	if (id < m_pMeshContainer.size())
	{
		return m_pMeshContainer[id]->getFbxMesh()->GetNode();
	}
	return nullptr;
}

MeshContainer::~MeshContainer()
{
	m_vertexData.clear();
	m_indexData.clear();

	// SkinAnime05
	//スキンアニメ用データ削除
	m_skinCount = 0;
	// ここまで

	auto meshMng = MyAccessHub::getMyGameEngine()->GetMeshManager();
	meshMng->removeVertexBuffer(m_MeshId, true);
}

void MeshContainer::setFbxMesh(FbxMesh* mesh)
{
	m_mesh = mesh;
	m_meshNodeName = m_mesh->GetNode(0)->GetName();

	// SkinAnime06
	//スキンアニメモーションリセット
	FbxTime timeZero;
	timeZero.SetFrame(0, FbxTime::EMode::eFrames60);

	m_meshNodeName = m_mesh->GetNode(0)->GetName();
	m_IBaseMatrix = m_mesh->GetNode(0)->EvaluateGlobalTransform(timeZero).Inverse();
	// ここまで
}

// SkinAnime07
//スキンアニメフレーム関係メソッド(MeshContainer)
void MeshContainer::InitSkinList(int skinCount)
{
	m_skinCount = skinCount;
}

// ここまで

MaterialContainer::~MaterialContainer()
{
	if (m_uniqueTextures)
	{
		TextureManager* texMng = MyAccessHub::getMyGameEngine()->GetTextureManager();
		for (auto id : m_diffuseTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_normalTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_specularTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_falloffTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_reflectionMapTextures)
		{
			texMng->ReleaseTexture(id);
		}

	}

	m_diffuseTextures.clear();
	m_normalTextures.clear();
	m_specularTextures.clear();
	m_falloffTextures.clear();
	m_reflectionMapTextures.clear();
}

//=======Specular
void MaterialContainer::UpdateD3DResource()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	engine->UpdateShaderResourceOnGPU(m_d3dresource.Get(), &materialInfo, sizeof(FbxMaterialInfo));
}
//=======Specular End

// SkinAnime08
//スキンアニメフレーム関係メソッド(MeshContainer)
int FBXDataContainer::GetClusterId(FbxCluster* pCluster)
{
	return GetClusterId(pCluster->GetLink());
}

int FBXDataContainer::GetClusterId(FbxNode* pNode)
{
	int size = m_boneNameList.size();
	const char* nodeName = pNode->GetName();

	for (int id = 0; id < size; id++)
	{
		if (strcmp(nodeName, m_boneNameList[id]) == 0)
			return id;
	}

	m_boneNameList.push_back(nodeName);

	//新しいノードなので逆行列も保存
	m_IboneMatrix.push_back(pNode->EvaluateGlobalTransform().Inverse());

	return size;	//末尾のIndexがID このsizeはpush_back前のsize()なので今は最終index
}

void FBXDataContainer::GetAnimatedMatrix(const FbxTime& animeTime, FbxScene* animeScene, std::vector<int>& idList, std::vector<XMFLOAT4X4>& resMtxArray)
{
	FbxNode* node;
	int size = idList.size();

	for (int i = 0; i < size; i++)
	{
		int boneId = idList[i];
		if (boneId < 0)
		{
			continue;
		}
		
		node = animeScene->GetNode(boneId);

		FbxAMatrix matrix = node->EvaluateGlobalTransform(animeTime) * m_IboneMatrix[i];

		convertFbxAMatrixToXMFLOAT4x4(matrix, resMtxArray[i]);
	}

}
// ここまで

HRESULT FBXDataContainerSystem::LoadModelFBX(const std::wstring fileName, const std::wstring id)
{

	std::unique_ptr<FBXDataContainer> un_fbx = make_unique<FBXDataContainer>();
	HRESULT hr = un_fbx->LoadFBX(fileName, id);
	if (SUCCEEDED(hr))
	{
		if (m_modelFbxMap[id] != nullptr)
			m_modelFbxMap[id].reset();

		m_modelFbxMap[id] = std::move(un_fbx);
	}

	return hr;
}

FBXDataContainer* FBXDataContainerSystem::GetModelFbx(const std::wstring id)
{
	if (m_modelFbxMap.find(id) == m_modelFbxMap.end())
		return nullptr;

	return m_modelFbxMap[id].get();
}

HRESULT FBXDataContainerSystem::LoadAnimationFBX(const std::wstring fileName, const std::wstring id)
{
	std::unique_ptr<FBXDataContainer> un_fbx = make_unique<FBXDataContainer>();
	HRESULT hr = un_fbx->LoadFBX(fileName, id, true);
	if (SUCCEEDED(hr))
	{
		if (m_animeFbxMap[id] != nullptr)
			m_animeFbxMap[id].reset();

		m_animeFbxMap[id] = std::move(un_fbx);
	}

	return hr;
}

FBXDataContainer* FBXDataContainerSystem::GetAnimeFbx(const std::wstring id)
{
	if (m_animeFbxMap.find(id) == m_animeFbxMap.end())
		return nullptr;

	return m_animeFbxMap[id].get();
}

void FBXDataContainerSystem::ClearModelFBX()
{
	m_modelFbxMap.clear();
}

void FBXDataContainerSystem::ClearAnimeFBX()
{
	m_animeFbxMap.clear();
}

void FBXDataContainerSystem::DeleteModelFBX(std::wstring id)
{
	if (m_modelFbxMap.find(id) != m_modelFbxMap.end())
		m_modelFbxMap.erase(id);
}

void FBXDataContainerSystem::DeleteAnimeFBX(std::wstring id)
{
	if (m_animeFbxMap.find(id) != m_animeFbxMap.end())
		m_animeFbxMap.erase(id);
}
