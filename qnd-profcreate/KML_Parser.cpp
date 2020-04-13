#include "KML_Parser.h"

KMLParser::KMLParser()
{
	verts = NULL;
	vertsCount = 0;
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
		//std::cout << "Failed to load KML: " << fileName.c_str() << std::endl;
		return false;
	}
	if (!SeekCoordsPosition())
	{
		return false;
	}
	if (!CountVertices())
	{
		return false;
	}
	if (!ExtractPath())
	{
		return false;
	}
	CloseKMLFile();
	isPathLoaded = true;
	return true;
}

//double ** KMLParser::GetPtrToVerts()
//{
//	return verts;
//}

int KMLParser::GetVertCount()
{
	return vertsCount;
}

void KMLParser::UnloadKML()
{
	/*if (!isPathLoaded)
		return false;*/
	
	/*for (int i = 0; i < vertsCount; i++)
	{
		delete verts[i];
		verts[i] = NULL;
	}
	delete[] verts;
	verts = NULL;
*/

	std::cout << "Unloading KML" << std::endl; //test

	if (verts != NULL)
	{
		for (int i = 0; i < pathsCount; i++)
		{
			std::cout << "Test" << std::endl; //test
			verts[i].~Array2D();
		}
		delete[] verts;
		verts = NULL;
	}

	vertsCount = 0;
	coordBeginPos = 0;
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

bool KMLParser::SeekCoordsPosition()
{
	char cbuffer;
	std::string sbuffer = "";
	coordBeginPos = kmlFile.beg; //will have to nix this if I ever modified this method to a "seek next coords" in multi-featured KML files.

	while (sbuffer != "coordinates")
	{
		kmlFile.get(cbuffer);

		if (cbuffer == '<')
		{
			sbuffer = "";
			kmlFile.get(cbuffer);

			while (cbuffer != '>')
			{
				sbuffer += cbuffer;
				kmlFile.get(cbuffer);
			}
			//std::cout << "Sbuffer: " << sbuffer.c_str() << std::endl; //test

		}
		if (kmlFile.eof())
		{
			std::cout << "[DevWarning: Reached EOF]\n";
			std::cout << "Error: Could not find coordinate data in KML file!\n";
			return false;
		}
	}
	coordBeginPos = kmlFile.tellg();
	return true;
}

bool KMLParser::CountVertices()
{
	char cbuffer = '1';
	while (cbuffer != '<' && cbuffer != '/')
	{
		kmlFile.get(cbuffer);
		if (cbuffer == ' ' || cbuffer == '<')
		{
			//hack to get around how software differ in terminating blocks
			//check that following the space is a number
			kmlFile.get(cbuffer);
			if (cbuffer != '<' && cbuffer != '\n')
			{
				vertsCount++;
			}
		}

		if (kmlFile.eof())
		{
			std::cout << "[DevWarning: Reached EOF]\n";
			std::cout << "Error: Could not extract coordinate data in KML file!\n";
			return false;
		}
	}
	return true;
}

bool KMLParser::ExtractPath()
{
	kmlFile.seekg(coordBeginPos, std::ios::beg);

	/*K_X = new double[vertsCount];
	K_Y = new double[vertsCount];*/

	/*verts = new double * [vertsCount];
	for (int i = 0; i < vertsCount; i++)
	{
		verts[i] = new double[2];
	}*/

	pathsCount = 1; //TODO modify this once you start reworking this class to extract and hold multiple paths.
	verts = new Array2D[pathsCount]; 
	verts[0] = Array2D(vertsCount, 2);


	//char cbuffer2[100];
	std::string sbuffer;
	char cbuffer;
	for (int i = 0; i < vertsCount; i++)
	{
		sbuffer = "";
		kmlFile.get(cbuffer);
		while (cbuffer != ',')
		{
			sbuffer += cbuffer;
			kmlFile.get(cbuffer);
		}
		//verts[i][0] = atof(sbuffer.c_str());
		verts[0][i][0] = atof(sbuffer.c_str());

		sbuffer = "";
		kmlFile.get(cbuffer);
		while (cbuffer != ',' && cbuffer != ' ' && cbuffer != '\n')
		{
			sbuffer += cbuffer;
			kmlFile.get(cbuffer);
		}
		//verts[i][1] = atof(sbuffer.c_str());
		verts[0][i][1] = atof(sbuffer.c_str());

		if (cbuffer == ',')
		{
			while (cbuffer != ' ' && cbuffer != '\n')
				kmlFile.get(cbuffer);
		}

	}


	//test
	std::cout << "KML path: " << std::endl;
	//verts[0].DisplayArrayInCLI();
	std::cout << verts[0][0][0] << "\t" << verts[0][0][1] << std::endl;
	std::cout << verts[0][1][0] << "\t" << verts[0][1][1] << std::endl;

	return true;

}
