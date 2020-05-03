#include "ProfileMaker.h"


ProfileMaker::ProfileMaker()
{
	//GDALRegister_GTiff();
	kmlParser = NULL;
	shpParser = NULL;
	geometryParser = NULL;
	//csvParser = NULL;

	isInterpolated = false;
	isCalculated = false;
}

ProfileMaker::~ProfileMaker()
{
	profile.~Array2D();
	profile_i.~Array2D();

	if (kmlParser != NULL)
		delete kmlParser;

	if (shpParser != NULL)
		delete shpParser;

	/*if (csvParser != NULL)
		delete csvParser;*/

	UnloadGeoTIFF();
}

bool ProfileMaker::LoadDEM(std::string inDEMLoc)
{

	if (!LoadGeoTIFF(inDEMLoc))
	{
		std::cout << "Error! Could not load DEM file: " << inDEMLoc << std::endl;
		return false;
	}
			
	//TODO check that geoDetails are set properly and to values this code supports.

	DisplayTIFFDetailsOnCLI();
	DisplayGeoTIFFDetailsOnCLI();
	//DisplayBitmapOnCLI();

	std::cout << "Successfully loaded DEM file: " << inDEMLoc << "\n\n";
	return true;
}

bool ProfileMaker::LoadGeometry(std::string geometryPath)
{
	//determine geometry type
	FileFormat format = DetermineFileFormat(geometryPath);

	//check status of geometryParser pointer, cleanup accordingely
	if (geometryParser != NULL)
	{		
		if (geometryParser->parserSupportedFormat != format) //There was a previously allocated FileParser, and it wasn't of the FileFormat of the current file. So, we delete our old parser
		{													//instance (since we don't store pointers to all alive parsers instances, only currently used instance).
			delete geometryParser;
			geometryParser = NULL;
		}
		else //this means we have an instantiated FileParser, and it is of the same format as the one we already have, so no need to delete anything, just prepare the parser for a new file
			geometryParser->UnLoadGeometry();
	}
	
	if (geometryParser == NULL) //This is a seperate if to accomodate the case of "if (geometryParser->parserSupportedFormat != format)" above.
	{
		switch (format)
		{
		case shapeFile:
			shpParser = new SHPParser();
			geometryParser = shpParser;
			break;
		case kml:
			kmlParser = new KMLParser();
			geometryParser = kmlParser;
			break;
		case csv:
			//csvParser = new CSVParser();
			//geometryParser = csvParser;
			break;
		default:
			std::cout << "ERROR! Loaded filel format is not supported" << std::endl;
			return false;
			break;
		}
	}
	
	//load geometry normally.
	if (!geometryParser->LoadGeometry(geometryPath))
		return false;

	//allocate our local Array2D and initialize the coordinate columns of it using the loaded geometry.
	profile = Array2D(geometryParser->GetPathByID(0)->Rows(), 4);
	profile.Overlay(*geometryParser->GetPathByID(0), 0, 0);

	//set CRS related flags
	switch (geometryParser->GeometryCRS())
	{
	case (CRS::UTM):
		isPathUTM = true;
		break;
	case (CRS::WGS84):
		isPathUTM = false;
		break;
	default:
		break;
	}

	//Fill out the distances column
	profile[0][3] = 0.0f;
	for (int i = 1; i < profile.Rows(); i++)
		profile[i][3] = CalculateDistance(profile[i - 1][0], profile[i - 1][1], profile[i][0], profile[i][1]);

	if (isDebug)
	{
		std::cout << "Input path" << std::endl;
		profile.DisplayArrayInCLI();
	}

	return true;
}

void ProfileMaker::DisplayPathInfo()
{
	if (isInterpolated)
		profile_i.DisplayArrayInCLI();
	else
		profile.DisplayArrayInCLI();
}

void ProfileMaker::InterpolateProfile(float step, bool maintainBends)
{
	int newVertsSum = profile.Rows();

	float totalLength = 0.0f;

	for (int i = 1; i < profile.Rows(); i++)
		totalLength += profile[i][3];

	newVertsSum = floor(totalLength / step) + 2; //the 2 are begining and ending verts

	if (maintainBends)
		newVertsSum += profile.Rows() - 2;

	profile_i = Array2D(newVertsSum, 4);
	profile_i[0][0] = profile[0][0];
	profile_i[0][1] = profile[0][1];


	int currentSegment = 1;
	float lastBendChainage = 0.0f;

	std::function<double(double, double, double, double) > interpolate = [&](double x0, double x1, double l, double dist)->double
	{ //too many methods in this class already, plus I already have other methods with interpolate in the name. This is a small one, leave it as a lambda
		return (dist * x1 / l) + ((l - dist) * x0 / l);
	}; //TODO consider rewriting this to take double[2] and do both xy interpolation at once.

	for (int i = 1; i < profile_i.Rows() - 1; i++)
	{
		if (i*step >= lastBendChainage + profile[currentSegment][3])
		{
			lastBendChainage += profile[currentSegment - 1][3];
			currentSegment++;

			if (maintainBends)
			{
				profile_i[i][0] = profile[i][0];
				profile_i[i][1] = profile[i][1];
				i++;
				if (i >= profile_i.Rows() - 1)
					break;
			}
		}

		float distFromLastBend = (step * i) - lastBendChainage;

		profile_i[i][0] = interpolate(profile[currentSegment - 1][0], profile[currentSegment][0], profile[currentSegment][3], distFromLastBend);
		profile_i[i][1] = interpolate(profile[currentSegment - 1][1], profile[currentSegment][1], profile[currentSegment][3], distFromLastBend);

	}

	profile_i[newVertsSum - 1][0] = profile[profile.Rows() - 1][0]; //copying last verts
	profile_i[newVertsSum - 1][1] = profile[profile.Rows() - 1][1];


	for (int i = 1; i < profile_i.Rows(); i++)
		profile_i[i][3] = CalculateDistance(profile_i[i][0], profile_i[i][1], profile_i[i - 1][0], profile_i[i - 1][1]);

	isInterpolated = true;

	if (isDebug)
		profile_i.DisplayArrayInCLI();
}

bool ProfileMaker::IsPathOOB()
{
	if (isDebug)
	{
		std::cout << "Checking path OOB for DEM boundaries:" << std::endl;
		std::cout << "Min = " << geoDetails.cornerSW[0] << ", " << geoDetails.cornerSW[1] << std::endl;
		std::cout << "Max = " << geoDetails.cornerNE[0] << ", " << geoDetails.cornerNE[1] << std::endl;
	}

	if (isInterpolated)
	{
		for (int i = 0; i < profile_i.Rows(); i++)
		{
			if (isDebug)
				std::cout << "Testing vertex: " << profile_i[i][0] << ", " << profile_i[i][1] << std::endl;

			if (profile_i[i][0] < geoDetails.cornerSW[0] || profile_i[i][0] > geoDetails.cornerNE[0])
			{
				return true;
			}
			if (profile_i[i][1] < geoDetails.cornerSW[1] || profile_i[i][2] > geoDetails.cornerNE[1])
			{
				return true;
			}
		}
	}
	else
	{
		for (int i = 0; i < profile.Rows(); i++)
		{
			if (isDebug)
				std::cout << "Testing vertex: " << profile[i][0] << ", " << profile[i][1] << std::endl;

			if (profile[i][0] < geoDetails.cornerSW[0] || profile[i][0] > geoDetails.cornerNE[0])
			{
				return true;
			}
			if (profile[i][1] < geoDetails.cornerSW[1] || profile[i][2] > geoDetails.cornerNE[1])
			{
				return true;
			}
		}
	}

	return false;
}

bool ProfileMaker::IsPointOOB(double x, double y)
{
	if (x < geoDetails.cornerSW[0] || x > geoDetails.cornerNE[0])
	{
		return false;
	}
	if (y < geoDetails.cornerSW[1] || y >  geoDetails.cornerNE[1])
	{
		return false;
	}
}

int ProfileMaker::CalculateProfile() //returning int for end state. 0: failure, 1: success, 2: success with gaps (for when implementing 
										//choice to calculate profile for paths that are partially within the provided DEM's boundaries.
{
	//in case dem is in UTM
	if (geoDetails.modelType ==  1 && !isPathUTM) //Note that testing modelType == 1 guarantees that the DTM is "Projected," but not necessarily UTM. Must check the projectedCRS for that.
	{
		ConvertPathToUTM();
		isPathUTM = true;
	}
	else if (geoDetails.modelType != 1 && isPathUTM)
	{
		ConvertPathToWGS84();
		isPathUTM = false;
	}


	//check if profile is out of bounds
	if (IsPathOOB()) //TODO remove the exit, return a custom error code, modify calling function to handle the code accordingly
	{
		std::cout << "Error: Loaded path is outside the boundry of the loaded DEM!\n";
		std::cout << "Press Enter to Continue.\n";
		std::cin.sync();
		std::cin.get();
		exit(1);
	}

	int first_larger_x_order, first_larger_y_order;

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		for (int j = 0; j < tiffDetails.height; j++)
		{
			if (profile_i[i][1] > geoDetails.tiePoints[1][1] - j * geoDetails.pixelScale[1])
			{
				first_larger_y_order = j;
				break;
			}
		}
		for (int k = 0; k < tiffDetails.width; k++)
		{
			if (profile_i[i][0] < geoDetails.tiePoints[1][0] + k * geoDetails.pixelScale[0])
			{
				first_larger_x_order = k;
				break;
			}
		}

		profile_i[i][2] = BicubicInterp(first_larger_x_order, first_larger_y_order, i);
	}

	isCalculated = true;

	if (isDebug)
		profile_i.DisplayArrayInCLI();

	return 0;
}

void ProfileMaker::CalculatePoint(double x, double y)
{
	//check if profile is out of bounds
	if (!IsPointOOB(x, y))
	{
		std::cout << "Point is outside the DEM boundries!\n";
		return;
	}

	float result;
	double boundingx[2], boundingy[2];
	float boundingz[4]; //NW -> NE -> SE -> SW
	int first_larger_x_order, first_larger_y_order;
	double A, B; //used as temp holders to clean up bilinear interp forumla

	for (int j = 0; j < tiffDetails.height; j++)
	{
		if (y >  geoDetails.tiePoints[1][1] - j * geoDetails.pixelScale[1])
		{
			boundingy[0] = geoDetails.tiePoints[1][1] - (j - 1) * geoDetails.pixelScale[1];
			boundingy[1] = geoDetails.tiePoints[1][1] - j * geoDetails.pixelScale[1];
			first_larger_y_order = j;
			break;
		}
	}
	for (int k = 0; k < tiffDetails.width; k++)
	{
		if (x < geoDetails.tiePoints[1][0] + k * geoDetails.pixelScale[0])
		{
			boundingx[0] = geoDetails.tiePoints[1][0] + (k - 1) * geoDetails.pixelScale[0];
			boundingx[1] = geoDetails.tiePoints[1][0] + k * geoDetails.pixelScale[0];
			first_larger_x_order = k;
			break;
		}
	}

	boundingz[0] = GetSample(first_larger_x_order - 1, first_larger_y_order - 1, 0);
	boundingz[1] = GetSample(first_larger_x_order, first_larger_y_order - 1, 0);
	boundingz[2] = GetSample(first_larger_x_order, first_larger_y_order, 0);
	boundingz[3] = GetSample(first_larger_x_order - 1, first_larger_y_order, 0);

	if (isDebug)
	{
		std::cout << "Boundingz[0]: " << boundingz[0] << std::endl; //test
		std::cout << "Boundingz[1]: " << boundingz[1] << std::endl; //test
		std::cout << "Boundingz[2]: " << boundingz[2] << std::endl; //test
		std::cout << "Boundingz[3]: " << boundingz[3] << std::endl; //test
		std::cout << "x: " << x << std::endl; //test
		std::cout << "y: " << y << std::endl; //test
		std::cout << "Boundingx[0]: " << boundingx[0] << std::endl; //test
		std::cout << "Boundingx[1]: " << boundingx[1] << std::endl; //test
		std::cout << "Boundingy[0]: " << boundingy[0] << std::endl; //test
		std::cout << "Boundingy[1]: " << boundingy[1] << std::endl; //test
	}

	A = boundingz[0] * (boundingx[1] - x) / (boundingx[1] - boundingx[0]) + boundingz[1] * (x - boundingx[0]) / (boundingx[1] - boundingx[0]);
	B = boundingz[2] * (boundingx[1] - x) / (boundingx[1] - boundingx[0]) + boundingz[3] * (x - boundingx[0]) / (boundingx[1] - boundingx[0]);

	result = A * (boundingy[1] - y) / (boundingy[1] - boundingy[0]) + B * (y - boundingy[0]) / (boundingy[1] - boundingy[0]);

	if (isDebug) std::cout << "Resultant Z: " << result << std::endl; //test
}

bool ProfileMaker::WriteProfile(std::string out_csv)
{
	if (!FileIsExist(out_csv))
		std::cout << "Attempting to create output file\n";

	result.open(out_csv);

	if (!result.is_open())
	{
		std::cout << "Error: failed to create or open file!\n";
		return false;
	}

	std::cout << "File creation is successfull\n";

	std::cout << "\nWriting results to disk" << std::endl;
	result << "Longitude,Latitude,Chainage,Height" << std::endl;

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		result << std::setprecision(10) << profile_i[i][0] << "," << profile_i[i][1] << ",";
		result << std::fixed << std::setprecision(3) << profile_i[i][3];
		result << "," << std::setprecision(4) << profile_i[i][2] << std::endl;
	}

	result.close();
	return true;
}

double ProfileMaker::CalculateDistance(double x1, double y1, double x2, double y2)
{
	double result = 0;
	//distance calculation for UTM coords is very simple, doing it here and returning value immediatly.
	if (isPathUTM)
	{
		result = sqrt(pow(abs(x1 - x2), 2.0) + pow(abs(y1 - y2), 2.0));

		if (isDebug)
			std::cout << "Calculating distance for UTM, result= " << result << std::endl; //test

		return result;
	}

	//distance calculation for decimal degrees using Vincenty's formulae 
	////https://en.wikipedia.org/wiki/Vincenty%27s_formulae#cite_note-opposite-3

	double azim; //azimuth
	double sigma;
	double u1, u2, dx;
	double lambda, lambda_old = 0.0;

	double a = 6378137.0; //WGS84 earth radius at equator
	double f = 1 / 298.257223563; //WGS84 elipsoid flattening
	double b = 6356752.314245; //WGS84 radius at poles

	double sin_sigma, cos_sigma, sin_azim, cos2_azim, cos_2sigmam, c;
	double Pi = 3.14159265359;

	x1 = x1 * Pi / 180; //convert to radians
	x2 = x2 * Pi / 180;
	y1 = y1 * Pi / 180;
	y2 = y2 * Pi / 180;

	u1 = atan((1 - f) * tan(y1));
	u2 = atan((1 - f) * tan(y2));
	dx = x2 - x1;
	lambda = dx;

	//check seems unnecesary.
	if (abs(x1 - x2) * 3600 < 0.0001 && abs(y1 - y1) * 3600 < 0.0001) //hack for when the two points are ~the same
															//otherwise, the algorithm sends weird values (NAN)
	{
		if (isDebug)
			std::cout << "\n[DevWarning: In CalculateDistance(), trigered near-points check, returning 0.0]\n"; //test
		return 0.0;
	}

#pragma region first_lambda_try
	sin_sigma = sqrt(pow((cos(u2)*sin(lambda)), 2.0) + pow((cos(u1)*sin(u2)) - (sin(u1) * cos(u2) * cos(lambda)), 2.0));
	cos_sigma = (sin(u1)*sin(u2)) + (cos(u1) * cos(u2) * cos(lambda));
	sigma = atan2(sin_sigma, cos_sigma);
	sin_azim = (cos(u1) * cos(u2) * sin(lambda)) / sin_sigma;
	cos2_azim = 1 - pow(sin_azim, 2.0);
	cos_2sigmam = cos_sigma - ((2 * sin(u1)*sin(u2)) / cos2_azim);
	c = (f / 16.0) * cos2_azim * (4 + (f * (4 - (3 * cos2_azim))));

	lambda_old = lambda;
	lambda = dx + (1 - c)* f * sin_azim * (sigma + c * sin_sigma * (cos_2sigmam + (c * cos_sigma * (-1 + 2 * pow(cos_2sigmam, 2.0)))));
#pragma endregion first lambda
	// Must calculate a lambda first outside the loop, else sin_sigma, cos_sigma 
	//and the others won't initialize in case longitude is the same (or very close)


	while ((abs(lambda - lambda_old)) > 0.000000000001)
	{
		sin_sigma = sqrt(pow((cos(u2)*sin(lambda)), 2.0) + pow((cos(u1)*sin(u2)) - (sin(u1) * cos(u2) * cos(lambda)), 2.0));
		cos_sigma = (sin(u1)*sin(u2)) + (cos(u1) * cos(u2) * cos(lambda));
		sigma = atan2(sin_sigma, cos_sigma);
		sin_azim = (cos(u1) * cos(u2) * sin(lambda)) / sin_sigma;
		cos2_azim = 1 - pow(sin_azim, 2.0);
		cos_2sigmam = cos_sigma - ((2 * sin(u1)*sin(u2)) / cos2_azim);
		c = (f / 16.0) * cos2_azim * (4 + (f * (4 - (3 * cos2_azim))));

		lambda_old = lambda;
		lambda = dx + (1 - c)* f * sin_azim * (sigma + c * sin_sigma * (cos_2sigmam + (c * cos_sigma * (-1 + 2 * pow(cos_2sigmam, 2.0)))));
	}


	double usqr = cos2_azim * ((pow(a, 2.0) - pow(b, 2.0)) / pow(b, 2.0));
	double A = 1 + ((usqr / 16384) * ((4096 + (usqr * (-768 + (usqr * (320 - (175 * usqr))))))));
	double B = (usqr / 1024) * (256 + (usqr*(-128 + (usqr * (75 - (47 * usqr))))));
	double dsigma = B * sin_sigma * (cos_2sigmam + ((B / 4) * (cos_sigma * (-1 + (2 * pow(cos_2sigmam, 2.0)))) - ((B / 6) * cos_2sigmam)*(-3 + (4 * pow(sin_sigma, 2.0)))*(-3 + (4 * pow(cos_2sigmam, 2.0)))));

	result = b * A * (sigma - dsigma);

	return result;
}

void ProfileMaker::ResetProfile()
{
	isInterpolated = false;
	isCalculated = false;
}

FileFormat ProfileMaker::DetermineFileFormat(std::string geometryPath)
{
	//TODO consider having a const public method in each of the geometry parsing classes that only checks if the provided string is of said file type, which you loop over them here.

	std::string extension = geometryPath.substr(geometryPath.length() - 4, 4);
	std::cout << "file extension: " << extension << std::endl; //test

	if (extension == ".kml" || extension == ".KML")
	{
		return FileFormat::kml;
	}
	else if (extension == ".kmz" || extension == ".KMZ")
	{
		std::cout << "WARNING! compressed KMZ format is not supported yet. Please recreate your path in uncompressed kml format and try again." << std::endl;
		return FileFormat::unsupported;
	}
	else if (extension == ".shp" || extension == ".SHP")
	{
		return FileFormat::shapeFile;
	}
	else if (extension == ".csv" || extension == ".CSV")
	{
		return FileFormat::csv;
	}
	else
	{
		return FileFormat::unsupported;
	}
}

float ProfileMaker::BilinearInterp(int first_larger_x, int first_larger_y, int point_order)
{
	//TODO Check this implementation
	double A, B;
	float  result_depth;

	float boundingz[4];
	double boundingx[2];
	double boundingy[2];
	double pointx, pointy;

	for (int i = 0; i < 2; i++)
	{
		boundingx[i] = geoDetails.tiePoints[1][0] + (first_larger_x - 1 + i) * geoDetails.pixelScale[0];
		boundingy[i] = geoDetails.tiePoints[1][1] - (first_larger_y - 1 + i) * geoDetails.pixelScale[1];
	}

	boundingz[0] = GetSample(first_larger_x - 1, first_larger_y - 1, 0);
	boundingz[1] = GetSample(first_larger_x, first_larger_y - 1, 0);
	boundingz[2] = GetSample(first_larger_x, first_larger_y, 0);
	boundingz[3] = GetSample(first_larger_x - 1, first_larger_y, 0);

	A = boundingz[0] * CalculateDistance(boundingx[1], boundingy[0], profile_i[point_order][0], boundingy[0]) / CalculateDistance(boundingx[1], boundingy[0], boundingx[0], boundingy[0]) + boundingz[1] * CalculateDistance(profile_i[point_order][0], boundingy[0], boundingx[0], boundingy[0]) / CalculateDistance(boundingx[1], boundingy[0], boundingx[0], boundingy[0]);
	B = boundingz[2] * CalculateDistance(boundingx[1], boundingy[1], profile_i[point_order][0], boundingy[1]) / CalculateDistance(boundingx[1], boundingy[1], boundingx[0], boundingy[1]) + boundingz[3] * CalculateDistance(profile_i[point_order][0], boundingy[1], boundingx[0], boundingy[1]) / CalculateDistance(boundingx[1], boundingy[1], boundingx[0], boundingy[1]);
	result_depth = A * CalculateDistance(boundingx[0], boundingy[1], boundingx[0], profile_i[point_order][1]) / CalculateDistance(boundingx[0], boundingy[0], boundingx[0], boundingy[1]);
	result_depth = result_depth + B * CalculateDistance(boundingx[0], profile_i[point_order][1], boundingx[0], boundingy[0]) / CalculateDistance(boundingx[0], boundingy[0], boundingx[0], boundingy[1]);

	return result_depth;
}

float ProfileMaker::BicubicInterp(int first_larger_x, int first_larger_y, int point_order)
{
	//reference:
	//http://www.paulinternet.nl/?page=bicubic

	float result_depth;

	float boundingz[4][4];
	double boundingx[4];
	double boundingy[4];
	double temp_value[4];
	double pointx, pointy;

	for (int i = 0; i < 4; i++)
	{
		boundingx[i] = geoDetails.tiePoints[1][0] + (first_larger_x - 2 + i) * geoDetails.pixelScale[0];
		boundingy[i] = geoDetails.tiePoints[1][1] - (first_larger_y - 2 + i) * geoDetails.pixelScale[1];
		for (int j = 0; j < 4; j++)
		{
			boundingz[i][j] = GetSample(first_larger_x - 2 + i, first_larger_y - 2 + j, 0);
		}
	}

	for (int i = 0; i < 4; i++)
	{
		pointx = abs((profile_i[point_order][0] - boundingx[i]) / boundingx[i]);

		temp_value[i] = boundingz[i][1] + 0.5 * pointx * (boundingz[i][2] - boundingz[i][0] + pointx * (2 * boundingz[i][0] - 5 * boundingz[i][1] + 4 * boundingz[i][2] - boundingz[i][3] + pointx * (3 * (boundingz[i][1] - boundingz[i][2]) + boundingz[i][3] - boundingz[i][0])));
	}

	if (!isInterpolated) //New logic always interpolates the profile before moving on to calculation, this check is unnecessary now.
	{
		pointy = abs((profile_i[point_order][1] - boundingy[1]) / boundingy[1]);
	}
	else
	{
		pointy = abs((profile_i[point_order][1] - boundingy[1]) / boundingy[1]);
	}

	result_depth = temp_value[1] + 0.5 * pointy * (temp_value[2] - temp_value[0] + pointy * (2 * temp_value[0] - 5 * temp_value[1] + 4 * temp_value[2] - temp_value[3] + pointy * (3 * (temp_value[1] - temp_value[2]) + temp_value[3] - temp_value[0])));

	return result_depth;
}

bool ProfileMaker::FileIsExist(std::string location) const
{
	std::cout << "Attempting to open " << location << std::endl;
	std::ifstream file_to_check;

	file_to_check.open(location);
	if (file_to_check.is_open())
	{
		file_to_check.close();
		return true;
	}
	else
	{
		file_to_check.close();
		return false;
	}
}

std::unique_ptr<double> ProfileMaker::ToUTM(double lng, double lat) const
{
	//converting this http://www.movable-type.co.uk/scripts/latlong-utm-mgrs.html
	// to c++
	//also https://www.uwgb.edu/dutchs/UsefulData/UTMFormulas.HTM
	//alternatively https://arxiv.org/pdf/1002.1417.pdf

	//std::cout << "\nDevWarning: Using Karney's method to convert from decimal degrees to UTM\n";

	//const double Pi = 3.14159265359;
	//const double a = 6378137; //WGS84 earth radius at equator
	//const double f = 1 / 298.257223563; //WGS84 elipsoid flattening
	//const double b = 6356752.3142; //WGS84 radius at poles
	//const double utm_scale_at_meridian = 0.9996;
	//const double falseEasting = 500000, falseNorthing = 10000000;

	int zone = floor((lng + 180.0) / 6.0) + 1;
	double central_meridian_longitude = ((zone - 1.0) * 6.0 - 180.0 + 3.0) * PI_CONSTANT / 180.0; //in radians
	//TODO consider Norway/Svalbard exceptions, not really necessary.

	const double e = sqrt(1.0 - pow((WGS_EARTH_RADIUS_POLES / WGS84_EARTH_RADIUS_EQUATOR), 2.0));
	const double n = (WGS84_ELIPSOID_FLATTENING / (2 - WGS84_ELIPSOID_FLATTENING));


	lat = lat * PI_CONSTANT / 180.0;
	lng = (lng * PI_CONSTANT / 180.0) - central_meridian_longitude;

	const double tao = tan(lat);
	const double sigma = sinh(e * atanh(e * tao / sqrt(1.0 + pow(tao, 2.0)))); //checked with source paper
	const double tao_prime = (tao * sqrt(1.0 + pow(sigma, 2.0))) - (sigma * sqrt(1.0 + pow(tao, 2.0))); //checked with source paper
	const double xi_prime = atan(tao_prime / cos(lng)); //checked with source paper
	const double eta_prime = asinh(sin(lng) / sqrt(pow(tao_prime, 2.0) + pow(cos(lng), 2))); //checked with source
	const double A = (WGS84_EARTH_RADIUS_EQUATOR / (1.0 + n)) * (1.0 + (1.0 / 4.0)*pow(n, 2.0) + (1.0 / 64.0)*pow(n, 4.0) + (1.0 / 256.0)*pow(n, 6.0)); //checked up to n^4 in source paper

	double alpha[6] = { (1.0 / 2.0)*n - (2.0 / 3.0) * pow(n,2.0) + (5.0 / 16.0) * pow(n,3.0) + (41.0 / 180.0)*pow(n,4.0) - (127.0 / 288.0)*pow(n,5.0) + (7891.0 / 37800.0)*pow(n, 6.0),
		(13.0 / 48.0) * pow(n, 2.0) - (3.0 / 5.0) * pow(n, 3.0) + (557.0 / 1440.0)*pow(n, 4.0) + (281.0 / 630.0)*pow(n, 5.0) - (1983433.0 / 1935360.0)*pow(n, 6.0),
		(61.0 / 240.0) * pow(n, 3.0) - (103.0 / 140.0)*pow(n, 4.0) + (15061.0 / 26880.0)*pow(n, 5.0) + (167603.0 / 181440.0)*pow(n, 6.0),
		(49561.0 / 161280.0)*pow(n, 4.0) - (179.0 / 168.0)*pow(n, 5.0) + (6601661.0 / 7257600.0)*pow(n, 6.0),
		(34729.0 / 80640.0)*pow(n, 5.0) - (3418889.0 / 1995840.0)*pow(n, 6.0),
		(212378941.0 / 319334400.0)*pow(n, 6.0) };

	double xi = xi_prime;
	double eta = eta_prime;
	double p_prime = 1.0, q_prime = 0.0;
	for (int i = 0; i < 6; i++)
	{
		xi = xi + alpha[i] * sin(2.0 * (i + 1.0)*xi_prime) * cosh(2.0 * (i + 1.0) *eta_prime);
		eta = eta + alpha[i] * cos(2.0 * (i + 1.0) *xi_prime) * sinh(2.0 * (i + 1.0) *eta_prime);
		p_prime = p_prime + 2.0*i*cos(2.0 * (i + 1.0) *xi_prime) * cosh(2.0 * (i + 1.0)* eta_prime);
		q_prime = q_prime + 2.0*i*sin(2.0 * (i + 1.0)*xi_prime) * sinh(2.0 * (i + 1.0) * eta_prime);
	}

	double x = UTM_MERIDIAN_SCALE * A * eta;
	double y = UTM_MERIDIAN_SCALE * A * xi;

	x = x + UTM_FALSE_EASTING;
	if (y < 0) y = y + UTM_FALSE_NORTHING; //in case the point was in sourthern hemisphere. for norther hemi, the y above is ok.

	std::unique_ptr<double> coords = std::unique_ptr<double> (new double [2]);
	coords.get()[0] = y;
	coords.get()[1] = x;

	if (isDebug)
		std::cout << "\n in ToUTM, returning coords: " << coords.get()[0] << " and " << coords.get()[1];
	
	return coords;
}

std::unique_ptr<double> ProfileMaker::ToWGS84(double easting, double northing) const
{
	//Using the some source-code referenced in ToUTM() and converting it to C++

	double _easting = easting - UTM_FALSE_EASTING;
	double _northing = isPathInNorthernHemisphere ? northing : northing - UTM_FALSE_NORTHING;

	double eccentricity = sqrt(WGS84_ELIPSOID_FLATTENING * (2.0f - WGS84_ELIPSOID_FLATTENING)); //this could be calculted outside and hardcoded into this program.
	double n = WGS84_ELIPSOID_FLATTENING / (2.0f - WGS84_ELIPSOID_FLATTENING); //ditto

	double n2 = n * n;
	double n3 = n2 * n;
	double n4 = n3 * n;
	double n5 = n4 * n;
	double n6 = n5 * n;

	double A = (WGS84_EARTH_RADIUS_EQUATOR / (1.0f + n)) * (1.0f + n2 * (1.0f / 4.0f) + n4 * (1.0f / 64.0f) + n6 * (1.0f / 256.0f));

	double eta = _easting / (UTM_MERIDIAN_SCALE * A);
	double xi = _northing / (UTM_MERIDIAN_SCALE * A);

	double beta[6] = {
		 n * (1.0f/2.0f)	-	n2 * (2.0f / 3.0f)	+	n3 * (37.0f / 96.0f)	-	n4 * (1.0f / 360.0f)		-	n5 * (81.0f / 512.0f)		+	n6 * (96199.0f / 604800.0f),
								n2 * (1.0f / 48.0f)	+	n3 * (1.0f / 15.0f)		-	n4 * (437.0f / 1440.0f)		+	n5 * (46.0f / 105.0f)		-	n6 * (1118711.0f / 3870720.0f),
														n3 * (17.0f / 480.0f)	-	n4 * (37.0f / 840.0f)		-	n5 * (209.0f / 4480.0f)		+	n6 * (5569.0f / 90720.0f),
																					n4 * (4397.0f / 161280.0f)	-	n5 * (11.0f / 504.0f)		-	n6 * (830251.0f / 7257600.0f),
																													n5 * (4583.0f / 161280.0f)	-	n6 * (108847.0f / 3991680.0f),
																																					n6 * (20648693.0f / 638668800.0f)
	};

	double xi_prime = xi;
	double eta_prime = eta;
	
	for (int i = 0; i < 6; i++)
	{
		xi_prime -= beta[i] * sin(2.0f * (i + 1) * xi) * cosh(2.0f * (i + 1) * eta);
		eta_prime -= beta[i] * cos(2.0f * (i + 1) * xi) * sinh(2.0f * (i + 1) * eta);
	}

	double sinh_eta_prime = sinh(eta_prime);
	double sin_xi_prime = sin(xi_prime);
	double cos_xi_prime = cos(xi_prime);

	double tau_prime = sin_xi_prime / sqrt(sinh_eta_prime * sinh_eta_prime + cos_xi_prime * cos_xi_prime);

	double deltaTau = 0.1f;
	double tau = tau_prime;

	while (abs(deltaTau) > 0.00000000001f)
	{
		double sigma = sinh(eccentricity * atanh(eccentricity * tau / sqrt(1.0f + tau * tau)));
		double tau_i_prime = tau * sqrt(1.0f + sigma * sigma) - sigma * sqrt(1 + tau * tau);
		deltaTau = ((tau_prime - tau_i_prime) / sqrt(1.0f + tau_i_prime * tau_i_prime)) * ((1.0f + (1.0f - eccentricity * eccentricity) * tau * tau) / ((1.0f - eccentricity * eccentricity) * sqrt(1.0f + tau * tau)));
		tau += deltaTau;
	}

	double phi = atan(tau);
	double lambda = atan2(sinh_eta_prime, cos_xi_prime);

	double p = 1.0f;
	double q = 0.0f;

	for (int i = 0; i < 6; i++)
	{
		p -= 2.0f * i * beta[i] * cos(2.0f * (i + 1) * xi) * cosh(2.0f * (i + 1) * eta);
		q += 2.0f * i * beta[i] * sin(2.0f * (i + 1) * xi) * sinh(2.0f * (i + 1) * eta);
	}

	double gamma_prime = atan(tan(xi_prime) * tanh(eta_prime));
	double gamme_prime_prime = atan2(q, p);
	double gamma = gamma_prime + gamme_prime_prime;

	double sin_phi = sin(phi);

	double k_prime = sqrt(1.0f - eccentricity * eccentricity *sin_phi * sin_phi) * sqrt(1.0f + tau * tau) * sqrt(sinh_eta_prime * sinh_eta_prime + cos_xi_prime * cos_xi_prime);
	double k_prime_prime = (A / WGS84_EARTH_RADIUS_EQUATOR ) * sqrt(p * p + q * q);
	double k = UTM_MERIDIAN_SCALE * k_prime * k_prime_prime;

	double lambda_0 = (PI_CONSTANT / 180.0f)* (double)((pathZone - 1) * 6 - 180 + 3);
	lambda += lambda_0;

	std::unique_ptr<double> coords = std::unique_ptr<double>(new double[2]);
	
	coords.get()[0] = lambda * (180.0f / PI_CONSTANT);
	coords.get()[1] = phi * (180.0f / PI_CONSTANT);


	double convergence = gamma * 180.0f / PI_CONSTANT;
	double scale = k;

	if (isDebug)
	{
		std::cout << "\n in ToUTM, returning coords: " << std::fixed << std::setprecision(11) << coords.get()[0] << " and " << coords.get()[1] << std::endl;
		std::cout << "Convergence: " << std::fixed << std::setprecision(11) << convergence << ", scale" << scale << std::endl;
	}

	return coords;
}

void ProfileMaker::ConvertPathToUTM()
{
	if (isDebug) std::cout << "\nConverting path to UTM\n"; //test
	
	std::unique_ptr<double> tempreturn;

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		tempreturn = ToUTM(profile_i[i][0], profile_i[i][1]);
		profile_i[i][0] = tempreturn.get()[1];
		profile_i[i][1] = tempreturn.get()[0];
	}

	if (isDebug)
	{
		std::cout << "\nConverted path\n";
		profile_i.DisplayArrayInCLI();
	}
}

void ProfileMaker::ConvertPathToWGS84()
{
	if (isDebug) std::cout << "\nConverting path to WGS-84 GCS\n"; //test
	
	std::unique_ptr<double> tempreturn;

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		tempreturn = ToWGS84(profile_i[i][0], profile_i[i][1]);
		//std::cout << "\n Recieved: " << tempreturn.get()[0] << " and " << tempreturn.get()[1];

		profile_i[i][0] = tempreturn.get()[0];
		profile_i[i][1] = tempreturn.get()[1];
		
		//std::cout << "\n Set: " << profile_i[i][0] << " and " << profile_i[i][1];
	}

	if (isDebug)
	{
		std::cout << "\nConverted path\n";
		profile_i.DisplayArrayInCLI();
	}
}
