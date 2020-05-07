#include "FilesystemBrowser.h"

std::vector<char> availableDrives;
std::unique_ptr<DirectoryNode> dirTree = std::unique_ptr<DirectoryNode>(nullptr);
bool isBrowserOpen = false;
char * browserOutPath = NULL;
DataType currentTargetType;
std::vector<std::string> * fileList;
bool * selectionFlags;

void PopulateDrivesList()
{
	//std::cout << "Populating drives list" << std::endl;
	unsigned long int driveMask = GetLogicalDrives();
	availableDrives.clear();
	for (int i = 0; i < 26; i++)
	{
		if (((unsigned char)driveMask & (unsigned long int)pow(2, i)) > 0)
			availableDrives.push_back((char)(i + 65));
	}

	//Initialize the Directory Tree dirTree
	std::vector<std::wstring> _drives;
	for (int i = 0; i < availableDrives.size(); i++)
	{
		std::wstring drivePath = L"";
		drivePath += availableDrives[i];
		drivePath += L":\\";
		_drives.push_back(drivePath);
	}
	dirTree = std::unique_ptr<DirectoryNode>(new DirectoryNode(L"Root", _drives));
}

DirectoryNode * TraverseDirectoryTree(std::wstring path)
{
	//std::cout << "Traversing Directory Tree for path: " << ToUTF8(path).c_str() << std::endl; //test
	DirectoryNode * node = dirTree.get();
	DirectoryNode * previousNode = node;
	while (node != NULL)
	{
		if (node->path == path) //will be used when traversing for only drive letters
			return node;

		previousNode = node;
		for (int i = 0; i < node->noOfSubNodes; i++)
		{
			if (node->subNodes[i] != NULL)
			{
				std::wstring subNodePath = node->subNodes[i]->path;
				//std::cout << "Comparing " << ToUTF8(subNodePath).c_str() << " to " << ToUTF8(path).c_str() << std::endl;
				if (subNodePath == path)
				{
					//std::cout << "Found node!"  << std::endl; //test
					return node->subNodes[i];
				}
				else if (subNodePath == path.substr(0, subNodePath.size()))
				{
					node = node->subNodes[i];
					break;
				}
			}
		}
		if (previousNode == node)
			return NULL;
	}
	return node;
}

void AddNode(std::wstring path, std::vector<std::wstring> content)
{
	//std::cout << "Attempting to add a new node for path " << ToUTF8(path).c_str() << std::endl; //test

	std::wstring parentPath = path;

	if (path.length() > 3)
	{
		if (parentPath.back() == '\\')
			parentPath.pop_back();

		while (parentPath.back() != '\\')
		{
			if (parentPath.length() < 1)
			{
				std::cout << "ERROR! Recieved bad path in AddNode()." << std::endl;
				return;
			}
			parentPath.pop_back();
		}
	}
	else
		parentPath = L"Root";

	DirectoryNode * parentNode = TraverseDirectoryTree(parentPath);
	if (parentNode == NULL)
	{
		std::cout << "ERROR! Parent node for recieved path not found. At AddNode()." << std::endl;
		return;
	}

	//first, check that no current node is set for our path.
	for (int i = 0; i < parentNode->noOfSubNodes; i++)
		if (parentNode->subNodes[i] != NULL && parentNode->subNodes[i]->path == path)
		{
			std::cout << "Warning! Attempting to add a node that is already added. At AddNode()." << std::endl;
			parentNode->subNodes[i]->content = content;
			return;
		}

	for (int i = 0; i < parentNode->noOfSubNodes; i++)
	{
		if (parentNode->subNodes[i] == NULL)
		{
			parentNode->subNodes[i] = new DirectoryNode(path, content);
			std::cout << "Succcessfully added node" << std::endl; //test
			break;
		}
	}
}

std::vector<std::wstring> QuerryDirectoryContent(std::wstring path)
{
	std::vector<std::wstring> content;
	//std::cout << "Quarrying Directory for path: " << ToUTF8(path).c_str() << std::endl; //test

	//first check if data is already loaded

	DirectoryNode * node = TraverseDirectoryTree(path);

	if (node == NULL)
	{
		//std::cout << "Adding new data for path: " << ToUTF8(path).c_str() << std::endl; //test
		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			//std::cout << "FileSystem result: " << ToUTF8(entry.path().wstring()).c_str() << std::endl; //test
			if (entry.is_directory())
				content.push_back((entry.path().wstring() + L"\\"));
		}
		AddNode(path, content);
	}
	else
	{
		return node->content;
	}

	//std::cout << std::endl; //test
	return content;
}

std::string RecursiveTree(std::wstring parentPath, ImGuiTreeNodeFlags treeFlags)
{
	std::vector<std::wstring> dirContent = QuerryDirectoryContent(parentPath);
	for (int j = 0; j < dirContent.size(); j++)
	{
		if (ImGui::TreeNodeEx(ToUTF8(dirContent[j]).c_str(), treeFlags))
		{
			if (ImGui::Button("Use This Directory"))
				return ToUTF8(dirContent[j]);
			
			return RecursiveTree(dirContent[j], treeFlags);
		}
	}

	ImGui::TreePop();
	return std::string();
}

void SetOutputPath(std::string path)
{
	if (browserOutPath == NULL)
		return;

	if (path.length() > MAX_PATH)
		path = path.substr(0, MAX_PATH);

	for (int i = 0; i < path.length(); i++)
		browserOutPath[i] = path[i];

	//null out the remaining fields (if any)
	for (int i = path.length(); i < MAX_PATH; i++)
		browserOutPath[i] = '\0';
}

void UpdateFileList(std::string directoryPath)
{
	UpdateFileList(directoryPath, fileList, selectionFlags, currentTargetType);
}

void UpdateFileList(std::string directoryPath, std::vector<std::string> * _fileList, bool * _selectionFlags, DataType _dataType) //This function will be called in MainWindo in the future.
{
	std::vector<std::wstring> content;

	for (auto& entry : std::filesystem::directory_iterator(directoryPath))
	{
		if (!entry.is_directory())
		{
			if (CheckFileFormatSupport(ToUTF8(entry.path().wstring()), currentTargetType))
				fileList->push_back(ToUTF8(entry.path().wstring()));
		}
	}

	selectionFlags = new bool[fileList->size()];
	for (int i = 0; i < fileList->size(); i++)
		selectionFlags[i] = defaulSelectionState;
}

void CloseFileBrowser()
{
	//dirTree = std::unique_ptr<DirectoryNode>(nullptr);
	//ImGui::CloseCurrentPopup();

	isBrowserOpen = false;
}

void DrawFileBrowser()
{
	if (!isBrowserOpen)
		return;

	ImGui::SetNextWindowPos(ImVec2(515, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 750), ImGuiCond_FirstUseEver);

	//if (ImGui::BeginPopupModal("Browse", &isBrowserOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
	if (ImGui::BeginPopupModal("Browse", &isBrowserOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings))
	{
		//ImGui::SetNextItemOpen(false, ImGuiCond_Appearing);
		
		ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_NoAutoOpenOnLog;

		if (ImGui::TreeNodeEx("This Computer", treeFlags))
		{
			for (int i = 0; i < availableDrives.size(); i++)
			{
				std::wstring drivePath = L"";
				drivePath += availableDrives[i];
				drivePath += L":\\";

				if (ImGui::TreeNodeEx(ToUTF8(drivePath).c_str(), treeFlags))
				{
					std::string result = RecursiveTree(drivePath, treeFlags);
					if (result.length() > 0)
					{
						SetOutputPath(result);
						UpdateFileList(result);
						CloseFileBrowser();
					}
				}
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::EndPopup();
	}
}

std::string ToUTF8(std::wstring wideString)
{
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, &wideString[0], (int)wideString.size(), NULL, 0, NULL, NULL);
	std::string result(bufferSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wideString[0], (int)wideString.size(), &result[0], bufferSize, NULL, NULL);

	return result;
}

void OpenFileBrowser(char * outPath, std::vector<std::string> * fileListBuffer, bool * selectionFlagsBuffer, DataType dataType) 
{
	isBrowserOpen = true;
	browserOutPath = outPath;
	currentTargetType = dataType;
	fileList = fileListBuffer;
	selectionFlags = selectionFlagsBuffer;

	ImGui::OpenPopup("Browse");
	PopulateDrivesList();
}