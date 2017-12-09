#pragma once

// GLog by Jakub Krzeslowski, created 8.12.2017
// Global inter-process log table, single header, no CRT, just WinAPI

#include <Windows.h>

struct GLog
{
	GLog() = delete;

	static const DWORD num_lines = 16 * 1024; // 16k lines

	union Line
	{
		static const DWORD size = 1024; // forced padding in bytes, http://www.catb.org/esr/structure-packing/

		Line& Prepare(DWORD pid = GetProcessId(GetCurrentProcess()), DWORD tid = GetCurrentThreadId())
		{
			GetSystemTimeAsFileTime(&timestamp);
			this->pid = pid;
			this->tid = tid;
			return *this;
		}

		struct {
			FILETIME timestamp;
			DWORD tid, pid, level, _reserved;
			char module[32], msg[size - 64];
		};

	private: char _some_nice_padding[size];
	};

	static_assert(sizeof(Line) == Line::size, "Line padding mismatch");
	static_assert(!(num_lines&(num_lines - 1)), "Number of lines has to be a power of 2!");


	static GLog& Instance()
	{
		// call this once prior to multi-threading, don't believe in magic statics
		static GLog& g_shared = MapInstance<GLog>("Local\\GLog");
		return g_shared;
	}

	DWORD Counter() const { return line_counter; } // not wrapped
	const Line* Lines() const { return lines; } // read access

	Line& NextLine() // write access
	{
		auto last = InterlockedIncrement(&line_counter) - 1;
		return lines[last%num_lines];
	}

	template<typename UberSingletonType>
	static UberSingletonType& MapInstance(const char name[])
	{
		// consecutive calls return the same destination, only at a different virtual address!
		const DWORD bytes = sizeof(UberSingletonType);
		HANDLE mapping = ::CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, bytes, name);
		LPVOID ptr = ::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, bytes);
		return *reinterpret_cast<UberSingletonType*>(ptr);
	}

private: // FIXME: the counter offsets and misaligns memory, find better solution

	volatile DWORD line_counter; // ~ number of lines written
	Line lines[num_lines];
};