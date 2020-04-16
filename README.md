# qnd-profcreate
Quick and dirty profile creator using GDAL libs.
Interpolates elevation profiles (for paths provided as an -uncompressed- KML or ShapeFile) from ground elevation data (provided as grayscale geotif DEM/DTM/DSM) and outputs results as a CSV file.



VS project is configured in that GDAL headers and libs/dlls are located in C:/lib/GDAL.

Current -known- limitations:
- Large DEMs cause issues with the geotiff reader.
- No coordinate system parsing for shapefiles (CRS must be hardcoded in SHPParser class, and in cause of UTM, zone and hemisphere must be hardcoded into ProfileCreator).
- DEM path must be hardcoded int main().
- KML and SHP parsing only support polylines.