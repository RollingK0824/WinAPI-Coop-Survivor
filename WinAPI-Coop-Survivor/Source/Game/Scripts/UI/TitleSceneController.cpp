#include "Engine/Core/pch.h"
#include "TitleSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"

static ComponentRegistrar<TitleSceneController> registrar(EngineKey::CustomComponent::TitleSceneController.data());

TitleSceneController::TitleSceneController(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
}

void TitleSceneController::Start()
{
	
}
