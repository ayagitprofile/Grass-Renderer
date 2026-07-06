#pragma once

#include "Types.h"
#include "ComputeShader.h"
#include <string>

namespace Grass {
	class ComputeShaderSource
	{
	private:
		std::string _source;
		Path _path;
		static bool DeleteCurrentLayoutDirective(std::string& source);
	public:
		ComputeShaderSource(const Path& sourcePath);
		void SetNumberOfThreads(i32 x, i32 y);
		void SetNumberOfThreads(i32 x);
		void InsertLine(const std::string& line);
		void InsertDefineDirective(const std::string& name, const std::string& value);
		const std::string& GetSource() const { return _source; }
		ComputeShader Compile() const;
	};
}