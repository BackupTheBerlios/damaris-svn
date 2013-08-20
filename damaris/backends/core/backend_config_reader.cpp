/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/

#include <cstring>

#include "backend_config_reader.h"
namespace DAMARIS
{
BackendConfigReader::BackendConfigReader(const gchar *configPath):
	_damarisConfigPath(configPath),
	_keyFile (NULL)
{

	loadConfig();
	// if loadConfig succeeded, we should have a valid config file
}

void BackendConfigReader::loadConfig()
{
	bool fileFound = false;
	// check if user config file exists
	const gchar* usr_config_dir = g_get_user_config_dir();
	int configPathLength = strlen(usr_config_dir) + strlen(_damarisConfigPath);
	gchar *cfgFileName = new gchar[configPathLength > 1024 ? configPathLength: 1024];

	strcpy(cfgFileName, usr_config_dir);
	strcat(cfgFileName, _damarisConfigPath);

	_keyFile=g_key_file_new ();
	GError *error=NULL;
	if (!loadConfigFile(cfgFileName, G_KEY_FILE_NONE, error)) {
		// if the user config doesn't exist, attempt to read from a system config
		const gchar *const *sys_config_dirs = g_get_system_config_dirs();
		unsigned int i = 0;
		while (true)
		{
			if (sys_config_dirs[i]==NULL) { break;} // no more paths to check

			// avoid buffer overrun
			configPathLength = strlen(sys_config_dirs[i]) + strlen(_damarisConfigPath);
			if (configPathLength > 1024)
			{
				gchar *temp = cfgFileName;
				cfgFileName = new gchar[configPathLength];
				delete[] temp;
			}


			strcpy(cfgFileName, sys_config_dirs[i]);
			strcat(cfgFileName, _damarisConfigPath);

			if (loadConfigFile(cfgFileName, G_KEY_FILE_NONE, error))
			{
				// found a config file, breaking
				fileFound = true;
				break;
			}
			i++;
		}
	} else
	{
		fileFound = true;
	}
	delete[] cfgFileName;

	if (!fileFound)
	{
		throw(BackendConfigException("No config file found\n"));
	}
}

bool BackendConfigReader::loadConfigFile(const gchar *keyFileName, GKeyFileFlags flags, GError *error)
{
	if (!g_key_file_load_from_file (_keyFile, keyFileName, flags, &error)) {
		if (error->code == G_FILE_ERROR_NOENT) // file not found
		{
			return false;
		} else
		{
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "%s", error->message);
		}
		error = NULL;
	}
	return true;
}

}// namespace DAMARIS
