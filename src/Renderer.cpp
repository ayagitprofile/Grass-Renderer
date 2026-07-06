#include "Renderer.h"

#include <glad/glad.h>

static void SetCullingMode(Grass::FaceCullingMode mode) {
	switch (mode) {
	case Grass::FaceCullingMode::Back:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;

	case Grass::FaceCullingMode::Front:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;

	case Grass::FaceCullingMode::Both:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
		break;

	case Grass::FaceCullingMode::Disabled:
	default:
		glDisable(GL_CULL_FACE);
		break;
	}
}

static void SetDepthWritingMode(bool value) {
	glDepthMask(value ? GL_TRUE : GL_FALSE);
}

static void SetDepthTestingMode(bool value) {
	if (value)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

static void SetDepthTestFunction(Grass::DepthTestFunction depthTestFunction) {
	glDepthFunc((Grass::u32)depthTestFunction);
}

Grass::Renderer::Renderer() {
	glEnable(GL_DEPTH_TEST);
}

static glm::mat3 CreateNormalMatrix(const glm::mat4& modelMatrix) {
	//return glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
	return glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
}

void Grass::Renderer::RenderMesh(const Mesh& mesh, Material& material, const glm::mat4& modelMatrix) {
	SetCullingMode(material.FaceCullingMode);
	SetDepthTestFunction(material.DepthTestFunction);
	SetDepthWritingMode(material.DepthWriting);
	SetDepthTestingMode(material.DepthTesting);

	u32 shaderid = material.GetShader().GetID();
	material.Bind();
	mesh.Bind();
	
	material.SetMat4("u_model", modelMatrix);

	//i32 normalMatrixID = material.GetUniformID("u_normalMatrix");
	//if (normalMatrixID >= 0) {
	//	material.SetMat3(normalMatrixID, CreateNormalMatrix(modelMatrix));
	//}
	//
	glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), (Grass::u32)mesh.GetIndexFormat(), 0);
}

void Grass::Renderer::RenderMeshInstanced(const Mesh& mesh, Material& material, i32 instanceCount) {
	SetCullingMode(material.FaceCullingMode);
	SetDepthTestFunction(material.DepthTestFunction);
	SetDepthWritingMode(material.DepthWriting);
	SetDepthTestingMode(material.DepthTesting);

	material.Bind();
	mesh.Bind();

	glDrawElementsInstanced(GL_TRIANGLES, mesh.GetIndexCount(), (u32)mesh.GetIndexFormat(), 0, instanceCount);
}

void Grass::Renderer::RenderMeshIndirect(const Mesh& mesh, Material& material, IndirectDrawBuffer& indirectDrawBuffer) {
	SetCullingMode(material.FaceCullingMode);
	SetDepthTestFunction(material.DepthTestFunction);
	SetDepthWritingMode(material.DepthWriting);
	SetDepthTestingMode(material.DepthTesting);

	material.Bind();
	mesh.Bind();
	glDrawElementsIndirect(GL_TRIANGLES, (u32)mesh.GetIndexFormat(), nullptr);
}

void Grass::Renderer::RenderTransparentScene(const Camera& camera) {
	glm::vec3 cameraPosition = camera.Transform.GetPosition();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	std::sort(_transparentDrawCallData.begin(), _transparentDrawCallData.end(),
		[&](const DrawCallData& a, const DrawCallData& b) {
			glm::vec3 apos = glm::vec3((*a.ModelMatrix)[3]);
			glm::vec3 bpos = glm::vec3((*b.ModelMatrix)[3]);

			return glm::distance(cameraPosition, apos) > glm::distance(cameraPosition, bpos);
		});

	for (const auto& drawData : _transparentDrawCallData) {
		SetCullingMode(drawData.Material->FaceCullingMode);
		SetDepthWritingMode(drawData.Material->DepthWriting);
		//SetDepthTestFunction(drawData.Material->DepthTestFunction);
		drawData.Material->Bind();
		drawData.Mesh->Bind();
		drawData.Material->SetMat4("u_model", *drawData.ModelMatrix);

		glDrawElements(GL_TRIANGLES, drawData.Mesh->GetIndexCount(), (Grass::u32)drawData.Mesh->GetIndexFormat(), 0);
	}
	_transparentDrawCallData.clear();

	glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);
}

void Grass::Renderer::RenderTransparentMesh(const Mesh* mesh, Material* material, glm::mat4* model) {
	_transparentDrawCallData.push_back({ mesh, material, model });
}

