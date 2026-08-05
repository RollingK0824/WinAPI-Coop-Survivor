#include "Engine/Core/pch.h"
#include "TitleSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"

static ComponentRegistrar<TitleSceneController> registrar(EngineKey::CustomComponent::TitleSceneController.data());

TitleSceneController::TitleSceneController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Host Button", &host_Btn);
	ExposeComponent("Join Button", &join_Btn);
	ExposeComponent("Exit Button", &exit_Btn);
}

void TitleSceneController::Awake()
{
	ScriptComponent::Awake();

	Scene* pActiveScene = gameObject.GetOwnerScene();
	if (pActiveScene != nullptr)
	{
		if (host_Btn == nullptr && m_hostBtnID != 0)
		{
			if (GameObject* pObj = pActiveScene->FindGameObjectByInstanceID(m_hostBtnID))
			{
				host_Btn = pObj->GetComponent<UIButtonComponent>();
			}
		}

		if (join_Btn == nullptr && m_joinBtnID != 0)
		{
			if (GameObject* pObj = pActiveScene->FindGameObjectByInstanceID(m_joinBtnID))
			{
				join_Btn = pObj->GetComponent<UIButtonComponent>();
			}
		}

		if (exit_Btn == nullptr && m_exitBtnID != 0)
		{
			if (GameObject* pObj = pActiveScene->FindGameObjectByInstanceID(m_exitBtnID))
			{
				exit_Btn = pObj->GetComponent<UIButtonComponent>();
			}
		}
	}
}

void TitleSceneController::Start()
{
	ScriptComponent::Start();

	if (host_Btn != nullptr)
	{
		host_Btn->SetOnClick([this]() { OnClickHostBtn(); });
	}

	if (join_Btn != nullptr)
	{
		join_Btn->SetOnClick([this]() { OnClickJoinBtn(); });
	}

	if (exit_Btn != nullptr)
	{
		exit_Btn->SetOnClick([this]() { OnClickExitBtn(); });
	}
}

void TitleSceneController::OnClickHostBtn()
{
	std::cout << "[TitleSceneController] HOST 버튼 클릭됨!" << std::endl;
	if (NetworkManager::GetInstance()->StartHost(9000))
	{
		SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
	}
}

void TitleSceneController::OnClickJoinBtn()
{
	std::cout << "[TitleSceneController] JOIN 버튼 클릭됨!" << std::endl;
	if (NetworkManager::GetInstance()->ConnectToHost("127.0.0.1", 9000))
	{
		SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
	}
}

void TitleSceneController::OnClickExitBtn()
{
	std::cout << "[TitleSceneController] EXIT 버튼 클릭됨!" << std::endl;
	PostQuitMessage(0);
}

void TitleSceneController::Serialize(json& outJson) const
{
	ScriptComponent::Serialize(outJson);

	if (host_Btn != nullptr)
	{
		outJson["HostBtnID"] = host_Btn->gameObject.GetInstanceID();
	}

	if (join_Btn != nullptr)
	{
		outJson["JoinBtnID"] = join_Btn->gameObject.GetInstanceID();
	}

	if (exit_Btn != nullptr)
	{
		outJson["ExitBtnID"] = exit_Btn->gameObject.GetInstanceID();
	}
}

void TitleSceneController::Deserialize(const json& inJson)
{
	ScriptComponent::Deserialize(inJson);

	if (inJson.contains("HostBtnID"))
	{
		m_hostBtnID = inJson["HostBtnID"].get<uint64>();
	}

	if (inJson.contains("JoinBtnID"))
	{
		m_joinBtnID = inJson["JoinBtnID"].get<uint64>();
	}

	if (inJson.contains("ExitBtnID"))
	{
		m_exitBtnID = inJson["ExitBtnID"].get<uint64>();
	}
}
