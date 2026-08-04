#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

struct ActionBinding
{
	int mainKey = 0;
	std::vector<int> modifierKeys;
};

class ActionManager : public Singleton<ActionManager>, public ISystem
{
	friend class Singleton<ActionManager>;
public:
	virtual bool Initialize() override;
	virtual void Release() override;

	bool BindAction(const std::string& actionName, int vkCode);
	bool BindShortcut(const std::string& actionName, int mainKey, const std::vector<int>& modifiers);
	bool UnbindAction(const std::string& actionName);

	bool GetActionDown(const std::string& actionName) const;
	bool GetActionPress(const std::string& actionName) const;
	bool GetActionUp(const std::string& actionName) const;
private:
	ActionManager() = default;
	virtual ~ActionManager() = default;

	std::map<std::string, std::vector<ActionBinding>> m_mActionMap;
};

