#pragma once
#include <fstream>
#include <iostream>
#include <functional>
#include "main.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "Array2D.h"
#include "KML_Parser.h"
#include "SHP_Parser.h"
#include "Globals.h"



struct DEMInfo
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



class ProfileMaker
{
public:
	ProfileMaker();
	~ProfileMaker();

	bool LoadDEM(std::string);
	bool LoadGeometry(std::string);

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
	void ResetDEM();

private:
	void SetDEMInfo();
	FileFormat DetermineFileFormat(std::string geometryPath);

	float BilinearInterp(int, int, int);
	float BicubicInterp(int, int, int);
	bool FileIsExist(std::string);
	double* ToUTM(double, double);
	void ConvertPathToUTM();

	std::ofstream result;

	//Note that profile and profile_i are 4 column arrays, the first 3 are x, y, z coords, the last is the distance between each point and the previous one (equal to zero for first point).
	Array2D profile;
	Array2D profile_i;
	Array2D heightsGrid;

	bool isInterpolated;
	bool isCalculated;
	bool isPathUTM;

	GDALDataset * demDataset;
	GDALRasterBand * demBand;
	DEMInfo demInfo;

	KMLParser * kmlParser;
	SHPParser * shpParser;
	//CSVParser * csvParser;
	FileParser * geometryParser;
};