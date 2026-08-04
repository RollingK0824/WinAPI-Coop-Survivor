#include "Engine/Core/pch.h"
#include "AnimatorComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Render/RenderComponent.h"

static ComponentRegistrar<AnimatorComponent> registrar(EngineKey::Component::Animator.data());

AnimatorComponent::AnimatorComponent(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform) 
{
    ExposeVariable("Is Playing", &m_bIsPlaying);
    ExposeVariable("Current Frame Idx", &m_CurrentFrameIdx);
    ExposeVariable("Accumulated Time", &m_AccTime);
}

AnimatorComponent::AnimatorComponent(const AnimatorComponent& other) 
    : ScriptComponent(other)
{
    this->m_bIsPlaying = other.m_bIsPlaying;
    this->m_CurrentFrameIdx = other.m_CurrentFrameIdx;
    this->m_AccTime = other.m_AccTime;

    this->m_MapClips = other.m_MapClips;

    if (other.m_pCurrentClip != nullptr)
    {
        this->m_pCurrentClip = &this->m_MapClips[other.m_pCurrentClip->name];
    }
    else
    {
        this->m_pCurrentClip = nullptr;
    }

    this->m_pRenderComp = nullptr;
}

void AnimatorComponent::Awake()
{
	m_pRenderComp = gameObject.GetComponent<RenderComponent>();

    if (m_pRenderComp != nullptr && m_bIsPlaying && m_pCurrentClip != nullptr)
    {
        if (m_CurrentFrameIdx < m_pCurrentClip->frames.size())
        {
            const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
            m_pRenderComp->SetAsSprite(currentFrame);
        }
    }
}

void AnimatorComponent::Update(float dt)
{
    //if (!m_bIsPlaying || m_pCurrentClip == nullptr || m_pRenderComp == nullptr) return;

    //m_AccTime += dt;

    //if (m_AccTime >= m_pCurrentClip->frameRate)
    //{
    //    m_AccTime -= m_pCurrentClip->frameRate;
    //    m_CurrentFrameIdx++;

    //    if (m_CurrentFrameIdx >= m_pCurrentClip->frames.size())
    //    {
    //        if (m_pCurrentClip->bIsLoop)
    //        {
    //            m_CurrentFrameIdx = 0;
    //        }
    //        else
    //        {
    //            m_CurrentFrameIdx = static_cast<int>(m_pCurrentClip->frames.size()) - 1;
    //            m_bIsPlaying = false;
    //            // TODO : 애니메이션 종료 동작 추가
    //        }
    //    }

    //    const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
    //    m_pRenderComp->SetAsSprite(currentFrame);
    //}
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

    if (m_pRenderComp)
    {
        const Sprite& currentFrame = m_pCurrentClip->frames[m_CurrentFrameIdx];
        m_pRenderComp->SetAsSprite(currentFrame);
    }
}

void AnimatorComponent::Stop()
{
    m_bIsPlaying = false;
}

void AnimatorComponent::Serialize(json& outJson) const
{
    ScriptComponent::Serialize(outJson);

    json clipKeysArray = json::array();

    for (const auto& pair : m_MapClips)
    {
        std::wstring wKey = pair.first;
        std::string strKey(wKey.begin(), wKey.end());

        clipKeysArray.push_back(strKey);
    }
    outJson["ClipKeys"] = clipKeysArray;

    if (m_pCurrentClip != nullptr)
    {
        std::wstring wName = m_pCurrentClip->name;
        std::string strName(wName.begin(), wName.end());

        outJson["DefaultPlay"] = strName;
    }
}

void AnimatorComponent::Deserialize(const json& inJson)
{
    ScriptComponent::Deserialize(inJson);

    if (inJson.contains("ClipKeys"))
    {
        for (const auto& keyJson : inJson["ClipKeys"])
        {
            std::string clipKey = keyJson.get<std::string>();
            std::wstring wClipKey(clipKey.begin(), clipKey.end());
            const AnimationClip* pResClip = ResourceManager::GetInstance()->GetAnimationClip(wClipKey);
            if (pResClip != nullptr)
            {
                m_MapClips[wClipKey] = *pResClip;
            }
        }
    }

    if (inJson.contains("DefaultPlay"))
    {
        std::string defaultPlayStr = inJson["DefaultPlay"].get<std::string>();
        std::wstring wDefaultPlay(defaultPlayStr.begin(), defaultPlayStr.end());
        Play(wDefaultPlay);
    }
}
