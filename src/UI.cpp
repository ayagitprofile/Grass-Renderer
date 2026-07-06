#include "UI.h"
#include <algorithm>

using namespace Grass;

static std::string AddSeparators(u32 value, char sep) {
	std::string s = std::to_string(value);

	bool negative = false;
	if (!s.empty() && s[0] == '-') {
		negative = true;
		s.erase(s.begin());
	}

	std::string result;
	int count = 0;

	// build from the end
	for (auto it = s.rbegin(); it != s.rend(); ++it) {
		if (count == 3) {
			result.push_back(sep);
			count = 0;
		}
		result.push_back(*it);
		count++;
	}

	if (negative) result.push_back('-');

	std::reverse(result.begin(), result.end());
	return result;
}

Grass::UI::UI(ImGuiIO& io, LightingSystem& ls, GrassRenderer& gr, App& app, Camera& camera, CameraController& cc)
	: _imGuiIO(io), _lightingSystem(ls), _grassRenderer(gr), _app(app), _camera(camera), _camCon(cc)
{
	_sunColor = _lightingSystem.MainLight.DiffuseColor;
	_ambientLightColor = _lightingSystem.AmbientLight;
	_fogRange = _lightingSystem.GetFogRange();
	_fov = _camera.GetFov();

}

void Grass::UI::Update(Input& input, float deltaTime) {
	if (input.GetKeyDown(KeyCode::TAB)) {
		_isVisible = !_isVisible;
		_camCon.ControlCameraRotation = !_isVisible;

		if (input.IsCursorEnabled()) {
			input.DisableCursor();
		}
		else {
			input.EnableCursor();
		}
	}

	if (_isVisible ) {
		if (input.GetMouseKey(1)) {
			const float scale = 0.1;
			glm::vec2 delta = input.MouseDelta() * scale;

			_camCon.AddYaw(delta.x);
			_camCon.AddPitch(delta.y);
		}
		
		if (glm::abs(input.ScrollDelta()) > 0.1) {
			_fov -= input.ScrollDelta() * 3;
			_camera.SetFov(_fov);
		}
	}

	_deltaTime = deltaTime;
	_time += deltaTime;

	if (_rotateSun) {
		_lightingSystem.MainLight.Direction = glm::normalize(glm::vec3(glm::sin(_time * 0.1f), -0.5, glm::cos(_time * 0.1f)));
	}
	else {
		float yaw = glm::radians(_sunEulerAngles.y * 360.f);
		float pitch = glm::radians(_sunEulerAngles.x * 360.f / 2 + 180);

		glm::vec3 direction;
		direction.x = cos(pitch) * sin(yaw);
		direction.y = sin(pitch);
		direction.z = cos(pitch) * cos(yaw);

		_lightingSystem.MainLight.Direction = glm::normalize(direction);
	}
}

void Grass::UI::Render() {
	static std::string frameTimeText = "Frame time: 0ms / FPS: 0";
	static std::string cullingTime = "";
	static std::string framesPerRangeString = "";
	static u32 visibleChunks = 0;
	static std::string cullRatio = "";

	static float _timeSinceLastFPSUpdate = 0.0f;
	static float _timeSinceLastFPSUpdate2 = 0.0f;

	_timeSinceLastFPSUpdate += _deltaTime;
	_timeSinceLastFPSUpdate2 += _deltaTime;

	float frameTimeMS = 1000.f * _deltaTime;

	if (_timeSinceLastFPSUpdate > 0.1f) {
		_timeSinceLastFPSUpdate = 0;
		frameTimeText = "Frame time: " + std::format("{:.3f}", frameTimeMS) + "ms / FPS: " + std::to_string((u32)(1.f / _deltaTime));
		cullingTime = std::format("GPU Grass culling took: {:.3f}ms", _grassRenderer.GetCullingTime());

	}
	if (_timeSinceLastFPSUpdate2 > 0.2f) {
		_timeSinceLastFPSUpdate2 = 0;
		cullRatio = std::format("GPU Culling To Frame Time Ratio: {:.2f}", glm::min(_grassRenderer.GetCullingTime() / frameTimeMS, 1.0f));
		
		if (_isVisible)
			visibleChunks = _grassRenderer.FetchVisibleChunks();
	}

	if (!_isVisible)
		return;

	ImGui::SetNextWindowPos({}, ImGuiCond_Appearing);
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::Text("");
		ImGui::Text("Lighting");

		if (ImGui::ColorEdit3("Sun Light Color", &_sunColor[0], ImGuiColorEditFlags_Float)) {
			_lightingSystem.MainLight.DiffuseColor = _sunColor * _sunLightStrength;
		}
		if (ImGui::SliderFloat("Sun Light Strength", &_sunLightStrength, 0, 5)) {
			_lightingSystem.MainLight.DiffuseColor = _sunColor * _sunLightStrength;
		}
		static float ambient = _lightingSystem.AmbientLight.x;
		if (ImGui::SliderFloat("Ambient Light Strength", &ambient, 0, 1)) {
			_lightingSystem.AmbientLight = glm::vec3(1, 1, 1) * ambient;
		}
		if (ImGui::SliderFloat("Fog Start", &_fogRange.x, 0.1, 100)) {
			_lightingSystem.SetFogRange(_fogRange.x, _fogRange.y);
		}
		if (ImGui::SliderFloat("Fog End", &(_fogRange.y), 0.1, _grassRenderer.GetChunkSize() * _grassRenderer.ChunkedDimensions().x * 1.5f)) {
			_lightingSystem.SetFogRange(_fogRange.x, _fogRange.y);
		}

		ImGui::Checkbox("Rotate Sun", &_rotateSun);
		if (!_rotateSun) {
			ImGui::SliderFloat2("Sun Angles", &_sunEulerAngles[0], 0, 1);
		}

		ImGui::Text("");
		ImGui::Text("Wind");

		static float WindStr = _grassRenderer.GetWindStrength();

		if (ImGui::SliderFloat("Wind Strength", &WindStr, 0, 3))
			_grassRenderer.SetWindStrength(WindStr);
		if (ImGui::SliderAngle("Wind Angle", &_windAngle))
			_grassRenderer.SetWindDirection(glm::vec3(0, 0, -1) * glm::quat(glm::vec3(0, _windAngle, 0)));
		if (ImGui::SliderFloat("Wind Speed", &_windSpeed, 0, 10))
			_grassRenderer.SetWindSpeed(_windSpeed);

		{
			ImGui::Text("");
			ImGui::Text("Rendering");

			ImGui::Text(frameTimeText.c_str());
			ImGui::Text(cullingTime.c_str());
			ImGui::Text(cullRatio.c_str());
			ImGui::Checkbox("Render Grass", &_app.RenderGrass);
			ImGui::Checkbox("Ebable Culling", &_grassRenderer.EnableChunkCulling);
			ImGui::Text(std::format("Visible Chunks: {}", visibleChunks * _app.RenderGrass).c_str());

			static float grassRenderingDistance = _grassRenderer.GetRenderingDistance();
			if (ImGui::SliderFloat("Grass Rendering Distance", &grassRenderingDistance, 0, 500)) {
				_grassRenderer.SetRenderingDistance(grassRenderingDistance);
			}

			u32 insanceCount = _grassRenderer.CalculateInstanceCount(visibleChunks) * _app.RenderGrass;
			static std::string s = "";
			s = AddSeparators(insanceCount, '.');
			ImGui::Text(std::format("Grass Blades Drawn: {}", s).c_str());
			ImGui::Text(std::format("Instance Data Buffer Size: {:.1f}MB", (float)_grassRenderer.GetInstanceBufferSize() / 1000000.0f).c_str());
		}

		{
			ImGui::Spacing();
			ImGui::Text("Camera");
			
			auto pos = _camera.Transform.GetPosition();
			ImGui::DragFloat3("Camera Position", &pos.x);

			if (ImGui::SliderFloat("Camera Fov", &_fov, 10, 150)) {
				_camera.SetFov(_fov);
			}
		}

	}
	ImGui::End();

}
