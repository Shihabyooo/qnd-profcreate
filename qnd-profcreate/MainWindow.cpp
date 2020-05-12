#include "MainWindow.h"

ProfileMaker * profileMaker = NULL;

char geometryFilePath[MAX_PATH] = "Enter path to geometry directory here or use the directory browser.";
char demFilePath[MAX_PATH] = "Enter path to DEM directory here or use the directory browser.";
char outputDirectoryPath[MAX_PATH] = "Enter output directory.";
std::unique_ptr<bool> selectedGeometry;
std::unique_ptr<bool> selectedDEM;
std::vector<std::string> geometryNames;
std::vector<std::string> demNames;
std::string outputDirectory;
bool defaulSelectionState = true;
double chainageSteps = 0.0f;
bool mainatainBends = false;
bool useInputDirForOutput = false;
InterpolationMethods interpolationMethod = InterpolationMethods::nearestNeighbour;


void DrawFileList(char * filePath, std::vector<std::string> * fileNames, std::unique_ptr<bool> * selectionStates, DataType dataType, bool singleSelection, char listID)
{	
	ImGui::Separator();
	ImGui::Text("Available Files\n");
	ImGui::SameLine();

	//Because this function should spawn two distinct buttons, the label should be modified with each seperate call to this function, we control that by listID argument.
	//This is not a the most gracefull of solutions, but it works.
	char buttonLabel[15] = "Update List##X"; //'X' is a place holder, will be changed to listID.
	buttonLabel[13] = listID;

	if (ImGui::Button(buttonLabel, ImVec2(100, 20)))
	{
		UpdateFileList(filePath, fileNames, selectionStates, dataType);
		std::cout << "Updating with path: " << std::string(filePath).c_str() << ", dataype: " << (int)dataType << std::endl; //test
	}

	ImGui::NewLine();
	for (int i = 0; i < fileNames->size(); i++)
	{		
		bool state = selectionStates->get()[i]; //feeding selectedGeometry.get()[i] directly to ImGui::Selectable() prevents it from switching selection state.
		ImGui::Selectable(ExtractFileName((*fileNames)[i]).c_str(), &state);
		selectionStates->get()[i] = state;
		
		//for single selection mode, check if this item is selected, and if so unselect remaining items. Ugly hack, but works.
		if (singleSelection && state == true)
		{
			for (int j = 0; j < fileNames->size(); j++)
				if (selectionStates->get()[j] && j != i)
					selectionStates->get()[j] = false;
		}
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

	//check that output directory is set.
	if (outputDirectory.length() < 3 || !IsDirectoryAccessible(outputDirectory))
	{
		std::cout << "ERROR! Invalid output directory." << std::endl;
		return false;
	}

	//if we reached here, means all input are set and ok (at GUI level).
	return true;
}

void BeginProcessing()
{
	outputDirectory = std::string(outputDirectoryPath);

	if (!CheckSelectionValidity())
		return;

	if (profileMaker == NULL)
	{
		std::cout << "ERROR! profileMaker is set to NULL." << std::endl;
		return;
	}

	//create list of selected geometries.
	std::vector<std::string> _geoemetryPaths;
	for (int i = 0; i < geometryNames.size(); i++)
	{
		if (selectedGeometry.get()[i])
			_geoemetryPaths.push_back(geometryNames[i]);
	}

	//Extract selected DEM from list.
	std::string _demPath;
	for (int i = 0; i < demNames.size(); i++)
	{
		if (selectedDEM.get()[i])
		{
			_demPath = demNames[i];
			break;
		}
	}

	bool result = profileMaker->BatchProfileProcessing(_geoemetryPaths, _demPath, outputDirectory, chainageSteps, interpolationMethod, mainatainBends);

}

void DrawInterpolationMethods() //The approach with the function is ugly in more ways that one. FIX IT!
{
	static char * _interpolationMethods[] = { "Nearest Neighbour", "Bilinear", "Bicubic" }; //TODO update this after adding more interpolation methods (or change to automatically adjust to them)
	static int _currentInterpMethod = int(interpolationMethod);

	if (ImGui::BeginCombo("Interpolation Method", _interpolationMethods[_currentInterpMethod], 0))
	{
		for (int i = 0; i < 3; i++) //TODO update this after adding more interpolation methods (or change to automatically adjust to them)
		{
			bool isSelected = (_currentInterpMethod == i);
			if (ImGui::Selectable(_interpolationMethods[i], isSelected))
			{
				_currentInterpMethod = i;
				interpolationMethod = (InterpolationMethods)i;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
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
	DrawFileList(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry, false, '0');
	ImGui::NewLine();

	//DEM data
	ImGui::Text("DEM");
	ImGui::InputText("DEM File Path", demFilePath, IM_ARRAYSIZE(demFilePath));

	if (ImGui::Button("Browse for DEM directory"))
		OpenFileBrowser(demFilePath, &demNames, &selectedDEM, DataType::dem);
	//DrawFileBrowser(); //The call is already made above...
	DrawFileList(demFilePath, &demNames, &selectedDEM, DataType::dem, true, '1');
	ImGui::NewLine();

	//Other input
	static char chainageStepsBuffer[11] = "";
	ImGui::PushItemWidth(85);
	ImGui::InputText("Chainage Steps", chainageStepsBuffer, 11, ImGuiInputTextFlags_CharsDecimal);
	chainageSteps = atof(chainageStepsBuffer);
	ImGui::Checkbox("Maintain Path Bends", &mainatainBends);
	if (ImGui::Checkbox("Use input directory for Output.", &useInputDirForOutput))
	{
		if (useInputDirForOutput)
		{
			if (geometryNames.size() > 0)
			{
				std::string _outputDir = ExtractParentDirectoryPath(geometryNames[0]);
				for (int i = 0; i < MAX_PATH; i++) //this approach is probably more expensive than making two loops.
				{
					if (i < _outputDir.length())
						outputDirectoryPath[i] = _outputDir[i];
					else
						outputDirectoryPath[i] = '\0';
				}
			}
			else //zero out the output dir.
				for (int i = 0; i < MAX_PATH; i++) 
					outputDirectoryPath[i] = '\0';
		}
	}
	ImGui::PopItemWidth();
	
	DrawInterpolationMethods();

	
	//output location
	ImGuiInputTextFlags outDirInputFlags = 0; 
	if (useInputDirForOutput)
		outDirInputFlags = ImGuiInputTextFlags_ReadOnly;
	ImGui::InputText("Output Diretory", outputDirectoryPath, IM_ARRAYSIZE(outputDirectoryPath), outDirInputFlags);
	if (!useInputDirForOutput)
	{
		if (ImGui::Button("Browse for output directory"))
			OpenFileBrowserSimple(outputDirectoryPath);
	}
	else
		ImGui::Dummy(ImVec2(0.0f, ImGui::GetFontSize() * 1.5f));
	
	//The scaling solution is ugly....
	ImGui::GetFont()->Scale = 1.5f;
	ImGui::PushFont(ImGui::GetFont());
	ImGui::NewLine();
	ImGui::NewLine();
	if (ImGui::Button("Process!", ImVec2(100, 50)))
		BeginProcessing();
	ImGui::PopFont();
	ImGui::GetFont()->Scale = 1.0f;


	ImGui::End();
}
