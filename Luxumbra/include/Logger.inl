#ifndef LOGGER_INL_INCLUDED
#define LOGGER_INL_INCLUDED

#include "Logger.h"

#include <iostream>

namespace lux
{
	template<class T>
	void Logger::Log(T var)
	{
		std::cout << var << std::endl;
	}

	template<class T, class... Types>
	void Logger::Log(T arg1, Types... arg2)
	{
		std::cout << arg1 << ' ';
		Log(arg2...);
	}

	template<class T, class... Types>
	void Logger::Log(LogLevel logLevel, T arg1, Types... arg2)
	{
		switch (logLevel)
		{
		case LogLevel::LOG_LEVEL_INFO:
			std::cout << "[INFO] ";
			break;
		case LogLevel::LOG_LEVEL_WARNING:
			std::cout << "[WARNING] ";
			break;
		case LogLevel::LOG_LEVEL_ERROR:
			std::cout << "[ERROR] ";
			break;
		default:
			break;
		}

		Log(arg1, arg2...);
	}
}

#endif // LOGGER_INL_INCLUDED