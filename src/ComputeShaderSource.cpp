#include "ComputeShaderSource.h"
#include <fstream>
#include <regex>
#include <iostream>
using namespace Grass;

static size_t GetInsertionIndex(const std::string& source) {
	auto index = source.find("#shader");

	if (index == std::string::npos) {
		std::cerr << "[ComputeShaderSource] Error: Shader type directive not found\n";
		return std::string::npos;
	}

	return source.find('\n', index) + 1;
}

bool Grass::ComputeShaderSource::DeleteCurrentLayoutDirective(std::string& source) {
	static const std::regex layoutRegex(
		R"(layout\s*\((.*?)\)\s*in\s*;\s*\n?)"
	);

	std::smatch layoutMatch;
	if (!std::regex_search(source, layoutMatch, layoutRegex))
		return false;

	// Parse everything inside layout(...)
	std::string params = layoutMatch[1].str();

	static const std::regex valueRegex(
		R"(local_size_([xyz])\s*=\s*(\d+))"
	);

	source.erase(layoutMatch.position(), layoutMatch.length());

	return true;
}

Grass::ComputeShaderSource::ComputeShaderSource(const Path& sourcePath) {
	_path = sourcePath;

	std::ifstream stream(sourcePath);
	std::string line;
	while (std::getline(stream, line)) {
		_source += line + '\n';
	}
}

void Grass::ComputeShaderSource::SetNumberOfThreads(i32 x, i32 y) {
	static const std::string LayoutDirective = "layout";

	DeleteCurrentLayoutDirective(_source);
	
	auto index = _source.find("#shader");

	if (index == std::string::npos) {
		std::cerr << "[ComputeShaderSource] Shader: " << _path.string() << " error: " << "cant insert number of threads directive because shader type directive is not found\n";
		return;
	}

	_source.insert(_source.find('\n', index) + 1, std::format("layout(local_size_x = {}, local_size_y = {}) in;\n", x, y));
}

void Grass::ComputeShaderSource::SetNumberOfThreads(i32 x) {
	static const std::string LayoutDirective = "layout";

	DeleteCurrentLayoutDirective(_source);

	auto index = _source.find("#shader");

	if (index == std::string::npos) {
		std::cerr << "[ComputeShaderSource] Shader: " << _path.string() << " error: " << "cant insert number of threads directive because shader type directive is not found\n";
		return;
	}

	_source.insert(_source.find('\n', index) + 1, std::format("layout(local_size_x = {}) in;\n", x));
}

void Grass::ComputeShaderSource::InsertLine(const std::string& line) {
	if (line.empty())
		return;
	_source.insert(GetInsertionIndex(_source), line[line.size() - 1] == '\n' ? line : line + '\n');
}

void Grass::ComputeShaderSource::InsertDefineDirective(const std::string& name, const std::string& value) {
	InsertLine(std::format("#define {} ({})\n", name, value));
}

ComputeShader Grass::ComputeShaderSource::Compile() const {
	return ComputeShader(_source, _path);
}
