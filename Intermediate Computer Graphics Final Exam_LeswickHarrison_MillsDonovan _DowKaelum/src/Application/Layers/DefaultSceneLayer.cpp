#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/Textures/Texture2DArray.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/Light.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Application/Layers/ImGuiDebugLayer.h"
#include "Application/Windows/DebugWindow.h"
#include "Gameplay/Components/ShadowCamera.h"
#include "Gameplay/Components/ShipMoveBehaviour.h"
#include "Gameplay/Components/EnemyComponent.h"
#include "Gameplay/Components/CharacterMovement.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		 
		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");  

		// Our foliage shader which manipulates the vertices of the mesh
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});  
		foliageShader->SetDebugName("Foliage");   

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },  
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing"); 

		// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");


		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr shipMesh   = ResourceManager::CreateAsset<MeshResource>("fenrir.obj");
		MeshResource::Sptr groundMesh = ResourceManager::CreateAsset<MeshResource>("ground.obj");
		MeshResource::Sptr knightMesh = ResourceManager::CreateAsset<MeshResource>("Knight.obj");
		MeshResource::Sptr linkMesh = ResourceManager::CreateAsset<MeshResource>("link.obj");
		MeshResource::Sptr flagMesh = ResourceManager::CreateAsset<MeshResource>("FLag.obj");



		Texture2D::Sptr    backgroundTex = ResourceManager::CreateAsset<Texture2D>("textures/background.png");
		Texture2D::Sptr    knightTex = ResourceManager::CreateAsset<Texture2D>("textures/knight.png");
		Texture2D::Sptr    linkTex = ResourceManager::CreateAsset<Texture2D>("textures/link.png");
		Texture2D::Sptr    groundTex = ResourceManager::CreateAsset<Texture2D>("textures/ground.png");
		Texture2D::Sptr    flagTex = ResourceManager::CreateAsset<Texture2D>("textures/flag.png");

		// Load some images for drag n' drop
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight.png");
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight-2.png");
		ResourceManager::CreateAsset<Texture2D>("textures/light_projection.png");

		Texture2DArray::Sptr particleTex = ResourceManager::CreateAsset<Texture2DArray>("textures/particles.png", 2, 2);

		//DebugWindow::Sptr debugWindow = app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>();

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidGrey[3] = { 0.5f, 0.5f, 0.5f };
		float solidBlack[3] = { 0.0f, 0.0f, 0.0f };
		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };

		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		});
		  
		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>();  

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");   

		// Configure the color correction LUT
		scene->SetColorLUT(lut);

	//This is our background texture and it is moderatly shiny
		Material::Sptr backgroundMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			backgroundMaterial->Name = "Box";
			backgroundMaterial->Set("u_Material.AlbedoMap", backgroundTex);
			backgroundMaterial->Set("u_Material.Shininess", 0.5f);
			backgroundMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		//this is our texture for link which is just a green square and it is very dull as it is supposed to be cloth
		Material::Sptr linkMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			linkMaterial->Name = "Box";
			linkMaterial->Set("u_Material.AlbedoMap", linkTex);
			linkMaterial->Set("u_Material.Shininess", 0.1f);
			linkMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		//this is our material for the knight which is very shiny because it is supposed to be metal
		Material::Sptr knightMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			knightMaterial->Name = "Box";
			knightMaterial->Set("u_Material.AlbedoMap", knightTex);
			knightMaterial->Set("u_Material.Shininess", 0.9f);
			knightMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr groundMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			groundMaterial->Name = "Box";
			groundMaterial->Set("u_Material.AlbedoMap", groundTex);
			groundMaterial->Set("u_Material.Shininess", 0.5f);
			groundMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr flagMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			flagMaterial->Name = "Box";
			flagMaterial->Set("u_Material.AlbedoMap", flagTex);
			flagMaterial->Set("u_Material.Shininess", 0.1f);
			flagMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

		for (int ix = 0; ix < 50; ix++) {
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPostion(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();



		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ 9, -9, 9 });
			camera->SetRotation(glm::vec3(70, 0, 0));
		}


		// Set up all our sample objects
		GameObject::Sptr background = scene->CreateGameObject("Background");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(1.0f)));
			tiledMesh->GenerateMesh();
			background->SetPostion(glm::vec3(11.17f, 12.0f, 7.13f));
			background->SetRotation(glm::vec3(78, 0, 0));
			background->SetScale(glm::vec3(0.9f, 0.8f, 1.0f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = background->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(backgroundMaterial);
		}



		GameObject::Sptr link = scene->CreateGameObject("Link");
		{
			// Set position in the scene
			link->SetPostion(glm::vec3(-4.0f, 0.0f, 1.0f));

			// Add some behaviour that relies on the physics body
			link->Add<JumpBehaviour>();
			link->Add<CharacterMovement>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = link->Add<RenderComponent>();
			renderer->SetMesh(linkMesh);
			renderer->SetMaterial(linkMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = link->Add<TriggerVolume>();
			SphereCollider::Sptr collider = SphereCollider::Create(1.8);
			//collider->SetPosition(glm::vec3(3.5f, 3.5f, 0.f));
			trigger->AddCollider(collider);

		}

		GameObject::Sptr knight = scene->CreateGameObject("Knight");
		{

			RigidBody::Sptr physics = knight->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(SphereCollider::Create(1.2));
			physics->SetMass(0.f);

			// Set position in the scene
			knight->SetPostion(glm::vec3(20.0f, 0.0f, 3.2f));
			knight->SetRotation(glm::vec3(0.0, 0, -113.0));

			// Add some behaviour that relies on the physics body
			knight->Add<EnemyComponent>();


			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = knight->Add<RenderComponent>();
			renderer->SetMesh(knightMesh);
			renderer->SetMaterial(knightMaterial);
		}

		GameObject::Sptr ground = scene->CreateGameObject("Ground");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(1.0f)));
			tiledMesh->GenerateMesh();
			ground->SetPostion(glm::vec3(9.47f, 1.57f, -0.51f));
			ground->SetScale(glm::vec3(5.86, 2.23, 1.6));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = ground->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(groundMaterial);
		}

		GameObject::Sptr flag = scene->CreateGameObject("Flag");
		{
			flag->SetPostion(glm::vec3(28.2f, 0.0f, 1.0f));
			flag->SetRotation(glm::vec3(81.0f, 1.0f, 47.0f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = flag->Add<RenderComponent>();
			renderer->SetMesh(flagMesh);
			renderer->SetMaterial(flagMaterial);
		}

		GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion({ 9, -15.5, 9 });
			shadowCaster->SetRotation(glm::vec3(65.05f, 0.0f, -2.546f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.01f, 100.0f));
		}



		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
