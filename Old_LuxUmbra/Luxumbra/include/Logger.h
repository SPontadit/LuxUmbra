#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include "Luxumbra.h"

namespace lux
{

	enum class LogLevel
	{
		LOG_LEVEL_INFO = 0,
		LOG_LEVEL_WARNING = 1,
		LOG_LEVEL_ERROR = 2
	};

	class Logger
	{
	public:
		template<class T>
		static void Log(T var);

		template<class T, class... Types>
		static void Log(T arg1, Types... arg2);

		template<class T, class... Types>
		static void Log(LogLevel logLevel, T arg1, Types... arg2);
	};

} // namespace lux

#include "Logger.inl"

#endif // LOGGER_H_INCLUDED