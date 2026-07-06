#pragma once

#include <chrono>
#include <iostream>

namespace Grass {
	class ScopedTimer
	{
		typedef std::chrono::steady_clock::time_point TimePoint;
	private:
		TimePoint _start;
		std::string _message;
	public:
		ScopedTimer(std::string message = "")
			: _start(std::chrono::high_resolution_clock::now()), _message(message)
		{}
		float ElapsedTimeMS() const {
			TimePoint end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - _start;
			return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000.f;
		}
		~ScopedTimer(){
			if (_message.empty())
				return;
			TimePoint end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - _start;
			std::cout << _message << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms\n";
		}
	};
}