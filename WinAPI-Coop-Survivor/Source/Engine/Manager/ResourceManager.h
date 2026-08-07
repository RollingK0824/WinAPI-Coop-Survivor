#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Renderer/Sprite.h"

struct ID2D1Bitmap;
struct IWICImagingFactory;

class ResourceManager : public Singleton<ResourceManager>, public ISystem
{
	friend class Singleton<ResourceManager>;

public:
	// IManager 인터페이스 구현
	virtual bool Initialize() override;
	virtual void Release() override;

	// 이미지를 읽어와 GPU 비트맵으로 등록
	ID2D1Bitmap* LoadTexture(const std::wstring& key, const std::wstring& filePath);

	// 등록된 비트맵을 꺼내 사용
	ID2D1Bitmap* GetTexture(const std::wstring& key) const;

	const Sprite* GetSprite(const std::wstring& spriteKey) const;

	bool LoadResourcesFromJson(const std::string& filePath);
	bool LoadSpriteAtlas(const std::string& jsonPath, const std::wstring& textureKey);
	bool LoadGridSpriteAtlas(const std::wstring& key, const std::wstring& filePath, float frameWidth, float frameHeight);

	const AnimationClip* GetAnimationClip(const std::wstring& clipKey)const;

	ID3D11ShaderResourceView* GetTextureSRV(const std::wstring& key) const;
	ID3D11ShaderResourceView* GetTextureSRV(ID2D1Bitmap* pBitmap) const;

	std::vector<std::string> GetLoadedTextureKeys() const;
	std::vector<std::string> GetLoadedSpriteKeys() const;
private:
	ResourceManager() = default;
	virtual ~ResourceManager() = default;

	IWICImagingFactory* m_pWICFactory = nullptr;

	std::unordered_map<std::wstring, ID2D1Bitmap*> m_texturePool;
	std::unordered_map<std::wstring, Sprite> m_spritePool;
	std::unordered_map<std::wstring, AnimationClip> m_MapClips;

	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_srvPool;

};