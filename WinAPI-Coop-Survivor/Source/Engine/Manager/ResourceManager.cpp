#include "Engine/Core/pch.h"
#include "ResourceManager.h"
#include "Engine/Core/Define.h"
#include "Engine/Renderer/GraphicManager.h"

#pragma comment(lib, "windowscodecs.lib")

bool ResourceManager::Initialize()
{
	// WIC Imaging Factory 생성
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pWICFactory)
	);

	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"WIC Factory 생성 실패", L"Error", MB_ICONERROR);
		return false;
	}
	return true;
}

ID2D1Bitmap* ResourceManager::LoadTexture(const std::wstring& key, const std::wstring& filePath)
{
	auto iter = m_texturePool.find(key);
	if (iter != m_texturePool.end()) return iter->second;
	if (!m_pWICFactory) return nullptr;
	ID2D1RenderTarget* pRenderTarget = GraphicManager::GetInstance()->GetRenderTarget();
	if (!pRenderTarget) return nullptr;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	ID2D1Bitmap* pBitmap = nullptr;
	if (FAILED(m_pWICFactory->CreateDecoderFromFilename(filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder)))
		return nullptr;
	if (FAILED(pDecoder->GetFrame(0, &pSource))) { pDecoder->Release(); return nullptr; }
	if (FAILED(m_pWICFactory->CreateFormatConverter(&pConverter))) { pSource->Release(); pDecoder->Release(); return nullptr; }
	if (SUCCEEDED(pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
	{
		// 1. Direct2D Bitmap 생성
		pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, &pBitmap);
		// 2. ★ ImGui용 DX11 Texture2D & SRV 동시 생성 ★
		UINT width = 0, height = 0;
		pConverter->GetSize(&width, &height);
		std::vector<BYTE> pixels(width * height * 4);
		pConverter->CopyPixels(nullptr, width * 4, static_cast<UINT>(pixels.size()), pixels.data());
		ID3D11Device* pD3DDevice = GraphicManager::GetInstance()->GetD3DDevice();
		if (pD3DDevice && width > 0 && height > 0)
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = pixels.data();
			initData.SysMemPitch = width * 4;
			ID3D11Texture2D* pTex2D = nullptr;
			if (SUCCEEDED(pD3DDevice->CreateTexture2D(&desc, &initData, &pTex2D)) && pTex2D)
			{
				ID3D11ShaderResourceView* pSRV = nullptr;
				if (SUCCEEDED(pD3DDevice->CreateShaderResourceView(pTex2D, nullptr, &pSRV)))
				{
					m_srvPool[key] = pSRV; // SRV 보관
				}
				pTex2D->Release();
			}
		}
	}
	if (pConverter) pConverter->Release();
	if (pSource) pSource->Release();
	if (pDecoder) pDecoder->Release();
	if (pBitmap)
	{
		m_texturePool[key] = pBitmap;

		D2D1_SIZE_F sz = pBitmap->GetSize();
		Sprite sp;
		sp.pTexture = pBitmap;
		sp.srcRect = D2D1::RectF(0.0f, 0.0f, sz.width, sz.height);
		sp.pivot = D2D1::Point2F(0.5f, 0.5f);
		sp.originalWidth = sz.width;
		sp.originalHeight = sz.height;
		m_spritePool[key] = sp;

		return pBitmap;
	}
	return nullptr;
}

ID2D1Bitmap* ResourceManager::GetTexture(const std::wstring& key) const
{
	auto iter = m_texturePool.find(key);
	if (iter != m_texturePool.end())
	{
		return iter->second;
	}
	return nullptr;
}

const Sprite* ResourceManager::GetSprite(const std::wstring& spriteKey) const
{
	if (spriteKey.empty()) return nullptr;

	auto iter = m_spritePool.find(spriteKey);
	if (iter != m_spritePool.end())
	{
		return &(iter->second);
	}

	auto texIter = m_texturePool.find(spriteKey);
	if (texIter != m_texturePool.end() && texIter->second != nullptr)
	{
		ID2D1Bitmap* pBitmap = texIter->second;
		D2D1_SIZE_F sz = pBitmap->GetSize();

		Sprite sp;
		sp.pTexture = pBitmap;
		sp.srcRect = D2D1::RectF(0.0f, 0.0f, sz.width, sz.height);
		sp.pivot = D2D1::Point2F(0.5f, 0.5f);
		sp.originalWidth = sz.width;
		sp.originalHeight = sz.height;

		const_cast<ResourceManager*>(this)->m_spritePool[spriteKey] = sp;
		return &(const_cast<ResourceManager*>(this)->m_spritePool[spriteKey]);
	}

	return nullptr;
}

bool ResourceManager::LoadResourcesFromJson(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open()) return false;

	json rootJson;
	file >> rootJson;
	file.close();

	if (rootJson.contains(EngineKey::Document::Textures.data()))
	{
		for (const auto& texData : rootJson[EngineKey::Document::Textures.data()])
		{
			std::string strKey = texData["Key"].get<std::string>();
			std::wstring wKey(strKey.begin(), strKey.end());

			// 아틀라스 JSON 경로가 있는 경우
			if (texData.contains("AtlasPath"))
			{
				std::string atlasPath = texData["AtlasPath"].get<std::string>();

				this->LoadSpriteAtlas(atlasPath, wKey);
			}
			// 일반 텍스처인 경우 단일 이미지 로드
			else if (texData.contains("Path"))
			{
				std::string strPath = texData["Path"].get<std::string>();
				std::wstring wPath(strPath.begin(), strPath.end());

				this->LoadTexture(wKey, wPath);
			}
		}
	}
	return true;
}

bool ResourceManager::LoadSpriteAtlas(const std::string& jsonPath, const std::wstring& textureKey)
{
	std::ifstream file(jsonPath);
	if (!file.is_open())return false;

	json atlasJson;
	file >> atlasJson;
	file.close();

	std::string textureName = atlasJson["meta"]["image"].get<std::string>();
	std::wstring wTextureKey(textureName.begin(), textureName.end());

	std::string fullPath = "Resources/Texture/" + textureName;
	std::wstring wFilePath(fullPath.begin(), fullPath.end());

	if (!LoadTexture(wTextureKey, wFilePath))return false;

	ID2D1Bitmap* pBitmap = GetTexture(wTextureKey);

	std::unordered_map<std::string, Sprite>tempSprites;
	for (const auto& f : atlasJson["frames"])
	{
		std::string filename = f["filename"].get<std::string>();

		Sprite sp;
		sp.pTexture = pBitmap;

		float x = f["frame"]["x"].get<float>();
		float y = f["frame"]["y"].get<float>();
		float w = f["frame"]["w"].get<float>();
		float h = f["frame"]["h"].get<float>();
		sp.srcRect = D2D1::RectF(x, y, x + w, y + h);

		sp.offset.x = f["spriteSourceSize"]["x"].get<float>();
		sp.offset.y = f["spriteSourceSize"]["y"].get<float>();

		sp.originalWidth = f["sourceSize"]["w"].get<float>();
		sp.originalHeight = f["sourceSize"]["h"].get<float>();

		sp.pivot = D2D1::Point2F(0.5f, 0.5f);

		tempSprites[filename] = sp;

		std::wstring wSpriteKey(filename.begin(), filename.end());
		std::wstring texKeyNoExt = wTextureKey;
		size_t dotPos = texKeyNoExt.find_last_of(L'.');
		if (dotPos != std::wstring::npos) texKeyNoExt = texKeyNoExt.substr(0, dotPos);

		// 네임스페이스 키 (AtlasName/FrameName) 등록
		m_spritePool[texKeyNoExt + L"/" + wSpriteKey] = sp;
		// 폴백 조회를 위한 단일 키 등록
		m_spritePool[wSpriteKey] = sp;
	}

	if (atlasJson["meta"].contains("custom_clips"))
	{
		for (const auto& clipData : atlasJson["meta"]["custom_clips"])
		{
			AnimationClip clip;
			std::string clipName = clipData["name"].get<std::string>();
			clip.name = std::wstring(clipName.begin(), clipName.end());
			clip.frameRate = clipData["frameRate"].get<float>();
			clip.bIsLoop = clipData["isLoop"].get<bool>();

			for (const auto& fNameJson : clipData["frames"])
			{
				std::string fName = fNameJson.get<std::string>();

				if (tempSprites.find(fName) != tempSprites.end())
				{
					clip.frames.push_back(tempSprites[fName]);
				}
			}
			m_MapClips[clip.name] = clip;
		}
	}

	return true;
}

bool ResourceManager::LoadGridSpriteAtlas(const std::wstring& key, const std::wstring& filePath, float frameWidth, float frameHeight)
{
	ID2D1Bitmap* pBitmap = LoadTexture(key, filePath);
	if (!pBitmap || frameWidth <= 0.0f || frameHeight <= 0.0f) return false;

	D2D1_SIZE_F sz = pBitmap->GetSize();
	int cols = static_cast<int>(sz.width / frameWidth);
	int rows = static_cast<int>(sz.height / frameHeight);

	std::wstring texKeyNoExt = key;
	size_t dotPos = texKeyNoExt.find_last_of(L'.');
	if (dotPos != std::wstring::npos) texKeyNoExt = texKeyNoExt.substr(0, dotPos);

	int index = 0;
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			Sprite sp;
			sp.pTexture = pBitmap;
			float x = c * frameWidth;
			float y = r * frameHeight;
			sp.srcRect = D2D1::RectF(x, y, x + frameWidth, y + frameHeight);
			sp.pivot = D2D1::Point2F(0.5f, 0.5f);
			sp.originalWidth = frameWidth;
			sp.originalHeight = frameHeight;

			std::wstring frameName = std::to_wstring(index);
			m_spritePool[texKeyNoExt + L"/" + frameName] = sp;
			m_spritePool[frameName] = sp;
			index++;
		}
	}
	return true;
}

const AnimationClip* ResourceManager::GetAnimationClip(const std::wstring& clipKey) const
{
	auto it = m_MapClips.find(clipKey);
	if (it != m_MapClips.end())return &(it->second);
	return nullptr;
}

ID3D11ShaderResourceView* ResourceManager::GetTextureSRV(const std::wstring& key) const
{
	auto iter = m_srvPool.find(key);
	if (iter != m_srvPool.end())
	{
		return iter->second;
	}
	return nullptr;
}

ID3D11ShaderResourceView* ResourceManager::GetTextureSRV(ID2D1Bitmap* pBitmap) const
{
	if (!pBitmap) return nullptr;
	for (const auto& pair : m_texturePool)
	{
		if (pair.second == pBitmap)
		{
			auto srvIter = m_srvPool.find(pair.first);
			if (srvIter != m_srvPool.end())
			{
				return srvIter->second;
			}
		}
	}
	return nullptr;
}

std::vector<std::string> ResourceManager::GetLoadedTextureKeys() const
{
	std::vector<std::string> keys;
	keys.reserve(m_texturePool.size());
	for (const auto& pair : m_texturePool)
	{
		std::string keyStr(pair.first.begin(), pair.first.end());
		keys.push_back(keyStr);
	}
	return keys;
}

std::vector<std::string> ResourceManager::GetLoadedSpriteKeys() const
{
	std::vector<std::string> keys;
	std::set<std::string> atlasPrefixes;

	// 1차 패스: 서브 스프라이트를 가지는 아틀라스 접두사 추출 (예: "Characeter_Green", "tilemap_packed")
	for (const auto& pair : m_spritePool)
	{
		std::string keyStr(pair.first.begin(), pair.first.end());
		size_t slashPos = keyStr.find('/');
		if (slashPos != std::string::npos)
		{
			atlasPrefixes.insert(keyStr.substr(0, slashPos));
		}
	}

	std::set<std::string> uniqueKeys;

	// 2차 패스: 슬라이스된 서브 스프라이트 키 수집 (AtlasName/FrameName)
	for (const auto& pair : m_spritePool)
	{
		std::string keyStr(pair.first.begin(), pair.first.end());
		if (keyStr.find('/') != std::string::npos)
		{
			uniqueKeys.insert(keyStr);
		}
	}

	// 3차 패스: 아틀라스가 아닌 단일 정적 텍스처 수집 (introBG.png 등)
	for (const auto& pair : m_texturePool)
	{
		std::string keyStr(pair.first.begin(), pair.first.end());
		std::string stem = keyStr;
		size_t dotPos = stem.find_last_of('.');
		if (dotPos != std::string::npos) stem = stem.substr(0, dotPos);

		if (atlasPrefixes.find(keyStr) == atlasPrefixes.end() && atlasPrefixes.find(stem) == atlasPrefixes.end())
		{
			uniqueKeys.insert(keyStr);
		}
	}

	for (const auto& k : uniqueKeys)
	{
		keys.push_back(k);
	}
	return keys;
}

void ResourceManager::Release()
{
	// 텍스처 풀에 등록된 모든 비트맵 리소스 해제
	for (auto& pair : m_texturePool)
	{
		if (pair.second)
		{
			pair.second->Release();
			pair.second = nullptr;
		}
	}
	m_texturePool.clear();

	for (auto& pair : m_srvPool)
	{
		if (pair.second) pair.second->Release();
	}
	m_srvPool.clear();

	// WIC Factory 해제
	if (m_pWICFactory)
	{
		m_pWICFactory->Release();
		m_pWICFactory = nullptr;
	}

}