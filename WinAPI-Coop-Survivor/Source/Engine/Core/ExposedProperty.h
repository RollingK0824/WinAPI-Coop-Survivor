#pragma once
#include <string>
#include <vector>
#include <functional>

class GameObject;

enum class PropType
{
	Int,
	Float,
	Bool,
	String,
	WString,
	Vector2,
	Point2F,
	Color,
	Rect,
	StringVector,
	Texture,
	ObjectRef,
	Asset
};

struct ExposedProperty
{
	std::string name;
	PropType type;
	void* data = nullptr;
	std::function<uint64(void*)> getInstanceID = nullptr;
	std::function<void(GameObject*, void*)> resolver = nullptr;
	std::function<GameObject* (void*)> getTargetGameObject = nullptr;
};
