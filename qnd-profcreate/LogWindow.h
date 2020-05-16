#pragma once
#include <wchar.h>
#include <vector>
#include <filesystem>
#include <Windows.h>
#include <iostream>

#include "imgui.h"
#include "Globals.h"

#define LOG_SUCCESS	LogEntryType::success
#define LOG_ERROR	LogEntryType::error
#define LOG_NORM	LogEntryType::normal
#define LOG_WARN	LogEntryType::warning

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
void UpdateLogWindowSizeAndPos(long int resolutionX, long int resolutionY);