#pragma once
#include <fstream>
#include <iostream>

class KMLParser
{

public:
	KMLParser();
	~KMLParser();

	bool LoadKML(std::string);
	double ** GetPtrToVerts(); //doesn't work. For now letting external objects access verts directly.
	int GetVertCount();
	bool UnloadKML();

private:
	bool OpenKMLFile(std::string);
	void CloseKMLFile();
	bool SeekCoordsPosition();
	bool CountVertices();
	bool ExtractPath();


private:
	std::fstream kmlFile;
	std::streampos coordBeginPos;
	int vertsCount;

public: 
	double ** verts; //Probably not the smartest idea...
	bool isPathLoaded;
};