#include "SHP_Parser.h"
#include <vector>

SHPParser::SHPParser()
{
	isPathLoaded = false;
}

SHPParser::~SHPParser()
{
	//Just in case, we check if our filestreams are open and close them if the are.
	if (shpFile.is_open())
		shpFile.close();
	
	if (shxFile.is_open())
		shxFile.close();

	//Now we delete our allocated arrays.
	if (verts != NULL) //TODO check if delete already checks for NULLity...
	{
		for (int i = 0; i < shapesCount; i++)
		{
			for (int j = 0; j < vertsCount[i]; j++)
				delete[] verts[i][j];
			delete[] verts[i];
		}
		delete verts;
	}

	if (vertsCount != NULL)
		delete vertsCount;
}

bool SHPParser::LoadSHP(std::string fileName)
{
	std::string fileNamePrefix = RemoveFileExtension(fileName);
	
	if (!CheckFileExistance(fileNamePrefix))
	{
		std::cout << "ERROR! One or more of the required ShapeFile files is missing.";
		return false;
	}

	LoadSHPParameters(fileNamePrefix);

	//allocate the paths holder array
	verts = new double**[shapesCount];

	for (int i = 0; i < shapesCount; i++)
	{
		verts[i] = new double*[vertsCount[i]];
		for (int j = 0; j < vertsCount[i]; j++)
		{
			verts[i][j] = new double[2]; //the lowest level of the array only holds two values: X and Y coords.
		}
	}

	OpenSHPFile(fileNamePrefix);
	if (!ExtractPaths())
	{
		std::cout << "ERROR! Something went wrong while extracting the paths" << std::endl; //TODO enrich this check
		return false;
	}

	CloseSHPFile();
	return true;
}

int SHPParser::GetVertsCount(int shapeNo)
{
	if (shapeNo < 0 || shapeNo > shapesCount || vertsCount == NULL)	//We test that: shapeNo is within shapesCount and the vertsCount is allocated
		return -1;
	else
		return vertsCount[shapeNo];
}

std::string SHPParser::RemoveFileExtension(std::string fileName)
{
	return fileName.substr(0, fileName.length() - 4);
}

bool SHPParser::CheckFileExistance(std::string fileNamePrefix)
{
	//In this method, we will check for the existence of all of our needed files. For now, we need the SHX and SHP files.
	//We'll have each failure set a more global bool to false instead of immediatly returning to simulate an error log message (i.e. so the user can know which files are missing).
	std::ifstream testStream;
	bool state = true;
	std::string shpFileComponent = fileNamePrefix + ".shp";
	std::string shxFileComponent = fileNamePrefix + ".shx";
	
	testStream.open(shpFileComponent, std::ios::binary | std::ios::in);
	if (!testStream.is_open())
	{
		std::cout << "ERROR! SHP file is missing." << std::endl;
		state = false;
	}
	testStream.close();
	
	testStream.open(shxFileComponent, std::ios::binary | std::ios::in);
	if (!testStream.is_open())
	{
		std::cout << "ERROR! SHX file is missing." << std::endl;
		state = false;
	}

	testStream.close();


	return state;
}

bool SHPParser::OpenSHPFile(std::string fileNamePrefix)
{
	std::string shpFileName = fileNamePrefix + ".shp";

	std::cout << "Attempting to open " << shpFileName.c_str() << "\n";
	shpFile.open(shpFileName, std::ios::binary | std::ios::in);
	
	if (!shpFile.is_open()) //redundant, we alread checked that we could open the file in CheckFileExistence(), but still...
	{
		std::cout << "Error: Could not open " << shpFileName.c_str() << "!\n\n";
		return false;
	}
	
	std::cout << "Successfully opened " << shpFileName.c_str() << "\n";
	return true;
}

void SHPParser::CloseSHPFile()
{
	if (shpFile.is_open())
		shpFile.close();
}

bool SHPParser::LoadSHPParameters(std::string fileNamePrefix)
{
	//This class is a single-pass-one, meaning we only scane the file once, load content into memory and be done with it. So we have no use for the SHX file's intended use of "fast seeking."
	//We will only use it figure out how many shapes the SHP contains, and the vertex count of each shape

	std::string shxFileName = fileNamePrefix + ".shx";

	std::cout << "Attempting to open " << shxFileName.c_str() << "\n";
	shxFile.open(shxFileName, std::ios::binary | std::ios::in);

	if (!shxFile.is_open())//redundant, we alread checked that we could open the file in CheckFileExistence(), but still...
	{
		std::cout << "Error: Could not open " << shxFileName.c_str() << "!\n\n";
		return false;
	}

	char byte[4]; //a 4 byte buffer to hold a single int32, will be used several times bellow.

	//We now check that our Shape Type is a polyline by checking that bytes 32-35 of the header are equal to 3 (the code for polylines).
	shxFile.seekg(32, shxFile.beg);
	shxFile.read(byte, sizeof(byte));
	//std::cout << "The type code of the contained geometry is: " << ByteToInt32(byte, false) << std::endl; //test

	if (ByteToInt32(byte, false) != 3)
	{
		std::cout << "ERROR! The provided SHP does not contain a polyline geometry." << std::endl;
		return false;
	}


	//Calculate the number of geometries in the SHP:
	shxFile.seekg(24, shxFile.beg); //go to location of file length data in header
	shxFile.read(byte, sizeof(byte)); //Read the shx file length
	//std::cout << "At loc " << shxFile.tellg() << ". File length: " << ByteToInt32(byte, true) << std::endl; //test

	//The shx file length is measured in WORDs, the file length include the header's size (fixed 100 bytes). Each geometry record afterwards is at a fixed 8 bytes (2 x 4bytes int32).
	//i.e. the number of geometries in the shape file = ((2 * file length) - 100) / 8

	shapesCount = ((2 * ByteToInt32(byte, true)) - 100) / 8; //in theory, the result of the outer brackets should always be devisible by 16, giving perfect ints...
	std::cout << "No. of records found: " << shapesCount << std::endl; //test

	if (shapesCount < 1) //if there are no shapes in the SHP, there is no point in continuting this process.
	{
		std::cout << "ERROR! No geometries were found in the provided SHP file." << std::endl;
		return false;
	}

	//now that we know how many geometries our SHP has, we can allocate our vertsCount array.
	vertsCount = new int(shapesCount);

	//Now to check the metadata of the actual geometries
	shxFile.seekg(100, shxFile.beg); //the header is fixed at 100 bytes, we simply skip it since we don't have a use for anything other than the Shape Type and File Length

	int counter = 0;
	while (true) //fstream.eof() don't get set until we read past it, so we have to do the eof check inside.
	{
		shxFile.read(byte, sizeof(byte)); //the first 4 bytes of a record header contains its offset, no need for them now.
		
		if (shxFile.eof())
			break;

		//std::cout << "At loc " << shxFile.tellg() << ". Found record at offset: " << ByteToInt32(byte, true) << "\t"; //test

		//Assuming each record has only one part (a requirement of this program), the coordinates of the record start 48 bytes from the record data start location.
		//The length of the record is measured in WORDs (2 bytes), the coordiantes are in xy pairs stored as doubles (i.e. each pair is 2 * 8bytes in size).
		//e.g. for a record of length = 104. Length in bytes = 104*2 = 208 bytes. Coords size = 208 - 48 = 160 bytes. Coords pair count = 160 / (8 * 2) = 10 coords.
		//In other words, the no.of vertices = no. of coord pairs = ((2 * record length) - 48) / 16.

		shxFile.read(byte, sizeof(byte)); //the second 4 bytes of a record header contains its length, we use this to calculate how many vertices a shape has.
		//std::cout << "At loc " << shxFile.tellg() << ". of length: " << ByteToInt32(byte, true) << std::endl; //test
		
		int verts = ((2 * ByteToInt32(byte, true)) - 48) / 16; //Again: in theory, the result of the outer brackets should always be devisible by 16, giving perfect ints...

		vertsCount[counter] = verts;
		counter++;
	}
	//TODO consider changing the While loop above to a for loop with shapeCount as limit

	//for (int i = 0; i < shapesCount; i++) //test
	//{
	//	std::cout << "Shape no. " << i << ", vertsCount: " << vertsCount[i] << std::endl; //test
	//}

	shxFile.close();
	return true;
}

bool SHPParser::ExtractPaths()
{
	//We assume that this method is called ONLY why shpFile filestream is still open.
	if (!shpFile.is_open())
	{
		std::cout << "ERROR! ExtractPaths() called without an open shpFile filestream" << std::endl;
		return false;
	}
	//alternatively, we could have this method recieve a string to the file and attempt to open it if it wasn't open already. Doubles the tests though...


	shpFile.seekg(100, shpFile.beg); //skip the header

	for (int i = 0; i < shapesCount; i++)
	{
		//At the beining of this loop, check that the parts number of the record (in32 starting from 36th Byte) is equal to 1.
		//Skip the record header (which we assume fixed at 48 bytes for a single part geometry, though we could estimate the jump by 44 + 4 x partNo).
		//Now start another loop with vertsCount[i] as limit
			//From here on, the data will be read in double pairs (x and y).
			//First double read will be assigned to verts[i][j][0];
			//Second double read will be assigned to verts[i][j][1];

	}


	return true;
}

long int SHPParser::ByteToInt32(char bytes[4], bool isBigEndian)
{
	if (isBigEndian)
		return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	else
		return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
}
