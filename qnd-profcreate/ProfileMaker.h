#pragma once
#include <fstream>
#include <iostream>
#include <functional>

#include "GeoTIFF_Parser.h"
#include "Array2D.h"
#include "KML_Parser.h"
#include "SHP_Parser.h"
#include "Globals.h"
#include "FilesystemBrowser.h" //need to use the std::string ExtractFileName(std::string path) function in it
#include "LogWindow.h"

extern std::string supportedGeometryFormats[];
extern std::string supportedDEMFormats[];

//extern std::vector<std::string> geometryList;
//extern std::string demLocation;

class ProfileMaker
{
public:
	ProfileMaker();
	~ProfileMaker();

	bool BatchProfileProcessing(std::vector<std::string> & geometryList, std::string & demLocation, std::string & outputDirectory, double chainageSteps, InterpolationMethods interpolationMethod, bool maintainBends); //Primarily for use in the GUI implementation.

	bool LoadDEM(std::string demPath); //Currently merely an interface for GeoTIFF_Parser. Need to implement verification of the GeoTIFF's DTM parameters and that the program supports them
	bool LoadGeometry(std::string geometryPath);

	void InterpolateProfile(const double step, const bool maintainBends); //Not to be confused with DEM-to-Profile Z coordinates interpolation. Interpolate Profile is simple linear interpolation.
	int CalculateProfile(InterpolationMethods method); //TODO replace hardcoded Bicubic inteprolation uses and replace with conditional statements based on an argument of type "InterpolationMethods"
	bool WriteProfileToDisk(std::string &out_csv, bool overWrite); 

	void ResetProfile(); //Profile container's memory management is delegated to Array2D classes. This method now only resets associated flags.
	void ResetDEM(); //To be implemented once we start supporing multi-DEM use cases.
	
	void DisplayPath(); //For CLI implementations.

	std::unique_ptr<DEMSummary> GetDEMSummary(std::string & demLocation);

private: 
	//Utilities
	FileFormat DetermineFileFormat(std::string geometryPath);
	bool FileIsExist(std::string location) const;
	std::string AppendSuffixToFileName(std::string &path, unsigned int maxSuffix); //maxSuffix is the *exclusive* upper limit to numerical suffix to be appended.
	double CalculateDistance(double x1, double y1, double x2, double y2, bool isUTM); //Calculates distance between two points based isUTM. True = simple cartesian calc, False = Use Vincenty's Formulae.
	bool IsPathOOB(); //Checks that stored paths is within DEM's boundaries. Test either "profile" or "profile_i" depending on state of isInterpoalted.
	bool IsPointOOB(double x, double y); //Checks whether a coordinate set is within the DEM's boundaries. Does *not* consider CRS differences. 
	bool CheckDEMLoaded(std::string demPath);


	//DEM-to-Profile interpolation
	double InterpolatePointHeight(unsigned long int first_larger_x, unsigned long int first_larger_y, unsigned long int point_order, InterpolationMethods method);
	double BilinearInterpolation(unsigned long int first_larger_x, unsigned long int first_larger_y, unsigned long int point_order); //TODO switch variables' storage from float to double.
	double BicubicInterpolation(unsigned long int first_larger_x, unsigned long int first_larger_y, unsigned long int point_order);
	double NearestNeighbourInterpolation(unsigned long int first_larger_x, unsigned long int first_larger_y, unsigned long int point_order); //TODO implement this
	
	//CRS conversion
	std::unique_ptr<double> ToUTM(double lng, double lat) const;	//converts a set of geoegraphic CRS (WGS84) coordinates to project UTM coordinates.
	std::unique_ptr<double> ToWGS84(double easting, double northing, bool isNortherHemisphere, int zone) const;	//converts a set of projected UTM coordinates to geographic WGS84 coordinates.
	void ConvertPathToUTM();
	void ConvertPathToWGS84(); //TODO merge ConvertPathToUTM() and ConvertPathToWGS84() into a single method that takes a bool toUTM to determine conversion direction.

private: 
	//Note that profile and profile_i are 4 column arrays, the first 3 are x, y, z coords, the last is the distance between each point and the previous one (equal to zero for first point).
	Array2D profile;
	Array2D profile_i;

	bool isInterpolated;
	bool isCalculated;
	bool isPathUTM;
	int pathZone = 36; //TODO reset this to an uninitialized variable
	bool isPathInNorthernHemisphere = true;  //TODO reset this to an uninitialized variable
	std::string loadedDEMPath = ""; //Probably a bad way to go about this problem. This value will store the currently loaded DEM path, to avoid loading the same DEM multiple times with successive calls
									//to BatchProfileProcessing(). 

	//Note: the xxxParsers will be instantiated only as required throughout the program's lifetime. It's possible to simplify LoadGeometry() by always instantiating all parsers in the constructor, at (argubly insignificant) memory cost.
	KMLParser * kmlParser;
	SHPParser * shpParser;
	//CSVParser * csvParser;
	FileParser * geometryParser; //Being the parent of the xxxParser classes, this will be used as an abstract when actually loading the geometry.
};

bool CheckFileFormatSupport(std::string path, DataType dataType);
