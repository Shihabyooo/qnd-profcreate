#include "LogWindow.h"


std::vector<LogEntry> logHistory;


static bool updateDimensions = false;
static WindowDimensions dimensions;

void DrawLogEntry(unsigned long int order)
{
	switch (logHistory[order].type)
	{
	case LogEntryType::normal:
		//ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		//ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::TextWrapped(logHistory[order].content.c_str());
		//ImGui::PopStyleColor();
		return;
	case LogEntryType::warning:
		//ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), logHistory[order].content.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
		ImGui::TextWrapped(logHistory[order].content.c_str());
		ImGui::PopStyleColor();
		return;
	case LogEntryType::error:
		//ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::TextWrapped(logHistory[order].content.c_str());
		ImGui::PopStyleColor();
		return;
	case LogEntryType::success:
		//ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), logHistory[order].content.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::TextWrapped(logHistory[order].content.c_str());
		ImGui::PopStyleColor();
		return;
	default:
		//ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), logHistory[order].content.c_str());
		ImGui::TextWrapped(logHistory[order].content.c_str());
		return;
	}

}

void DrawLogWindow()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	
	if (updateDimensions)
	{
		ImGui::SetNextWindowPos(ImVec2(dimensions.positionX, dimensions.positionY), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(dimensions.width, dimensions.height), ImGuiCond_Always);
		updateDimensions = false;
	}

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

void UpdateLogWindowSizeAndPos(long int resolutionX, long int resolutionY)
{
	long int positionX = resolutionX > WINDOW_MAIN_MIN_WIDTH ? (resolutionX > WINDOW_MAIN_MAX_WIDTH ? WINDOW_MAIN_MAX_WIDTH : resolutionX) : WINDOW_MAIN_MIN_WIDTH;
	long int width = (resolutionX - WINDOW_MAIN_MIN_WIDTH) < WINDOW_LOG_MIN_WIDTH ? WINDOW_LOG_MIN_WIDTH : (resolutionX - positionX);
	long int height = WINDOW_LOG_HEIGHT_PERCENTAGE * resolutionY;
	

	dimensions = WindowDimensions(positionX, ((1.0f - WINDOW_LOG_HEIGHT_PERCENTAGE) * resolutionY), width, height);


	updateDimensions = true;
}
