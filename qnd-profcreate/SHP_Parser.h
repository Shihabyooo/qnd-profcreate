#pragma once
#include <fstream>

class SHPParser
{
public:
	SHPParser();
	~SHPParser();

	bool LoadSHP(std::string);
	int GetVertsCount();
	bool UnLoadSHP();

private:
	bool OpenSHPFile(std::string);
	void CloseSHPFile();
	bool ExtractPath();


private:
	double ** verts;
	bool isPathLoaded;

	std::ifstream shpFile;
	int vertsCount;
};