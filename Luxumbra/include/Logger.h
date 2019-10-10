#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

namespace lux
{

	enum class LOG_LEVEL
	{
		INFO = 0,
		WARNING = 1,
		ERROR = 2
	};

	class Logger
	{
	public:
		template<class T>
		static void Log(T var);

		template<class T, class... Types>
		static void Log(T arg1, Types... arg2);

		template<class T, class... Types>
		static void Log(LOG_LEVEL logLevel, T arg1, Types... arg2);
	};

}

#include "Logger.inl"

#endif // LOGGER_H_INCLUDED