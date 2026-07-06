#include "App.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "glfwNoOpengl.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Mesh.h"
#include "Shader.h"
#include "CameraController.h"
#include "Input.h"
#include "Material.h"
#include "Renderer.h"
#include "Model.h"
#include "Const.h"
#include "LightingSystem.h"
#include "GrassRenderer.h"
#include "UI.h"
#include "Terrain.h"

using namespace Grass;

const u32 SCR_WIDTH = 1280;
const u32 SCR_HEIGHT = 720;

void APIENTRY glDebugOutput(
	GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam);

static void GLFWErrorCallback(int error, const char* description) {
	//fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	std::cout << "[GLFW] Error: " << description << '\n';
}

static void ImguiInit(const Window& window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window.GetResourceHandle(), true);
	ImGui_ImplOpenGL3_Init(Const::OpenGLVersionDirective);
}

static void ImguiShutdown() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

static void ImguiBeginFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

static void ImguiEndFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Grass::App::App() 
	:
	_uniformBuffer(),
	_initializedSuccessfully(false), 
	_camera(std::make_shared<Camera>(70.f, (float)SCR_WIDTH / (float)SCR_HEIGHT))
{
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);


	_window = Window(SCR_WIDTH, SCR_HEIGHT);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	if (_window.IsOpen() == false)
		return;

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes callback run on same thread
	glEnable(GL_MULTISAMPLE);

	glDebugMessageCallback(glDebugOutput, nullptr);

	glfwSetErrorCallback(GLFWErrorCallback);

	ImguiInit(_window);
	
	_initializedSuccessfully = true;
}

int Grass::App::Run() {
	if (_initializedSuccessfully == false)
		return -1;

	_window.SetTitle("Grass Renderer  /  Press Tab to toggle UI");

	_window.SetResizeHandler([this](int width, int height){
		this->_camera->SetAspectRatio((float)width / (float)height);
	});

	_camera->SetClipRange(0.1, 250);

	Input input(_window);
	Renderer renderer;

	auto cubeMesh = std::make_shared<Mesh>(Path(Const::MeshesPath) / "cube.obj", MeshCreationFlags::CalculateTangents);
	
	//auto containerTexture = std::make_shared<Texture>(Path(Const::TexturesPath) / "container.jpg");
	auto brickWallTexture = std::make_shared<Texture>(Path(Const::TexturesPath) / "brickwall/color.jpg");
	auto brickWallNormalTexture = std::make_shared<Texture>(Path(Const::TexturesPath) / "brickwall/normal.jpg");
	
	Model cube(cubeMesh, std::make_shared<Material>(Path(Const::ShadersPath) / "Shader2.glsl"));
	cube.Transform.SetPosition(glm::vec3(0, 10, -3));
	cube.Material->SetTexture("u_colorTexture", brickWallTexture);
	cube.Material->SetTexture("u_normalMap", brickWallNormalTexture);

	Model sphere(std::make_shared<Mesh>(Path(Const::MeshesPath) / "sphere.obj"), std::make_shared<Material>(Path(Const::ShadersPath) / "ReflectiveSphere.glsl"));
	sphere.Transform.SetPosition(2, 10, 0);
	sphere.Material->SetFloat("u_specPow", 128);
	sphere.Material->SetFloat("u_specStr", 0.5);

	CameraController camController(*_camera, input);
	Terrain terrain( { 400, 400 });

	glm::vec2 terrainHalfArea = terrain.GetArea() * 0.5f;
	glm::vec3 offset = glm::vec3(terrainHalfArea.x, 0, terrainHalfArea.y);
	camController.SetConstrainingBounds(terrain.GetCenter() - offset - glm::vec3(0, 1, 0), terrain.GetCenter() + offset + glm::vec3(0, 100, 0));

	GrassRenderer grassRenderer;
	grassRenderer.GenerateData(terrain.GetTerrainHeightTexture(), terrain.GetArea(), 10);
	//grassRenderer.GenerateData(terrain.GetTerrainHeightTexture());

	LightingSystem lighting;
	lighting.SetSkyboxMesh(cubeMesh);
	lighting.SetSkyboxMaterial(std::make_shared<Material>(Path(Const::ShadersPath) / "SkyboxShader.glsl"));
	lighting.AmbientLight = glm::vec3(0.5);
	lighting.MainLight.Direction = glm::vec3(0, 0, -1) * glm::quat(glm::radians(glm::vec3(36, 45, 0)));
	lighting.MainLight.DiffuseColor = glm::vec3(1.f, 0.996f, 0.827f);
	lighting.SetFogRange(30, 120);
	grassRenderer.SetRenderingDistance(lighting.GetFogRange().y + grassRenderer.GetChunkSize());
	_camera->SetClipRange(_camera->GetClippingRange().x, grassRenderer.ChunkedDimensions().x * grassRenderer.GetChunkSize() * 1.5);
	grassRenderer.SetWindSpeed(1.5f);
	grassRenderer.SetWindStrength(0.75f);

	auto& imguiIO = ImGui::GetIO();
	UI uiSystem(imguiIO, lighting, grassRenderer, *this, *_camera, camController);

	float timeLastFrame = glfwGetTime();

	_camera->Transform.SetPosition(glm::vec3(0, 10, 10));

	glfwSwapInterval(0);

	while (_window.IsOpen()) {
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - timeLastFrame;
		timeLastFrame = currentTime;

		input.Update(_frameCount);

		glfwPollEvents();
		
		grassRenderer.Update(deltaTime, *_camera);
		lighting.UploadData(deltaTime);
		camController.Update(deltaTime);
		uiSystem.Update(input, deltaTime);
		

		if (input.GetKey(KeyCode::ESCAPE)) {
			_window.Close();
			continue;
		}

		cube.Transform.SetEulerAngles(15, currentTime * 20, 0);

		_window.Clear();

		renderer.RenderMesh(*sphere.Mesh, *sphere.Material, sphere.Transform.GetModelMatrix());
		renderer.RenderMesh(*cube.Mesh, *cube.Material, cube.Transform.GetModelMatrix());
		if (RenderGrass)
			grassRenderer.Render(renderer);
		terrain.Render(renderer);

		lighting.RenderSkybox(renderer);
		renderer.RenderTransparentScene(*_camera);

		ImguiBeginFrame();

		uiSystem.Render();

		ImguiEndFrame();

		_window.SwapBuffers();

		++_frameCount;
	}

	return 0;
}

Grass::App::~App() {
	ImguiShutdown();
	glfwTerminate();
}

static void APIENTRY glDebugOutput(
	GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam) {

#if _DEBUG
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;
#else
	if (severity != GL_DEBUG_SEVERITY_HIGH)
		return;
#endif

	std::cerr << "GL DEBUG MESSAGE:\n";
	//std::cerr << "  ID: " << id << "\n";
	std::cerr << "Message: " << message << "\n";

#if 0
	std::cerr << "  Source: ";
	switch (source) {
	case GL_DEBUG_SOURCE_API:             std::cerr << "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Application"; break;
	default: std::cerr << "Other";
	}

	std::cerr << "\n  Type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               std::cerr << "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Performance"; break;
	default: std::cerr << "Other";
	}

	std::cerr << "\n  Severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Notification"; break;
	}
#endif

	//std::cerr << "\n\n";
}

