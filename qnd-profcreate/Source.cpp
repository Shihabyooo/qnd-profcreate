#include "main.h"

//bool isDebug = true;
//bool maintainBendLocations = false; //for when interpolating profile


//Known issues:
//When converting path to UTM, it doesn't retain the zone. Therefor, a path can be outside the bounds of dem, yet the IsOOB check
//would still pass if easting and northing is withing east/north of DEM.

ProfileMaker instance;

//void GenerateProfile(std::string geometryPath, float interpolationSteps);


int main(int argc, char *argv[])
{
	std::string settingsPath = "QnD_Config.ini";
	
	if (!LoadSettings(settingsPath)) //if no current settings files is found, create a new one based on default values.
		if (!WriteCurrentSettings(settingsPath)) //if we could not create the file, warn user
		{
			MessageBox(NULL, L"Could not create a settings file. Using built-in default settings. Any changes to the settings during this run may not be saved for future runs,", L"WARNING!", 0);
		}


	int result = StartGUI(&instance);
	std::cout << "GUI result: " << result; //test



	///*SHPParser shp;
	//shp.LoadSHP("2.shp");*/
	////std::string demLocation = "DEM.tif";
	////std::string demLocation = "dem_GCS.tif"; //test
	////std::string demLocation = "wdem_uncomp.tif"; //test
	//std::string demLocation = "wdem_packbits.tif"; //test
	//
	//argc = 2;//test
	//argv[1] = "2.shp";//test
	////argv[2] = "ATEST_GEO_GEART.kml";//test
	//
	//std::cout << "\n Enter Interpolation steps: ";
	//float interpolationStep = 100.0f;
	//std::cin >> interpolationStep;
	//
	//std::cout << "\nLoading DEM\n\n";
	//if (!instance.LoadDEM(demLocation))
	//{
	//	std::cout << "\nPress enter to exit.";
	//	std::cin.sync();
	//	std::cin.get();
	//	exit(0);
	//}
	//std::cout << "\nDEM Info\n\n";
	////instance.DisplayDEMInfo();
	//
	//for (int i = 1; i < argc; i++)
	//{
	//	std::string geometryPath = argv[i];
	//	GenerateProfile(geometryPath, interpolationStep);
	//	std::cout << "Finished extracting profile for " << geometryPath << std::endl;
	//}
	//
	//
	//std::cout << "\n\nPress Enter to continue";
	////std::cin.sync();
	////std::cin.get();

	return result;
}

//void GenerateProfile(std::string geometryPath, float interpolationSteps)
//{
//	std::string outputPath;
//
//	std::cout << "\n Processing file:" << geometryPath << std::endl;;
//	outputPath = geometryPath + ".csv";
//
//	std::cout << "\nLoading geometry\n\n";
//	if (!instance.LoadGeometry(geometryPath))
//	{
//		std::cout << "\nPress enter to exit.";
//		std::cin.sync();
//		std::cin.get();
//		exit(0);
//	}
//
//	if (isDebug)
//	{
//		std::cout << "\nBefore interpolation\n\n";
//		instance.DisplayPath();
//	}
//
//	std::cout << "\nInterpolating Profile\n\n";
//	instance.InterpolateProfile(interpolationSteps, maintainBendLocations);
//
//	std::cout << "\nCalculating Profile\n\n";
//	instance.CalculateProfile();
//
//	if (isDebug)
//	{
//		std::cout << "\nAfter Z calculations\n\n";
//		instance.DisplayPath();
//	}
//
//	std::cout << "\nWriting\n\n";
//	instance.WriteProfileToDisk(outputPath, false);
//
//	std::cout << "\nPrepping for Next Path\n\n";
//	instance.ResetProfile();
//}
