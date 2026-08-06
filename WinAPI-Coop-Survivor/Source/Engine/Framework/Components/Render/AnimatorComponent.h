#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Renderer/Sprite.h"
#include "Engine/Core/ObserverPtr.h"

class SpriteRendererComponent;

class AnimatorComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(AnimatorComponent)

	AnimatorComponent(GameObject* owner, TransformComponent* transform);
	virtual ~AnimatorComponent() override = default;

	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void Update(float dt) override;
	virtual void PostDeserialize(Scene* pScene) override;

	void AddClip(const AnimationClip& clip);
	void Play(const std::wstring& clipName);
	void Stop();
	bool IsFinished() const
	{
		const AnimationClip* pClip = GetCurrentClip();
		return !m_bIsPlaying 
			&& pClip != nullptr 
			&& !pClip->bIsLoop;
	}

	void EnsureClipsLoaded() const;

	AnimationClip* GetCurrentClip() const
	{
		EnsureClipsLoaded();
		if (m_currentClipName.empty()) return nullptr;
		auto it = m_MapClips.find(m_currentClipName);
		return (it != m_MapClips.end()) ? const_cast<AnimationClip*>(&it->second) : nullptr;
	}

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::Animator; }

private:
	ObserverPtr<SpriteRendererComponent> m_pSpriteRenderer = nullptr;

	mutable std::unordered_map<std::wstring, AnimationClip> m_MapClips;
	std::vector<std::string> m_vClipKeys;
	std::string m_defaultPlayClip = "";
	mutable std::wstring m_currentClipName = L"";

	int m_CurrentFrameIdx = 0;
	float m_AccTime = 0.0f;
	bool m_bIsPlaying = false;
};