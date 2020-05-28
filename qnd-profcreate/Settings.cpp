#include "Settings.h"

Settings settings;

SettingsEntry entry; //to avoid passing the entire struct between LoadNextEntry and LoadSettings for every line.

bool LoadNextEntry(std::ifstream * settingsStream) //returns false if the line is a comment (begins with #) or if EoF, true otherwise.
{
	char cBuffer = 'A';
	entry.name = "";

	settingsStream->read(&cBuffer, sizeof(cBuffer));
	if (cBuffer == '#')
		return false;
	entry.name += cBuffer;

	while (cBuffer != '=' && cBuffer != ' ')
	{
		settingsStream->read(&cBuffer, sizeof(cBuffer));
		if (settingsStream->eof() || cBuffer == '\n' || cBuffer	== '\r\n')
			return false;

		entry.name += cBuffer;
	}

	entry.name.pop_back(); //remove the excess =

	cBuffer = ' ';
	std::string sBuffer = "";
	while (cBuffer != '\r\n' && cBuffer != '\n') //shouldn't '\n' alone suffice? is '\r\n' considered a single char?
	{
		settingsStream->read(&cBuffer, sizeof(cBuffer));
		if (settingsStream->eof())
			return false;

		sBuffer += cBuffer;
	}

	entry.value = atoi(sBuffer.c_str());
	
	return true;
}

void ApplyLoadedEntry()
{
	if (entry.name == SET_WIDTH)
	{
		if (entry.value >= MIN_WINDOW_WIDTH)
			settings.windowWidth = entry.value;
		else
			settings.windowWidth = DEFAULT_WINDOW_WIDTH;
	}
	else if (entry.name == SET_HEIGHT)
	{
		settings.windowHeight = entry.value;

		if (entry.value >= MIN_WINDOW_HEIGHT)
			settings.windowHeight = entry.value;
		else
			settings.windowHeight = DEFAULT_WINDOW_HEIGHT;
	}
	else if (entry.name == SET_RENDER)
	{
		switch (entry.value)
		{
		case (static_cast<size_t>(RenderingAPI::directX10)):
			settings.renderer = RenderingAPI::directX10;
			break;
		case (static_cast<size_t>(RenderingAPI::directX11)):
			settings.renderer = RenderingAPI::directX11;
			break;
		case (static_cast<size_t>(RenderingAPI::openGL2)):
			settings.renderer = RenderingAPI::openGL2;
			break;
		case (static_cast<size_t>(RenderingAPI::openGL3)):
			settings.renderer = RenderingAPI::openGL3;
			break;
		default:
			settings.renderer = DEFAULT_RENDERER;
			break;
		}
	}
	else if (entry.name == SET_CLI_VERBOSE)
	{
		settings.verboseCLI = static_cast<bool>(entry.value);
	}
}

bool LoadSettings(std::string & path) //returns true if setting file exists (regardless of whether its content are usable or not.
{
	std::cout << "Attempting to load settings" << std::endl; //test
	std::ifstream settingsStream;
	settingsStream.open(path);

	if (!settingsStream.is_open())
	{
		std::cout << "Failed to open config file. File Does not exist?" << std::endl; //TODO should use FileSystem to check whether the issue is with file existance or inability to open it.
		return false;
	}

	while (!settingsStream.eof())
	{
		while (!LoadNextEntry(&settingsStream)) //cycles the lines until we reach a line with a valid entry.
		{
			if (settingsStream.eof())
			{
				settingsStream.close();
				return true;
			}
		}
		ApplyLoadedEntry();

		//LoadNextEntry(&settingsStream);
	}


	settingsStream.close();
	return true;
}

bool WriteCurrentSettings(std::string &path)
{
	std::cout << "Attempting to write settings" << std::endl; //test

	std::ofstream settingsStream;
	settingsStream.open(path, std::ios::trunc);

	if (!settingsStream.is_open())
	{
		std::cout << "Failed to open config file." << std::endl;
		return false;
	}

	settingsStream << "#Configuration File for QnD Profile Creator." << std::endl;
	settingsStream << "#Lines preceded with hash # are comments." << std::endl;
	settingsStream << "#" << std::endl;
	settingsStream << "#" << std::endl;
	settingsStream << "#Width and Height values less than" << MIN_WINDOW_WIDTH  << " and " << MIN_WINDOW_HEIGHT  << " will be overridden."<< std::endl;
	settingsStream << SET_WIDTH"=" << settings.windowWidth << std::endl;
	settingsStream << SET_HEIGHT"=" << settings.windowHeight << std::endl;
	settingsStream << "#Renderer code 0 = D3D10, 1 = D3D11, 2 = OpenGL2, 3 = OpenGL3." << std::endl;
	settingsStream << SET_RENDER"=" << static_cast<size_t>(settings.renderer) << std::endl;
	settingsStream << SET_CLI_VERBOSE"=" << static_cast<size_t>(settings.verboseCLI) << std::endl;


	settingsStream.close();
	return true;
}


