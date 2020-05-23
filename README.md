# qnd-profcreate
Quick-and-dirty profile creator.
Interpolates elevation profiles (for paths provided as an -uncompressed- KML or an ESRI ShapeFile) from ground elevation data (provided as a GeoTIFF DEM/DTM/DSM) and outputs results as a CSV file.

The GUI makes use of the awesome [Dear ImGui](https://github.com/ocornut/imgui) library.
The file parsers are all home-brewed, so they are -currently- severely lacking in feature support.


This code Requires C++17 support. This code was tested on Microsoft Visual Studio 2017 (MSVC 14) on Windows 10 - 1909.


Some -known- limitations:
- GeoTIFF parser only supports non-compressed and PackBits compressed TIFFs.
- No coordinate system parsing for shapefiles (CRS must be hardcoded in SHPParser class, and in cause of UTM, zone and hemisphere must be hardcoded into ProfileCreator).
- KML and SHP parsing only support polylines.