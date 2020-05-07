#include "MainWindow.h"


std::unique_ptr<bool> selectedGeometry;
std::unique_ptr<std::string> geometryNames;




void DrawGeometryList()
{

}



void DrawMainWindow()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);

	ImGui::Begin("MainWindow", NULL, windowFlags);
	ImGui::PushItemWidth(ImGui::GetFontSize() * -12); //Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font size.

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
			//ImGui::MenuItem("New", NULL, &newFile);
			ImGui::EndMenu();
		if (ImGui::BeginMenu("Edit"))
			ImGui::EndMenu();
		if (ImGui::BeginMenu("Help"))
			ImGui::EndMenu();
		ImGui::EndMenuBar();
	}

	ImGui::Text("QnD Profile Creator v.0.x.x");
	ImGui::Separator();

	ImGui::Text("Geometry Sources");

	static char filePath[MAX_PATH] = "test";
	ImGui::InputText("File Path", filePath, IM_ARRAYSIZE(filePath));

	if (ImGui::Button("Browse for directory"))
	{
		OpenFileBrowser(filePath, geometryNames.get(), DataType::geometry);
	}

	DrawFileBrowser();

	DrawGeometryList();


	ImGui::End();

}
