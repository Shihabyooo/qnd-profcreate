#pragma once
#include <wchar.h>
#include <vector>
#include <filesystem>
#include <Windows.h>
#include <iostream>

#include "imgui.h"
#include "Globals.h"


enum class LogEntryType
{
	normal, warning, error, success
};

struct LogEntry
{
public:
	LogEntry(std::string _content, LogEntryType _type)
	{
		content = _content;
		type = _type;
	}

	//TODO add timestamp
	std::string content;
	LogEntryType type;
};

void DrawLogWindow();
bool Log(std::string &content, LogEntryType type);