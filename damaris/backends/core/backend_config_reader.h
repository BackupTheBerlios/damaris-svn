/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/
#ifndef DAMARIS_backend_config_reader_h
#define DAMARIS_backend_config_reader_h


#include <glib.h>
#include <stdexcept>

namespace DAMARIS
{

/**
 * Exception class for the config file.
 */
class BackendConfigException : public std::runtime_error
{
public:
	explicit BackendConfigException(const std::string& msg) throw() : std::runtime_error(msg) {};
	explicit BackendConfigException(const char* msg) throw() : std::runtime_error(msg) {};
	virtual ~BackendConfigException() throw() {};

};

/**
 * \defgroup BackendConfigReader Backend Config Reader
 * \ingroup machines
 *
 * Allows reading in a backend configuration file implemented as a Glib Key-Value file
 */
class BackendConfigReader
{
public:
	BackendConfigReader(const gchar *configPath);
	
	~BackendConfigReader() { g_key_file_free(_keyFile); }; // I hope this is exception safe

	/* -------------------------------------------------------------------- */
	/* The following functions are encapsulating the underlying GKeyFile functionality */

	inline gboolean getBoolean(const gchar *groupName, const gchar *key) {GError* err = NULL; bool ret = g_key_file_get_boolean(_keyFile, groupName, key, &err); if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);} return ret;};
	inline gdouble getDouble(const gchar *groupName, const gchar *key) {GError* err = NULL; double ret = g_key_file_get_double(_keyFile, groupName, key, &err); if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);} return ret;};
	inline gint getInteger(const gchar *groupName, const gchar *key) {GError* err = NULL; int ret = g_key_file_get_integer(_keyFile, groupName, key, &err); if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);} return ret;};
	inline gchar* getValue(const gchar *groupName, const gchar *key) {GError* err = NULL; gchar* ret = g_key_file_get_value(_keyFile, groupName, key, &err); if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);} return ret;};
	inline gchar* getString(const gchar *groupName, const gchar *key) {GError* err = NULL; gchar* ret = g_key_file_get_string(_keyFile, groupName, key, &err); if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);} return ret;};

	/**
	 * This allows you to pass a function pointer to BackendConfigReader in order to call one of the functions on GKeyFile that are not driectly provided.
	 *
	 * Because a function pointer needs to know the full signature, the
	 * Example usage:
	 *  gchar* groupName; // initialized
	 *  gchar* key; // initialized
	 * 	gint64 (*f)(GKeyFile*, const gchar *, const gchar *, GError*) = &g_key_file_get_int64;
	 * 	gint64 value = getSomething<gint64> (f, groupName, key);
	 */
	template<typename returnType>
	returnType getSomething(returnType (*func)(GKeyFile*, const gchar *, const gchar *, GError*), const gchar *groupName, const gchar *key)
	{
		GError* err = NULL;
		returnType ret = func(_keyFile, groupName, key, &err);
		if (err) {g_error("GLib Error code %i, message: %s", err->code, err->message);}
		return ret;
	}

	/* -------------------------------------------------------------------- */

private:

	/**
	 * Attempt to load the defined relative config path.
	 *
	 * First, attempt to load the file from the user config dir. If that fails, attempt to load from the system config dir. If that fails as well, exit the program.
	 */
	void loadConfig();

	/**
	 * Load a specific config file. Returns true on success, false on file not found. Throws an error otherwise.
	 */
	bool loadConfigFile(const gchar *keyFileName, GKeyFileFlags flags, GError *error);
	
	/**
	 * (Relative) Path to the damaris backend config
	 */
	const gchar *_damarisConfigPath;

	GKeyFile *_keyFile;
};


} // namespace DAMARIS

#endif // include guard
