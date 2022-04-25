#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class CharacterMovement : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<CharacterMovement> Sptr;

	CharacterMovement() = default;
	
	float xVelocity;
	float jumpVelocity = 0;
	float zVelocity = 0;
	int lives;

	bool canJump;
	bool win;
	bool lose;
	bool xpos;//s
	bool xneg;//w


	float _velMultiplier;
	Gameplay::Physics::RigidBody::Sptr _body;

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;
	virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static CharacterMovement::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(CharacterMovement);

protected:
	bool _playerDamage = false;
};

