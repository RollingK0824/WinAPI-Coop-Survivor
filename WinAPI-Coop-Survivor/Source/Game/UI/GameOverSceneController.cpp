#include "Engine/Core/pch.h"
#include "GameOverSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"

static ComponentRegistrar<GameOverSceneController> registrar(EngineKey::CustomComponent::GameOverSceneController.data());

GameOverSceneController::GameOverSceneController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("OK Button", &m_okBtn);
}

void GameOverSceneController::Start()
{
	ScriptComponent::Start();

	NetworkManager::GetInstance()->StopNetwork();

	if (m_okBtn != nullptr)
	{
		m_okBtn->SetOnClick([this]() { OnClickOkBtn(); });
	}
}

void GameOverSceneController::OnClickOkBtn()
{
	NetworkManager::GetInstance()->StopNetwork();
	SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/Title.scene");
}
