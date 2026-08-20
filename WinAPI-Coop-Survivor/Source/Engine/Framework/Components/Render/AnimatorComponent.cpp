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
	ExposeVariable("PlaySpeed", &m_Speed);
	ExposeVariable("ClipKeys", &m_vClipKeys);
	ExposeVariable("DefaultPlay", &m_defaultPlayClip);
}

void AnimatorComponent::EnsureClipsLoaded()
{
	if (!m_vClipKeys.empty())
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

	if (!m_currentClipName.empty())
	{
		auto it = m_MapClips.find(m_currentClipName);
		m_pCurrentClip = (it != m_MapClips.end()) ? &it->second : nullptr;
	}
}

void AnimatorComponent::Awake()
{
	EnsureClipsLoaded();
	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();

	if (m_pSpriteRenderer.IsValid() && m_pCurrentClip != nullptr && !m_pCurrentClip->frames.empty())
	{
		if (m_CurrentFrameIdx < static_cast<int>(m_pCurrentClip->frames.size()))
		{
			const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
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

	if (m_pCurrentClip == nullptr && !m_defaultPlayClip.empty())
	{
		std::wstring wDefaultPlay(m_defaultPlayClip.begin(), m_defaultPlayClip.end());
		EnsureClipsLoaded();
		auto it = m_MapClips.find(wDefaultPlay);
		if (it != m_MapClips.end())
		{
			m_currentClipName = wDefaultPlay;
			m_pCurrentClip = &it->second;
		}
	}

	if (m_pSpriteRenderer.IsValid() && m_pCurrentClip != nullptr && !m_pCurrentClip->frames.empty())
	{
		if (m_CurrentFrameIdx >= static_cast<int>(m_pCurrentClip->frames.size()))
		{
			m_CurrentFrameIdx = 0;
		}
		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::OnDisable()
{
}

void AnimatorComponent::Update(float dt)
{
	if (!m_bIsPlaying || m_pCurrentClip == nullptr || m_pCurrentClip->frames.empty()) return;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
		if (!m_pSpriteRenderer.IsValid()) return;
	}

	m_AccTime += dt * m_Speed;

	if (m_AccTime >= m_pCurrentClip->frameDuration)
	{
		m_AccTime -= m_pCurrentClip->frameDuration;
		m_CurrentFrameIdx++;

		bool bFinished = false;
		if (m_CurrentFrameIdx >= static_cast<int>(m_pCurrentClip->frames.size()))
		{
			if (m_pCurrentClip->bIsLoop)
			{
				m_CurrentFrameIdx = 0;
			}
			else
			{
				m_CurrentFrameIdx = static_cast<int>(m_pCurrentClip->frames.size()) - 1;
				m_bIsPlaying = false;
				bFinished = true;
			}
		}

		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);

		if (bFinished && m_onAnimationFinished)
		{
			auto cb = m_onAnimationFinished;
			cb(m_currentClipName);
		}
	}
}

void AnimatorComponent::AddClip(const AnimationClip& clip)
{
	m_MapClips[clip.name] = clip;
	if (m_currentClipName == clip.name)
	{
		m_pCurrentClip = &m_MapClips[clip.name];
	}
}

void AnimatorComponent::Play(const std::wstring& clipName, bool bRestart)
{
	EnsureClipsLoaded();
	auto it = m_MapClips.find(clipName);
	if (it == m_MapClips.end())
	{
		const AnimationClip* pClip = ResourceManager::GetInstance()->GetAnimationClip(clipName);
		if (pClip)
		{
			AddClip(*pClip);
			it = m_MapClips.find(clipName);
		}
		else
		{
			return;
		}
	}

	if (m_pCurrentClip == &it->second && !bRestart)
	{
		m_bIsPlaying = true;
		return;
	}

	m_currentClipName = clipName;
	m_pCurrentClip = &it->second;
	m_CurrentFrameIdx = 0;
	m_AccTime = 0.0f;
	m_bIsPlaying = true;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	if (m_pSpriteRenderer.IsValid() && m_pCurrentClip && !m_pCurrentClip->frames.empty())
	{
		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::Pause()
{
	m_bIsPlaying = false;
}

void AnimatorComponent::Resume()
{
	if (m_pCurrentClip != nullptr)
	{
		m_bIsPlaying = true;
	}
}

void AnimatorComponent::Stop()
{
	m_bIsPlaying = false;
	m_CurrentFrameIdx = 0;
	m_AccTime = 0.0f;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	if (m_pSpriteRenderer.IsValid() && m_pCurrentClip && !m_pCurrentClip->frames.empty())
	{
		const Sprite& currentFrame = m_pCurrentClip->frames[0];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::SetCurrentFrameIdx(int frameIdx)
{
	if (m_pCurrentClip == nullptr || m_pCurrentClip->frames.empty()) return;

	m_CurrentFrameIdx = std::clamp(frameIdx, 0, static_cast<int>(m_pCurrentClip->frames.size()) - 1);
	m_AccTime = 0.0f;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	if (m_pSpriteRenderer.IsValid())
	{
		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::PostDeserialize(Scene* pScene)
{
	ScriptComponent::PostDeserialize(pScene);

	EnsureClipsLoaded();

	if (m_pCurrentClip == nullptr && !m_defaultPlayClip.empty())
	{
		std::wstring wDefaultPlay(m_defaultPlayClip.begin(), m_defaultPlayClip.end());
		auto it = m_MapClips.find(wDefaultPlay);
		if (it != m_MapClips.end())
		{
			m_currentClipName = wDefaultPlay;
			m_pCurrentClip = &it->second;
		}
	}
}
