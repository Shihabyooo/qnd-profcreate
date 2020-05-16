#pragma once
#include "Array2D.h"

//Mapping related defines.
#define UTM_FALSE_EASTING (double)(500000.0f)
#define UTM_FALSE_NORTHING (double)(10000000.0f)
#define UTM_MERIDIAN_SCALE (double)(0.9996f)
#define PI_CONSTANT (double)(3.14159265359f)
#define WGS84_EARTH_RADIUS_EQUATOR (double)(6378137.0f)
#define WGS_EARTH_RADIUS_POLES (double)(6356752.3142f)
#define WGS84_ELIPSOID_FLATTENING (double)(1.0f/298.257223563f)

//GUI related defines
#define DEFAULT_WINDOW_WIDTH 1024 //XGA width
#define DEFAULT_WINDOW_HEIGHT 768 //XGA height

#define WINDOW_MAIN_MIN_WIDTH 300
#define WINDOW_MAIN_MAX_WIDTH 500
#define WINDOW_MAIN_HEIGHT_PERCENTAGE 1.0f

#define WINDOW_LOG_MIN_WIDTH 500
//#define WINDOW_LOG_MAX_WIDTH 
#define WINDOW_LOG_HEIGHT_PERCENTAGE 0.3f



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

struct WindowDimensions
{
public:
	WindowDimensions() {}
	WindowDimensions(long int _positionX, long int _positionY, long int _width, long int _height)
	{
		positionX = _positionX;
		positionY = _positionY;
		width = _width;
		height = _height;
	}

	long int positionX;
	long int positionY;
	long int width;
	long int height;
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

