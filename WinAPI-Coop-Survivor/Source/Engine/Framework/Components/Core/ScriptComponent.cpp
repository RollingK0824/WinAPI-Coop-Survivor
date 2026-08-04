#include "Engine/Core/pch.h"
#include "ScriptComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

ScriptComponent::ScriptComponent(GameObject* owner, TransformComponent* transform) : Component(owner, transform) {}
