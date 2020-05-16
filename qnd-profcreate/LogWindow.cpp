#include "LogWindow.h"


std::vector<LogEntry> logHistory;


void DrawLogEntry(unsigned long int order)
{
	switch (logHistory[order].type)
	{
	case LogEntryType::normal:
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		return;
	case LogEntryType::warning:
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), logHistory[order].content.c_str());
		return;
	case LogEntryType::error:
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		return;
	case LogEntryType::success:
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		return;
	default:
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), logHistory[order].content.c_str());
		return;
	}

}

void DrawLogWindow()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowPos(ImVec2(400, 468), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(610, 300), ImGuiCond_Once);
	// 1024, 768 

	ImGui::Begin("Log", NULL, windowFlags);
	ImGui::PushItemWidth(ImGui::GetFontSize() * -12); //Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font s

	for (unsigned long int i = 0; i < logHistory.size(); i++)
		DrawLogEntry(i);

	ImGui::End();
}

bool Log(std::string & content, LogEntryType type)
{

	logHistory.push_back(LogEntry(content, type));

	return true;
}
