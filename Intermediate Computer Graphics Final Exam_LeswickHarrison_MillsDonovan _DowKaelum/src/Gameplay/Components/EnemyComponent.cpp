#include "Gameplay/Components/EnemyComponent.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void EnemyComponent::Awake() {
	walkcounter = 0;
	forward = 0;
}

void EnemyComponent::Update(float deltaTime) {
	if (walkcounter == 200) {
		if (forward == 0) {
			forward = 1;
			walkcounter = 0;
			GetGameObject()->SetPostion(glm::vec3(10, 0, 3.2));
		}
		else if (forward == 1) {
			forward = 0;
			walkcounter = 0;
			GetGameObject()->SetPostion(glm::vec3(20, 0, 3.2));
		}
	}
	if (forward == 0) {
		GetGameObject()->SetRotation(glm::vec3(90.f, 0.f, -90.f));
		GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x - 0.05, 0, 3.2));
		walkcounter++;
	}
	if (forward == 1) {
		GetGameObject()->SetRotation(glm::vec3(90.f, 0.f, 90.f));
		GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x + 0.05, 0, 3.2));
		walkcounter++;
	}
}

void EnemyComponent::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Walk Counter", &walkcounter);
}

nlohmann::json EnemyComponent::ToJson() const {
	return {
		{ "walk_counter", walkcounter }
	};
}

EnemyComponent::Sptr EnemyComponent::FromJson(const nlohmann::json& data) {
	EnemyComponent::Sptr result = std::make_shared<EnemyComponent>();
	
	return result;
}
