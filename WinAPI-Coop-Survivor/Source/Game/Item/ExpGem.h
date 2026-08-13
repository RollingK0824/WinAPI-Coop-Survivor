#pragma once
#include "Engine/Core/Types.h"
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class GameObject;

// 몬스터 처치 시 드롭되는 경험치 보석 컴포넌트 (Player 흡수 전용 수동 피전달체)
class ExpGem : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(ExpGem)

	ExpGem(GameObject* owner, TransformComponent* transform);
	virtual ~ExpGem() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::ExpGem;
	}

	virtual void Start() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void Update(float dt) override;

	void Init(int32 expAmount);

	int32 GetExpAmount() const { return m_expAmount; }
	bool HasTargetPlayer() const { return m_targetPlayer.IsValid(); }
	void SetTargetPlayer(GameObject* pPlayerObj) { m_targetPlayer = pPlayerObj; }
	void Despawn();

private:
	int32 m_expAmount = 10;
	float m_flySpeed = 650.0f; // 플레이어 방향으로 흡수 날아가는 속도

	ObserverPtr<GameObject> m_targetPlayer;
};
