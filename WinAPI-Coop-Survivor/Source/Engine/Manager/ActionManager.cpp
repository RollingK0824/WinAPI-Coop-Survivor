#include "Engine/Core/pch.h"
#include "ActionManager.h"
#include "Engine/Manager/InputManager.h"

bool ActionManager::Initialize()
{
    return true;
}

void ActionManager::Release()
{
    m_mActionMap.clear();
	std::map<std::string, std::vector<ActionBinding>>().swap(m_mActionMap);
}

bool ActionManager::BindAction(const std::string& actionName, int vkCode)
{
    m_mActionMap[actionName].clear();
    m_mActionMap[actionName].push_back({ vkCode, {} });
    return true;
}

bool ActionManager::BindShortcut(const std::string& actionName, int mainKey, const std::vector<int>& modifiers)
{
    m_mActionMap[actionName].clear();
    m_mActionMap[actionName].push_back(ActionBinding{ mainKey, modifiers });
    return true;
}

bool ActionManager::UnbindAction(const std::string& actionName)
{
	return m_mActionMap.erase(actionName) > 0;
}

bool ActionManager::GetActionDown(const std::string& actionName) const
{
    auto it = m_mActionMap.find(actionName);
    if (it == m_mActionMap.end())
    {
        return false;
    }

    InputManager* pInput = InputManager::GetInstance();

    for (const auto& binding : it->second)
    {
        bool modifiersHeld = true;
        for (int modKey : binding.modifierKeys)
        {
            if (!pInput->GetKeyPress(modKey))
            {
                modifiersHeld = false;
                break;
            }
        }
        if (modifiersHeld && pInput->GetKeyDown(binding.mainKey))
        {
            return true;
        }
    }
    return false;
}

bool ActionManager::GetActionPress(const std::string& actionName) const
{
    auto it = m_mActionMap.find(actionName);
    if (it == m_mActionMap.end())
    {
        return false;
    }

    InputManager* pInput = InputManager::GetInstance();

    for (const auto& binding : it->second)
    {
        bool modifiersHeld = true;
        for (int modKey : binding.modifierKeys)
        {
            if (!pInput->GetKeyPress(modKey))
            {
                modifiersHeld = false;
                break;
            }
        }
        if (modifiersHeld && pInput->GetKeyPress(binding.mainKey))
        {
            return true;
        }
    }
    return false;
}

bool ActionManager::GetActionUp(const std::string& actionName) const
{
    auto it = m_mActionMap.find(actionName);
    if (it == m_mActionMap.end())
    {
        return false;
    }

    InputManager* pInput = InputManager::GetInstance();
    for (const auto& binding : it->second)
    {
        if (pInput->GetKeyUp(binding.mainKey))
        {
            bool modifiersValid = true;
            for (int modKey : binding.modifierKeys)
            {
                if (!pInput->GetKeyPress(modKey) && !pInput->GetKeyUp(modKey))
                {
                    modifiersValid = false;
                    break;
                }
            }
            if (modifiersValid)
            {
                return true;
            }
        }
    }
    return false;
}
