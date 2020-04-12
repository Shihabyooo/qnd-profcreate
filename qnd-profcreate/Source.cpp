#include "main.h"

bool isDebug = true;
bool maintainBendLocations = false; //when interpolating profile


//Known issues:
//When converting path to UTM, it doesn't retain the zone. Therefor, a path can be outside the bounds of dem, yet the IsOOB check
//would still pass if easting and northing is withing east/north of DEM.

ProfileMake instance;

void GenerateProfile(std::string geometryPath, int interpolationSteps);


void main(int argc, char *argv[])
{

	/*SHPParser shp;
	shp.LoadSHP("2.shp");*/
	//std::string demLocation = "DEM.tif";
	std::string demLocation = "WDEM.tif"; //test

	argc = 2;//test
	argv[1] = "2.kml";//test
	//argv[2] = "ATEST_GEO_GEART.kml";//test
	
	std::cout << "\n Enter Interpolation steps: ";
	float interpolationStep = 100.0f;
	std::cin >> interpolationStep;
	
	std::cout << "\nLoading DEM\n\n";
	if (!instance.LoadDEM(demLocation))
	{
		std::cout << "\nPress enter to exit.";
		std::cin.sync();
		std::cin.get();
		exit(0);
	}
	std::cout << "\nDEM Info\n\n";
	//instance.DisplayDEMInfo();


	//TODO: Move calls to ProfileMake methods to its own function that recieves name of input file
	//and returns false if any of the calls used bellow return false. In which case the loop just cycles
	//to next kml.

	for (int i = 1; i < argc; i++)
	{
		std::string geometryPath = argv[i];
		GenerateProfile(geometryPath, interpolationStep);
	}


	std::cout << "\n\nPress Enter to continue";
	//std::cin.sync();
	//std::cin.get();
}

void GenerateProfile(std::string geometryPath, int interpolationSteps)
{
	std::string outputPath;

	std::cout << "\n Processing file:" << geometryPath << std::endl;;
	outputPath = geometryPath + ".csv";

	std::cout << "\nLoading KML\n\n";
	if (!instance.LoadKML(geometryPath))
	{
		std::cout << "\nPress enter to exit.";
		std::cin.sync();
		std::cin.get();
		exit(0);
	}

	if (isDebug)
	{
		std::cout << "\nBefore interpolation\n\n";
		instance.DisplayPathInfo();
	}

	std::cout << "\nInterpolating Profile\n\n";
	instance.InterpolateProfile(interpolationSteps, maintainBendLocations);

	std::cout << "\nCalculating Profile\n\n";
	instance.CalculateProfile();

	if (isDebug)
	{
		std::cout << "\nAfter Z calculations\n\n";
		instance.DisplayPathInfo();
	}

	std::cout << "\nWriting\n\n";
	instance.WriteProfile(outputPath);

	//std::cout << "\nPrepping for Next Path\n\n";
	instance.ResetProfile();
}