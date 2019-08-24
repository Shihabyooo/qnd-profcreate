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
	//bool IsDecimal;
	bool WGS84;
};


class ProfileMake
{
public:
	ProfileMake();
	~ProfileMake();

	bool PLoadDEM(std::string);
	bool PLoadKML(std::string); //checks kml exists, valid, calculates P_PathVertices to dynamic allocate arrays for PExtractPath()
	bool PLoadCSV(std::string);
	bool PManualPath(double, double);
	
	void PDisplayDEMInfo();
	void PDisplayPathInfo();

	void PInterpolateProfile(float, bool); //implement your profile interpolator algorithm here, float is step
	bool PIsPathOOB(); //checks that profile is within DEM
	bool PIsPointOOB(double, double);
	int PCalculateProfile();
	void PCalculatePoint(double, double); //mainly for testing purposes. Might implement later in PCalculateProfile() to simplify it.
	

	bool PWriteProfile(std::string);

	double PCalculateDistance(double, double, double, double); //calculates distance for non-projected coords

	void PResetProfile();
	void PResetDEM();

	//TODO Add getters method to transport the results outside this class to be used when implementing
	//the designer and setting-out-sheets-er
	//TODO When doing that, I suggest you do the two implementations as external classes (for modulerization sake).

private:
	bool PExtractPath();
	void PSetDEMInfo(); //carefull not to call this function outside PLoadDEM, because the DEM is unloaded at end of PLoadDem and... well....
						//TODO Implement this one alongside PDisplayDEMInfo() later.
	float PBilinearInterp(int, int, int);
	float PBicubicInterp(int, int, int);
	bool PFileIsExist(std::string);
	double* PToUTM(double, double);
	void PConvertPathToUTM();


	std::string P_KMLLocation;
	std::string P_DEMLocation;
	std::string P_OutuputLocation;

	//std::ifstream P_Path;
	std::ofstream P_Result;
	
	double * P_X;
	double * P_Y;
	double * P_Xi;
	double * P_Yi;
	float * P_Z;
	float * P_PathLength; //why is this a thing? Answer: conserve computational resources.
	float * P_PathLengthI;

	bool P_IsInterpolated;
	bool P_IsCalculated;
	bool P_IsConverted;
	//bool P_IsUTM; //redundant. Value is stored in DEMInfo struct
	
	float ** P_Heights;

	GDALDataset * P_DEM;
	GDALRasterBand * P_DEMBand;
	DEM_Info P_DEMInfo;

	int P_PathVertices;
	int P_PathVerticesI;

	KMLParser P_Path;

};