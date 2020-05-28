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

#define WINDOW_MAIN_MIN_WIDTH 300
#define WINDOW_MAIN_MAX_WIDTH 500
#define WINDOW_MAIN_HEIGHT_PERCENTAGE 1.0f

#define WINDOW_LOG_MIN_WIDTH 500
//#define WINDOW_LOG_MAX_WIDTH 
#define WINDOW_LOG_HEIGHT_PERCENTAGE 0.3f

#define PROGRAMNAME		_T("QnD_ProfCreate")
#define WINDOWNAME		_T("QnD Profile Creator")

#define WINDOW_INIT_FAIL 100

//extern bool isDebug;
//extern bool defaulSelectionState;

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

struct DEMSummary //To be used for GUI, since the info required are spread over two structs (TIFFDetails and GeoTIFFDetails) but we require one struct to be retured from a ProfileMaker method.
{
public:
	DEMSummary() {}
	DEMSummary(unsigned long int _width, unsigned long int _height, std::string & _compressionMethod, unsigned int _crsCode, std::string & _crsCitation, double _boundingRect[4], bool _isSupported)
	{
		width = _width;
		height = _height;
		compressionMethod = _compressionMethod;
		crsCode = _crsCode;
		crsCitation = _crsCitation;
		
		for (int i = 0; i < 4; i++)
			boundingRect[i] = _boundingRect[i];

		isSupported = _isSupported;
	}


	unsigned long int width;
	unsigned long int height;
	std::string compressionMethod;
	
	//CRS crsType;
	unsigned int crsCode;
	std::string crsCitation;
	double boundingRect[4]; //MinX, minY, maxX, maxY

	bool isSupported; //Since current implementation of GeoTIFF-Parser is highly limited in its decompressor support.
};

struct ProcessingOrder
{
public:
	ProcessingOrder() {}
	
	ProcessingOrder(std::vector<std::string> * _geometryList,
					std::string * _demLocation,
					std::string * _outputDirectory,
					double _chainageSteps,
					InterpolationMethods _interpolationMethod,
					bool _maintainBends,
					bool _processAllSubGeometries,
					bool _overwriteOutputFile,
					int _outputCRS,
					CRS _outputCRSOverride = CRS::WGS84)
	{
		geometryList = _geometryList;
		demLocation = _demLocation;
		outputDirectory = _outputDirectory;
		chainageSteps = _chainageSteps;
		InterpolationMethods interpolationMethod = _interpolationMethod;
		maintainBends = _maintainBends;
		processAllSubGeometries = _processAllSubGeometries; //for geometry files that contain multiple paths, if false: process only first path, else process all paths.
		overwriteOutputFile = _overwriteOutputFile;
		outputCRS = _outputCRS; //0: use DEM CRS, 1: use original geometry CRS, 2: use override CRS.
		outputCRSOverride = _outputCRSOverride;
	}

	~ProcessingOrder() {} 



	std::vector<std::string> * geometryList = NULL;
	std::string * demLocation = NULL;
	std::string * outputDirectory = NULL;
	double chainageSteps;
	InterpolationMethods interpolationMethod = InterpolationMethods::nearestNeighbour;

	//other flags
	bool maintainBends = false;
	bool processAllSubGeometries = false; //for geometry files that contain multiple paths, if false: process only first path, else process all paths.
	bool overwriteOutputFile = false;
	int outputCRS = 0; //0: use DEM CRS, 1: use original geometry CRS, 2: use override CRS.
	CRS outputCRSOverride = CRS::WGS84;

};


class FileParser
{
public:
	FileParser() {};
	~FileParser() {};

	virtual bool LoadGeometry(std::string &fileName) { return false; };
	virtual void UnLoadGeometry() {};

	virtual Array2D const * const GetPathByID(int id) { return nullptr; };
	virtual bool IsPathLoaded() { return false; };

	virtual CRS GeometryCRS() { return geometryCRS; };
	virtual unsigned int UTMZone() { return 0; };
	virtual bool IsNorthernHemisphere() { return true; };

public:
	const FileFormat parserSupportedFormat = FileFormat::unsupported;

private:
	CRS geometryCRS = CRS::undefined;
	unsigned int zone; //For use with UTM CRS only
	bool isNorthernHemisphere; //For use with UTM CRS only
};

