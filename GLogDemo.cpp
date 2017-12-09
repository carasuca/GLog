#include "GLog.h"

void Write(const char fmt[], ...)
{
	va_list va;
	va_start(va, fmt);

	char buffer[128];
	auto count = FormatMessageA(FORMAT_MESSAGE_FROM_STRING, fmt, 0, 0, buffer, sizeof(buffer), &va);
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, count, &count, 0);
}

void main()
{
	Write("Try running multiple instances at the same time.%n");
	Write("Hold space to add log lines.%n");

	auto& glog = GLog::Instance();

	while (!::GetAsyncKeyState(VK_ESCAPE))
	{
		if (::GetAsyncKeyState(VK_SPACE))
		{
			auto& line = glog.NextLine().Prepare();
			lstrcpynA(line.msg, "Hello", sizeof(line.msg));
			lstrcpynA(line.module, __FUNCTION__, sizeof(line.module));
			Write("GLog::Counter: %1!lu!%n", glog.Counter());
		}

		else Sleep(0);
	}
}