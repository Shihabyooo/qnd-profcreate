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
//bool defaulSelectionState = false;
double chainageSteps = 0.0f;
bool mainatainBends = false;
bool useInputDirForOutput = false;
InterpolationMethods interpolationMethod = InterpolationMethods::nearestNeighbour;
bool processingPopupState = false;

static bool updateDimensions = false;
static WindowDimensions dimensions;

bool isProcessing = false;
std::future<int> processingResult; //useless at this moment.
std::mutex isProcessingMutex;

void DisplayDEMSummary(std::string &path)
{
	std::unique_ptr<DEMSummary> summary = profileMaker->GetDEMSummary(path);

	if (summary.get() == NULL)
	{
		Log("Selected DEM could not be parsed.", LOG_WARN);
		return;
	}

	std::string _message;
	_message = "Summary for DEM " + path;
	Log(_message, LOG_SUCCESS);

	_message = "Dimensions: " + std::to_string(summary.get()->width) + "x" + std::to_string(summary.get()->width);
	Log(_message);

	_message = "Compression: " + summary.get()->compressionMethod;
	Log(_message);

	_message = "Coordinate Reference System: " + std::to_string(summary.get()->crsCode) ;
	Log(_message);

	_message = summary.get()->crsCitation;
	Log(_message);

	_message = "Extent: min:" + std::to_string(summary.get()->boundingRect[0]) + "x" + std::to_string(summary.get()->boundingRect[1]) + ", max:" + std::to_string(summary.get()->boundingRect[2]) + std::to_string(summary.get()->boundingRect[3]);
	Log(_message);

	if (!summary.get()->isSupported)
		Log("WARNING: This DEM file is not supported.", LOG_WARN);
}

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

		if (listID == DEM_LIST_ID) //ugly hack, but the alternative is even uglier modifications to FileSystemBrowser...
		{
			for (size_t i = 0; i < fileNames->size(); i++)
				selectionStates->get()[i] = false;
		}

		//std::cout << "Updating with path: " << std::string(filePath).c_str() << ", dataype: " << (int)dataType << std::endl; //test
	}

	ImGui::NewLine();
	for (int i = 0; i < fileNames->size(); i++)
	{		
		bool state = selectionStates->get()[i]; //feeding selectedGeometry.get()[i] directly to ImGui::Selectable() prevents it from switching selection state.
		if (ImGui::Selectable(ExtractFileName((*fileNames)[i]).c_str(), &state))
		{
			selectionStates->get()[i] = state;

			//for single selection mode, check if this item is selected, and if so unselect remaining items. Ugly hack, but works.
			//if (singleSelection && state)
			if (singleSelection)
			{
				for (int j = 0; j < fileNames->size(); j++)
					if (selectionStates->get()[j] && j != i)
						selectionStates->get()[j] = false;
			}
			if (listID == DEM_LIST_ID)
			{
				DisplayDEMSummary((*fileNames)[i]);
			}
		}
	}
	ImGui::Separator();
}

void DrawProcessingPopup()
{
	std::lock_guard<std::mutex> _lock{ isProcessingMutex };
	processingPopupState = isProcessing;

	if (!processingPopupState)
		return;

	//In the current implementation, it's imperative that user cannot use the program while this window is active. This is due to the fact that there are no lock guards in place when editing the values of the input.
	ImGui::BeginPopupModal("Processing", &processingPopupState, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Processing Profiles - Please wait.");
	ImGui::EndPopup();

}

bool CheckInputValidity()
{
	bool inputValidity = true; //To have all check errors show on the log, we don't immediatly return when one fails.

	//check that geometry files are loaded to file list
	if (geometryNames.size() < 1)// || demNames.size() < 1)
	{
		//std::cout << "ERROR! At least one geometry file must be provided." << std::endl;
		Log("ERROR! At least one geometry file must be provided.", LOG_ERROR);
		inputValidity = false;
	}
	
	//check dem files are loaded to file list
	if (demNames.size() < 1)
	{
		//std::cout << "ERROR! A DEM file must be provided." << std::endl;
		Log("ERROR! A DEM file must be provided.", LOG_ERROR);

		inputValidity = false;
	}

	//check that at least one geometry file from file list is selected
	for (int i = 0; i < geometryNames.size(); i++)
	{
		if (selectedGeometry.get()[i] == true)
			break;
		else if (i == geometryNames.size() - 1)//reached last element and none are selected
		{
			//std::cout << "ERROR! At least one geometry file must be selected." << std::endl;
			Log("ERROR! At least one geometry file must be selected.", LOG_ERROR);

			inputValidity = false;
		}
	}

	//check that a dem file from file list is selected.
	for (int i = 0; i < demNames.size(); i++)
	{
		if (selectedDEM.get()[i] == true)
			break;
		else if (i == demNames.size() - 1)
		{
			//std::cout << "ERROR! A DEM file must be selected." << std::endl;
			Log("ERROR! A DEM file must be selected.", LOG_ERROR);

			inputValidity = false;
		}
	}

	//check that chainageSteps is set > 0.0f
	if (chainageSteps < 0.001f)
	{
		//std::cout << "ERROR! Chainage Steps is set to a very small value or zero." << std::endl;
		Log("ERROR! Chainage Steps is set to a very small value or zero.", LOG_ERROR);

		inputValidity = false;
	}

	//check that output directory is set.
	if (outputDirectory.length() < 3 || !IsDirectoryAccessible(outputDirectory))
	{
		//std::cout << "ERROR! Invalid output directory." << std::endl;
		Log("ERROR! Invalid output directory.", LOG_ERROR);

		inputValidity = false;
	}

	//if we reached here, means all input are set and ok (at GUI level).
	return inputValidity;
}

void BeginProcessing()
{
	outputDirectory = std::string(outputDirectoryPath);

	if (!CheckInputValidity())
		return;

	if (profileMaker == NULL)
	{
		std::cout << "ERROR! profileMaker is set to NULL. This should not happen (unless the programmer sucked more than he thought he does)." << std::endl;
		Log("ERROR! profileMaker is set to NULL. This should not happen (unless the programmer sucked more than he thought he does).", LOG_ERROR);

		return;
	}

	//create list of selected geometries.
	static std::vector<std::string> _geoemetryPaths;
	for (int i = 0; i < geometryNames.size(); i++)
	{
		if (selectedGeometry.get()[i])
			_geoemetryPaths.push_back(geometryNames[i]);
	}

	//Extract selected DEM from list.
	static std::string _demPath;
	for (int i = 0; i < demNames.size(); i++)
	{
		if (selectedDEM.get()[i])
		{
			_demPath = demNames[i];
			break;
		}
	}

	//TODO Decouple processing from UI drawing; read bellow.
	//The "Processing" popup will not appear during processing with this implementation, this is due to the fact that call to this function, and in turn DrawMainWindow() will be stalled until
	// profileMaker->BatchProfileProcessing() returns. Meaning ImGui::End() and (in the GUIHandler's ProgramLoop()) both the ImGui::Render() and the D3D context updating won't be executed.
	//There are two possible solutions to this:
	//A:	Have BatchProfileProcessing() run on a different thread, with a flag local to the parent process it sets when its done (so the parent process can know that processing is done, and we
	//		can claose the popup.
	//B:	Move prcessing calls to GuiHandler, and have this function set a flag there in addition to starting the popup. At the end of the render/message loop (i.e. after g_pSwapChain->Present(1, 0))
	//		an if-statment checks the flag and if set, start the call BatchProfileProcessing().
	//Later idea: B could actually be done inside MainWindow. Just have an if(isprocessing) at the begining of DrawMainWindow() with a call to batchprofileprocessing inside.


	processingPopupState = true;
	ImGui::OpenPopup("Processing");

	Log("Begining processing."); //note: this won't appear probably because it depends on ImGui finishing drawing a window, which won't happen unless BatchPorileProcessing returns.

	static ProcessingOrder order(&_geoemetryPaths, &_demPath, &outputDirectory, chainageSteps, interpolationMethod, mainatainBends, false, true, 0);
	isProcessing = true;

	//int result = profileMaker->BatchProfileProcessing(_geoemetryPaths, _demPath, outputDirectory, chainageSteps, interpolationMethod, mainatainBends);
	/*int result = profileMaker->BatchProfileProcessing(order);
	switch (result)
	{
	case PROCESSING_SUCCESS:
		Log("Finished processing successfully.", LOG_SUCCESS);
		break;
	case PROCESSING_PARTIAL_SUCCESS:
		Log("Finished processing with errors.", LOG_WARN);
		break;
	case PROCESSING_FAIL_DEM_LOAD:
		Log("Processing failed. Could not Load DEM", LOG_WARN);
		break;
	case PROCESSING_FAIL_GEOMETRY_LOAD:
		Log("Processing failed. Could not Load Geometries", LOG_WARN);
		break;
	default:
		Log("Finished processing with an unknown state.", LOG_WARN);
		break;
	}*/

	
	//std::cout << "Before async, "<< order.demLocation->c_str() << std::endl;//test
	processingResult = std::future(std::async(std::launch::async, static_cast<int(ProfileMaker::*)(ProcessingOrder&, bool*, std::mutex *)>(&ProfileMaker::BatchProfileProcessing), profileMaker, order, &isProcessing, &isProcessingMutex));

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

	if (updateDimensions)
	{
		ImGui::SetNextWindowPos(ImVec2(dimensions.positionX, dimensions.positionY), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(dimensions.width, dimensions.height), ImGuiCond_Always);
		updateDimensions = false;
	}

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
	DrawFileList(geometryFilePath, &geometryNames, &selectedGeometry, DataType::geometry, false, GEOMETRY_LIST_ID);
	ImGui::NewLine();

	//DEM data
	ImGui::Text("DEM");
	ImGui::InputText("DEM File Path", demFilePath, IM_ARRAYSIZE(demFilePath));

	if (ImGui::Button("Browse for DEM directory"))
		OpenFileBrowser(demFilePath, &demNames, &selectedDEM, DataType::dem, false);
	//DrawFileBrowser(); //The call is already made above...
	DrawFileList(demFilePath, &demNames, &selectedDEM, DataType::dem, true, DEM_LIST_ID);
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
	//ImGui::GetFont()->Scale = 1.5f;
	//ImGui::PushFont(ImGui::GetFont());
	ImGui::NewLine();
	ImGui::NewLine();
	if (ImGui::Button("Process!", ImVec2(100, 50)))
		BeginProcessing();
	/*ImGui::PopFont();
	ImGui::GetFont()->Scale = 1.0f;*/

	DrawProcessingPopup();

	ImGui::End();
}

void UpdateMainWindowSizeAndPos(long int resolutionX, long int resolutionY)
{
	long int width = resolutionX > WINDOW_MAIN_MIN_WIDTH ? (resolutionX > WINDOW_MAIN_MAX_WIDTH ? WINDOW_MAIN_MAX_WIDTH : resolutionX) : WINDOW_MAIN_MIN_WIDTH;
	long int height = WINDOW_MAIN_HEIGHT_PERCENTAGE * resolutionY;

	//std::cout << "Main Window Res: " << width << " x " << height << std::endl;

	dimensions = WindowDimensions(0, 0, width, height);

	updateDimensions = true;
}
