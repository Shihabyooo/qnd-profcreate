#include "main.h"

ProfileMake::ProfileMake()
{
	GDALAllRegister(); //TODO See to registering relevant drivers only
	P_KMLLocation = "input.kml";
	P_DEMLocation = "input.tif";
	P_OutuputLocation = "output.csv";
	P_PathVertices = 0;
	P_PathVerticesI = 0;
	P_IsInterpolated = false;
	P_IsCalculated = false;
	P_IsConverted = false;
	//P_IsUTM = false;
}

ProfileMake::~ProfileMake()
{
	//P_Path.~KMLParser();
	if (P_X != NULL)
		delete[] P_X;
	if (P_Y != NULL)
		delete[] P_Y;
	if (P_Xi != NULL)
		delete[] P_Xi;
	if (P_Yi != NULL)
		delete[] P_Yi;
	if (P_Z != NULL)
		delete[] P_Z;
	if (P_PathLength != NULL)
		delete[] P_PathLength;

	int x = sizeof(P_Heights);
	int y;
	y = x + 0;
	
	if (P_Heights != NULL)
	{
		for (int i = 0; i < P_DEMInfo.y; i++)
			if (P_Heights[i] != NULL)
				delete[] P_Heights[i];
		delete[] P_Heights;
	}
}

bool ProfileMake::PLoadDEM(std::string inDEMLoc)
{
	P_DEMLocation = inDEMLoc;
	if (!PFileIsExist(inDEMLoc))
	{
		std::cout << "Error! Could not open DEM file: " << inDEMLoc << ". \nFile doesn't exist?\n\n";
		return false;
	}

	const char * demloc = inDEMLoc.c_str();
	P_DEM = (GDALDataset *)GDALOpen(demloc, GA_ReadOnly);

	if (P_DEM == NULL)
	{
		std::cout << "Error! Could not load DEM file.\n\n";
		return false;
	}
	
	P_DEMBand = P_DEM->GetRasterBand(1); //Should consider checking that this is a greyscale (single band) dem first.
	
	PSetDEMInfo();
	
	int demx = P_DEM->GetRasterXSize();
	int demy = P_DEM->GetRasterYSize();
	

	P_Heights = new float*[demy]; 
	float * scanline;
	int demxsize = P_DEMBand->GetXSize();
	scanline = (float *)CPLMalloc(sizeof(float) * demxsize);

	for (int i = 0; i < demy; i++)
	{
		P_Heights[i] = new float[demx]; 
		P_DEMBand->RasterIO(GF_Read, 0, i, demxsize, 1, scanline, demxsize, 1, GDT_Float32, 0, 0);

		for (int j = 0; j < demx; j++)
		{
			P_Heights[i][j] = scanline[j];
		}
	}
	CPLFree(scanline);
	GDALClose(P_DEM);
	std::cout << "Successfully loaded DEM file: " << inDEMLoc << "\n\n";
	
	return true;
}

bool ProfileMake::PLoadKML(std::string inKMLLoc)
{
	
	if (!P_Path.KLoadKML(inKMLLoc))
	{
		return false;
	}

	P_X = new double[P_Path.KGetVertCount()];
	P_Y = new double[P_Path.KGetVertCount()];

	//double ** vertsArrayPointer = P_Path.KGetPtrToVerts;

	for (int i = 0; i < P_Path.KGetVertCount(); i++)
	{
		//P_X[i] = P_Path.KGetPtrToVerts[i][0];
		P_X[i] = P_Path.K_Verts[i][0];
		//P_Y[i] = P_Path.KGetPtrToVerts[i][1];
		P_Y[i] = P_Path.K_Verts[i][1];
	}

	P_PathVertices = P_Path.KGetVertCount();
	if (isDebug)
	{
		std::cout << "verts: " << P_PathVertices << std::endl;
		for (int i = 0; i < P_Path.KGetVertCount(); i++)
		{
			std::cout << "values: " << P_X[i] << ", " << P_Y[i] << std::endl;
		}
	}
		P_PathLength = new float[P_PathVertices - 1];
		for (int i = 0; i < P_PathVertices - 1; i++)
		{
			P_PathLength[i] = PCalculateDistance(P_X[i], P_Y[i], P_X[i + 1], P_Y[i + 1]);
		}
	return true;
}

void ProfileMake::PDisplayDEMInfo()
{
	std::cout << "\n==================================";
	std::cout << "\n\tDEM info\n";
	std::cout << "==================================\n\n";
	std::cout << "Coordinate System: " << P_DEMInfo.ProjectionReference <<std::endl;
	std::cout << "Rasters in-file: " << P_DEMInfo.RasterCount << std::endl;
	std::cout << "\nFor raster #1\n";
	std::cout << "Resolution: " << P_DEMInfo.x << " by " << P_DEMInfo.y << " pixels.\n";
	std::cout << "Data type: " << P_DEMInfo.RasterDataType << std::endl;
	std::cout << "Block size: " << P_DEMInfo.BlockSize_x << " by " << P_DEMInfo.BlockSize_y << " pixels.\n";
	std::cout << "Color interpretation: " << P_DEMInfo.ColorInterpretation << std::endl;
	std::cout << "Minimum value: " << P_DEMInfo.z_min << std::endl;
	std::cout << "Maximum value: " << P_DEMInfo.z_max << std::endl;
	if (P_DEMInfo.ColorEntryCount > 0)
	{
		std::cout << "Color table entries: " << P_DEMInfo.ColorEntryCount << std::endl;
	}
	else
	{
		std::cout << "Color table entries: N/A\n";
	}
	std::cout << "\nRaster Geographical Transforms:\n";
	std::cout << "Origin coordinates: " <<std::setprecision(14)<< P_DEMInfo.originx << ", " << P_DEMInfo.originy << "\n";
	std::cout << "Pixel Size: " << std::setprecision(14) << P_DEMInfo.PixelSize_x << " by " << P_DEMInfo.PixelSize_y << "\n";
	std::cout << "[Unsupported]: " << P_DEMInfo.geotransform_2 << ", " << P_DEMInfo.geotransform_4 << "\n"; //TODO geotransform 2 and 4
	std::cout << "[Border coordinates NW -> NE -> SE -> SW\n";
	std::cout << P_DEMInfo.NW_x << ", " << P_DEMInfo.NW_y << std::endl;
	std::cout << P_DEMInfo.NE_x << ", " << P_DEMInfo.NE_y << std::endl;
	std::cout << P_DEMInfo.SE_x << ", " << P_DEMInfo.SE_y << std::endl;
	std::cout << P_DEMInfo.SW_x << ", " << P_DEMInfo.SW_y << std::endl;
}

void ProfileMake::PDisplayPathInfo()
{
	float tempdist = 0, tempz = 0;
	if (!P_IsInterpolated)
	{
		for (int i = 0; i < P_PathVertices; i++)
		{
			if (i == 0)
			{
				tempdist = 0;
			}
			else
			{
				tempdist = tempdist + P_PathLength[i - 1];
			}
			if (P_IsCalculated == true) tempz = P_Z[i];

			std::cout << " " << std::fixed << std::setprecision(7) << P_X[i] << "	" << P_Y[i] << "	" << std::setprecision(2) << tempdist << "		" << tempz << std::endl;
		}
	}
	else
	{
		for (int i = 0; i < P_PathVerticesI; i++)
		{
			if (i == 0)
			{
				tempdist = 0;
			}
			else
			{
				tempdist = tempdist + P_PathLengthI[i - 1];
			}
			if (P_IsCalculated == true) tempz = P_Z[i];

			std::cout << " " << std::fixed << std::setprecision(7) <<P_Xi[i] << "	" << P_Yi[i] << "	" << std::setprecision(2) << tempdist << "		" << tempz << std::endl;
		}
	}
}

//void ProfileMake::PInterpolateProfile(float step, bool maintainBends)
//{
//	int * newverts = new int[P_PathVertices - 1];
//	int newvertssum = P_PathVertices;
//	
//	for (int i = 0; i < P_PathVertices - 1; i++)
//	{
//		newverts [i] = floor(P_PathLength[i] / step);
//		newvertssum = newvertssum + newverts[i];
//	}
//
//
//	P_Xi = new double[newvertssum];
//	P_Yi = new double[newvertssum];
//
//	//calculate tempx, tempy here
//	int currentid = 0;
//	for (int i = 0; i < P_PathVertices - 1; i++)
//	{
//		P_Xi[currentid] = P_X[i];
//		P_Yi[currentid] = P_Y[i];
//		currentid++;
//		for (int j = 0; j < newverts[i]; j++)
//		{
//			P_Xi[currentid] = (2 * P_X[i] * (1 - (step * (j + 1)) / P_PathLength[i]) + 2 * P_X[i + 1] * (step * (j + 1)) / P_PathLength[i])/2;
//			P_Yi[currentid] = (2 * P_Y[i] * (1 - (step * (j + 1)) / P_PathLength[i]) + 2 * P_Y[i + 1] * (step * (j + 1)) / P_PathLength[i])/2;
//			currentid++;
//		}
//	}
//	P_Xi[newvertssum - 1] = P_X[P_PathVertices-1]; //copying last verts
//	P_Yi[newvertssum - 1] = P_Y[P_PathVertices-1];
//	P_PathVerticesI = newvertssum;
//	
//	P_PathLengthI = new float[P_PathVerticesI - 1];
//
//
//	for (int i = 0; i < P_PathVerticesI - 1; i++)
//		{
//			P_PathLengthI[i] = PCalculateDistance(P_Xi[i], P_Yi[i], P_Xi[i + 1], P_Yi[i + 1]);
//		}
//	
//	P_IsInterpolated = true;
//}

void ProfileMake::PInterpolateProfile(float step, bool maintainBends)
{
	int newvertssum = P_PathVertices;

	float totalLength = 0.0f;
	for (int i = 0; i < P_PathVertices - 1; i++)
	{
		totalLength += P_PathLength[i];
	}

	newvertssum = floor(totalLength / step) + 2; //the 2 are begining and ending verts

	if (maintainBends)
		newvertssum += P_PathVertices - 2;


	P_Xi = new double[newvertssum];
	P_Yi = new double[newvertssum];

	P_Xi[0] = P_X[0];
	P_Yi[0] = P_Y[0];

	int currentSegment = 0;
	float lastBendChainage = 0.0f;

	std::function<float(float, float, float, float)> interpolate = [&](float x0, float x1, float l, float dist)->float
	{ //too many methods in this class already, plus I already have other methods with interpolate in the name. This is a small one, leave it as a lambda
		return (dist * x1 / l) + ((l - dist) * x0 / l);
	};

	for (int i = 1; i < newvertssum - 1; i++)
	{
		if (i*step >= lastBendChainage + P_PathLength[currentSegment])
		{
			lastBendChainage += P_PathLength[currentSegment];
			currentSegment++;

			if (maintainBends)
			{
				P_Xi[i] = P_X[currentSegment];
				P_Yi[i] = P_Y[currentSegment];
				i++;
				if (i >= newvertssum - 1)
					break;
			}
		}

		float distFromLastBend = (step * i) - lastBendChainage;

		P_Xi[i] = interpolate(P_X[currentSegment], P_X[currentSegment + 1], P_PathLength[currentSegment], distFromLastBend);
		P_Yi[i] = interpolate(P_Y[currentSegment], P_Y[currentSegment + 1], P_PathLength[currentSegment], distFromLastBend);
	}

	P_Xi[newvertssum - 1] = P_X[P_PathVertices - 1]; //copying last verts
	P_Yi[newvertssum - 1] = P_Y[P_PathVertices - 1];
	P_PathVerticesI = newvertssum;

	P_PathLengthI = new float[P_PathVerticesI - 1]; //useless? in the new logic?
													//TODO remove after checking the rest of code


	for (int i = 0; i < P_PathVerticesI - 1; i++) //TODO remove after checking the rest of code
	{
		P_PathLengthI[i] = PCalculateDistance(P_Xi[i], P_Yi[i], P_Xi[i + 1], P_Yi[i + 1]);
	}

	P_IsInterpolated = true; //Useless?



}

bool ProfileMake::PIsPathOOB() 
{
	double *tempx, *tempy;
	int tempverts;

	if (P_IsInterpolated)
	{
		tempx = P_Xi;
		tempy = P_Yi;
		tempverts = P_PathVerticesI;
	}
	else
	{
		tempx = P_Xi; //TODO check whether this is correct. Shouldn't this be P_X and P_Y?
		tempy = P_Yi;
		tempverts = P_PathVertices;
	}

	for (int i = 0; i < tempverts; i++ )
	{
		if (tempx[i] < P_DEMInfo.NW_x || tempx[i] > P_DEMInfo.SE_x)
		{
			return true;
		}
		if (tempy[i] > P_DEMInfo.NW_y || tempy[i] < P_DEMInfo.SE_y)
		{
			return true;
		}
	}

	return false;
}

bool ProfileMake::PIsPointOOB(double x, double y)
{
	if (x < P_DEMInfo.NW_x || x > P_DEMInfo.SE_x)
	{
		return false;
	}
	if (y > P_DEMInfo.NW_y || y < P_DEMInfo.SE_y)
	{
		return false;
	}
}

int ProfileMake::PCalculateProfile() //returning int for end state. 0: failure, 1: success, 2: success with gaps (for when implementing 
										//choice to calculate profile for paths that are partially within the provided DEM's boundaries.
{
	//in case dem is in UTM
	if (P_DEMInfo.IsUTM)
	{
		PConvertPathToUTM();
		P_IsConverted = true;
	}

	//check if profile is out of bounds
	if (PIsPathOOB())
	{
		std::cout << "Error: Loaded path is outside the boundry of the loaded DEM!\n";
		//return;
		std::cout << "Press Enter to Continue.\n";
		//std::cin.ignore();
		std::cin.sync();
		std::cin.get();
		exit(1);
	}
	P_Z = new float[P_PathVerticesI]; //Remember that we use P_PathVerticesI here, not P_PathVertices.
	int first_larger_x_order, first_larger_y_order;

	for (int i = 0; i < P_PathVerticesI; i++) //TODO if this method is calcualted without interpolating profile, this method will not return any value
												//TODO fix this part
	{
		for (int j = 0; j < P_DEMInfo.y; j++)
		{
			if (P_Yi[i] > P_DEMInfo.originy + j * P_DEMInfo.PixelSize_y)
			{
				first_larger_y_order = j;
				break;
			}
		}
		for (int k = 0; k < P_DEMInfo.x; k++)
		{
			if (P_Xi[i] < P_DEMInfo.originx + k * P_DEMInfo.PixelSize_x)
			{
				first_larger_x_order = k;
				break;
			}
		}

		P_Z[i] = PBicubicInterp(first_larger_x_order, first_larger_y_order, i);

	}
	P_IsCalculated = true;
	
	return 1;
}

void ProfileMake::PCalculatePoint(double x, double y)
{
	//check if profile is out of bounds
	if (!PIsPointOOB(x, y))
	{
		std::cout << "Point is outside the DEM boundries!\n";
		return;
	}

	float result;
	double boundingx[2], boundingy[2];
	float boundingz[4]; //NW -> NE -> SE -> SW
	int first_larger_x_order, first_larger_y_order;
	double A, B; //used as temp holders to clean up bilinear interp forumla
	//std::cout << "test1\n" << std::endl; //test

		for (int j = 0; j < P_DEMInfo.y; j++)
		{
			if (y > P_DEMInfo.originy + j * P_DEMInfo.PixelSize_y)
			{
				boundingy[0] = P_DEMInfo.originy + (j - 1) * P_DEMInfo.PixelSize_y;
				boundingy[1] = P_DEMInfo.originy + j * P_DEMInfo.PixelSize_y;
				first_larger_y_order = j;
				break;
			}
		}
		for (int k = 0; k < P_DEMInfo.x; k++)
		{
			if (x < P_DEMInfo.originx + k * P_DEMInfo.PixelSize_x)
			{
				boundingx[0] = P_DEMInfo.originx + (k - 1) * P_DEMInfo.PixelSize_x;
				boundingx[1] = P_DEMInfo.originx + k * P_DEMInfo.PixelSize_x;
				first_larger_x_order = k;
				break;
			}
		}
		
		//std::cout << "test2\n" << std::endl; //test
		//std::cout << "setting boundingz\n"; //test
		boundingz[0] = P_Heights[first_larger_y_order - 1][first_larger_x_order - 1]; //flipped x and y
		boundingz[1] = P_Heights[first_larger_y_order - 1][first_larger_x_order];
		boundingz[2] = P_Heights[first_larger_y_order][first_larger_x_order];
		boundingz[3] = P_Heights[first_larger_y_order][first_larger_x_order - 1];

	/*	boundingz[0] = P_Heights[254][0]; 
		boundingz[1] = P_Heights[P_DEMInfo.y-10][P_DEMInfo.x-10];
		boundingz[2] = P_Heights[P_DEM->GetRasterXSize()][P_DEM->GetRasterYSize()];
		boundingz[3] = P_Heights[256][283];*/


		// x1 = boundingx[0]
		// x2 = boundingx[1]
		// y1 = boundingy[1]
		// y2 = boundingy[1]
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

		//std::cout << "calculating A and B\n"; //test
		A = boundingz[0] * (boundingx[1] - x) / (boundingx[1] - boundingx[0]) + boundingz[1] * (x - boundingx[0]) / (boundingx[1] - boundingx[0]);
		B = boundingz[2] * (boundingx[1] - x) / (boundingx[1] - boundingx[0]) + boundingz[3] * (x - boundingx[0]) / (boundingx[1] - boundingx[0]);

		//std::cout << "Resultant A: " << A << std::endl; //test
		//std::cout << "Resultant B: " << B << std::endl; //test
		//std::cout << "Calculating P_Z[i]\n"; //test
		result = A * (boundingy[1] - y) / (boundingy[1] - boundingy[0]) + B * (y - boundingy[0]) / (boundingy[1] - boundingy[0]);


		if (isDebug) std::cout << "Resultant Z: " << result<<std::endl; //test

	
}

bool ProfileMake::PWriteProfile(std::string out_csv)
{

	if (!PFileIsExist(out_csv))
	{
		std::cout << "Attempting to create output file\n";
	}

	P_Result.open(out_csv); //TODO Enrich this.
	//add open test here

	if (P_Result.is_open())
	{
		std::cout << "File creation is successfull\n";
	}
	else
	{
		std::cout << "Error: failed to create or open file!\n";
		return false;
	}

	std::cout << "\nWriting results to disk\n"; //test
	P_Result << "Longitude,Latitude,Chainage,Height" << std::endl;
	
	float _chainage = 0.0f;
	for (int i = 0; i < P_PathVerticesI; i++)
	{
		P_Result << std::setprecision(10) << P_Xi[i] << "," << P_Yi[i] << ",";
		
		if (i > 0) _chainage += PCalculateDistance(P_Xi[i-1], P_Yi[i-1], P_Xi[i], P_Yi[i]);
		//P_Result << std::fixed<< std::setprecision(3) << PCalculateDistance(P_Xi[0], P_Yi[0], P_Xi[i], P_Yi[i]); //OK, this was a GENIUS move! </sarcasm>
		P_Result << std::fixed << std::setprecision(3) <<  _chainage;
		P_Result << "," << std::setprecision(4) << P_Z[i] << std::endl;
	}

	P_Result.close();
	return true;
}

double ProfileMake::PCalculateDistance(double x1, double y1, double x2, double y2)
{
	double result = 0;
	//distance calculation for UTM coords is very simple, doing it here and returning value immediatly.
	if (P_IsConverted)
	{
		result = sqrt(pow(abs(x1 - x2), 2.0) + pow(abs(y1 - y2), 2.0));
		if (isDebug) std::cout << "Calculating distance for UTM, result= " << result << std::endl; //test
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
	if (abs(x1 - x2)*3600 < 0.0001 && abs(y1 - y1)*3600 < 0.0001) //hack for when the two points are ~the same
															//otherwise, the algorithm sends weird values (NAN)
	{
		if (isDebug) std::cout << "\n[DevWarning: In PCalculateDistance(), trigered near-points check, returning 0.0]\n"; //test
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
	lambda = dx + (1 - c)* f * sin_azim * (sigma + c *sin_sigma * (cos_2sigmam + (c * cos_sigma * (-1 + 2 * pow(cos_2sigmam, 2.0)))));
#pragma endregion first lambda
	// Must calculate a lambda first outside the loop, else sin_sigma, cos_sigma 
	//and the others won't initialize in case longitude is the same (or very close)
					
				
	while ((abs(lambda - lambda_old))  > 0.000000000001)
	{
		sin_sigma = sqrt(pow((cos(u2)*sin(lambda)), 2.0) + pow((cos(u1)*sin(u2)) - (sin(u1) * cos(u2) * cos(lambda)), 2.0));
		cos_sigma = (sin(u1)*sin(u2)) + (cos(u1) * cos(u2) * cos(lambda));
		sigma = atan2(sin_sigma, cos_sigma);
		sin_azim = (cos(u1) * cos(u2) * sin(lambda)) / sin_sigma;
		cos2_azim = 1 - pow(sin_azim, 2.0);
		cos_2sigmam = cos_sigma - ((2 * sin(u1)*sin(u2)) / cos2_azim);
		c = (f / 16.0) * cos2_azim * (4 + (f * (4 - (3 * cos2_azim))));

		lambda_old = lambda;
		lambda = dx + (1 - c)* f * sin_azim * (sigma + c *sin_sigma * (cos_2sigmam + (c * cos_sigma * (-1 + 2 * pow(cos_2sigmam, 2.0)))));
	}


	double usqr = cos2_azim * ((pow(a, 2.0) - pow(b, 2.0)) / pow(b, 2.0));
	double A = 1 + ((usqr / 16384) * ((4096 + (usqr * (-768 + (usqr * (320 - (175 * usqr))))))));
	double B = (usqr / 1024) * (256 + (usqr*(-128 + (usqr * (75 - (47 * usqr))))));
	double dsigma = B * sin_sigma * (cos_2sigmam + ((B / 4) * (cos_sigma * (-1 + (2 * pow(cos_2sigmam, 2.0)))) - ((B / 6) * cos_2sigmam)*(-3 + (4 * pow(sin_sigma, 2.0)))*(-3 + (4 * pow(cos_2sigmam, 2.0)))));

	result = b * A * (sigma - dsigma);

	return result;
}

void ProfileMake::PResetProfile()
{
	if (P_X != NULL)
		delete[] P_X;
	P_X = NULL;
	if (P_Y != NULL)
		delete[] P_Y;
	P_Y = NULL;
	if (P_Xi != NULL)
		delete[] P_Xi;
	P_Xi = NULL;
	if (P_Yi != NULL)
		delete[] P_Yi;
	P_Yi = NULL;
	if (P_Z != NULL)
		delete[] P_Z; 
	P_Z = NULL;
	if (P_PathLength != NULL)
		delete[] P_PathLength;
	P_PathLength = NULL;
	if (P_PathLengthI != NULL)
		delete[] P_PathLengthI;
	P_PathLengthI = NULL;
	
	P_IsInterpolated = false;
	P_IsCalculated = false;
	P_IsConverted = false;
	
	P_PathVertices = 0;
	P_PathVerticesI = 0;

	P_Path.KUnloadKML();
}

void ProfileMake::PSetDEMInfo()
{
	double temptransform[6];
	int min, max;
	double tempminmax[2];
	
	if (P_DEM->GetProjectionRef() != NULL)
		{
			P_DEMInfo.ProjectionReference = P_DEM->GetProjectionRef();
		}

	P_DEMInfo.x = P_DEMBand->GetXSize();
	P_DEMInfo.y = P_DEMBand->GetYSize();

	P_DEM->GetGeoTransform(temptransform);
	P_DEMInfo.originx = temptransform[0];
	P_DEMInfo.originy = temptransform[3];
	P_DEMInfo.PixelSize_x = temptransform[1];
	P_DEMInfo.PixelSize_y = temptransform[5];
	P_DEMInfo.geotransform_2 = temptransform[2]; //TODO change these values when you fix the related name thingy.
	P_DEMInfo.geotransform_4 = temptransform[4];
	P_DEMInfo.z_min = P_DEMBand->GetMinimum(&min);
	P_DEMInfo.z_max = P_DEMBand->GetMaximum(&max);
	if (!(min && max))
	{
		GDALComputeRasterMinMax((GDALRasterBandH)P_DEMBand, true, tempminmax);
		P_DEMInfo.z_min = tempminmax[0];
		P_DEMInfo.z_max = tempminmax[1];
	}
	P_DEMBand->GetBlockSize(&P_DEMInfo.BlockSize_x, &P_DEMInfo.BlockSize_y);
	P_DEMInfo.RasterCount = P_DEM->GetRasterCount();
	P_DEMInfo.RasterDataType = GDALGetDataTypeName(P_DEMBand->GetRasterDataType());
	P_DEMInfo.OverviewCount = P_DEMBand->GetOverviewCount();
	P_DEMInfo.ColorInterpretation = GDALGetColorInterpretationName(P_DEMBand->GetColorInterpretation());
	
	if (P_DEMBand->GetColorTable() != NULL)
	{
		P_DEMInfo.ColorEntryCount = P_DEMBand->GetColorTable()->GetColorEntryCount();
	}
	else
	{
		P_DEMInfo.ColorEntryCount = -1;
	}


	//setting corner coords
	//Necessary?
	P_DEMInfo.NW_x = P_DEMInfo.originx;
	P_DEMInfo.NW_y = P_DEMInfo.originy;

	P_DEMInfo.NE_x = P_DEMInfo.originx + P_DEMInfo.x * P_DEMInfo.PixelSize_x;
	P_DEMInfo.NE_y = P_DEMInfo.originy;

	P_DEMInfo.SE_x = P_DEMInfo.originx + P_DEMInfo.x * P_DEMInfo.PixelSize_x;
	P_DEMInfo.SE_y = P_DEMInfo.originy + P_DEMInfo.y * P_DEMInfo.PixelSize_y;

	P_DEMInfo.SW_x = P_DEMInfo.originx;
	P_DEMInfo.SW_y = P_DEMInfo.originy + P_DEMInfo.y * P_DEMInfo.PixelSize_y;

	//checking whether its geographic or utm
	//note this implementation is risky, based on a very simple and short observation

	std::string tempstring;

	tempstring = P_DEMInfo.ProjectionReference;

	//std::cout << "\n ********************* tempstring: " << tempstring.substr(0,6) << std::endl; //test
	if (tempstring.substr(0, 6) == "PROJCS")
	{
		P_DEMInfo.IsUTM = true;
		//P_DEMInfo.IsDecimal = false;
	}
	else
	{
		P_DEMInfo.IsUTM = false;
		//P_DEMInfo.IsDecimal = true;
	}

}

float ProfileMake::PBilinearInterp(int first_larger_x, int first_larger_y, int point_order)
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
		boundingx[i] = P_DEMInfo.originx + (first_larger_x - 1 + i) * P_DEMInfo.PixelSize_x;
		boundingy[i] = P_DEMInfo.originy + (first_larger_y - 1 + i) * P_DEMInfo.PixelSize_y;
	}

	boundingz[0] = P_Heights[first_larger_y - 1][first_larger_x - 1];
	boundingz[1] = P_Heights[first_larger_y - 1][first_larger_x];
	boundingz[2] = P_Heights[first_larger_y][first_larger_x];
	boundingz[3] = P_Heights[first_larger_y][first_larger_x - 1];

	A = boundingz[0] * PCalculateDistance(boundingx[1], boundingy[0], P_Xi[point_order], boundingy[0]) / PCalculateDistance(boundingx[1], boundingy[0], boundingx[0], boundingy[0]) + boundingz[1] * PCalculateDistance(P_Xi[point_order], boundingy[0], boundingx[0], boundingy[0]) / PCalculateDistance(boundingx[1], boundingy[0], boundingx[0], boundingy[0]);
	B = boundingz[2] * PCalculateDistance(boundingx[1], boundingy[1], P_Xi[point_order], boundingy[1]) / PCalculateDistance(boundingx[1], boundingy[1], boundingx[0], boundingy[1]) + boundingz[3] * PCalculateDistance(P_Xi[point_order], boundingy[1], boundingx[0], boundingy[1]) / PCalculateDistance(boundingx[1], boundingy[1], boundingx[0], boundingy[1]);
	result_depth = A * PCalculateDistance(boundingx[0], boundingy[1], boundingx[0], P_Yi[point_order]) / PCalculateDistance(boundingx[0], boundingy[0], boundingx[0], boundingy[1]);
	result_depth = result_depth + B *  PCalculateDistance(boundingx[0], P_Yi[point_order], boundingx[0], boundingy[0]) / PCalculateDistance(boundingx[0], boundingy[0], boundingx[0], boundingy[1]);
	
	return result_depth;
}

float ProfileMake::PBicubicInterp(int first_larger_x, int first_larger_y, int point_order)
{
	//reference:
	//http://www.paulinternet.nl/?page=bicubic

	float result_depth;

	float boundingz[4][4];
	double boundingx[4];
	double boundingy[4];
	double temp_value [4];	
	double pointx, pointy;

	for (int i = 0; i < 4; i++)
	{
		boundingx[i] = P_DEMInfo.originx + (first_larger_x - 2 + i) * P_DEMInfo.PixelSize_x;
		boundingy[i] = P_DEMInfo.originy + (first_larger_y - 2 + i) * P_DEMInfo.PixelSize_y;
		for (int j = 0; j < 4; j++)
		{
			boundingz[i][j] = P_Heights[first_larger_y- 2 + j][first_larger_x - 2 + i];
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (!P_IsInterpolated) //Note the original Pcalculateprofile method still hasn't implemented this check. This is a preemptive strike.
		{
			pointx = abs((P_X[point_order] - boundingx[i]) / boundingx[i]);
		}
		else
		{
			pointx = abs((P_Xi[point_order] - boundingx[i]) / boundingx[i]);
		}
		temp_value[i] = boundingz[i][1] + 0.5 * pointx * (boundingz[i][2] - boundingz[i][0] + pointx * (2 * boundingz[i][0] - 5 * boundingz[i][1] + 4 * boundingz[i][2] - boundingz[i][3] + pointx*(3*(boundingz[i][1] - boundingz[i][2])+boundingz[i][3] - boundingz[i][0])));
	}

	if (!P_IsInterpolated) //Note the original Pcalculateprofile method still hasn't implemented this check. This is a preemptive strike.
	{
		pointy = abs((P_Y[point_order] - boundingy[1]) / boundingy[1]);
	}
	else
	{
		pointy = abs((P_Yi[point_order] - boundingy[1]) / boundingy[1]);
	}
	result_depth = temp_value[1]+ 0.5 * pointy * (temp_value[2]- temp_value[0]+ pointy * (2 * temp_value[0] - 5 * temp_value[1]+ 4 * temp_value[2]- temp_value[3]+ pointy*(3 * (temp_value[1]- temp_value[2]) + temp_value[3]- temp_value[0])));

	return result_depth;
}

bool ProfileMake::PFileIsExist(std::string location)
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

double * ProfileMake::PToUTM(double lng, double lat)
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

	//double gamma_prime = atan(tao_prime / sqrt(1.0 + pow(tao_prime, 2.0)) *  tan(lng));
	//double gamma_double_prime = atan2(q_prime, p_prime);
	//double gamma = gamma_prime + gamma_double_prime;

	//double k_prime = sqrt(1.0 - pow(e, 2.0) * pow(sin(lat), 2.0)) * sqrt(1.0 + pow(tan(lat), 2.0)) / sqrt(pow(tao_prime, 2.0) + pow(cos(lat), 2.0));
	//double k_double_prime = A / a * sqrt(pow(p_prime, 2.0) + pow(q_prime, 2.0));
	//double k = utm_scale_at_meridian * k_prime * k_double_prime;

	x = x + falseEasting;
	if (y < 0) y = y + falseNorthing; //in case the point was in sourthern hemisphere. for norther hemi, the y above is ok.

	//double coords[2];
	double * coords;
	coords = new double[2]; //I don't know why, but this fixed it! Just having double coords[2] causes function to return false values in release builds outside the safe haven of debug...
	coords[0] = y;
	coords[1] = x;

	if (isDebug) std::cout << "\n in PtoUTM, returning coords: " << coords[0] << " and " << coords[1];//test
	return coords;
}

void ProfileMake::PConvertPathToUTM()
{

	double *tempx, *tempy;
	int tempverts;
	double * tempreturn;

	if (isDebug) std::cout << "\nConverting path to UTM\n"; //test


	if (P_IsInterpolated)
	{
		tempx = P_Xi;
		tempy = P_Yi;
		tempverts = P_PathVerticesI;
	}
	else
	{
		tempx = P_Xi;
		tempy = P_Yi;
		tempverts = P_PathVertices;
	}


	for (int i = 0; i < tempverts; i++)
	{
		tempreturn = PToUTM(tempx[i], tempy[i]);
		tempx[i] = tempreturn[1];
		tempy[i] = tempreturn[0];
	}

	if (isDebug)
	{
		std::cout << "\nConverted path\n";
		for (int i = 0; i < P_PathVerticesI; i++)
		{
			std::cout << P_Xi[i] << ", " << P_Yi[i] << std::endl;
		}
	}

}

