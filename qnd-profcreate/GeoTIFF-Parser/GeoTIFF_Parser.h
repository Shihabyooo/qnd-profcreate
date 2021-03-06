//Currently only supports TIFF files with one image (IFD), as in this program will only parse the contents of the first image within the TIFF file, disregarding the rest.
//Support for compressed TIFF files is not yet implemented.
//Support for Tile-Bitmaped TIFF files is not yet implemented.

#pragma once
#include <iostream>
#include <fstream>
#include <memory>

#include "Array2D.h"
#include "GeoTIFF_Parser_Globals.h"
#include "Decompressor.h"

bool LoadGeoTIFFHeaders(const char * filePath, bool closeStreamAtEnd = true);
bool LoadGeoTIFFHeaders(const std::string &filePath, bool closeStreamAtEnd = true);
bool LoadGeoTIFF(const char * filePath);
bool LoadGeoTIFF(const std::string &filePath);
void UnloadGeoTIFF();
const Array2D * GetPointerToBitmap();
double GetSample(unsigned long int x, unsigned long int y, unsigned int sampleOrder);

void DisplayTIFFDetailsOnCLI();
void DisplayGeoTIFFDetailsOnCLI();
void DisplayBitmapOnCLI();
