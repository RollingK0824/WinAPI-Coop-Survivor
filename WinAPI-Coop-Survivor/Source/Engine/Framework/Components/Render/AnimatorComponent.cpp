#include "Engine/Core/pch.h"
#include "AnimatorComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"

static ComponentRegistrar<AnimatorComponent> registrar(EngineKey::Component::Animator.data());

AnimatorComponent::AnimatorComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("IsPlaying", &m_bIsPlaying);
	ExposeVariable("CurrentFrameIdx", &m_CurrentFrameIdx);
	ExposeVariable("AccumulatedTime", &m_AccTime);
	ExposeVariable("ClipKeys", &m_vClipKeys);
	ExposeVariable("DefaultPlay", &m_defaultPlayClip);
}

void AnimatorComponent::EnsureClipsLoaded() const
{
	if (m_MapClips.empty() && !m_vClipKeys.empty())
	{
		for (const auto& clipKey : m_vClipKeys)
		{
			std::wstring wClipKey(clipKey.begin(), clipKey.end());
			const AnimationClip* pResClip = ResourceManager::GetInstance()->GetAnimationClip(wClipKey);
			if (pResClip != nullptr)
			{
				m_MapClips[wClipKey] = *pResClip;
			}
		}
	}

	if (m_currentClipName.empty() && !m_defaultPlayClip.empty())
	{
		m_currentClipName = std::wstring(m_defaultPlayClip.begin(), m_defaultPlayClip.end());
	}
}

void AnimatorComponent::Awake()
{
	EnsureClipsLoaded();
	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();

	AnimationClip* pCurrentClip = GetCurrentClip();
	if (m_pSpriteRenderer.IsValid() && pCurrentClip != nullptr && !pCurrentClip->frames.empty())
	{
		if (m_CurrentFrameIdx < static_cast<int>(pCurrentClip->frames.size()))
		{
			const Sprite& currentFrame = pCurrentClip->frames[m_CurrentFrameIdx];
			m_pSpriteRenderer->SetAsSprite(currentFrame);
		}
	}
}

void AnimatorComponent::OnEnable()
{
	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	AnimationClip* pCurrentClip = GetCurrentClip();
	if (pCurrentClip == nullptr && !m_defaultPlayClip.empty())
	{
		std::wstring wDefaultPlay(m_defaultPlayClip.begin(), m_defaultPlayClip.end());
		Play(wDefaultPlay);
		pCurrentClip = GetCurrentClip();
	}
	else if (pCurrentClip != nullptr)
	{
		m_bIsPlaying = true;
	}

	if (m_pSpriteRenderer.IsValid() && pCurrentClip != nullptr && !pCurrentClip->frames.empty())
	{
		if (m_CurrentFrameIdx >= static_cast<int>(pCurrentClip->frames.size()))
		{
			m_CurrentFrameIdx = 0;
		}
		const Sprite& currentFrame = pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::OnDisable()
{
}

void AnimatorComponent::Update(float dt)
{
	AnimationClip* pCurrentClip = GetCurrentClip();
	if (!m_bIsPlaying || pCurrentClip == nullptr || pCurrentClip->frames.empty()) return;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
		if (!m_pSpriteRenderer.IsValid()) return;
	}

	m_AccTime += dt;

	if (m_AccTime >= pCurrentClip->frameRate)
	{
		m_AccTime -= pCurrentClip->frameRate;
		m_CurrentFrameIdx++;

		if (m_CurrentFrameIdx >= static_cast<int>(pCurrentClip->frames.size()))
		{
			if (pCurrentClip->bIsLoop)
			{
				m_CurrentFrameIdx = 0;
			}
			else
			{
				m_CurrentFrameIdx = static_cast<int>(pCurrentClip->frames.size()) - 1;
				m_bIsPlaying = false;
			}
		}

		const Sprite& currentFrame = pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::AddClip(const AnimationClip& clip)
{
	m_MapClips[clip.name] = clip;
}

void AnimatorComponent::Play(const std::wstring& clipName)
{
	EnsureClipsLoaded();
	auto it = m_MapClips.find(clipName);
	if (it == m_MapClips.end()) return;

	m_currentClipName = clipName;
	m_CurrentFrameIdx = 0;
	m_AccTime = 0.0f;
	m_bIsPlaying = true;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	AnimationClip* pCurrentClip = &it->second;
	if (m_pSpriteRenderer.IsValid() && pCurrentClip && !pCurrentClip->frames.empty())
	{
		const Sprite& currentFrame = pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::Stop()
{
	m_bIsPlaying = false;
}

void AnimatorComponent::PostDeserialize(Scene* pScene)
{
	ScriptComponent::PostDeserialize(pScene);

	m_MapClips.clear();
	for (const auto& clipKey : m_vClipKeys)
	{
		std::wstring wClipKey(clipKey.begin(), clipKey.end());
		const AnimationClip* pResClip = ResourceManager::GetInstance()->GetAnimationClip(wClipKey);
		if (pResClip != nullptr)
		{
			m_MapClips[wClipKey] = *pResClip;
		}
	}
	if (!m_defaultPlayClip.empty())
	{
		std::wstring wDefaultPlay(m_defaultPlayClip.begin(), m_defaultPlayClip.end());
		Play(wDefaultPlay);
	}
}
