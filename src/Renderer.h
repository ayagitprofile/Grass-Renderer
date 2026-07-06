#pragma once

#include "Material.h"
#include "Mesh.h"
#include "Window.h"
#include "Camera.h"
#include "IndirectDrawBuffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Grass {
class Renderer
{
private:
	struct DrawCallData {
		const Mesh* Mesh;
		Material* Material;
		glm::mat4* ModelMatrix;
	};

	std::vector<DrawCallData> _transparentDrawCallData;

public:
	Renderer();
	void RenderMesh(const Mesh& mesh, Material& material, const glm::mat4& modelMatrix);
	void RenderMeshInstanced(const Mesh& mesh, Material& material, i32 instanceCount);
	void RenderMeshIndirect(const Mesh& mesh, Material& material, IndirectDrawBuffer& indirectDrawBuffer);

	void RenderTransparentMesh(const Mesh* mesh, Material* material, glm::mat4* modelMatrix);

	void RenderTransparentScene(const Camera& camera);
};
}