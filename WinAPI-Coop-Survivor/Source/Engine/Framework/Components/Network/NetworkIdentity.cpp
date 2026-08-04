#include "Engine/Core/pch.h"
#include "NetworkIdentity.h"
#include "Engine/Core/ComponentRegister.h"

static ComponentRegistrar<NetworkIdentity>registrar(EngineKey::Component::NetworkIdentity.data());

NetworkIdentity::NetworkIdentity(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner,transform)
{
}
