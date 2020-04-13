#pragma once
#include <iostream>
#include <math.h>
#include <iomanip>
#include <fstream>
#include <string>
#include <functional>
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "KML_Parser.h"
#include "SHP_Parser.h"
#include "Array2D.h"


//TODO move ProfileMake into its own header/source files, make main.h/.cpp contain only multiply-used utility functions (like BytestoType()) enums (CRS?) and structs.
//TODO make LoadKML() and the undefined LoadCSV private and add LoadSHP to them, add a public method called LoadGeometry that takes a string (file location), determines file format, and
//calls appropriate method.

extern bool isDebug;

struct DEM_Info
{
	int x, y;
	int RasterCount;
	int BlockSize_x, BlockSize_y;
	int OverviewCount;
	int ColorEntryCount;

	std::string ProjectionReference;
	std::string RasterDataType;
	std::string ColorInterpretation;

	double originx, originy;
	double PixelSize_x, PixelSize_y;
	double geotransform_2, geotransform_4; //TODO find out what these figures stand for
	double z_max, z_min;

	double	NW_x, NW_y, 
			NE_x, NE_y,
			SE_x, SE_y,
			SW_x, SW_y;
	bool IsUTM;
	bool WGS84;
};


class ProfileMake
{
public:
	ProfileMake();
	~ProfileMake();

	bool LoadDEM(std::string);
	bool LoadKML(std::string); //checks kml exists, valid, calculates pathVerts to dynamic allocate arrays for PExtractPath()
	bool LoadCSV(std::string);
	bool ManualPath(double, double);
	
	void DisplayDEMInfo();
	void DisplayPathInfo();

	void InterpolateProfile(float, bool); //implement your profile interpolator algorithm here, float is step
	bool IsPathOOB(); //checks that profile is within DEM
	bool IsPointOOB(double, double);
	int CalculateProfile();
	void CalculatePoint(double, double); //mainly for testing purposes. Might implement later in CalculateProfile() to simplify it.
	

	bool WriteProfile(std::string);

	double CalculateDistance(double, double, double, double); //calculates distance for non-projected coords

	void ResetProfile();
	//void ResetDEM();

private:
	void SetDEMInfo(); //carefull not to call this function outside LoadDEM, because the DEM is unloaded at end of LoadDEM and... well...
	float BilinearInterp(int, int, int);
	float BicubicInterp(int, int, int);
	bool FileIsExist(std::string);
	double* ToUTM(double, double);
	void ConvertPathToUTM();

	std::string kmlLocation;
	std::string demLocation;
	std::string outputLocation;

	std::ofstream result;
	
	//Note that profile and profile_i are 4 column arrays, the first 3 are x, y, z coords, the last is the distance between each point and the previous one (equal to zero for first point).
	Array2D profile;
	Array2D profile_i;
	Array2D heightsGrid;

	bool isInterpolated;
	bool isCalculated;
	bool isConverted;
	
	GDALDataset * demDataset;
	GDALRasterBand * demBand;
	DEM_Info demInfo;

	KMLParser P_Path;
};