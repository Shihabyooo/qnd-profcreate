#pragma once
#include <fstream>
#include <iostream>
#include "Array2D.h"

//struct Vertex
//{
//public:
//	Vertex() {};
//	~Vertex() { std::cout << "Vertex delete called" << std::endl; };
//
//	double x;
//	double y;
//};

class SHPParser
{
public:
	SHPParser();
	~SHPParser();

	bool LoadSHP(std::string fileName);
	int GetVertsCount(int shapeNo);	//returns -1 if shape doesn't exist or vertsCount array isn't allocated
	bool UnLoadSHP();

private:
	const std::string RemoveFileExtension(const std::string fileName);
	bool CheckFileExistance(const std::string fileNamePrefix);
	void AllocateVertsArray();
	//bool OpenSHPFile(std::string fileNamePrefix);
	//void CloseSHPFile();
	bool LoadSHPParameters(const std::string fileNamePrefix);	//this method will load the SHX file, and fill out shapesCount and vertsCount. Future implementation: parse DBF file to look for path names.
	bool ExtractPaths(const std::string fileNamePrefix);
	const long int ByteToInt32(const char bytes[4], bool isBigEndian);
	double ByteToDouble(char byte[8], bool isBigEndian);	//for Future implementation. For now, I'm sticking to static_cast<double> when reading the file.

private:
	//Vertex ** verts;
	Array2D * verts;
	bool isPathLoaded;

	long int * vertsCount;	//array containing vertex count for each shape, the topmost level is the shape number/ID, the second is the vertex order, the third is the vertex coord (east and north).
	long int shapesCount;	//array containing the number of shapes the SHP file has
};