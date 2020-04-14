#include "KML_Parser.h"

KMLParser::KMLParser()
{
	verts = NULL;
	//vertsCount = new int [1];
	//vertsCount[0] = 0;
	pathsCount = 0;
}

KMLParser::~KMLParser()
{
	/*if (verts != NULL)
	{
		for (int i = 0; i < vertsCount; i++)
			if (verts[i] != NULL)
			{
				delete[] verts[i];
			}
		delete[] verts;
	}*/

	UnloadKML();
}

bool KMLParser::LoadKML(std::string fileName)
{
	if (!OpenKMLFile(fileName))
	{
		std::cout << "Failed to load KML: " << fileName.c_str() << std::endl;
		return false;
	}

	if (!ExtractPaths())
	{
		std::cout << "Failed to extract paths from KML: " << fileName.c_str() << std::endl;
		return false;
	}

	CloseKMLFile();
	isPathLoaded = true;
	return true;
}

void KMLParser::UnloadKML()
{
	std::cout << "Unloading KML" << std::endl; //test

	if (verts != NULL)
	{
		for (int i = 0; i < pathsCount; i++)
			verts[i].~Array2D();

		delete[] verts;
		verts = NULL;
	}

	pathsCount = 0;
	//kmlFile.clear();
	isPathLoaded = false;
}

bool KMLParser::OpenKMLFile(std::string fileName)
{
	kmlFile.open(fileName, std::ios::in | std::ios::binary);
	if (!kmlFile.is_open())
	{
		std::cout << "Error: Could not load KML file!\n\n";
		return false;
	}
	return true;
}

void KMLParser::CloseKMLFile()
{
	kmlFile.close();
}

bool KMLParser::ExtractPaths()
{
	kmlFile.clear();
	kmlFile.seekg(0, kmlFile.beg);

	std::unique_ptr<char> tag = AdvanceToNextTag();

	//Find how many lines the kml has -> Allocate matrices accordingely.
	while (!kmlFile.eof() && tag != NULL)
	{
		if (CompareTags(tag.get(), KML_POLYLINE_TAG))
			pathsCount++;

		tag = AdvanceToNextTag();
	}
	
	std::cout << "Found " << pathsCount << " paths in file." << std::endl; //test
	
	verts = new Array2D[pathsCount];
	pathsNames = new std::string[pathsCount];
	
	kmlFile.clear();
	kmlFile.seekg(0, kmlFile.beg);
	tag = AdvanceToNextTag();

	//std::cout << "Relooping to extract data" << std::endl;
	
	int counter = 0;
	while (!kmlFile.eof() && tag != NULL)
	{
		//outer Extraction loop
			//Find <Placemark>
				//Find <name> -> save value to pathsNames;
				//Find <LineString>
				//Find <coordinates>
					//Read coordinates in pair, make sure that you check whether after the second coord their is a third (Google Earth adds it even if zero, QGIS doesn't)
					//Increment vertsCount for this path

		tag = AdvanceToNextTag();

		if (CompareTags(tag.get(), "<Placemark>"))
		{
			std::cout << "Found a Placemark" << std::endl;
			while (!CompareTags(tag.get(), "</Placemark>") && tag != NULL)
			{
				tag = AdvanceToNextTag(); 
				
				if (CompareTags(tag.get(), KML_NAME_TAG))
				{
					KMLElement element = GetCurrentElementValue();
					std::string _name(element.ptrToData.get());
					
					_name[element.dataLength] = '\0';

					std::cout << "Found name: " << _name.c_str() << std::endl;
					pathsNames[counter] = _name;
				}
				else if (CompareTags(tag.get(), KML_COORDINATES_TAG))
				{
					KMLElement element = GetCurrentElementValue();

					if (!ExtractCoordinatesFromKMLElement(&element, counter))
						return false;
				}
			}
			counter++;
		}
	}

	return true;
}

//bool KMLParser::LoadKML(std::string fileName)
//{
//	if (!OpenKMLFile(fileName))
//	{
//		//std::cout << "Failed to load KML: " << fileName.c_str() << std::endl;
//		return false;
//	}
//	if (!SeekCoordsPosition())
//	{
//		return false;
//	}
//	if (!CountVertices())
//	{
//		return false;
//	}
//	if (!ExtractPath())
//	{
//		return false;
//	}
//	CloseKMLFile();
//	isPathLoaded = true;
//	return true;
//}
//
////double ** KMLParser::GetPtrToVerts()
////{
////	return verts;
////}
//
//
//void KMLParser::UnloadKML()
//{
//	/*if (!isPathLoaded)
//		return false;*/
//	
//	/*for (int i = 0; i < vertsCount; i++)
//	{
//		delete verts[i];
//		verts[i] = NULL;
//	}
//	delete[] verts;
//	verts = NULL;
//*/
//
//	std::cout << "Unloading KML" << std::endl; //test
//
//	if (verts != NULL)
//	{
//		for (int i = 0; i < pathsCount; i++)
//			verts[i].~Array2D();
//
//		delete[] verts;
//		verts = NULL;
//	}
//
//	vertsCount = 0;
//	coordBeginPos = 0;
//	pathsCount = 0;
//	//kmlFile.clear();
//	isPathLoaded = false;
//}
//
//bool KMLParser::OpenKMLFile(std::string fileName)
//{
//	kmlFile.open(fileName, std::ios::in | std::ios::binary);
//	if (!kmlFile.is_open())
//	{
//		std::cout << "Error: Could not load KML file!\n\n";
//		return false;
//	}
//	return true;
//}
//
//void KMLParser::CloseKMLFile()
//{
//	kmlFile.close();
//}
//
//bool KMLParser::SeekCoordsPosition()
//{
//	char cbuffer;
//	std::string sbuffer = "";
//	coordBeginPos = kmlFile.beg; //will have to nix this if I ever modified this method to a "seek next coords" in multi-featured KML files.
//
//	while (sbuffer != "coordinates")
//	{
//		kmlFile.get(cbuffer);
//
//		if (cbuffer == '<')
//		{
//			sbuffer = "";
//			kmlFile.get(cbuffer);
//
//			while (cbuffer != '>')
//			{
//				sbuffer += cbuffer;
//				kmlFile.get(cbuffer);
//			}
//			//std::cout << "Sbuffer: " << sbuffer.c_str() << std::endl; //test
//
//		}
//		if (kmlFile.eof())
//		{
//			std::cout << "[DevWarning: Reached EOF]\n";
//			std::cout << "Error: Could not find coordinate data in KML file!\n";
//			return false;
//		}
//	}
//	coordBeginPos = kmlFile.tellg();
//	return true;
//}
//
//bool KMLParser::CountVertices()
//{
//	char cbuffer = '1';
//	while (cbuffer != '<' && cbuffer != '/')
//	{
//		kmlFile.get(cbuffer);
//		if (cbuffer == ' ' || cbuffer == '<')
//		{
//			//hack to get around how software differ in terminating blocks
//			//check that following the space is a number
//			kmlFile.get(cbuffer);
//			if (cbuffer != '<' && cbuffer != '\n')
//			{
//				vertsCount[0]++;
//			}
//		}
//
//		if (kmlFile.eof())
//		{
//			std::cout << "[DevWarning: Reached EOF]\n";
//			std::cout << "Error: Could not extract coordinate data in KML file!\n";
//			return false;
//		}
//	}
//	return true;
//}
//
//bool KMLParser::ExtractPath()
//{
//	kmlFile.seekg(coordBeginPos, std::ios::beg);
//
//	/*K_X = new double[vertsCount];
//	K_Y = new double[vertsCount];*/
//
//	/*verts = new double * [vertsCount];
//	for (int i = 0; i < vertsCount; i++)
//	{
//		verts[i] = new double[2];
//	}*/
//
//	pathsCount = 1; //TODO modify this once you start reworking this class to extract and hold multiple paths.
//	verts = new Array2D[pathsCount]; 
//	verts[0] = Array2D(vertsCount[0], 2);
//
//
//	//char cbuffer2[100];
//	std::string sbuffer;
//	char cbuffer;
//	for (int i = 0; i < vertsCount[0]; i++)
//	{
//		sbuffer = "";
//		kmlFile.get(cbuffer);
//		while (cbuffer != ',')
//		{
//			sbuffer += cbuffer;
//			kmlFile.get(cbuffer);
//		}
//		//verts[i][0] = atof(sbuffer.c_str());
//		verts[0][i][0] = atof(sbuffer.c_str());
//
//		sbuffer = "";
//		kmlFile.get(cbuffer);
//		while (cbuffer != ',' && cbuffer != ' ' && cbuffer != '\n')
//		{
//			sbuffer += cbuffer;
//			kmlFile.get(cbuffer);
//		}
//		//verts[i][1] = atof(sbuffer.c_str());
//		verts[0][i][1] = atof(sbuffer.c_str());
//
//		if (cbuffer == ',')
//		{
//			while (cbuffer != ' ' && cbuffer != '\n')
//				kmlFile.get(cbuffer);
//		}
//
//	}
//
//
//	//testing a new implementation for KML file (doing it here to avoid breaking the program)
//	kmlFile.clear();
//	kmlFile.seekg(0, kmlFile.beg);
//
//	std::string * _pathsNames;
//	int _pathsCount = 0;
//	Array2D * _verts;
//	std::unique_ptr<char> tag = AdvanceToNextTag();
//
//	//Find how many lines the kml has -> Allocate matrices accordingely.
//	while (!kmlFile.eof() && tag != NULL)
//	{
//		if (CompareTags(tag.get(), KML_POLYLINE_TAG))
//			_pathsCount++;
//
//		tag = AdvanceToNextTag();
//	}
//	
//	std::cout << "Found " << _pathsCount << " paths in file." << std::endl; //test
//	
//	_verts = new Array2D[_pathsCount];
//	_pathsNames = new std::string[_pathsCount];
//	
//	kmlFile.clear();
//	kmlFile.seekg(0, kmlFile.beg);
//	tag = AdvanceToNextTag();
//	std::cout << "Relooping to extract data" << std::endl;
//	
//	int counter = 0;
//	while (!kmlFile.eof() && tag != NULL)
//	{
//		//outer Extraction loop
//			//Find <Placemark>
//				//Find <name> -> save value to pathsNames;
//				//Find <LineString>
//				//Find <coordinates>
//					//Read coordinates in pair, make sure that you check whether after the second coord their is a third (Google Earth adds it even if zero, QGIS doesn't)
//					//Increment vertsCount for this path
//
//		tag = AdvanceToNextTag();
//
//		if (CompareTags(tag.get(), "<Placemark>"))
//		{
//			std::cout << "Found a Placemark" << std::endl;
//			while (!CompareTags(tag.get(), "</Placemark>") && tag != NULL)
//			{
//				tag = AdvanceToNextTag(); 
//				
//				if (CompareTags(tag.get(), KML_NAME_TAG))
//				{
//					KMLElement element = GetCurrentElementValue();
//					std::string _name(element.ptrToData.get());
//					
//					_name[element.dataLength] = '\0';
//
//					std::cout << "Found name: " << _name.c_str() << std::endl;
//					_pathsNames[counter] = _name;
//				}
//				else if (CompareTags(tag.get(), KML_COORDINATES_TAG))
//				{
//					KMLElement element = GetCurrentElementValue();
//
//					//This part should go into a method that takes KMLElement* and an int for id
//					double _xCoord, _yCoord;
//
//					CountCoordinatesInKMLElement(&element);
//
//
//					for (int i = 0; i < element.dataLength; i++)
//					{
//						//std::cout << element.ptrToData.get()[i];
//
//					}												
//					//std::cout << std::endl;
//				}
//			}
//			counter++;
//		}
//	}
//
//	return true;
//}

std::unique_ptr<char> KMLParser::AdvanceToNextTag()
{
	if (!kmlFile.is_open())
		return NULL;

	char letter = ' ';
	unsigned long long int tagBeginLoc, tagEndLoc;

	while (letter != '<') //seek begining of tag;
	{
		kmlFile.read(&letter, sizeof(letter));
		if (kmlFile.eof())
			return NULL;
	}

	tagBeginLoc = kmlFile.tellg();
	tagBeginLoc -= 1; //to accomodate the character we've already read.

	letter = ' ';
	while (letter != '>')
	{
		kmlFile.read(&letter, sizeof(letter));
		if (kmlFile.eof())
			return NULL;
	}

	tagEndLoc = kmlFile.tellg();

	std::unique_ptr<char> tag(new char[tagEndLoc - tagBeginLoc]);

	kmlFile.seekg(tagBeginLoc, kmlFile.beg);

	//std::cout << "Found tag: "; //test
	for (int i = 0; i < tagEndLoc - tagBeginLoc; i++)
	{
		kmlFile.read(&letter, sizeof(letter));
		tag.get()[i] = letter;
		//std::cout << tag.get()[i]; //test
	}
	//std::cout << std::endl; //test

	return tag;
}

KMLElement KMLParser::GetCurrentElementValue()
{
	if (!kmlFile.is_open())
		return KMLElement();

	char letter = ' ';
	unsigned long long int valBeginLoc, valEndLoc;

	valBeginLoc = kmlFile.tellg();

	letter = ' ';
	while (letter != '<')
	{
		kmlFile.read(&letter, sizeof(letter));
		if (kmlFile.eof())
			return KMLElement();
	}

	valEndLoc = kmlFile.tellg();
	valEndLoc -= 1;

	KMLElement value;

	value.ptrToData = std::unique_ptr<char>(new char[valEndLoc - valBeginLoc]);
	value.dataLength = valEndLoc - valBeginLoc;

	kmlFile.seekg(valBeginLoc, kmlFile.beg);

	//std::cout << "Found element value: "; //test
	for (int i = 0; i < valEndLoc - valBeginLoc; i++)
	{
		kmlFile.read(&letter, sizeof(letter));
		value.ptrToData.get()[i] = letter;
		//std::cout << value.get()[i]; //test
	}
	//std::cout << std::endl; //test

	return value;
}

bool KMLParser::CompareTags(char * tag1, char * tag2) const
{
	////test
	//std::cout << "Comparing ";
	//
	//char * tempP = tag1;
	//while (*tempP != '>')
	//{
	//	std::cout << *tempP;
	//	tempP++;
	//}
	//std::cout << "> to ";
	//tempP = tag2;
	//while (*tempP != '>')
	//{
	//	std::cout << *tempP;
	//	tempP++;
	//}
	//std::cout << ">" << std::endl;
	////end test

	if (tag1 == NULL)
	{
		std::cout << "WARNING! Trying to compare a tag of NULL value" << std::endl;
		return false;
	}

	while (*tag1 == *tag2)
	{
		if (*tag1 == '>' && *tag2 == '>')
			return true;

		tag1++;
		tag2++;
	}
	
	return false;
}

bool KMLParser::ExtractCoordinatesFromKMLElement(KMLElement * element, int pathID)
{
	int commaDelimitersPerCoordinate = 0;

	//first, determine whether coordinates are saved in pairs or trios.
	for (int i = 0; i < element->dataLength; i++)
	{
		if (element->ptrToData.get()[i] == ' ')
		{
			for (int j = i - 1; j >= 0; j--)
			{
				if (element->ptrToData.get()[j] == ',')
					commaDelimitersPerCoordinate++;
			}
			break;
		}
		else if (i == element->dataLength - 1) //reached end of element without finding a space delimiter
			return false;
	}

	if (commaDelimitersPerCoordinate < 1 || commaDelimitersPerCoordinate > 2)
		return false;

	//std::cout << "commaDelimitersPerCoordinate: " << commaDelimitersPerCoordinate << std::endl;
	//double _x, _y;

	//Now we read our elements to a temporary Vector.
	Array2D coord = Array2D(3, 1);
	std::vector<Array2D> tempContainer;
	std::string sBuffer;

	for (int i = 0; i < element->dataLength; i++) //loop through entire element
	{
		if (element->ptrToData.get()[i] == '0' && i < element->dataLength - 1)
			i++;

		for (int j = 0; j < commaDelimitersPerCoordinate + 1; j++) //loop through a single pair/trio.
		{
			sBuffer = "";
			while (element->ptrToData.get()[i] != ',' && element->ptrToData.get()[i] != ' ' && i < element->dataLength) 
			{
				sBuffer += element->ptrToData.get()[i];
				i++;
			}
			coord[j][0] = std::stod(sBuffer.c_str());
			i++;
		}
		
		i--; //the last i++ of the j-loop would jump over the space delimiter, resulting in a removal of the largest significant digit of the following x-coordinates.
		tempContainer.push_back(coord);
	}

	//std::cout << "content of element: " << pathID << ", length: " << tempContainer.size() << std::endl;//test
	//for (std::vector<Array2D>::iterator it = tempContainer.begin(); it < tempContainer.end(); ++it)
	//{
	//	std::cout << it->GetValue(0, 0) << ", " << it->GetValue(1, 0) << ", " << it->GetValue(2, 0) << std::endl; //test
	//}

	//We then move the data from our vector to our Array2D *;
	verts[pathID] = Array2D(tempContainer.size(), 2);
	int counter = 0;
	for (std::vector<Array2D>::iterator it = tempContainer.begin(); it < tempContainer.end(); ++it)
	{
		verts[pathID][counter][0] = it->GetValue(0, 0);
		verts[pathID][counter][1] = it->GetValue(1, 0);
		counter++;
	}
	
	return true;
}
