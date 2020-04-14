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
	bool SeekCoordsPosition();
	bool CountVertices();
	//bool ExtractPath();
	bool ExtractPaths();

	std::unique_ptr<char> AdvanceToNextTag();
	KMLElement GetCurrentElementValue();
	bool CompareTags(char * tag1, char * tag2) const;
	bool ExtractCoordinatesFromKMLElement(KMLElement * element, int pathID);

private:
	std::fstream kmlFile;
	//std::streampos coordBeginPos;
	//
	//std::string * pathsNames;

	//int * vertsCount;
	//int pathsCount;

	std::string * pathsNames;
	int pathsCount = 0;
	

public: 
//	//double ** verts; //Probably not the smartest idea...
	Array2D * verts;
	bool isPathLoaded;
};