#include "KML_Parser.h"

KMLParser::KMLParser()
{
}

KMLParser::~KMLParser()
{
	if (K_Verts != NULL)
	{
		for (int i = 0; i < K_VertsCount; i++)
			if (K_Verts[i] != NULL)
			{
				delete[] K_Verts[i];
			}
		delete[] K_Verts;
	}
	K_VertsCount = 0;
}

bool KMLParser::KLoadKML(std::string fileName)
{
	if (!KOpenKMLFile(fileName))
	{
		//std::cout << "Failed to load KML: " << fileName.c_str() << std::endl;
		return false;
	}
	if (!KSeekCoordsPosition())
	{
		return false;
	}
	if (!KCountVertices())
	{
		return false;
	}
	if (!KExtractPath())
	{
		return false;
	}
	KCloseKMLFile();
	K_IsPathLoaded = true;
	return true;
}

double ** KMLParser::KGetPtrToVerts()
{
	return K_Verts;
}

int KMLParser::KGetVertCount()
{
	return K_VertsCount;
}

bool KMLParser::KUnloadKML()
{
	if (!K_IsPathLoaded)
		return false;
	
	for (int i = 0; i < K_VertsCount; i++)
	{
		delete K_Verts[i];
		K_Verts[i] = NULL;
	}
	delete[] K_Verts;
	K_Verts = NULL;

	K_coordBeginPos = 0;
	K_KMLfile.clear();
	K_VertsCount = 0;
	K_IsPathLoaded = false;
	return true;
}

bool KMLParser::KOpenKMLFile(std::string fileName)
{
	K_KMLfile.open(fileName, std::ios::in | std::ios::binary);
	if (!K_KMLfile.is_open())
	{
		std::cout << "Error: Could not load KML file!\n\n";
		return false;
	}
	return true;
}

void KMLParser::KCloseKMLFile()
{
	K_KMLfile.close();
}

bool KMLParser::KSeekCoordsPosition()
{
	char cbuffer;
	std::string sbuffer = "";
	K_coordBeginPos = K_KMLfile.beg; //will have to nix this if I ever modified this method to a "seek next coords" in multi-featured KML files.

	while (sbuffer != "coordinates")
	{
		K_KMLfile.get(cbuffer);

		if (cbuffer == '<')
		{
			sbuffer = "";
			K_KMLfile.get(cbuffer);

			while (cbuffer != '>')
			{
				sbuffer += cbuffer;
				K_KMLfile.get(cbuffer);
			}
			//std::cout << "Sbuffer: " << sbuffer.c_str() << std::endl; //test

		}
		if (K_KMLfile.eof())
		{
			std::cout << "[DevWarning: Reached EOF]\n";
			std::cout << "Error: Could not find coordinate data in KML file!\n";
			return false;
		}
	}
	K_coordBeginPos = K_KMLfile.tellg();
	return true;
}

bool KMLParser::KCountVertices()
{
	char cbuffer = '1';
	while (cbuffer != '<' && cbuffer != '/')
	{
		K_KMLfile.get(cbuffer);
		if (cbuffer == ' ' || cbuffer == '<')
		{
			//hack to get around how software differ in terminating blocks
			//check that following the space is a number
			K_KMLfile.get(cbuffer);
			if (cbuffer != '<' && cbuffer != '\n')
			{
				K_VertsCount++;
			}
		}

		if (K_KMLfile.eof())
		{
			std::cout << "[DevWarning: Reached EOF]\n";
			std::cout << "Error: Could not extract coordinate data in KML file!\n";
			return false;
		}
	}
	return true;
}

bool KMLParser::KExtractPath()
{
	K_KMLfile.seekg(K_coordBeginPos, std::ios::beg);

	/*K_X = new double[K_VertsCount];
	K_Y = new double[K_VertsCount];*/

	K_Verts = new double * [K_VertsCount];
	for (int i = 0; i < K_VertsCount; i++)
	{
		K_Verts[i] = new double[2];
	}



	//char cbuffer2[100];
	std::string sbuffer;
	char cbuffer;
	for (int i = 0; i < K_VertsCount; i++)
	{
		sbuffer = "";
		K_KMLfile.get(cbuffer);
		while (cbuffer != ',')
		{
			sbuffer += cbuffer;
			K_KMLfile.get(cbuffer);
		}
		K_Verts[i][0] = atof(sbuffer.c_str());

		sbuffer = "";
		K_KMLfile.get(cbuffer);
		while (cbuffer != ',' && cbuffer != ' ' && cbuffer != '\n')
		{
			sbuffer += cbuffer;
			K_KMLfile.get(cbuffer);
		}
		K_Verts[i][1] = atof(sbuffer.c_str());

		if (cbuffer == ',')
		{
			while (cbuffer != ' ' && cbuffer != '\n')
				K_KMLfile.get(cbuffer);
		}

	}

	return true;

}
