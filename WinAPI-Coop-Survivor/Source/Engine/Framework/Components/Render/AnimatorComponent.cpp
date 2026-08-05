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

AnimatorComponent::AnimatorComponent(const AnimatorComponent& other)
	: ScriptComponent(other)
{
	this->m_bIsPlaying = other.m_bIsPlaying;
	this->m_CurrentFrameIdx = other.m_CurrentFrameIdx;
	this->m_AccTime = other.m_AccTime;
	this->m_vClipKeys = other.m_vClipKeys;
	this->m_defaultPlayClip = other.m_defaultPlayClip;
	this->m_MapClips = other.m_MapClips;

	if (other.m_pCurrentClip != nullptr)
	{
		this->m_pCurrentClip = &this->m_MapClips[other.m_pCurrentClip->name];
	}
	else
	{
		this->m_pCurrentClip = nullptr;
	}

	this->m_pSpriteRenderer = nullptr;
}

void AnimatorComponent::Awake()
{
	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();

	if (m_pSpriteRenderer != nullptr && m_bIsPlaying && m_pCurrentClip != nullptr)
	{
		if (m_CurrentFrameIdx < m_pCurrentClip->frames.size())
		{
			const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
			m_pSpriteRenderer->SetAsSprite(currentFrame);
		}
	}
}

void AnimatorComponent::Update(float dt)
{
	if (!m_bIsPlaying || m_pCurrentClip == nullptr || m_pSpriteRenderer == nullptr) return;

	m_AccTime += dt;

	if (m_AccTime >= m_pCurrentClip->frameRate)
	{
		m_AccTime -= m_pCurrentClip->frameRate;
		m_CurrentFrameIdx++;

		if (m_CurrentFrameIdx >= m_pCurrentClip->frames.size())
		{
			if (m_pCurrentClip->bIsLoop)
			{
				m_CurrentFrameIdx = 0;
			}
			else
			{
				m_CurrentFrameIdx = static_cast<int>(m_pCurrentClip->frames.size()) - 1;
				m_bIsPlaying = false;
			}
		}

		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
		m_pSpriteRenderer->SetAsSprite(currentFrame);
	}
}

void AnimatorComponent::AddClip(const AnimationClip& clip)
{
	m_MapClips[clip.name] = clip;
}

void AnimatorComponent::Play(const std::wstring& clipName)
{
	auto it = m_MapClips.find(clipName);
	if (it == m_MapClips.end()) return;
	if (m_pCurrentClip == &it->second && m_bIsPlaying) return;

	m_pCurrentClip = &it->second;
	m_CurrentFrameIdx = 0;
	m_AccTime = 0.0f;
	m_bIsPlaying = true;

	if (m_pSpriteRenderer && !m_pCurrentClip->frames.empty())
	{
		const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
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
