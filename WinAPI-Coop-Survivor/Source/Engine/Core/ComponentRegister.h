#pragma once
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Framework/GameObject.h"

template<typename T>
class ComponentRegistrar 
{
public:
    ComponentRegistrar(const std::string& name) {
        JsonSerializer::RegisterComponentFactory(name, [](GameObject* owner) -> Component* {
            return owner->AddComponent<T>();
            });
    }
};