#include "ProfileMaker.h"


ProfileMaker::ProfileMaker()
{
	//GDALAllRegister();
	GDALRegister_GTiff();
	kmlLocation = "input.kml";
	demLocation = "input.tif";
	outputLocation = "output.csv";
	isInterpolated = false;
	isCalculated = false;
	isConverted = false;
}

ProfileMaker::~ProfileMaker()
{
	profile.~Array2D();
	profile_i.~Array2D();
	heightsGrid.~Array2D();
}

bool ProfileMaker::LoadDEM(std::string inDEMLoc)
{
	demLocation = inDEMLoc;
	if (!FileIsExist(inDEMLoc))
	{
		std::cout << "Error! Could not open DEM file: " << inDEMLoc << ". \nFile doesn't exist?\n\n";
		return false;
	}

	demDataset = (GDALDataset *)GDALOpen(inDEMLoc.c_str(), GA_ReadOnly);

	if (demDataset == NULL)
	{
		//TODO review GDAL docs for CPLError() and see if you can use it to display a more informative error message.
		std::cout << "Error! Could not load DEM file.\n\n";
		return false;
	}

	demBand = demDataset->GetRasterBand(1); //Should consider checking that this is a greyscale (single band) dem first.

	SetDEMInfo();
	DisplayDEMInfo();

	int demx = demDataset->GetRasterXSize();
	int demy = demDataset->GetRasterYSize();

	std::cout << "demX: " << demx << ", demY: " << demy << std::endl; //test

	heightsGrid = Array2D(demy, demx);

	double * scanline;
	int demxsize = demBand->GetXSize();
	scanline = (double *)CPLMalloc(sizeof(double) * demxsize);

	//TODO the loop bellow bugs out in large rasters, fix this!
	for (int i = 0; i < demy; i++)
	{
		//std::cout << i << std::endl; //test 
		demBand->RasterIO(GF_Read, 0, i, demxsize, 1, scanline, demxsize, 1, GDT_Float64, 0, 0);

		for (int j = 0; j < demx; j++)
			heightsGrid[i][j] = scanline[j];
	}

	CPLFree(scanline);
	GDALClose(demDataset);
	std::cout << "Successfully loaded DEM file: " << inDEMLoc << "\n\n";

	//heightsGrid.DisplayArrayInCLI(); //test
	return true;
}

bool ProfileMaker::LoadKML(std::string inKMLLoc)
{

	if (!P_Path.LoadKML(inKMLLoc))
		return false;

	//for (int i = 0; i < P_Path.GetVertCount(); i++)
	//{
	//	profile[i][0] = P_Path.verts[i][0];
	//	profile[i][1] = P_Path.verts[i][1];
	//}

	profile = Array2D(P_Path.GetPathVertexCountByID(0), 4);
	profile.Overlay(*P_Path.GetPathByID(0), 0, 0);


	profile[0][3] = 0.0f;
	for (int i = 1; i < profile.Rows(); i++)
	{
		profile[i][3] = CalculateDistance(profile[i - 1][0], profile[i - 1][1], profile[i][0], profile[i][1]);
	}

	if (isDebug)
	{
		std::cout << "Input path" << std::endl;
		profile.DisplayArrayInCLI();
	}

	return true;
}

void ProfileMaker::DisplayDEMInfo()
{
	std::cout << "\n==================================";
	std::cout << "\n\tDEM info\n";
	std::cout << "==================================\n\n";
	std::cout << "Coordinate System: " << demInfo.ProjectionReference << std::endl;
	std::cout << "Rasters in-file: " << demInfo.RasterCount << std::endl;
	std::cout << "\nFor raster #1\n";
	std::cout << "Resolution: " << demInfo.x << " by " << demInfo.y << " pixels.\n";
	std::cout << "Data type: " << demInfo.RasterDataType << std::endl;
	std::cout << "Block size: " << demInfo.BlockSize_x << " by " << demInfo.BlockSize_y << " pixels.\n";
	std::cout << "Color interpretation: " << demInfo.ColorInterpretation << std::endl;
	std::cout << "Minimum value: " << demInfo.z_min << std::endl;
	std::cout << "Maximum value: " << demInfo.z_max << std::endl;
	if (demInfo.ColorEntryCount > 0)
	{
		std::cout << "Color table entries: " << demInfo.ColorEntryCount << std::endl;
	}
	else
	{
		std::cout << "Color table entries: N/A\n";
	}
	std::cout << "\nRaster Geographical Transforms:\n";
	std::cout << "Origin coordinates: " << std::setprecision(14) << demInfo.originx << ", " << demInfo.originy << "\n";
	std::cout << "Pixel Size: " << std::setprecision(14) << demInfo.PixelSize_x << " by " << demInfo.PixelSize_y << "\n";
	std::cout << "[Unsupported]: " << demInfo.geotransform_2 << ", " << demInfo.geotransform_4 << "\n"; //TODO geotransform 2 and 4
	std::cout << "[Border coordinates NW -> NE -> SE -> SW\n";
	std::cout << demInfo.NW_x << ", " << demInfo.NW_y << std::endl;
	std::cout << demInfo.NE_x << ", " << demInfo.NE_y << std::endl;
	std::cout << demInfo.SE_x << ", " << demInfo.SE_y << std::endl;
	std::cout << demInfo.SW_x << ", " << demInfo.SW_y << std::endl;
}

void ProfileMaker::DisplayPathInfo()
{
	if (!isInterpolated)
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

	//std::cout << "newVertsSum: " << newVertsSum << std::endl; //test

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
	//TODO reimplement this method.

	double *tempx, *tempy;
	int tempverts;

	/*if (isInterpolated)
	{
		tempx = P_Xi;
		tempy = P_Yi;
		tempverts = pathVerts_i;
	}
	else
	{
		tempx = P_X;
		tempy = P_Y;
		tempverts = pathVerts;
	}

	for (int i = 0; i < tempverts; i++ )
	{
		if (tempx[i] < demInfo.NW_x || tempx[i] > demInfo.SE_x)
		{
			return true;
		}
		if (tempy[i] > demInfo.NW_y || tempy[i] < demInfo.SE_y)
		{
			return true;
		}
	}*/

	return false;
}

bool ProfileMaker::IsPointOOB(double x, double y)
{
	if (x < demInfo.NW_x || x > demInfo.SE_x)
	{
		return false;
	}
	if (y > demInfo.NW_y || y < demInfo.SE_y)
	{
		return false;
	}
}

int ProfileMaker::CalculateProfile() //returning int for end state. 0: failure, 1: success, 2: success with gaps (for when implementing 
										//choice to calculate profile for paths that are partially within the provided DEM's boundaries.
{
	//in case dem is in UTM
	if (demInfo.IsUTM)
	{
		ConvertPathToUTM();
		isConverted = true;
	}

	//check if profile is out of bounds
	if (IsPathOOB()) //TODO remove the exit, return a custom error code, modify calling function to handle the code accordingly
	{
		std::cout << "Error: Loaded path is outside the boundry of the loaded DEM!\n";
		//return;
		std::cout << "Press Enter to Continue.\n";
		//std::cin.ignore();
		std::cin.sync();
		std::cin.get();
		exit(1);
	}

	int first_larger_x_order, first_larger_y_order;

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		for (int j = 0; j < demInfo.y; j++)
		{
			if (profile_i[i][1] > demInfo.originy + j * demInfo.PixelSize_y)
			{
				first_larger_y_order = j;
				break;
			}
		}
		for (int k = 0; k < demInfo.x; k++)
		{
			if (profile_i[i][0] < demInfo.originx + k * demInfo.PixelSize_x)
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

	for (int j = 0; j < demInfo.y; j++)
	{
		if (y > demInfo.originy + j * demInfo.PixelSize_y)
		{
			boundingy[0] = demInfo.originy + (j - 1) * demInfo.PixelSize_y;
			boundingy[1] = demInfo.originy + j * demInfo.PixelSize_y;
			first_larger_y_order = j;
			break;
		}
	}
	for (int k = 0; k < demInfo.x; k++)
	{
		if (x < demInfo.originx + k * demInfo.PixelSize_x)
		{
			boundingx[0] = demInfo.originx + (k - 1) * demInfo.PixelSize_x;
			boundingx[1] = demInfo.originx + k * demInfo.PixelSize_x;
			first_larger_x_order = k;
			break;
		}
	}

	boundingz[0] = heightsGrid[first_larger_y_order - 1][first_larger_x_order - 1]; //flipped x and y
	boundingz[1] = heightsGrid[first_larger_y_order - 1][first_larger_x_order];
	boundingz[2] = heightsGrid[first_larger_y_order][first_larger_x_order];
	boundingz[3] = heightsGrid[first_larger_y_order][first_larger_x_order - 1];

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
	if (isConverted)
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
	isConverted = false;

	P_Path.UnloadKML();
}

void ProfileMaker::SetDEMInfo()
{
	double temptransform[6];
	int min, max;
	double tempminmax[2];

	if (demDataset->GetProjectionRef() != NULL)
	{
		demInfo.ProjectionReference = demDataset->GetProjectionRef();
	}

	demInfo.x = demBand->GetXSize();
	demInfo.y = demBand->GetYSize();

	demDataset->GetGeoTransform(temptransform);
	demInfo.originx = temptransform[0];
	demInfo.originy = temptransform[3];
	demInfo.PixelSize_x = temptransform[1];
	demInfo.PixelSize_y = temptransform[5];
	demInfo.geotransform_2 = temptransform[2]; //TODO change these values when you fix the related name thingy.
	demInfo.geotransform_4 = temptransform[4];
	demInfo.z_min = demBand->GetMinimum(&min);
	demInfo.z_max = demBand->GetMaximum(&max);
	if (!(min && max))
	{
		GDALComputeRasterMinMax((GDALRasterBandH)demBand, true, tempminmax);
		demInfo.z_min = tempminmax[0];
		demInfo.z_max = tempminmax[1];
	}
	demBand->GetBlockSize(&demInfo.BlockSize_x, &demInfo.BlockSize_y);
	demInfo.RasterCount = demDataset->GetRasterCount();
	demInfo.RasterDataType = GDALGetDataTypeName(demBand->GetRasterDataType());
	demInfo.OverviewCount = demBand->GetOverviewCount();
	demInfo.ColorInterpretation = GDALGetColorInterpretationName(demBand->GetColorInterpretation());

	if (demBand->GetColorTable() != NULL)
	{
		demInfo.ColorEntryCount = demBand->GetColorTable()->GetColorEntryCount();
	}
	else
	{
		demInfo.ColorEntryCount = -1;
	}

	//setting corner coords
	//Necessary?
	demInfo.NW_x = demInfo.originx;
	demInfo.NW_y = demInfo.originy;

	demInfo.NE_x = demInfo.originx + demInfo.x * demInfo.PixelSize_x;
	demInfo.NE_y = demInfo.originy;

	demInfo.SE_x = demInfo.originx + demInfo.x * demInfo.PixelSize_x;
	demInfo.SE_y = demInfo.originy + demInfo.y * demInfo.PixelSize_y;

	demInfo.SW_x = demInfo.originx;
	demInfo.SW_y = demInfo.originy + demInfo.y * demInfo.PixelSize_y;

	//checking whether its geographic or utm
	//note this implementation is risky, based on a very simple and short observation

	std::string tempstring;

	tempstring = demInfo.ProjectionReference;

	//std::cout << "\n ********************* tempstring: " << tempstring.substr(0,6) << std::endl; //test
	if (tempstring.substr(0, 6) == "PROJCS")
	{
		demInfo.IsUTM = true;
		//demInfo.IsDecimal = false;
	}
	else
	{
		demInfo.IsUTM = false;
		//demInfo.IsDecimal = true;
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
		boundingx[i] = demInfo.originx + (first_larger_x - 1 + i) * demInfo.PixelSize_x;
		boundingy[i] = demInfo.originy + (first_larger_y - 1 + i) * demInfo.PixelSize_y;
	}

	boundingz[0] = heightsGrid[first_larger_y - 1][first_larger_x - 1];
	boundingz[1] = heightsGrid[first_larger_y - 1][first_larger_x];
	boundingz[2] = heightsGrid[first_larger_y][first_larger_x];
	boundingz[3] = heightsGrid[first_larger_y][first_larger_x - 1];

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
		boundingx[i] = demInfo.originx + (first_larger_x - 2 + i) * demInfo.PixelSize_x;
		boundingy[i] = demInfo.originy + (first_larger_y - 2 + i) * demInfo.PixelSize_y;
		for (int j = 0; j < 4; j++)
		{
			boundingz[i][j] = heightsGrid[first_larger_y - 2 + j][first_larger_x - 2 + i];
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

bool ProfileMaker::FileIsExist(std::string location)
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

double * ProfileMaker::ToUTM(double lng, double lat)
{
	//converting this http://www.movable-type.co.uk/scripts/latlong-utm-mgrs.html
	// to c++
	//also https://www.uwgb.edu/dutchs/UsefulData/UTMFormulas.HTM

	//std::cout << "\nDevWarning: Using Karney's method to convert from decimal degrees to UTM\n";

	const double Pi = 3.14159265359;
	const double a = 6378137; //WGS84 earth radius at equator
	const double f = 1 / 298.257223563; //WGS84 elipsoid flattening
	const double b = 6356752.3142; //WGS84 radius at poles
	const double utm_scale_at_meridian = 0.9996;
	const double falseEasting = 500000, falseNorthing = 10000000;

	int zone = floor((lng + 180.0) / 6.0) + 1;
	double central_meridian_longitude = ((zone - 1.0) * 6.0 - 180.0 + 3.0) * Pi / 180.0; //in radians
	//TODO consider Norway/Svalbard exceptions, not really necessary.

	const double e = sqrt(1.0 - pow((b / a), 2.0));
	const double n = (f / (2 - f));


	lat = lat * Pi / 180.0;
	lng = (lng * Pi / 180.0) - central_meridian_longitude;

	const double tao = tan(lat);
	const double sigma = sinh(e * atanh(e * tao / sqrt(1.0 + pow(tao, 2.0)))); //checked with source paper
	const double tao_prime = (tao * sqrt(1.0 + pow(sigma, 2.0))) - (sigma * sqrt(1.0 + pow(tao, 2.0))); //checked with source paper
	const double xi_prime = atan(tao_prime / cos(lng)); //checked with source paper
	const double eta_prime = asinh(sin(lng) / sqrt(pow(tao_prime, 2.0) + pow(cos(lng), 2))); //checked with source
	const double A = (a / (1.0 + n)) * (1.0 + (1.0 / 4.0)*pow(n, 2.0) + (1.0 / 64.0)*pow(n, 4.0) + (1.0 / 256.0)*pow(n, 6.0)); //checked up to n^4 in source paper

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

	double x = utm_scale_at_meridian * A * eta;
	double y = utm_scale_at_meridian * A * xi;

	x = x + falseEasting;
	if (y < 0) y = y + falseNorthing; //in case the point was in sourthern hemisphere. for norther hemi, the y above is ok.

	double * coords;
	coords = new double[2]; //I don't know why, but this fixed it! Just having double coords[2] causes function to return false values in release builds outside the safe haven of debug...
	coords[0] = y;
	coords[1] = x;

	if (isDebug)
		std::cout << "\n in ToUTM, returning coords: " << coords[0] << " and " << coords[1];
	return coords;
}

void ProfileMaker::ConvertPathToUTM()
{
	double * tempreturn;

	if (isDebug) std::cout << "\nConverting path to UTM\n"; //test

	for (int i = 0; i < profile_i.Rows(); i++)
	{
		tempreturn = ToUTM(profile_i[i][0], profile_i[i][1]);
		profile_i[i][0] = tempreturn[1];
		profile_i[i][1] = tempreturn[0];
	}

	if (isDebug)
	{
		std::cout << "\nConverted path\n";
		profile_i.DisplayArrayInCLI();
	}
}
