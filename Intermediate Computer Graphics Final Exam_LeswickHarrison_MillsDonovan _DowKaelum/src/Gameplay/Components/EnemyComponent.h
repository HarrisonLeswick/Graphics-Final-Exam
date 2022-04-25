#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class EnemyComponent : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<EnemyComponent> Sptr;

	float walkcounter;
	bool forward;
	EnemyComponent() = default;
	
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static EnemyComponent::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(EnemyComponent);
};

