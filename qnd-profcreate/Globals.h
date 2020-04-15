#pragma once
#include "Array2D.h"


extern bool isDebug;

enum FileFormat
{
	shapeFile, kml, csv, unsupported
};

enum CRS
{
	WGS84, UTM, undefined
};

class FileParser
{
public:
	FileParser() {};
	~FileParser() {};

	virtual bool LoadGeometry(std::string fileName) { return false; };
	virtual void UnLoadGeometry() {};

	virtual Array2D const * const GetPathByID(int id) { return nullptr; };
	virtual bool IsPathLoaded() { return false; };

	virtual CRS GeometryCRS() { return geometryCRS; };

public:
	const FileFormat parserSupportedFormat = FileFormat::unsupported;

private:
	CRS geometryCRS = CRS::undefined;
};