#pragma once
#include <wchar.h>
#include <vector>
#include <filesystem>
#include <Windows.h>
#include <iostream>

#include "imgui.h"
#include "ProfileMaker.h"
#include "Globals.h"

class DirectoryNode
{
public: //TODO rewrite this code to protect variables, add getters of const pointers.
	DirectoryNode(const std::wstring thisPath, const std::vector<std::wstring> thisContent)
	{
		path = thisPath;
		content = thisContent;
		noOfSubNodes = thisContent.size();
		subNodes = new DirectoryNode *[noOfSubNodes];

		for (unsigned long int i = 0; i < noOfSubNodes; i++) //Set all subnodes to NULL
			subNodes[i] = NULL;
	}

	~DirectoryNode()
	{
		if (subNodes != NULL)
		{
			for (unsigned long int i = 0; i < noOfSubNodes; i++)
				delete subNodes[i];
			delete[] subNodes;
		}
	}

	std::wstring path;
	std::vector<std::wstring> content;
	DirectoryNode ** subNodes = NULL;
	unsigned long int noOfSubNodes = 0;
};


std::string ToUTF8(std::wstring wideString);
void DrawFileBrowser();
void OpenFileBrowserSimple(char * outPath); //Shows a browser, and updates outPath upon selecting a directory without doing any list updates.
void OpenFileBrowser(char * outPath, std::vector<std::string> * fileListBuffer, std::unique_ptr<bool> * selectionFlagsBuffer,  DataType dataType, bool _defaultSelectionState = true);
void UpdateFileList(std::string directoryPath, std::vector<std::string> * _fileList, std::unique_ptr<bool> * _selectionFlags, DataType _dataType);

std::string ExtractFileName(std::string path);
std::string ExtractParentDirectoryPath(std::string filePath);

bool IsDirectoryAccessible(const std::wstring path);
bool IsDirectoryAccessible(const std::string path);