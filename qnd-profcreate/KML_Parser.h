#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "Array2D.h"
#include "Globals.h"

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

class KMLParser : public FileParser
{
public:
	KMLParser();
	~KMLParser();

	bool LoadGeometry(std::string &fileName);
	void UnLoadGeometry();
	
	Array2D const * const GetPathByID(int id);
	bool IsPathLoaded();

	virtual CRS GeometryCRS();

private:
	bool OpenKMLFile(std::string &fileName);
	void CloseKMLFile();
	bool ExtractPaths();

	std::unique_ptr<char> AdvanceToNextTag();
	KMLElement GetCurrentElementValue();
	bool CompareTags(char * tag1, char * tag2) const;
	void ExtractNameFromKMLElement(KMLElement * element, int pathID);
	bool ExtractCoordinatesFromKMLElement(KMLElement * element, int pathID);

public:
	const FileFormat parserSupportedFormat = FileFormat::kml;

private:
	std::fstream kmlFile;
	bool isPathLoaded;

	Array2D * verts;
	std::string * pathsNames;
	long int pathsCount = 0;

	CRS geometryCRS = CRS::WGS84; //KML is always WGS84
	unsigned int zone; //For use with UTM CRS only
	bool isNorthernHemisphere; //For use with UTM CRS only
};