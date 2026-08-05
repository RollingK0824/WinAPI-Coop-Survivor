#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Renderer/Sprite.h"

class SpriteRendererComponent;

class AnimatorComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(AnimatorComponent)

	AnimatorComponent(GameObject* owner, TransformComponent* transform);
	AnimatorComponent(const AnimatorComponent& other);
	virtual ~AnimatorComponent() override = default;

	virtual void Awake() override;
	virtual void Update(float dt) override;
	virtual void PostDeserialize(Scene* pScene) override;

	void AddClip(const AnimationClip& clip);
	void Play(const std::wstring& clipName);
	void Stop();
	bool IsFinished() const
	{
		return !m_bIsPlaying 
			&& m_pCurrentClip != nullptr 
			&& !m_pCurrentClip->bIsLoop;
	}

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::Animator; }

private:
	SpriteRendererComponent* m_pSpriteRenderer = nullptr;

	std::unordered_map<std::wstring, AnimationClip> m_MapClips;
	std::vector<std::string> m_vClipKeys;
	std::string m_defaultPlayClip = "";
	AnimationClip* m_pCurrentClip = nullptr;

	int m_CurrentFrameIdx = 0;
	float m_AccTime = 0.0f;
	bool m_bIsPlaying = false;
};