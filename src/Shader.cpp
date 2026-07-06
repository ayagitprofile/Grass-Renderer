#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Const.h"
#include "ScopedTimer.h"

namespace Grass {
	
static constexpr i32 VertexShaderID = GL_VERTEX_SHADER, FragmentShaderID = GL_FRAGMENT_SHADER;

u32 CompileSubShader(const char*, i32);
u32 CreateShaderFrom(const std::string&, const std::string&);
u32 CreateComputeShaderFrom(const std::string&);

std::string AddLineNumbers(const std::string& text) {
	std::istringstream input(text);
	std::ostringstream output;

	std::string line;
	int lineIndex = 0;

	while (std::getline(input, line)) {
		output << std::to_string(lineIndex++) << ": " << line;
		if (!input.eof())
			output << '\n';
	}

	return output.str();
}

static void TrimSpaces(std::string& s) {
	// Trim left
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		[](unsigned char ch) {
			return !std::isspace(ch);
		}));

	// Trim right
	s.erase(std::find_if(s.rbegin(), s.rend(),
		[](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
}

Shader::Shader() : _id(0) {
}

Shader::Shader(Shader&& other) noexcept : _id(other._id) {
	other._id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
	if (this == &other)
		return *this;

	glDeleteProgram(_id);
	_id = other._id;
	other._id = 0;

	return *this;
}

static std::string ExtractIncludePath(const std::string& line) {
	auto firstQuote = line.find('"');
	auto lastQuote = line.find_last_of('"');

	if (firstQuote == std::string::npos || lastQuote == std::string::npos || firstQuote == lastQuote)
		return "";

	return line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
}

Shader::Shader(const std::string& shaderSource, const Path& includePath) : _id(0) {
	ScopedTimer timer("[Shader] Unnamed shader compilation took: ");

	std::string vertexSource;
	std::string fragmentSource;
	std::string computeSource;

	std::string* targetString = nullptr;

	std::stringstream stream(shaderSource);

	std::string line;

	while (std::getline(stream, line)) {
		static const std::string IncludeDirective = "#include";

		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos || line.find("vert") != std::string::npos)
				targetString = &vertexSource;
			else if (line.find("fragment") != std::string::npos || line.find("frag") != std::string::npos)
				targetString = &fragmentSource;
			else if (line.find("compute") != std::string::npos)
				targetString = &computeSource;
			else
				std::cerr << "[Shader] This shader type is not supported: " << line << '\n';

			continue;
		}
		else if (line.find(IncludeDirective) != std::string::npos && targetString != nullptr) {
			std::filesystem::path ip = includePath;
			ip.remove_filename();
			ip.append(ExtractIncludePath(line));

			std::ifstream includeFile(ip);
			if (!includeFile) {
				std::cerr << "[Shader] Error: Failed to open included file: " << ip << std::endl;
				continue;
			}

			std::cout << "[Shader] Including file: " << ip << '\n';

			std::string includedLine;
			while (std::getline(includeFile, includedLine)) {
				*targetString += includedLine + '\n';
			}

			continue;
		}

		if (targetString == nullptr)
			continue;

		*targetString += line + '\n';
	}

	if (!computeSource.empty()) {
		computeSource.insert(0, "#version 460 core\n#define COMPUTE_SHADER\n");
		_id = CreateComputeShaderFrom(computeSource);
		if (_id == 0)
			std::cout << "[Shader] Compute source dump:\n" << AddLineNumbers(computeSource) << '\n';
	}
	else {
		vertexSource.insert(0, "#version 460 core\n#define VERTEX_SHADER\n");
		fragmentSource.insert(0, "#version 460 core\n#define FRAGMENT_SHADER\n");
		_id = CreateShaderFrom(vertexSource, fragmentSource);
		if (_id == 0)
			std::cout << "[Shader] Vertex source dump:\n" << AddLineNumbers(vertexSource) << "Fragment source dump:\n" << AddLineNumbers(fragmentSource) << '\n';
	}
}

Shader::Shader(std::filesystem::path shaderSourceFile) : _id(0) {
	ScopedTimer timer("[Shader] Compilation took: ");
	std::cout << "\n" << "[Shader] Compiling shader: " << shaderSourceFile << "\n";

	std::ifstream file(shaderSourceFile);

	if (!file) {
		std::cerr << "[Shader] Failed to load shader file: " << shaderSourceFile.string() << std::endl;
		return;
	}

	std::string vertexSource;
	std::string fragmentSource;
	std::string computeSource;

	std::string* targetString = nullptr;

	std::string line;

	while (std::getline(file, line)) {
		static const std::string IncludeDirective = "#include";

		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos || line.find("vert") != std::string::npos)
				targetString = &vertexSource;
			else if (line.find("fragment") != std::string::npos || line.find("frag") != std::string::npos)
				targetString = &fragmentSource;
			else if (line.find("compute") != std::string::npos)
				targetString = &computeSource;
			else
				std::cerr << "[Shader] This shader type is not supported: " << line << '\n';

			continue;
		}
		else if (line.find(IncludeDirective) != std::string::npos && targetString != nullptr) {
			std::filesystem::path includePath = shaderSourceFile;
			includePath.remove_filename();
			includePath.append(ExtractIncludePath(line));

			std::ifstream includeFile(includePath);
			if (!includeFile) {
				std::cerr << "[Shader] Failed to open included file: " << includePath << std::endl;
				continue;
			}

			std::cout << "[Shader] Including file: " << includePath << '\n';

			std::string includedLine;
			while (std::getline(includeFile, includedLine)) {
				*targetString += includedLine + '\n';
			}

			continue;
		}

		if (targetString == nullptr)
			continue;

		*targetString += line + '\n';
	}

	if (!computeSource.empty()) {
		computeSource.insert(0, "#version 460 core\n#define COMPUTE_SHADER\n");
		_id = CreateComputeShaderFrom(computeSource);
		if (_id == 0)
			std::cout << "[Shader] Compute source dump:\n" << AddLineNumbers(computeSource) << '\n';
	}
	else {
		vertexSource.insert(0, "#version 460 core\n#define VERTEX_SHADER\n");
		fragmentSource.insert(0, "#version 460 core\n#define FRAGMENT_SHADER\n");
		_id = CreateShaderFrom(vertexSource, fragmentSource);
		if (_id == 0)
			std::cout << "[Shader] Vertex source dump:\n" << AddLineNumbers(vertexSource) << "Fragment source dump:\n" << AddLineNumbers(fragmentSource) << '\n';
	}
}

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
	: _id(CreateShaderFrom(vertexSource, fragmentSource))
{
}

static u32 CreateShaderFrom(const std::string& vertexSource, const std::string& fragmentSource) {
	u32 vertSubID = CompileSubShader(vertexSource.c_str(), VertexShaderID);
	u32 fragSubID = CompileSubShader(fragmentSource.c_str(), FragmentShaderID);

	if (vertSubID == 0 || fragSubID == 0)
		return 0;

	u32 shaderID = glCreateProgram();
	
	glAttachShader(shaderID, vertSubID);
	glAttachShader(shaderID, fragSubID);
	glLinkProgram(shaderID);

	glUseProgram(shaderID);

	i32 linkingStatus;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &linkingStatus);

	if (linkingStatus != GL_TRUE) {
		const i32 bufsize = 1024;
		i32 msgLen;
		char message[bufsize];
		glGetProgramInfoLog(shaderID, bufsize, &msgLen, message);
		std::cout << "[Shader] Shader linking error:\n" << message << std::endl;
		return 0;
	}
	
	glDeleteShader(vertSubID);
	glDeleteShader(fragSubID);

	return shaderID;
}

static u32 CreateComputeShaderFrom(const std::string& compute) {
	u32 computeID = CompileSubShader(compute.c_str(), GL_COMPUTE_SHADER);

	if (computeID == 0)
		return 0;

	u32 shaderID = glCreateProgram();

	glAttachShader(shaderID, computeID);
	glLinkProgram(shaderID);

	glUseProgram(shaderID);

	i32 linkingStatus;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &linkingStatus);

	if (linkingStatus != GL_TRUE) {
		const i32 bufsize = 1024;
		i32 msgLen;
		char message[bufsize];
		glGetProgramInfoLog(shaderID, bufsize, &msgLen, message);
		std::cout << "[Shader] Shader linking error:\n" << message << std::endl;
		return 0;
	}

	glDeleteShader(computeID);

	return shaderID;
}

void Shader::Bind() const {
	glUseProgram(_id);
}

static u32 CompileSubShader(const char* sourceCString, i32 shaderType) {
	u32 subShaderID = glCreateShader(shaderType);
	glShaderSource(subShaderID, 1, &sourceCString, nullptr);
	glCompileShader(subShaderID);

	i32 compilationStatus;
	glGetShaderiv(subShaderID, GL_COMPILE_STATUS, &compilationStatus);
	if (compilationStatus != GL_TRUE) {
		const i32 bufsize = 1024;
		i32 msgLen;
		char message[bufsize];
		glGetShaderInfoLog(subShaderID, bufsize, &msgLen, message);
		std::cout << "[Shader] " << (shaderType == VertexShaderID ? "Vertex" : shaderType == FragmentShaderID ? "Fragment" : "Compute") << " shader compilation error:\n" << message << std::endl;
		return 0;
	}

	return subShaderID;
}
}
