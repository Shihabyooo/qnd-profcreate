#include "main.h"
//#include <list>

bool isDebug = false;

//Known issues:
//When converting path to UTM, it doesn't retain the zone. Therefor, a path can be outside the bounds of dem, yet the IsOOB check
//would still pass if easting and northing is withing east/north of DEM.

void main(int argc, char *argv[])
{
	//populate a list with drains-to-process
	//std::list<std::string> drainList;


	std::cout << "\n Enter Interpolation steps: ";
	float interp_steps = 100.0f;
	std::cin >> interp_steps;

	for (int i = 1; i < argc; i++)
	{

		std::string pathKML, outputFile;
		//pathKML = "Drain.kml";
		//pathKML = "drain.kml"; //test

		//if (argc > 1) 
			pathKML = argv[i];
		
		std::cout << "\n Processing file:" << pathKML << std::endl;;

		outputFile = pathKML + ".csv";
		
		//TODO modify ProfileMake class and add reset functionality. Instantiate once outside loop here, and reset at end
		//move DEM loading and displaying demdata outside as well (so it's only done once).

		ProfileMake instance;

		std::cout << "\nLoading DEM\n\n";
		if (!instance.PLoadDEM("DEM.tif"))
		{
			std::cout << "\nPress enter to exit.";
			std::cin.sync();
			std::cin.get();
			exit(0);
		}

		std::cout << "\nLoading KML\n\n";
		if (!instance.PLoadKML(pathKML))
		{
			std::cout << "\nPress enter to exit.";
			std::cin.sync();
			std::cin.get();
			exit(0);
		}

		std::cout << "\nDEM Info\n\n";
		instance.PDisplayDEMInfo();

		if (isDebug)
		{
			std::cout << "\nBefore interpolation\n\n";
			instance.PDisplayPathInfo();
		}
		std::cout << "\nInterpolating Profile\n\n";
		instance.PInterpolateProfile(interp_steps);

		std::cout << "\nCalculating Profile\n\n";
		instance.PCalculateProfile();

		if (isDebug)
		{
			std::cout << "\nAfter Z calculations\n\n";
			instance.PDisplayPathInfo();
		}

		std::cout << "\nWriting\n\n";
		instance.PWriteProfile(outputFile);

	}
	std::cout << "Press Enter to continue";
	//std::cin.ignore();
	//std::cin.ignore(); //dunno why, but for some reason the first cin.ignore executes immediatly. 
	std::cin.sync();
	std::cin.get();
}
