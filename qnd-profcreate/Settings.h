#pragma once
#include <fstream>
#include "LogWindow.h"
#include "FilesystemBrowser.h"

//TODO replace this implementation with a decent (xml based?) one in future.
#define DEFAULT_RENDERER Renderer::directX11;
#define DEFAULT_WINDOW_WIDTH 1024 //XGA width
#define DEFAULT_WINDOW_HEIGHT 768 //XGA height
#define MIN_WINDOW_WIDTH 100 //minimum width less than which the default width would be forced (on startup)
#define MIN_WINDOW_HEIGHT 100 ////minimum height less than which the default height would be forced (on startup)



#define SET_WIDTH "WindowWidth"
#define SET_HEIGHT "WindowHeight"
#define SET_RENDER "Renderer"
#define SET_CLI_VERBOSE "CLIVerboseOutput"

enum class Renderer
{
	directX10 = 0, directX11 = 1, openGL2 = 2, openGL3 = 3
};

struct Settings
{
public:
	size_t windowWidth = DEFAULT_WINDOW_WIDTH;
	size_t windowHeight = DEFAULT_WINDOW_HEIGHT;

	Renderer renderer = DEFAULT_RENDERER;

	bool verboseCLI = false;
};


struct SettingsEntry
{
public:
	std::string name;
	std::size_t value;
};


extern Settings settings;

bool LoadSettings(std::string &path);
bool WriteCurrentSettings(std::string &path);