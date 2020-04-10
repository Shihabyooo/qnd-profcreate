//Current known limitation:
	//Files where records are of length > 127 will bug out.
	//Assumes (and checks for, aborts otherwise) an SHP with only Polyline geometry.
	//Assumes (and checks for, warns but doens't abort) geometries with single parts (but any number of vertices, up to limit that causes length bug above).
	//Doesn't check the SHP's CRS.

//Things to add:
	//Implement and ByteToDouble() and switch uses of casting char* to double* to it instead.
	//Rename "byte" to "bytes"
	//Research and -accordingly- adjust handling of SHP files with geometries with more than 1 part.
	//Fix ByteToInt32's issue with ints > 127.
	//Allow the use of SHP files with point geometries (Will output a single path, with vertices going from first point to last inorder).
	//Allow use of multiple geometries SHP files, in which only polylines will be considered.
	//Parse DBF files and look for names to label the paths with (stored as std::string* or char**).
	//Parse PRJ files and and save the CRS of the SHP (enum?) for use by ProfileMake when interpolating elevations (i.e. to determine whether conversion between UTM/GCS is needed).

#pragma once
#include <fstream>
#include <iostream>
#include "Array2D.h"

class SHPParser
{
public:
	SHPParser();
	~SHPParser();

	bool LoadSHP(std::string fileName);
	int GetVertsCount(int shapeNo);	//returns -1 if shape doesn't exist or vertsCount array isn't allocated
	void UnLoadSHP();

private:
	const std::string RemoveFileExtension(const std::string fileName);
	bool CheckFileExistance(const std::string fileNamePrefix);
	void AllocateVertsArray();
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