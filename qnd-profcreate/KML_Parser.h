#pragma once
#include <fstream>
#include <iostream>

//TODO modify the naming scheme to remote this K letter prefix for methods and variables.
class KMLParser
{

public:
	KMLParser();
	~KMLParser();

	bool KLoadKML(std::string);
	double ** KGetPtrToVerts(); //doesn't work. For now letting external objects access K_Verts directly.
	int KGetVertCount();
	bool KUnloadKML();

private:
	bool KOpenKMLFile(std::string);
	void KCloseKMLFile();
	bool KSeekCoordsPosition();
	bool KCountVertices();
	bool KExtractPath();


private:
	std::fstream K_KMLfile;
	std::streampos K_coordBeginPos;
	int K_VertsCount;

public: 
	double ** K_Verts; //Probably not the smartest idea...
	bool K_IsPathLoaded;
};