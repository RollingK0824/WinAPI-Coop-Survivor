#pragma once
#include <string_view>

namespace EngineKey
{
	namespace FilePath
	{
		constexpr std::string_view ResourceList = "Resources/Json/ResourceList.json";
		constexpr std::string_view CharacterTexture = "Resources/Texture/Character.json";
		constexpr std::string_view DefaultScene = "Resources/Scenes/Title.scene";
	}

	namespace Document
	{
		constexpr std::string_view SceneName = "SceneName";
		constexpr std::string_view GameObjects = "GameObjects";
		constexpr std::string_view Textures = "Textures";
	}

	namespace Scene
	{
		constexpr std::string_view Title = "TitleScene";
		constexpr std::string_view Loading = "LoadingScene";
		constexpr std::string_view Editor = "EditorScene";
	}

	namespace Component
	{
		constexpr std::string_view Trnasform = "TransformComponent";
		constexpr std::string_view Render = "RenderComponent";
		constexpr std::string_view SpriteRenderer = "SpriteRendererComponent";
		constexpr std::string_view Animator = "AnimatorComponent";
		constexpr std::string_view BoxCollider = "BoxColliderComponent";
		constexpr std::string_view CircleCollider = "CircleColliderComponent";
		constexpr std::string_view Script = "ScriptComponent";
		constexpr std::string_view Camera = "CameraComponent";
		constexpr std::string_view NetworkIdentity = "NetworkIdentity";
		constexpr std::string_view UIPanelComponent = "UIPanelComponent";
		constexpr std::string_view UITextComponent = "UITextComponent";
		constexpr std::string_view UIImageComponent = "UIImageComponent";
		constexpr std::string_view UIButtonComponent = "UIButtonComponent";
		constexpr std::string_view HUDPresenter = "HUDPresenter";
		constexpr std::string_view DebugHUDComponent = "DebugHUDComponent";
	}

	namespace Property
	{
		constexpr std::string_view Name = "Name";
		constexpr std::string_view IsActive = "IsActive";
		constexpr std::string_view IsEnabled = "IsEnabled";
		constexpr std::string_view Components = "Components";
		constexpr std::string_view Type = "Type";
		constexpr std::string_view Data = "Data";

		constexpr std::string_view Position = "Position";
		constexpr std::string_view Rotation = "Rotation";
		constexpr std::string_view Scale = "Scale";

		constexpr std::string_view RenderType = "RenderType";
		constexpr std::string_view TextureKey = "TextureKey";
		constexpr std::string_view SrcRect = "SrcRect";
		constexpr std::string_view ZOrder = "ZOrder";

		constexpr std::string_view Density = "Density";
		constexpr std::string_view Friction = "Friction";
		constexpr std::string_view Restitution = "Restitution";
		constexpr std::string_view IsSensor = "IsSensor";
		constexpr std::string_view BodyType = "BodyType";
	}

	namespace CustomComponent
	{
		constexpr std::string_view Background = "Background";
		constexpr std::string_view Player = "Player";
		constexpr std::string_view Enemy = "Enemy";
		constexpr std::string_view Bullet = "Bullet";
		constexpr std::string_view Effect = "Effect";
		constexpr std::string_view TitleSceneController = "TitleSceneController";
		constexpr std::string_view NetworkController = "NetworkController";
		constexpr std::string_view LocalController = "LocalController";
		constexpr std::string_view Controller = "Controller";
		constexpr std::string_view InGameManager = "InGameManager";
	}
}

#define INTRO_SCENE "IntroScene"
#define MOVE_LEFT "MoveLeft"
#define MOVE_RIGHT "MoveRight"

struct EngineTime
{
	float deltaTime = 0.0f;
	float fixedDeltaTime = 0.0f;
	float accumulator = 0.0f;
};

enum class DisplayMode
{
	Windowed,
	Borderless,
	Fullscreen
};

namespace PhysicsLayer
{
	enum Category : uint32
	{
		None       = 0,
		Default    = 1 << 0, // 0x0001
		Player     = 1 << 1, // 0x0002
		Monster    = 1 << 2, // 0x0004
		Projectile = 1 << 3, // 0x0008
		Wall       = 1 << 4, // 0x0010
		Item       = 1 << 5, // 0x0020

		All        = 0xFFFFFFFF
	};
}