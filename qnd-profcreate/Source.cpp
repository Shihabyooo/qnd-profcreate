#include "main.h"
#include "GUIHandler.h"

bool isDebug = true;
bool maintainBendLocations = false; //for when interpolating profile


//Known issues:
//When converting path to UTM, it doesn't retain the zone. Therefor, a path can be outside the bounds of dem, yet the IsOOB check
//would still pass if easting and northing is withing east/north of DEM.

ProfileMaker instance;

void GenerateProfile(std::string geometryPath, float interpolationSteps);


void main(int argc, char *argv[])
{

	int result = StartGUI();
	std::cout << "GUI result: " << result; //test

	/*SHPParser shp;
	shp.LoadSHP("2.shp");*/
	//std::string demLocation = "DEM.tif";
	//std::string demLocation = "dem_GCS.tif"; //test
	//std::string demLocation = "wdem_uncomp.tif"; //test
	std::string demLocation = "wdem_packbits.tif"; //test

	argc = 2;//test
	argv[1] = "2.shp";//test
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

	for (int i = 1; i < argc; i++)
	{
		std::string geometryPath = argv[i];
		GenerateProfile(geometryPath, interpolationStep);
		std::cout << "Finished extracting profile for " << geometryPath << std::endl;
	}


	std::cout << "\n\nPress Enter to continue";
	//std::cin.sync();
	//std::cin.get();
}

void GenerateProfile(std::string geometryPath, float interpolationSteps)
{
	std::string outputPath;

	std::cout << "\n Processing file:" << geometryPath << std::endl;;
	outputPath = geometryPath + ".csv";

	std::cout << "\nLoading geometry\n\n";
	if (!instance.LoadGeometry(geometryPath))
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

	std::cout << "\nPrepping for Next Path\n\n";
	instance.ResetProfile();
}
