#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "Array2D.h"


#define KML_POLYLINE_TAG "<LineString>"
#define KML_NAME_TAG "<name>"
#define KML_COORDINATES_TAG "<coordinates>"

struct KMLElement
{
public:
	KMLElement()
	{
		ptrToData = NULL;
		dataLength = 0;
	}

	std::unique_ptr<char> ptrToData;
	long long int dataLength;
};

class KMLParser
{
public:
	KMLParser();
	~KMLParser();

	bool LoadKML(std::string);
	void UnloadKML();
	
private:
	bool OpenKMLFile(std::string);
	void CloseKMLFile();
	bool ExtractPaths();

	std::unique_ptr<char> AdvanceToNextTag();
	KMLElement GetCurrentElementValue();
	bool CompareTags(char * tag1, char * tag2) const;
	void ExtractNameFromKMLElement(KMLElement * element, int pathID);
	bool ExtractCoordinatesFromKMLElement(KMLElement * element, int pathID);


private:
	std::fstream kmlFile;

	std::string * pathsNames;
	int pathsCount = 0;
	

public: 
	Array2D * verts;
	bool isPathLoaded;
};