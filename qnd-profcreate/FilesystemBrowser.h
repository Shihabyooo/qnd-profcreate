#pragma once

#include <wchar.h>
#include <vector>
#include <filesystem>
#include <Windows.h>
#include <iostream>

#include "imgui.h"


class DirectoryNode
{
public: //TODO rewrite this code to protect variables, add getters of const pointers.
	DirectoryNode(const std::wstring thisPath, const std::vector<std::wstring> thisContent)
	{
		path = thisPath;
		content = thisContent;
		noOfSubNodes = thisContent.size();
		subNodes = new DirectoryNode *[noOfSubNodes];

		for (int i = 0; i < noOfSubNodes; i++) //Set all subnodes to NULL
			subNodes[i] = NULL;
	}

	~DirectoryNode()
	{
		if (subNodes != NULL)
		{
			for (int i = 0; i < noOfSubNodes; i++)
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
void OpenFileBrowser(char * outPath);