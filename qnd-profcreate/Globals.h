#pragma once
#include "Array2D.h"

#define UTM_FALSE_EASTING (double)(500000.0f)
#define UTM_FALSE_NORTHING (double)(10000000.0f)
#define UTM_MERIDIAN_SCALE (double)(0.9996f)
#define PI_CONSTANT (double)(3.14159265359f)
#define WGS84_EARTH_RADIUS_EQUATOR (double)(6378137.0f)
#define WGS_EARTH_RADIUS_POLES (double)(6356752.3142f)
#define WGS84_ELIPSOID_FLATTENING (double)(1.0f/298.257223563f)


//const double Pi = 3.14159265359;
//const double a = 6378137; //WGS84 earth radius at equator
//const double f = 1 / 298.257223563; //WGS84 elipsoid flattening
//const double b = 6356752.3142; //WGS84 radius at poles
//const double utm_scale_at_meridian = 0.9996;
//const double falseEasting = 500000, falseNorthing = 10000000;

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