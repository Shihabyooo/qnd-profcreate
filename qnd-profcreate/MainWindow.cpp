#include "MainWindow.h"


char geometryFilePath[MAX_PATH] = "Enter path to geometry directory here or use the directory browser.";
char demFilePath[MAX_PATH] = "Enter path to DEM directory here or use the directory browser.";
std::unique_ptr<bool> selectedGeometry;
std::unique_ptr<bool> selectedDEM;
std::vector<std::string> geometryNames;
std::vector<std::string> demNames;
bool defaulSelectionState = true;

void DrawFileList(char * filePath, std::vector<std::string> * fileNames, std::unique_ptr<bool> * selectionStates, DataType dataType)
{	
	ImGui::Separator();
	ImGui::Text("Available Geometries\n");
	ImGui::SameLine();
	if (ImGui::Button("Update List", ImVec2(50, 20)))
		UpdateFileList(filePath, fileNames, selectionStates, dataType);

	ImGui::Spacing();
	for (int i = 0; i < fileNames->size(); i++)
	{		
		bool state = selectionStates->get()[i]; //feeding selectedGeometry.get()[i] directly to ImGui::Selectable() prevents it from switching selection state.
		ImGui::Selectable(ExtractFileName((*fileNames)[i]).c_str(), &state);
		selectionStates->get()[i] = state;
	}

	ImGui::Separator();
}

void DrawMainWindow()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
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
	ImGui::Spacing();

	ImGui::Separator();
	ImGui::Text("Geometry Sources");
	ImGui::InputText("Geometry File Path", geometryFilePath, IM_ARRAYSIZE(geometryFilePath));

	if (ImGui::Button("Browse for geometry directory"))
		OpenFileBrowser(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry);
	
	DrawFileBrowser();
	DrawFileList(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry);

	ImGui::Separator();
	ImGui::Text("DEM");
	ImGui::InputText("DEM File Path", demFilePath, IM_ARRAYSIZE(demFilePath));

	if (ImGui::Button("Browse for DEM directory"))
		OpenFileBrowser(demFilePath, &demNames, &selectedDEM, DataType::dem);
	////DrawFileBrowser(); //The call is already made above...
	DrawFileList(demFilePath, &demNames, &selectedDEM, DataType::dem);

	ImGui::Spacing();
	ImGui::Spacing();

	
	ImGui::Button("Process!", ImVec2(100, 50));

	ImGui::End();
}
