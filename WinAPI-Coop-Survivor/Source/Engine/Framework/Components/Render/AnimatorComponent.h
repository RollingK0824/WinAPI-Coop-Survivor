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
	void Play(const std::wstring& clipName, bool bRestart = false);
	void Pause();
	void Resume();
	void Stop();

	void SetSpeed(float speed) { m_Speed = speed; }
	float GetSpeed() const { return m_Speed; }

	bool IsPlaying() const { return m_bIsPlaying; }
	int GetCurrentFrameIdx() const { return m_CurrentFrameIdx; }
	void SetCurrentFrameIdx(int frameIdx);

	bool IsFinished() const
	{
		return !m_bIsPlaying
			&& m_pCurrentClip != nullptr
			&& !m_pCurrentClip->bIsLoop;
	}

	void EnsureClipsLoaded();

	AnimationClip* GetCurrentClip() const { return m_pCurrentClip; }

	void SetOnAnimationFinished(std::function<void(const std::wstring&)> callback) { m_onAnimationFinished = callback; }
	void SetOnAnimationFinished(std::function<void()> callback)
	{
		if (callback)
			m_onAnimationFinished = [callback](const std::wstring&) { callback(); };
		else
			m_onAnimationFinished = nullptr;
	}
	void SetOnAnimationFinished(std::nullptr_t) { m_onAnimationFinished = nullptr; }

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::Animator; }

private:
	ObserverPtr<SpriteRendererComponent> m_pSpriteRenderer = nullptr;

	std::unordered_map<std::wstring, AnimationClip> m_MapClips;
	AnimationClip* m_pCurrentClip = nullptr;

	std::vector<std::string> m_vClipKeys;
	std::string m_defaultPlayClip = "";
	std::wstring m_currentClipName = L"";

	int m_CurrentFrameIdx = 0;
	float m_AccTime = 0.0f;
	float m_Speed = 1.0f;
	bool m_bIsPlaying = false;

	std::function<void(const std::wstring&)> m_onAnimationFinished = nullptr;
};