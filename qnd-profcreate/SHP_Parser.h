#pragma once
#include <fstream>
#include <iostream>

class SHPParser
{
public:
	SHPParser();
	~SHPParser();

	bool LoadSHP(std::string fileName);
	int GetVertsCount();
	bool UnLoadSHP();

private:
	std::string RemoveFileExtension(std::string fileName);
	bool CheckFileExistance(std::string fileNamePrefix);
	bool OpenSHPFile(std::string fileNamePrefix);
	void CloseSHPFile();
	bool LoadSHPParameters(std::string fileNamePrefix);	//this method will load the SHX file, and fill out shapesCount and vertsCount. Future implementation: parse DBF file to look for path names.
	bool ExtractPath();
	long int ByteToInt32(char bytes[4], bool isBigEndian);
	double ByteToDouble(char byte[8], bool isBigEndian);	//for Future implementation. For now, I'm sticking to static_cast<double> when reading the file.

private:
	double *** verts;
	bool isPathLoaded;

	std::ifstream shpFile;
	std::ifstream shxFile;
	int * vertsCount;	//array containing vertex count for each shape
	int shapesCount;	//array containing the number of shapes the SHP file has
};