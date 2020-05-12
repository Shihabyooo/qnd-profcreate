#pragma once
#include "Array2D.h"

#define UTM_FALSE_EASTING (double)(500000.0f)
#define UTM_FALSE_NORTHING (double)(10000000.0f)
#define UTM_MERIDIAN_SCALE (double)(0.9996f)
#define PI_CONSTANT (double)(3.14159265359f)
#define WGS84_EARTH_RADIUS_EQUATOR (double)(6378137.0f)
#define WGS_EARTH_RADIUS_POLES (double)(6356752.3142f)
#define WGS84_ELIPSOID_FLATTENING (double)(1.0f/298.257223563f)

extern bool isDebug;
extern bool defaulSelectionState;

enum class DataType
{
	dem, geometry
};

enum class FileFormat
{
	shapeFile, kml, csv, unsupported
};

enum class InterpolationMethods //IMPORTANT! Do not change this enum without updating its uses in MainWindow.cpp -> DrawInterpolationMethods. It depends critically on the values of the enums.
{
	nearestNeighbour = 0, bilinear = 1, bicubic =2
};

enum class CRS
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
