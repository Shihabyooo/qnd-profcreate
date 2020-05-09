#include "MainWindow.h"


char geometryFilePath[MAX_PATH] = "Enter path to geometry directory here or use the directory browser.";
char demFilePath[MAX_PATH] = "Enter path to DEM directory here or use the directory browser.";
std::unique_ptr<bool> selectedGeometry;
std::unique_ptr<bool> selectedDEM;
std::vector<std::string> geometryNames;
std::vector<std::string> demNames;
bool defaulSelectionState = true;
double chainageSteps = 0.0f;
bool mainatainBends = false;

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

bool CheckSelectionValidity()
{
	//check that geometry files are loaded to file list
	if (geometryNames.size() < 1)// || demNames.size() < 1)
	{
		std::cout << "ERROR! At least one geometry file must be provided." << std::endl;
		return false;
	}
	
	//check dem files are loaded to file list
	if (demNames.size() < 1)
	{
		std::cout << "ERROR! A DEM file must be provided." << std::endl;
		return false;
	}

	//check that at least one geometry file from file list is selected
	for (int i = 0; i < geometryNames.size(); i++)
	{
		if (selectedGeometry.get()[i] == true)
			break;
		else if (i == geometryNames.size() - 1)//reached last element and none are selected
		{
			std::cout << "ERROR! At least one geometry file must be selected." << std::endl;
			return false;	
		}
	}

	//check that a dem file from file list is selected.
	for (int i = 0; i < demNames.size(); i++)
	{
		if (selectedDEM.get()[i] == true)
			break;
		else if (i == demNames.size() - 1)
		{
			std::cout << "ERROR! A DEM file must be selected." << std::endl;
			return false;
		}
	}

	//check that chainageSteps is set > 0.0f
	if (chainageSteps < 0.001f)
	{
		std::cout << "ERROR! Chainage Steps is set to a very small value or zero." << std::endl;
		return false;
	}


	//if we reached here, means all input are set and ok (at GUI level).
	return true;
}

void BeginProcessing()
{
	if (!CheckSelectionValidity())
		return;



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
	ImGui::Separator();
	ImGui::NewLine();

	//Geometry data
	ImGui::Text("Geometry Sources");
	ImGui::InputText("Geometry File Path", geometryFilePath, IM_ARRAYSIZE(geometryFilePath));

	if (ImGui::Button("Browse for geometry directory"))
		OpenFileBrowser(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry);
	
	DrawFileBrowser();
	DrawFileList(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry);
	ImGui::NewLine();

	//DEM data
	ImGui::Text("DEM");
	ImGui::InputText("DEM File Path", demFilePath, IM_ARRAYSIZE(demFilePath));

	if (ImGui::Button("Browse for DEM directory"))
		OpenFileBrowser(demFilePath, &demNames, &selectedDEM, DataType::dem);
	//DrawFileBrowser(); //The call is already made above...
	DrawFileList(demFilePath, &demNames, &selectedDEM, DataType::dem);
	ImGui::NewLine();

	//Other data
	static char chainageStepsBuffer[11] = "";
	ImGui::PushItemWidth(85);
	ImGui::InputText("Chainage Steps", chainageStepsBuffer, 11, ImGuiInputTextFlags_CharsDecimal);
	chainageSteps = atof(chainageStepsBuffer);
	ImGui::Checkbox("Maintain Path Bends", &mainatainBends);

	
	ImGui::NewLine();
	ImGui::NewLine();
	if (ImGui::Button("Process!", ImVec2(100, 50)))
		BeginProcessing();

	ImGui::End();
}
