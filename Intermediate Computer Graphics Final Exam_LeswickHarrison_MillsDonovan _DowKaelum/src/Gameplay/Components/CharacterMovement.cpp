#include "Gameplay/Components/CharacterMovement.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/InputEngine.h"

void CharacterMovement::Awake() {
	_body = GetGameObject()->Get<Gameplay::Physics::RigidBody>();
	_velMultiplier = 0.5;
	jumpVelocity = 0;
	canJump = 1;
	lose = 0;
	win = 0;
	lives = 3;
}

void CharacterMovement::Update(float deltaTime) {
	if (_playerDamage == true) {
		lives--;
		GetGameObject()->SetPostion(glm::vec3(-4.0f, 0.0f, 1.0f));
		std::cout << "You lost a life." << std::endl;
	}
	while (lives == 0) {
		std::cout << "You lost." << std::endl;

	}

		if (InputEngine::GetKeyState(GLFW_KEY_D) == ButtonState::Down) {
			xpos = 1;
			GetGameObject()->SetRotation(glm::vec3(0.f, 0.f, 90.f));
		}
		else {
			xpos = 0;
		}
		if (InputEngine::GetKeyState(GLFW_KEY_A) == ButtonState::Down) {
			xneg = 1;
			GetGameObject()->SetRotation(glm::vec3(0.f, 0.f, -90.f));
		}
		else {
			xneg = 0;
		}
		if (xpos == 0 && xneg == 0) {
			GetGameObject()->SetRotation(glm::vec3(0.f, 0.f, 0.f));
		}
		if (InputEngine::GetKeyState(GLFW_KEY_SPACE) == ButtonState::Down || InputEngine::GetKeyState(GLFW_KEY_W) == ButtonState::Down) {
			if (canJump == 1) {
				jumpVelocity = 1;
				canJump = 0;
				std::cout << "jumpVelocity set";
			}
		}
		if (jumpVelocity > -1 && canJump == 0) {
			jumpVelocity = jumpVelocity - 0.06;
		}
		if (GetGameObject()->GetPosition().z <= 0.9 && canJump == 0) {
			jumpVelocity = 0;
			canJump = 1;
			std::cout << "Jump done" << canJump;
			GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, 0, 1));
		}
		xVelocity = (xpos * _velMultiplier) - (xneg * _velMultiplier);

	if(win == 0 && lose == 0){
		GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x + xVelocity, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z + jumpVelocity));

		if (GetGameObject()->GetPosition().x >= 27.0) {
			win = true;
		}
	}
	else if (win == 1) {
		GetGameObject()->SetPostion(glm::vec3(8.79, -6.6, 5.78));
	}

}

void CharacterMovement::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body)
{
	_playerDamage = true;
}

void CharacterMovement::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body)
{
	_playerDamage = false;
}

void CharacterMovement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Velocity", &_velMultiplier);
}

nlohmann::json CharacterMovement::ToJson() const {
	return {
			{ "velocity_multiplier", _velMultiplier }
	};
}

CharacterMovement::Sptr CharacterMovement::FromJson(const nlohmann::json& data) {
	CharacterMovement::Sptr result = std::make_shared<CharacterMovement>();
	
	return result;
}
