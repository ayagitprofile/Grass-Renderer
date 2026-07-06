#include "ComputeShader.h"
#include <iostream>
#include <glad/glad.h>


void Grass::ComputeShader::BindTextures() {
	if (_computeDataTexture == nullptr)
		return;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _computeDataTexture->GetID());
}

Grass::ComputeShader::ComputeShader() : Shader() {
}

void Grass::ComputeShader::DispatchSync(u32 numOfGroups) {
	if (numOfGroups == 0) {
		std::cerr << "[Compute Shader] Error number of groups cant be 0\n";
		return;
	}
	this->Bind();
	BindTextures();
	glDispatchCompute(numOfGroups, 1, 1);

	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
	glDeleteSync(sync);
}

void Grass::ComputeShader::DispatchSync(u32 numOfGroupsX, u32 numOfGroupsY) {
	this->Bind();
	BindTextures();
	glDispatchCompute(numOfGroupsX, numOfGroupsY, 1);

	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
	glDeleteSync(sync);
}

void Grass::ComputeShader::SetDataTexture(const std::string& name, Ref<Texture> texture) {
	if (texture == nullptr)
		return;

	_computeDataTexture = texture;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _computeDataTexture->GetID());
	glUniform1i(glGetUniformLocation(_id, name.c_str()), 0);
}
